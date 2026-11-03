const { test, expect } = require('@playwright/test');
const release = process.env.MEU_RELEASE === '1';
const state = page => page.evaluate(() => window.MEU.readState());
let errors;
test.beforeEach(async ({ page }) => {
  errors = [];
  page.on('pageerror',error => errors.push(String(error)));
  page.on('console',message => { if (message.type()==='error') errors.push(message.text()); });
  await page.addInitScript(() => {
    window.__MEU_CONTROLS__ = [];
    for (const type of ['pointerdown','pointerup','pointercancel','gotpointercapture','lostpointercapture','keydown','keyup','resize','visibilitychange','blur','focus'])
      window.addEventListener(type,event => window.__MEU_CONTROLS__.push({
        type, code:event.code, id:event.pointerId, x:event.clientX,y:event.clientY,
        target:event.target?.closest?.('button,[role="group"]')?.getAttribute('aria-label') || event.target?.id,
        time:performance.now(), state:window.MEU?.readState?.()
      }),true);
  });
});
test.afterEach(async ({ page }, info) => {
  if (release) return;
  await info.attach('controls-and-lifecycle', {body:JSON.stringify(await page.evaluate(() => ({
    state:window.MEU?.readState?.(),inputs:window.__MEU_CONTROLS__,hidden:document.hidden,focus:document.hasFocus(),
    viewport:{width:innerWidth,height:innerHeight,ratio:devicePixelRatio}
  })),null,2),contentType:'application/json'});
  expect(errors).toEqual([]);
});
async function ready(page) {
  await page.goto('./');
  await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  await expect(page.locator('body')).toHaveAttribute('data-rendered','true');
}
async function driver(page) {
  const session=await page.context().newCDPSession(page), points=new Map();
  const down = async (locator,id) => {
    const b=await locator.boundingBox();
    points.set(id,{x:b.x+b.width/2,y:b.y+b.height/2,id});
    await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[...points.values()]});
  };
  const move = async (id,dx,dy) => {
    const p=points.get(id); points.set(id,{...p,x:p.x+dx,y:p.y+dy});
    await session.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[...points.values()]});
  };
  const end = async (cancel=false) => {
    if (points.size) await session.send('Input.dispatchTouchEvent',{type:cancel?'touchCancel':'touchEnd',touchPoints:[]});
    points.clear();
  };
  return {down,move,end,session};
}
const button = (page,name) => page.getByRole('button',{name,exact:true});
const poll = async (page,predicate,timeout=7000) => expect.poll(async()=>predicate(await state(page)),{timeout,intervals:[30,50,100]}).toBe(true);

test('settings apply without starting sound and require an explicit resume',async ({ page,isMobile },info) => {
  test.skip(release,'C assertions use read-only diagnostics.');
  await ready(page);
  const tap = async locator => isMobile?locator.tap():locator.click();
  await tap(button(page,'Settings'));
  await expect(page.getByRole('dialog',{name:'Settings',exact:true})).toBeVisible();
  expect((await state(page)).context).toBe(4);
  expect((await state(page)).audioAttempts).toBe(0);
  const sensitivity=page.getByLabel('Look sensitivity',{exact:false});
  await sensitivity.scrollIntoViewIfNeeded();
  if (isMobile) { const b=await sensitivity.boundingBox(); await sensitivity.tap({position:{x:b.width-3,y:b.height/2}}); }
  else { await sensitivity.focus(); await sensitivity.press('End'); }
  await poll(page,s=>s.sensitivity>2.9);
  for (const id of ['invert-y','walk-bob','machine-sway']) {
    const label=page.locator('label[for="'+id+'"]'); await label.scrollIntoViewIfNeeded(); await tap(label);
  }
  expect((await state(page)).invertY).toBe(true);
  expect((await state(page)).walkBob).toBe(true); expect((await state(page)).machineSway).toBe(true);
  const volume=page.getByLabel('Sound volume',{exact:false});
  await volume.scrollIntoViewIfNeeded();
  if (isMobile) { const b=await volume.boundingBox(); await volume.tap({position:{x:2,y:b.height/2}}); }
  else { await volume.focus(); await volume.press('Home'); }
  await poll(page,s=>s.volume===0);
  await page.screenshot({path:info.outputPath('settings.png')});
  await tap(button(page,'Close settings'));
  const paused=await state(page);
  expect(paused.paused).toBe(true); expect(paused.context).toBe(3); expect(paused.ticks).toBe(0);
  await page.waitForTimeout(100); expect((await state(page)).ticks).toBe(0);
  await tap(button(page,'Begin exploration'));
  await poll(page,s=>s.ticks>0);
  expect((await state(page)).audioAttempts).toBe(1); expect((await state(page)).audioGain).toBe(0);
  const before=await state(page);
  if (isMobile) {
    const d=await driver(page);
    await d.down(page.getByRole('group',{name:'Drag to look'}),1);
    await d.move(1,40,20); await d.end(); await d.session.detach();
  } else {
    await page.mouse.move(500,300); await page.mouse.down(); await page.mouse.move(540,320); await page.mouse.up();
  }
  const after=await state(page);
  expect(after.yaw-before.yaw).toBeCloseTo(.004*40*after.sensitivity,3);
  expect(after.pitch-before.pitch).toBeCloseTo(.004*20*after.sensitivity,3);
  expect(after.robotYaw).toBe(before.robotYaw);
  await tap(button(page,'Settings')); await tap(button(page,'Reset defaults'));
  const reset=await state(page);
  expect(reset.sensitivity).toBe(1); expect(reset.volume).toBe(.6);
  expect(reset.invertY || reset.walkBob || reset.machineSway).toBe(false);
  await tap(button(page,'Close settings'));
  expect((await state(page)).paused).toBe(true);
});

test('multitouch ownership survives cancellation, capture loss and station transitions',async ({ page,isMobile }) => {
  test.skip(release || !isMobile,'Uses genuine phone touch events and C diagnostics.');
  test.setTimeout(45000);
  await ready(page); await button(page,'Begin exploration').tap();
  const d=await driver(page);
  await d.down(button(page,'Move forward'),1); await d.down(button(page,'Move right'),2);
  await d.down(page.getByRole('group',{name:'Drag to look'}),3); await d.move(3,20,-10);
  await poll(page,s=>(s.held&9)===9 && s.yaw>.05);
  await d.end(true); await poll(page,s=>s.held===0);
  const stopped=await state(page); await page.waitForTimeout(100);
  expect((await state(page)).x).toBe(stopped.x); expect((await state(page)).z).toBe(stopped.z);
  await d.down(button(page,'Move forward'),4); await d.down(button(page,'Move forward'),5);
  // Activate pending capture before taking it away (otherwise no loss event is due).
  await d.move(4,20,0); await d.move(5,20,0);
  const ids=await page.evaluate(()=>window.__MEU_CONTROLS__.filter(i=>i.type==='pointerdown'&&i.target==='Move forward').slice(-2).map(i=>i.id));
  await page.evaluate(id=>document.querySelector('[data-action="0"]').releasePointerCapture(id),ids[0]);
  await d.move(4,20,0);
  expect((await state(page)).held&1).toBe(1);
  await expect(button(page,'Move forward')).toHaveClass(/pressed/);
  await page.evaluate(id=>document.querySelector('[data-action="0"]').releasePointerCapture(id),ids[1]);
  await d.move(5,20,0); await poll(page,s=>(s.held&1)===0); await d.end();
  await d.down(button(page,'Move forward'),7); await poll(page,s=>s.use===5);
  await d.down(button(page,'Take bridge helm · E'),8);
  await poll(page,s=>s.helm && s.held===0);
  await d.move(7,10,0); await page.waitForTimeout(100);
  expect((await state(page)).speed).toBe(0);
  await d.end();
  await d.down(button(page,'Drive forward'),9); await poll(page,s=>s.speed>2);
  await d.down(button(page,'Pause'),10);
  const paused=await state(page);
  expect(paused.paused).toBe(true); expect(paused.held).toBe(0); expect(paused.helm).toBe(false);
  await page.waitForTimeout(100); expect((await state(page)).ticks).toBe(paused.ticks);
  // Old fingers remain down: a third finger must be able to explicitly resume.
  await d.down(button(page,'Resume exploration'),11);
  await poll(page,s=>!s.paused);
  await d.move(9,10,0);
  expect((await state(page)).held).toBe(0);
  await d.end(); await poll(page,s=>s.parked);
  const before=await state(page);
  await d.down(button(page,'Move backward'),12); await poll(page,s=>s.z>before.z+.1);
  await d.end(); await d.session.detach();
});

test('rotation and browser backgrounding freeze the game until explicit resume',async ({ page,isMobile }) => {
  test.skip(release,'C assertions use read-only diagnostics.');
  test.setTimeout(30000);
  await ready(page);
  const tap=async locator=>isMobile?locator.tap():locator.click();
  await tap(button(page,'Begin exploration'));
  const d=isMobile?await driver(page):null;
  const hold=async()=>d?d.down(button(page,'Move forward'),1):page.keyboard.down('KeyW');
  const end=async()=>d?d.end():page.keyboard.up('KeyW');
  await hold(); await poll(page,s=>(s.held&1)!==0);
  const v=page.viewportSize(); await page.setViewportSize({width:v.height,height:v.width});
  await poll(page,s=>s.paused && s.held===0);
  let frozen=await state(page); await page.waitForTimeout(150);
  expect((await state(page)).ticks).toBe(frozen.ticks);
  await end(); await tap(button(page,'Resume exploration')); await poll(page,s=>!s.paused);
  expect((await state(page)).z).toBe(frozen.z);
  await hold(); await poll(page,s=>(s.held&1)!==0);
  // Playwright forces every tab to report focus; disable that for this check.
  const lifecycle=await page.context().newCDPSession(page);
  await lifecycle.send('Emulation.setFocusEmulationEnabled',{enabled:false});
  const {windowId}=await lifecycle.send('Browser.getWindowForTarget');
  try {
    await lifecycle.send('Browser.setWindowBounds',{windowId,bounds:{windowState:'minimized'}});
    await poll(page,s=>s.paused && s.held===0);
    frozen=await state(page);
    expect(await page.evaluate(()=>document.hidden || !document.hasFocus())).toBe(true);
    await lifecycle.send('Page.setWebLifecycleState',{state:'frozen'});
    await page.waitForTimeout(150); expect((await state(page)).ticks).toBe(frozen.ticks);
  } finally {
    await lifecycle.send('Page.setWebLifecycleState',{state:'active'});
    await lifecycle.send('Browser.setWindowBounds',{windowId,bounds:{windowState:'normal'}});
    await page.bringToFront();
  }
  expect((await state(page)).paused).toBe(true);
  await end(); await tap(button(page,'Resume exploration')); await poll(page,s=>!s.paused);
  expect((await state(page)).z).toBe(frozen.z);
  await lifecycle.detach();
  if (d) await d.session.detach();
});

test('settings stay within safe areas and support keyboard activation',async ({ page,isMobile },info) => {
  test.skip(release,'Layout and control-context regression check.');
  await ready(page);
  await page.addStyleTag({content:':root{--safe-top:20px;--safe-bottom:16px;--safe-left:24px;--safe-right:24px;}'});
  if (isMobile) await button(page,'Settings').tap();
  else { await button(page,'Settings').focus(); await page.keyboard.press('Enter'); }
  await expect(page.getByRole('dialog',{name:'Settings',exact:true})).toBeVisible();
  for (const selector of ['#settings-panel','#close-settings','#reset-settings']) {
    const b=await page.locator(selector).boundingBox();
    expect(b.x).toBeGreaterThanOrEqual(24); expect(b.y).toBeGreaterThanOrEqual(20);
    expect(b.x+b.width).toBeLessThanOrEqual(page.viewportSize().width-24);
    expect(b.y+b.height).toBeLessThanOrEqual(page.viewportSize().height-16);
    if (selector!=='#settings-panel') expect(b.height).toBeGreaterThanOrEqual(48);
  }
  await page.screenshot({path:info.outputPath('safe-settings.png')});
  if (!isMobile) {
    await button(page,'Close settings').focus(); await page.keyboard.press('Tab');
    expect(await page.locator('#sensitivity').evaluate(e=>e===document.activeElement)).toBe(true);
    await page.keyboard.press('Escape');
  } else await button(page,'Close settings').tap();
  expect((await state(page)).paused).toBe(true);
});
