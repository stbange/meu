/* Platform adapter only: all movement, pause, and timing rules live in the C core. */
(() => {
  const body = document.body;
  const canvas = Module.canvas;
  const begin = document.querySelector('#begin');
  const pauseButton = document.querySelector('#pause');
  const status = document.querySelector('#status');
  const buttons = [...document.querySelectorAll('[data-action]')];
  const heldKeys = new Set();
  const pointers = new Map();
  let ready = false;
  const active = () => ready && body.dataset.paused === 'false';
  const call = (name, types = [], args = []) => Module.ccall(name, null, types, args);
  const action = (id, pressed) => { if (ready) call('meu_web_action', ['number', 'number'], [id, +pressed]); };
  const keyMap = { KeyW: 0, KeyS: 1, KeyA: 2, KeyD: 3, ArrowLeft: 4, ArrowRight: 5, ArrowUp: 6, ArrowDown: 7 };

  function clearInput() {
    heldKeys.clear();
    pointers.clear();
    buttons.forEach(button => button.classList.remove('pressed'));
    window.dispatchEvent(new Event('meu-input-reset'));
  }
  function pause() { if (ready) call('meu_web_pause'); }
  function resize() {
    const ratio = Math.min(window.devicePixelRatio || 1, 2);
    canvas.width = Math.max(1, Math.round(canvas.clientWidth * ratio));
    canvas.height = Math.max(1, Math.round(canvas.clientHeight * ratio));
  }
  window.MEU = {
    ready() {
      ready = true;
      resize();
      body.dataset.ready = 'true';
      begin.disabled = pauseButton.disabled = false;
      begin.textContent = 'Begin exploration';
      status.textContent = 'Ready. Sound starts when you begin.';
      if (typeof Module._meu_state_json === 'function') {
        this.readState = () => JSON.parse(Module.ccall('meu_state_json', 'string', [], []));
      }
    },
    paused() {
      clearInput();
      body.dataset.paused = 'true';
      begin.textContent = body.dataset.started === 'true' ? 'Resume exploration' : 'Begin exploration';
      status.textContent = 'Take your time. The unit is waiting.';
    },
    started() {
      clearInput();
      body.dataset.started = 'true';
      body.dataset.paused = 'false';
    },
    fail(message) {
      ready = false;
      body.dataset.paused = 'true';
      body.dataset.error = 'true';
      status.textContent = message;
      begin.textContent = 'Reload to try again';
      begin.disabled = pauseButton.disabled = true;
    }
  };
  begin.addEventListener('click', () => { if (ready) { call('meu_web_start'); canvas.focus(); } });
  pauseButton.addEventListener('click', pause);
  window.addEventListener('keydown', event => {
    if (event.code === 'Escape') { pause(); event.preventDefault(); return; }
    if (event.code === 'Enter' && ready && !active()) { call('meu_web_start'); canvas.focus(); return; }
    if (!(event.code in keyMap) || !active() || event.repeat) return;
    heldKeys.add(event.code);
    action(keyMap[event.code], true);
    event.preventDefault();
  });
  window.addEventListener('keyup', event => {
    if (!(event.code in keyMap)) return;
    heldKeys.delete(event.code);
    if (![...pointers.values()].includes(keyMap[event.code])) action(keyMap[event.code], false);
    event.preventDefault();
  });
  buttons.forEach(button => {
    button.addEventListener('pointerdown', event => {
      if (!active()) return;
      event.preventDefault();
      button.setPointerCapture(event.pointerId);
      pointers.set(event.pointerId, Number(button.dataset.action));
      button.classList.add('pressed');
      action(Number(button.dataset.action), true);
    });
    const release = event => {
      const id = pointers.get(event.pointerId);
      if (id === undefined) return;
      pointers.delete(event.pointerId);
      button.classList.remove('pressed');
      if (![...pointers.values()].includes(id) && ![...heldKeys].some(key => keyMap[key] === id)) action(id, false);
    };
    button.addEventListener('pointerup', release);
    button.addEventListener('pointercancel', release);
    button.addEventListener('lostpointercapture', release);
    button.addEventListener('contextmenu', event => event.preventDefault());
  });
  for (const element of [canvas, document.querySelector('#look')]) {
    let looking = null;
    element.addEventListener('pointerdown', event => {
      if (!active() || looking !== null) return;
      looking = { id: event.pointerId, x: event.clientX, y: event.clientY };
      element.setPointerCapture(event.pointerId);
      event.preventDefault();
    });
    element.addEventListener('pointermove', event => {
      if (!looking || looking.id !== event.pointerId || !active()) return;
      call('meu_web_look', ['number','number'], [(event.clientX-looking.x)*0.004, -(event.clientY-looking.y)*0.004]);
      looking.x = event.clientX; looking.y = event.clientY;
    });
    const stop = () => { looking = null; };
    element.addEventListener('pointerup', stop);
    element.addEventListener('pointercancel', stop);
    element.addEventListener('lostpointercapture', stop);
    window.addEventListener('blur', stop);
    window.addEventListener('meu-input-reset', stop);
  }
  window.addEventListener('blur', pause);
  document.addEventListener('visibilitychange', () => { if (document.hidden) pause(); });
  window.addEventListener('resize', () => { if (active()) pause(); resize(); });
  canvas.addEventListener('webglcontextlost', event => {
    event.preventDefault(); pause(); window.MEU.fail('Graphics were interrupted. Reload to continue.');
  });
})();
