/* Browser input owns physical gestures; the C core owns context and settings. */
(() => {
  const body = document.body, canvas = Module.canvas;
  const $ = selector => document.querySelector(selector);
  const begin = $('#begin'), pauseButton = $('#pause'), settingsButton = $('#settings');
  const buttons = [...document.querySelectorAll('[data-action]')];
  const keys = new Set(), pointers = new Map();
  const keyMap = { KeyW:0, KeyS:1, KeyA:2, KeyD:3, ArrowLeft:4, ArrowRight:5, ArrowUp:6, ArrowDown:7, KeyE:8, Digit1:9, Digit2:10, Digit3:11, Space:12, KeyP:13 };
  let ready = false, failed = false, context = 3, epoch = 0, looking = null;
  let settingsOpen = false, pauseReason = 'menu', layout = '';
  const contexts = ['foot','helm','panel','paused','settings'];
  const call = (name, types = [], args = [], result = null) => Module.ccall(name,result,types,args);
  const playing = () => ready && (context === 0 || context === 1);
  const owner = id => [...pointers.values()].some(p => p.action === id) || [...keys].some(key => keyMap[key] === id);
  function paint() {
    buttons.forEach(button => button.classList.toggle('pressed', owner(Number(button.dataset.action))));
  }
  function releaseCapture(element, id) {
    if (element?.hasPointerCapture(id)) element.releasePointerCapture(id);
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
    begin.disabled = !ready || failed || document.hidden;
    pauseButton.disabled = settingsButton.disabled = !ready || failed;
  }
  function pause(reason = 'menu') {
    pauseReason = reason;
    if (ready && !failed) call('meu_web_pause');
    available();
  }
  function start() {
    if (!ready || failed || settingsOpen || document.hidden || !document.hasFocus()) return;
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
      body.dataset.ready = 'true'; begin.textContent = 'Begin exploration';
      $('#status').textContent = 'Ready. Sound starts when you begin.';
      if (typeof Module._meu_state_json === 'function')
        this.readState = () => JSON.parse(call('meu_state_json',[],[],'string'));
    },
    context(id, generation) {
      if (id === context && generation === epoch) return;
      clearInput(); context = id; epoch = generation;
      body.dataset.context = contexts[id]; mode(id);
      $('#unit-info').hidden = id !== 2;
      if (id === 2) $('#close-info').focus();
    },
    preferences(open, sensitivity, invert, bob, sway, volume) {
      const changed = settingsOpen !== !!open;
      settingsOpen = !!open;
      $('#settings-panel').hidden = !settingsOpen;
      $('#welcome').hidden = settingsOpen;
      settingsButton.setAttribute('aria-expanded',String(settingsOpen));
      $('#sensitivity').value = sensitivity;
      $('#sensitivity-value').textContent = sensitivity.toFixed(2) + '×';
      $('#invert-y').checked = !!invert; $('#walk-bob').checked = !!bob; $('#machine-sway').checked = !!sway;
      $('#volume').value = volume; $('#volume-value').textContent = Math.round(volume*100) + '%';
      if (changed) (settingsOpen ? $('#settings-title') : begin).focus();
    },
    hud(room, height, aboard, moving, current, target, use, info, liftReady) {
      const names = ['Bridge','Living quarters','Engineering hall','Service lift'];
      const uses = ['', 'Read unit dimensions','Toggle reading lamp','Toggle inspection lights','Call service lift','Take bridge helm'];
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
    navigation(helm, speed, heading, parked, parking, reason, stopping) {
      $('#speed').textContent = speed.toFixed(1) + ' m/s';
      $('#heading').textContent = 'Heading ' + String(Math.round((heading % 360 + 360) % 360)).padStart(3,'0') + '°';
      $('#stopping').textContent = 'Stop ' + stopping.toFixed(1) + ' m';
      const reasons = ['', 'Structure in stopping path','Unsupported ground ahead','Travel unavailable'];
      $('#travel-status').textContent = reason ? (parked ? 'Parked · ' : '') + reasons[reason] :
        parked ? 'Parked · support verified' : parking ? 'Braking to park…' : helm ? 'Manual travel · release drive to brake' : 'Stopping…';
    },
    paused() {
      clearInput(); body.dataset.paused = 'true'; body.dataset.pauseReason = pauseReason;
      begin.textContent = body.dataset.started === 'true' ? 'Resume exploration' : 'Begin exploration';
      const reasons = { resize:'The screen changed. Resume when ready.', hidden:'Paused while away. Resume when ready.',
        focus:'The window lost focus. Resume when ready.', menu:'Take your time. The unit is waiting.',
        pagehide:'Paused while away. Resume when ready.' };
      $('#status').textContent = reasons[pauseReason] || reasons.menu;
      available();
    },
    started() {
      clearInput(); body.dataset.started = 'true'; body.dataset.paused = 'false';
    },
    fail(message) {
      failed = true; clearInput(); body.dataset.paused = 'true'; body.dataset.error = 'true';
      $('#status').textContent = message; begin.textContent = 'Reload to try again'; available();
    }
  };
  press(begin,start);
  press(pauseButton,() => pause());
  press(settingsButton,() => { if (ready) call('meu_web_settings',['number'],[settingsOpen ? 1 : 0]); });
  press($('#close-settings'),() => call('meu_web_settings',['number'],[1]));
  press($('#reset-settings'),() => call('meu_web_settings',['number'],[2]));
  press($('#close-info'),() => action(8,true));
  document.querySelectorAll('[data-setting]').forEach(input => input.addEventListener('input',() => {
    if (!ready || !settingsOpen) return;
    const value = input.type === 'checkbox' ? +input.checked : Number(input.value);
    call('meu_web_setting',['number','number'],[Number(input.dataset.setting),value],'number');
  }));
  window.addEventListener('keydown', event => {
    if (event.code === 'F1' && ready) {
      call('meu_web_settings',['number'],[settingsOpen ? 1 : 0]); event.preventDefault(); return;
    }
    if (event.code === 'Escape') {
      if (settingsOpen) call('meu_web_settings',['number'],[1]); else pause();
      event.preventDefault(); return;
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
      if (!playing() || looking) return;
      looking = { id:event.pointerId,element,x:event.clientX,y:event.clientY,epoch };
      element.setPointerCapture(event.pointerId); event.preventDefault();
    });
    element.addEventListener('pointermove',event => {
      if (!looking || looking.id !== event.pointerId || looking.epoch !== epoch || !playing()) return;
      call('meu_web_look',['number','number'],[event.clientX-looking.x,event.clientY-looking.y]);
      looking.x = event.clientX; looking.y = event.clientY;
    });
    for (const type of ['pointerup','pointercancel','lostpointercapture']) element.addEventListener(type,event => {
      if (looking?.id === event.pointerId) looking = null;
    });
  }
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
