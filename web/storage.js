/* Durable browser journal. The C codec is the validation boundary; only an
   IndexedDB transaction's completion can produce the Saved status. */
(() => {
  const DB='meu-local-saves', STORE='snapshots';
  const keys=['latest','previous','damaged-latest','damaged-previous'];
  const $=selector=>document.querySelector(selector);
  const call=(name,types=[],args=[],result=null)=>Module.ccall(name,result,types,args);
  // Bound the host-to-Wasm copy as well as the decoder itself. A damaged
  // IndexedDB row must not allocate an unbounded ccall string on the C stack.
  const probe=data=>typeof data==='string' && [256,512].includes(data.length)?
    JSON.parse(call('meu_web_probe',['string'],[data],'string')):{result:1,generation:0};
  const message=result=>call('meu_web_save_message',['number'],[result],'string');
  let database=null,opening=null,busy=false,records={};
  function controls() {
    $('#save-journey').disabled=busy;
    $('#load-journey').disabled=busy || probe(records.latest).result!==0;
    $('#recover-journey').disabled=busy || probe(records.previous).result!==0;
    for (const id of ['export-journey','import-journey','close-journeys']) $('#'+id).disabled=busy;
    $('#export-damaged').hidden=damaged()===undefined;
    $('#export-damaged').disabled=busy;
    window.MEU?.storageBusy?.(busy);
  }
  function status(text,result) {
    $('#save-status').textContent=text; $('#save-note').textContent=text;
    document.body.dataset.saveState=result;
  }
  function describe() {
    const latest=probe(records.latest),previous=probe(records.previous);
    $('#save-details').textContent=(latest.result===0?'Latest: journey '+latest.generation+'. ':'No valid latest journey. ')+
      (previous.result===0?'Previous: journey '+previous.generation+'.':'No previous journey yet.');
    controls();
  }
  function damaged() {
    if (records.latest!==undefined && probe(records.latest).result!==0) return records.latest;
    if (records['damaged-latest']!==undefined) return records['damaged-latest'];
    if (records.previous!==undefined && probe(records.previous).result!==0) return records.previous;
    return records['damaged-previous'];
  }
  function open() {
    if (database) return Promise.resolve(database);
    if (opening) return opening;
    opening=new Promise((resolve,reject)=>{
      let settled=false,request;
      try { request=indexedDB.open(DB,1); } catch(error) { reject(error); return; }
      request.onupgradeneeded=()=>{if(!request.result.objectStoreNames.contains(STORE))request.result.createObjectStore(STORE);};
      request.onerror=()=>{settled=true;reject(request.error);};
      request.onblocked=()=>{settled=true;reject(new Error('Another MEU tab is holding the save store.'));};
      request.onsuccess=()=>{
        if(settled){request.result.close();return;}
        database=request.result;
        database.onversionchange=()=>{database?.close();database=null;opening=null;status('Save storage changed in another tab. Export a backup or reload.','error');};
        resolve(database);
      };
    }).catch(error=>{opening=null;throw error;});
    return opening;
  }
  async function read() {
    const db=await open();
    return new Promise((resolve,reject)=>{
      const tx=db.transaction(STORE,'readonly'),store=tx.objectStore(STORE),rows={};
      for(const key of keys){const request=store.get(key);request.onsuccess=()=>{rows[key]=request.result;};}
      tx.oncomplete=()=>resolve(rows);
      tx.onabort=()=>reject(tx.error || new Error('Save read interrupted.'));
      tx.onerror=()=>{};
    });
  }
  function snapshot(generation) {
    const result=JSON.parse(call('meu_web_snapshot',['number'],[generation],'string'));
    if(result.result!==0)throw new Error(message(result.result));
    return result.data;
  }
  async function write() {
    const db=await open();
    return new Promise((resolve,reject)=>{
      // No await between reading journal generations and queuing writes. The
      // read/write transaction also serializes saves from other MEU tabs.
      const tx=db.transaction(STORE,'readwrite',{durability:'strict'}),store=tx.objectStore(STORE);
      const rows={};let remaining=keys.length,published,error;
      tx.oncomplete=()=>resolve(published);
      tx.onabort=()=>reject(error || tx.error || new Error('Save transaction interrupted.'));
      tx.onerror=()=>{};
      for(const key of keys){
        const request=store.get(key);
        request.onsuccess=()=>{
          rows[key]=request.result;
          if(--remaining)return;
          try {
            const latest=probe(rows.latest),previous=probe(rows.previous);
            const generation=Math.max(latest.result===0?latest.generation:0,previous.result===0?previous.generation:0)+1;
            if(generation>0xffffffff)throw new Error('Save counter exhausted. Export this journey.');
            const data=snapshot(generation);
            if(rows.latest!==undefined && latest.result!==0){store.put(rows.latest,'damaged-latest');rows['damaged-latest']=rows.latest;}
            if(rows.previous!==undefined && previous.result!==0){store.put(rows.previous,'damaged-previous');rows['damaged-previous']=rows.previous;}
            if(latest.result===0){store.put(rows.latest,'previous');rows.previous=rows.latest;}
            else if(previous.result!==0){store.delete('previous');rows.previous=undefined;}
            store.put(data,'latest');rows.latest=data;
            published={rows,generation};
          } catch(reason) { error=reason;tx.abort(); }
        };
      }
    });
  }
  async function operation(pending,work) {
    if(busy)return;
    busy=true;controls();status(pending,'pending');
    try { await work(); }
    catch(error){status((error?.name==='QuotaExceededError'?'Save storage is full. ':String(error?.message || 'Storage is unavailable.')+' ')+
      'Last confirmed journey kept. Export a backup or try again.','error');}
    finally {busy=false;controls();}
  }
  function restore(data) {
    const info=probe(data);
    if(info.result!==0)throw new Error(message(info.result));
    const result=call('meu_web_restore',['string'],[data],'number');
    if(result!==0)throw new Error(message(result));
    return info;
  }
  function bytes(hex) {return Uint8Array.from(hex.match(/../g)||[],pair=>parseInt(pair,16));}
  function download(data,name,type) {
    const url=URL.createObjectURL(new Blob([data],{type})),link=document.createElement('a');
    link.href=url;link.download=name;document.body.append(link);link.click();link.remove();
    setTimeout(()=>URL.revokeObjectURL(url),1000);
  }
  window.MEUStorage={
    get busy(){return busy;},
    async boot(){
      try {
        records=await read();describe();
        const latest=probe(records.latest),previous=probe(records.previous);
        if(latest.result===0){restore(records.latest);status('Loaded saved journey '+latest.generation+'. Resume when ready.','loaded');}
        else if(records.latest!==undefined)status(message(latest.result)+(previous.result===0?' Recover the previous journey in Saved journeys.':''),'error');
        else if(previous.result===0)status('Latest save missing. Recover the previous journey in Saved journeys.','error');
        else if(records.previous!==undefined)status(message(previous.result),'error');
        else status('No saved journey here yet. Saves include your settings.','empty');
      } catch {status('Local save storage is unavailable. You can still play, import or export a journey.','error');describe();}
    },
    save(){return operation('Saving journey… waiting for storage confirmation.',async()=>{
      const result=await write();records=result.rows;describe();
      document.body.dataset.saveGeneration=String(result.generation);
      status('Saved journey '+result.generation+'. Settings included.','saved');
    });},
    load(previous=false){return operation(previous?'Recovering previous journey…':'Loading latest journey…',async()=>{
      records=await read();describe();const info=restore(records[previous?'previous':'latest']);
      status((previous?'Recovered':'Loaded')+' journey '+info.generation+'. Close, then resume. '+(previous?'Source data kept.':''),'loaded');
    });},
    export(){
      if(busy)return;
      try{download(bytes(snapshot(1)),'MEU-journey.meu','application/octet-stream');status('Backup download started. Local save unchanged.','exported');}
      catch(error){status(error.message,'error');}
    },
    async import(file){
      if(!file)return;
      return operation('Reading journey file…',async()=>{
        if(![128,256].includes(file.size))throw new Error('Unsupported or incomplete journey file. The source was kept.');
        const data=new Uint8Array(await file.arrayBuffer());
        const hex=Array.from(data,b=>b.toString(16).padStart(2,'0')).join('');
        restore(hex);status('Journey imported. Save here to keep it. Local save unchanged.','imported');
      });
    },
    exportDamaged(){
      const data=damaged();if(data===undefined || busy)return;
      try{
        if(typeof data==='string' && /^(?:[0-9a-f]{2})+$/.test(data))download(bytes(data),'MEU-damaged.meu','application/octet-stream');
        else download(typeof data==='string'?data:JSON.stringify(data),'MEU-damaged.txt','text/plain');
        status('Damaged-source download started. Stored source kept.','exported');
      }catch{status('This damaged record could not be downloaded. Stored source kept.','error');}
    }
  };
})();
