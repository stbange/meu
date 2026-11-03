const {expect}=require('@playwright/test');
const fs=require('node:fs/promises');
const state=page=>page.evaluate(()=>window.MEU.readState());
const button=(page,name)=>page.getByRole('button',{name,exact:true});
const tap=(locator,mobile)=>mobile?locator.tap():locator.click();
async function ready(page){
  await page.goto('./');await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  await expect(page.locator('body')).toHaveAttribute('data-rendered','true');
}
async function menu(page,mobile){
  if(await page.locator('body').getAttribute('data-paused')!=='true')await tap(button(page,'Pause'),mobile);
  await tap(button(page,'Saved journeys'),mobile);
  await expect(page.getByRole('dialog',{name:'Saved journeys',exact:true})).toBeVisible();
}
async function save(page,mobile,generation){
  await tap(button(page,'Save journey'),mobile);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','saved');
  if(generation)await expect(page.locator('body')).toHaveAttribute('data-save-generation',String(generation));
}
async function rows(page){
  return page.evaluate(()=>new Promise((resolve,reject)=>{
    const request=indexedDB.open('meu-local-saves',1);
    request.onerror=()=>reject(String(request.error));
    request.onsuccess=()=>{
      const db=request.result,tx=db.transaction('snapshots','readonly'),store=tx.objectStore('snapshots'),result={};
      for(const key of ['latest','previous','damaged-latest','damaged-previous']){
        const r=store.get(key);r.onsuccess=()=>{result[key]=r.result;};
      }
      tx.oncomplete=()=>{db.close();resolve(result);};tx.onabort=()=>{db.close();reject(String(tx.error));};
    };
  }));
}
async function setRecord(page,key,value){
  await page.evaluate(({key,value})=>new Promise((resolve,reject)=>{
    const request=indexedDB.open('meu-local-saves',1);
    request.onsuccess=()=>{
      const db=request.result,tx=db.transaction('snapshots','readwrite');
      tx.objectStore('snapshots').put(value,key);
      tx.oncomplete=()=>{db.close();resolve();};tx.onabort=()=>{db.close();reject(String(tx.error));};
    };
    request.onerror=()=>reject(String(request.error));
  }),{key,value});
}
async function exported(page,mobile,name='Export journey'){
  const pending=page.waitForEvent('download');await tap(button(page,name),mobile);
  const download=await pending;expect(await download.failure()).toBe(null);
  return fs.readFile(await download.path());
}
async function imported(page,mobile,buffer,name='journey.meu'){
  const pending=page.waitForEvent('filechooser');await tap(button(page,'Import journey'),mobile);
  const chooser=await pending;await chooser.setFiles({name,mimeType:'application/octet-stream',buffer});
}
module.exports={state,button,tap,ready,menu,save,rows,setRecord,exported,imported};
