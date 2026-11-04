/* Browser input owns physical gestures; the C core owns context and settings. */
(() => {
  const body = document.body, canvas = Module.canvas;
  const $ = selector => document.querySelector(selector);
  const begin = $('#begin'), pauseButton = $('#pause'), settingsButton = $('#settings');
  const buttons = [...document.querySelectorAll('[data-action]')];
  const keys = new Set(), pointers = new Map();
  const keyMap = { KeyW:0, KeyS:1, KeyA:2, KeyD:3, ArrowLeft:4, ArrowRight:5, ArrowUp:6, ArrowDown:7, KeyE:8, Digit1:9, Digit2:10, Digit3:11, Space:12, KeyP:13 };
  let ready = false, storageReady=false, storageBusy=false, failed = false, context = 3, epoch = 0, looking = null;
  let settingsOpen = false, pauseReason = 'menu', layout = '';
  let mouseLocked=false,lockPending=false,lockAllowed=false,mouseReleaseRequested=false,lockRequest=0;
  const contexts = ['foot','helm','panel','paused','settings','journeys','station','help'];
  const call = (name, types = [], args = [], result = null) => Module.ccall(name,result,types,args);
  const playing = () => ready && (context === 0 || context === 1);
  const owner = id => [...pointers.values()].some(p => p.action === id) || [...keys].some(key => keyMap[key] === id);
  function paint() {
    buttons.forEach(button => button.classList.toggle('pressed', owner(Number(button.dataset.action))));
  }
  function releaseCapture(element, id) {
    if (element?.hasPointerCapture(id)) element.releasePointerCapture(id);
  }
  function releaseMouse() {
    ++lockRequest;lockPending=false;lockAllowed=false;
    if(mouseLocked || document.pointerLockElement===canvas)mouseReleaseRequested=true;
    if(document.pointerLockElement===canvas)document.exitPointerLock();
  }
  function lockMouse() {
    if(!playing() || lockPending || document.pointerLockElement===canvas || !canvas.requestPointerLock)return;
    const request=++lockRequest;lockPending=true;lockAllowed=true;
    const denied=()=>{if(request===lockRequest){lockPending=false;lockAllowed=false;captureLook();}};
    try { const result=canvas.requestPointerLock();result?.catch(denied); }
    catch {denied();}
  }
  function captureLook() {
    if(!looking || lockPending || document.pointerLockElement)return;
    try { looking.element.setPointerCapture(looking.id); }
    catch { looking=null; }
  }
  function clearInput() {
    const old = [...pointers.entries()];
    const oldLook = looking;
    pointers.clear(); keys.clear(); looking = null; paint();
    old.forEach(([id, p]) => releaseCapture(p.button,id));
    if (oldLook) releaseCapture(oldLook.element,oldLook.id);
  }
  function clearDriveOwners() {
    const old = [...pointers].filter(([,p]) => p.action < 4);
    old.forEach(([id]) => pointers.delete(id));
    [...keys].filter(key => keyMap[key] < 4).forEach(key => keys.delete(key));
    old.forEach(([id,p]) => releaseCapture(p.button,id));
    paint();
  }
  function action(id, pressed) {
    if (!ready) return false;
    if (id === 12 && pressed) clearDriveOwners();
    return !!call('meu_web_action',['number','number'],[id,+pressed],'number');
  }
  function available() {
    begin.disabled = !ready || !storageReady || storageBusy || failed || document.hidden;
    pauseButton.disabled = !ready || failed;
    settingsButton.disabled = !ready || !storageReady || storageBusy || failed;
    $('#journeys').disabled = !ready || !storageReady || storageBusy || failed;
    $('#help').disabled = !ready || !storageReady || storageBusy || failed;
  }
  function pause(reason = 'menu') {
    pauseReason = reason;
    if (ready && !failed) call('meu_web_pause');
    available();
  }
  function start() {
    if (!ready || !storageReady || storageBusy || failed || settingsOpen || context===5 || context===7 || document.hidden || !document.hasFocus()) return;
    call('meu_web_start'); canvas.focus();
  }
  function resize() {
    const ratio = Math.min(window.devicePixelRatio || 1,2);
    const next = [canvas.clientWidth,canvas.clientHeight,ratio].join('/');
    if (next === layout) return;
    if (ready && body.dataset.started === 'true') pause('resize');
    layout = next;
    canvas.width = Math.max(1,Math.round(canvas.clientWidth*ratio));
    canvas.height = Math.max(1,Math.round(canvas.clientHeight*ratio));
  }
  function press(button, fn) {
    button.addEventListener('pointerdown', event => {
      if (button.disabled) return;
      event.preventDefault(); fn();
    });
    // Keyboard and assistive activation have no preceding pointerdown.
    button.addEventListener('click', event => { if (!button.disabled && event.detail === 0) fn(); });
  }
  function mode(id) {
    const helm = id === 1;
    body.dataset.helm = String(helm);
    $('#pilot-actions').hidden = !helm;
    $('#move-label').textContent = helm ? 'PILOT' : 'WALK';
    const names = helm ? ['Drive forward','Drive reverse','Turn left','Turn right'] :
      ['Move forward','Move backward','Move left','Move right'];
    const captions = helm ? ['Forward','Reverse','Turn left','Turn right'] : ['Forward','Back','Left','Right'];
    document.querySelectorAll('#movement button').forEach(button => {
      const actionId = Number(button.dataset.action);
      button.setAttribute('aria-label',names[actionId]);
      button.querySelector('small').textContent = captions[actionId];
    });
  }
  window.MEU = {
    ready() {
      ready = true; resize(); available();
      begin.textContent = 'Begin exploration';
      $('#status').textContent = 'Ready. Sound starts when you begin.';
      if (typeof Module._meu_state_json === 'function')
        this.readState = () => JSON.parse(call('meu_state_json',[],[],'string'));
      window.MEUStorage.boot().finally(()=>{storageReady=true;body.dataset.ready='true';available();});
    },
    context(id, generation) {
      if (id === context && generation === epoch) return;
      const leftJourneys=(context===5 || context===7) && id!==context;
      clearInput(); context = id; epoch = generation;
      body.dataset.context = contexts[id]; mode(id);
      if(!playing())releaseMouse();
      if(id!==6)$('#expedition-panel').hidden=true;
      else this.station();
      $('#unit-info').hidden = id !== 2;
      $('#journeys-panel').hidden = id !== 5;
      this.help();
      $('#welcome').hidden = id === 5 || id===7 || settingsOpen;
      if(id===5) $('#journeys-title').focus();
      else if(leftJourneys && id===3) begin.focus();
      if (id === 2) $('#close-info').focus();
    },
    help() {
      const panel=$('#help-panel'),opening=panel.hidden,article=$('#help-article'),scrollBefore=article.scrollTop;
      panel.hidden=context!==7;$('#help-backdrop').hidden=context!==7;
      if(context!==7)return;
      const text=field=>call('meu_web_help_text',['number'],[field],'string');
      const select=$('#help-topic');
      if(!select.options.length)for(let i=0;i<16;i++) {
        const label=text(-10-i);if(!label)break;
        const option=document.createElement('option');option.value=String(i);option.textContent=label;select.append(option);
      }
      const topic=String(call('meu_web_help',['number'],[-1],'number')),changed=panel.dataset.topic!==topic;
      panel.dataset.topic=topic;
      select.value=topic;
      for(const [field,id] of [[0,'help-heading'],[1,'help-place'],[2,'help-action'],[3,'help-tip']]) {
        const node=$('#'+id),value=text(field);if(node.textContent!==value)node.textContent=value;
      }
      const hint=$('#help-scroll-hint');hint.hidden=true;
      hint.hidden=$('#help-article').scrollHeight<=$('#help-article').clientHeight+1;
      if(opening || changed)article.scrollTo({top:0,left:0,behavior:'instant'});
      else article.scrollTop=scrollBefore;
      if(opening)$('#help-title').focus();
    },
    station() {
      $('#expedition-objective').textContent=call('meu_web_station_text',['number'],[-4],'string');
      $('#expedition-readout').textContent=call('meu_web_station_text',['number'],[-1],'string');
      const panel=$('#expedition-panel'),wasHidden=panel.hidden; panel.hidden=context!==6;
      if(context!==6)return;
      const text=row=>call('meu_web_station_text',['number'],[row],'string');
      $('#expedition-title').textContent=text(-3);
      if(wasHidden)$('#expedition-title').focus();
      $('#expedition-status').textContent=text(-1);
      $('#expedition-message').textContent=text(-2);
      const host=$('#expedition-options');
      for(let i=0;i<8;i++){
        let button=host.children[i];
        if(!button){button=document.createElement('button');host.append(button);press(button,()=>call('meu_web_station',['number'],[i]));}
        const label=text(i);button.hidden=!label;
        if(button.textContent!==label)button.textContent=label;
      }
    },
    preferences(open, sensitivity, invert, bob, sway, volume) {
      const changed = settingsOpen !== !!open;
      settingsOpen = !!open;
      $('#settings-panel').hidden = !settingsOpen;
      $('#welcome').hidden = settingsOpen || context===5 || context===7;
      settingsButton.setAttribute('aria-expanded',String(settingsOpen));
      $('#sensitivity').value = sensitivity;
      $('#sensitivity-value').textContent = sensitivity.toFixed(2) + '×';
      $('#invert-y').checked = !!invert; $('#walk-bob').checked = !!bob; $('#machine-sway').checked = !!sway;
      $('#volume').value = volume; $('#volume-value').textContent = Math.round(volume*100) + '%';
      if (changed) (settingsOpen ? $('#settings-title') : context===7?$('#help-title'):begin).focus();
    },
    hud(room, height, aboard, moving, current, target, use, info, liftReady) {
      const names = ['Bridge','Living quarters','Engineering hall','Service lift'];
      const uses = ['', 'Read unit dimensions','Toggle reading lamp','Toggle inspection lights','Call service lift','Take bridge helm','Expedition chart','Power and infrastructure','Salvage cradle and test bench','Wrist airlock','Phase coupler and bypass'];
      body.dataset.room=String(room);
      $('#room').textContent = names[room];
      $('#altitude').textContent = (room === 3 ? 'Lift ' : 'Deck ') + Math.round(height) + ' m · Unit height 240 m';
      $('#navigation').hidden = room !== 0; $('#lift-panel').hidden = !aboard;
      $('#lift-status').textContent = moving ? 'Travelling to ' + names[target] + '… Gates locked.' :
        liftReady ? 'Choose a deck · walk forward to leave' : 'Step into the centre to choose a deck';
      $('#lift-panel').querySelectorAll('button').forEach((button, i) => {
        button.disabled = !liftReady || i === current;
        button.setAttribute('aria-current',String(!moving && i === current));
      });
      $('#use').hidden = !use || !!aboard || info || context !== 0;
      $('#use').textContent = uses[use] + ' · E';
      $('#unit-info').hidden = !info;
    },
    navigation(helm, speed, heading, parked, parking, reason, stopping, x, z) {
      $('#coordinates').textContent='Position X '+Math.round(x)+' · Z '+Math.round(z)+' m';
      $('#speed').textContent = speed.toFixed(1) + ' m/s';
      $('#heading').textContent = 'Heading ' + String(Math.round((heading % 360 + 360) % 360)).padStart(3,'0') + '°';
      $('#stopping').textContent = 'Stop ' + stopping.toFixed(1) + ' m';
      const reasons = ['', 'Structure in stopping path','Unsupported ground ahead','Travel unavailable'];
      $('#travel-status').textContent = reason ? (parked ? 'Parked · ' : '') + reasons[reason] :
        parked ? 'Parked · support verified' : parking ? 'Braking to park…' : helm ? 'Manual travel · release drive to brake' : 'Stopping…';
    },
    paused() {
      releaseMouse();clearInput(); body.dataset.paused = 'true'; body.dataset.pauseReason = pauseReason;
      begin.textContent = body.dataset.started === 'true' ? 'Resume exploration' : 'Begin exploration';
      const reasons = { resize:'The screen changed. Resume when ready.', hidden:'Paused while away. Resume when ready.',
        focus:'The window lost focus. Resume when ready.', menu:'Take your time. The unit is waiting.',
        pointer:"Mouse released. Resume when ready.", pagehide:'Paused while away. Resume when ready.' };
      $('#status').textContent = reasons[pauseReason] || reasons.menu;
      available();
    },
    started() {
      clearInput(); body.dataset.started = 'true'; body.dataset.paused = 'false';
    },
    storageBusy(value) { storageBusy=!!value;available(); },
    restored() {
      releaseMouse();clearInput();body.dataset.started='true';body.dataset.paused='true';
      begin.textContent='Resume exploration';available();
    },
    fail(message) {
      failed = true; releaseMouse();clearInput(); body.dataset.paused = 'true'; body.dataset.error = 'true';
      $('#status').textContent = message; begin.textContent = 'Reload to try again'; available();
    }
  };
  press($('#close-expedition'),()=>call('meu_web_station',['number'],[-1]));
  press($('#help'),()=>call('meu_web_help',['number'],[context===7?1:0]));
  press($('#close-help'),()=>call('meu_web_help',['number'],[1]));
  press($('#help-previous'),()=>call('meu_web_help',['number'],[2]));
  press($('#help-next'),()=>call('meu_web_help',['number'],[3]));
  $('#help-topic').addEventListener('change',()=>{if(context===7)call('meu_web_help',['number'],[4+Number($('#help-topic').value)]);});
  press(begin,start);
  press(pauseButton,() => pause());
  press(settingsButton,() => { if (ready && !storageBusy) call('meu_web_settings',['number'],[settingsOpen ? 1 : 0]); });
  press($('#journeys'),()=>call('meu_web_journeys',['number'],[0]));
  press($('#close-journeys'),()=>{if(!storageBusy)call('meu_web_journeys',['number'],[1]);});
  press($('#save-journey'),()=>window.MEUStorage.save());
  press($('#load-journey'),()=>window.MEUStorage.load());
  press($('#recover-journey'),()=>window.MEUStorage.load(true));
  press($('#export-journey'),()=>window.MEUStorage.export());
  press($('#import-journey'),()=>$('#import-file').click());
  press($('#export-damaged'),()=>window.MEUStorage.exportDamaged());
  $('#import-file').addEventListener('change',event=>{
    const file=event.target.files[0];event.target.value='';window.MEUStorage.import(file);
  });
  press($('#close-settings'),() => call('meu_web_settings',['number'],[1]));
  press($('#reset-settings'),() => call('meu_web_settings',['number'],[2]));
  press($('#close-info'),() => action(8,true));
  document.querySelectorAll('[data-setting]').forEach(input => input.addEventListener('input',() => {
    if (!ready || !settingsOpen) return;
    const value = input.type === 'checkbox' ? +input.checked : Number(input.value);
    call('meu_web_setting',['number','number'],[Number(input.dataset.setting),value],'number');
  }));
  window.addEventListener('keydown', event => {
    if (storageBusy && ['F1','F2','F3','KeyH','Escape','Enter'].includes(event.code)) {event.preventDefault();return;}
    if(['KeyH','F3'].includes(event.code) && ready && storageReady && !failed) {
      if(!event.repeat)call('meu_web_help',['number'],[context===7?1:0]);event.preventDefault();return;
    }
    if(event.code==='F2' && ready && storageReady){
      call('meu_web_journeys',['number'],[context===5?1:0]);event.preventDefault();return;
    }
    if (event.code === 'F1' && ready && storageReady) {
      call('meu_web_settings',['number'],[settingsOpen ? 1 : 0]); event.preventDefault(); return;
    }
    if (event.code === 'Escape') {
      if(context===7)call('meu_web_help',['number'],[1]);
      else if(context===5)call('meu_web_journeys',['number'],[1]);
      else if (settingsOpen) call('meu_web_settings',['number'],[1]); else pause();
      event.preventDefault(); return;
    }
    if(context===7) {
      if(event.code==='Tab') {
        const fields=[...$('#help-panel').querySelectorAll('button,select,[tabindex="0"]')];
        const first=fields[0],last=fields[fields.length-1],selected=document.activeElement;
        if(event.shiftKey && (selected===first || selected===$('#help-title'))) {last.focus();event.preventDefault();}
        else if(!event.shiftKey && selected===last) {first.focus();event.preventDefault();}
      }
      return;
    }
    if(context===5){
      if(event.code==='Tab'){
        const fields=[...$('#journeys-panel').querySelectorAll('button')].filter(e=>!e.disabled && !e.hidden);
        if(!fields.length){$('#journeys-title').focus();event.preventDefault();return;}
        const first=fields[0],last=fields[fields.length-1],selected=document.activeElement;
        if(event.shiftKey && (selected===first || selected===$('#journeys-title'))){last.focus();event.preventDefault();}
        else if(!event.shiftKey && selected===last){first.focus();event.preventDefault();}
      }
      return;
    }
    if (context === 2 && event.code === 'Tab') { $('#close-info').focus(); event.preventDefault(); return; }
    if (settingsOpen) {
      if (event.code === 'Tab') {
        const fields = [...$('#settings-panel').querySelectorAll('button,input')];
        const first = fields[0], last = fields[fields.length-1], selected = document.activeElement;
        if (event.shiftKey && (selected === first || selected === $('#settings-title'))) { last.focus(); event.preventDefault(); }
        else if (!event.shiftKey && selected === last) { first.focus(); event.preventDefault(); }
      }
      return;
    }
    if(context===6){
      if(!event.repeat && /^Digit[1-8]$/.test(event.code)){call('meu_web_station',['number'],[Number(event.code.slice(-1))-1]);event.preventDefault();}
      else if(!event.repeat && ['KeyE','Digit0'].includes(event.code)){call('meu_web_station',['number'],[-1]);event.preventDefault();}
      return;
    }
    if ((event.code === 'Enter' || event.code === 'Space') && event.target.closest?.('button')) return;
    if (event.code === 'Enter' && !playing() && context !== 2) { start(); event.preventDefault(); return; }
    if (!(event.code in keyMap) || event.repeat || (!playing() && !(context === 2 && keyMap[event.code] === 8))) return;
    const id = keyMap[event.code]; keys.add(event.code);
    if (!action(id,true)) keys.delete(event.code);
    paint(); event.preventDefault();
  });
  window.addEventListener('keyup', event => {
    if (!(event.code in keyMap) || settingsOpen) return;
    const id = keyMap[event.code]; keys.delete(event.code);
    if (!owner(id)) action(id,false);
    paint(); event.preventDefault();
  });
  buttons.forEach(button => {
    button.addEventListener('pointerdown', event => {
      const id = Number(button.dataset.action);
      if (button.disabled || (!playing() && !(context === 2 && id === 8))) return;
      event.preventDefault(); button.setPointerCapture(event.pointerId);
      pointers.set(event.pointerId,{ action:id, button, epoch });
      if (!action(id,true)) { pointers.delete(event.pointerId); releaseCapture(button,event.pointerId); }
      paint();
    });
    const release = event => {
      const p = pointers.get(event.pointerId);
      if (!p) return;
      pointers.delete(event.pointerId);
      if (!owner(p.action)) action(p.action,false);
      paint();
    };
    for (const type of ['pointerup','pointercancel','lostpointercapture']) button.addEventListener(type,release);
    button.addEventListener('click', event => {
      if (event.detail === 0 && !button.disabled && Number(button.dataset.action) >= 8) {
        action(Number(button.dataset.action),true); action(Number(button.dataset.action),false);
      }
    });
    button.addEventListener('contextmenu',event => event.preventDefault());
  });
  for (const element of [canvas,$('#look')]) {
    element.addEventListener('pointerdown',event => {
      if(element===canvas && event.pointerType==='mouse') {
        if(context===3)start();
        if(playing())lockMouse();
      }
      if (!playing() || looking || document.pointerLockElement===canvas) return;
      looking = { id:event.pointerId,element,x:event.clientX,y:event.clientY,epoch };
      // A pending Pointer Lock can already prohibit pointer capture before
      // pointerLockElement changes. Acquire fallback capture only after denial.
      captureLook(); event.preventDefault();
    });
    element.addEventListener('pointermove',event => {
      if (document.pointerLockElement===canvas || !looking || looking.id !== event.pointerId || looking.epoch !== epoch || !playing()) return;
      call('meu_web_look',['number','number'],[event.clientX-looking.x,event.clientY-looking.y]);
      looking.x = event.clientX; looking.y = event.clientY;
    });
    for (const type of ['pointerup','pointercancel','lostpointercapture']) element.addEventListener(type,event => {
      if (looking?.id === event.pointerId) looking = null;
    });
  }
  document.addEventListener('pointerlockchange',()=>{
    const owned=document.pointerLockElement===canvas,previous=mouseLocked;
    lockPending=false;mouseLocked=owned;body.dataset.mouseLocked=String(owned);
    if(owned) {
      if(!lockAllowed || !playing() || document.hidden || !document.hasFocus()) { releaseMouse();return; }
      mouseReleaseRequested=false;
      const old=looking;looking=null;if(old)releaseCapture(old.element,old.id);
    } else {
      lockAllowed=false;const intentional=mouseReleaseRequested;mouseReleaseRequested=false;
      if(previous && playing() && !intentional)pause('pointer');
    }
  });
  document.addEventListener('pointerlockerror',()=>{lockPending=false;lockAllowed=false;captureLook();});
  document.addEventListener('mousemove',event=>{
    if(mouseLocked && document.pointerLockElement===canvas && playing())call('meu_web_look',['number','number'],[event.movementX,event.movementY]);
  });
  window.addEventListener('blur',() => pause('focus'));
  window.addEventListener('focus',available);
  document.addEventListener('visibilitychange',() => { if (document.hidden) pause('hidden'); available(); });
  window.addEventListener('pagehide',() => pause('pagehide'));
  window.addEventListener('pageshow',() => { if (ready) pause('pagehide'); });
  document.addEventListener('freeze',() => pause('hidden'));
  window.addEventListener('resize',resize);
  window.addEventListener('orientationchange',() => { pause('resize'); resize(); });
  if (typeof ResizeObserver !== 'undefined') new ResizeObserver(resize).observe(canvas);
  canvas.addEventListener('webglcontextlost',event => {
    event.preventDefault(); pause(); window.MEU.fail('Graphics were interrupted. Reload to continue.');
  });
})();
