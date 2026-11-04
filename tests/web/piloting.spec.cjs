const { test, expect } = require('@playwright/test');
const release = process.env.MEU_RELEASE === '1';
const state = page => page.evaluate(() => window.MEU.readState());
let errors;
test.beforeEach(async ({ page }) => {
  errors = [];
  page.on('pageerror', error => errors.push(String(error)));
  page.on('console', message => { if (message.type() === 'error') errors.push(message.text()); });
  await page.addInitScript(() => {
    window.__MEU_PILOT_INPUTS__ = [];
    for (const type of ['keydown','keyup','pointerdown','pointerup','pointercancel','visibilitychange']) {
      window.addEventListener(type, event => window.__MEU_PILOT_INPUTS__.push({
        type, code: event.code, x:event.clientX, y:event.clientY, pointerId:event.pointerId, target: event.target?.getAttribute?.('aria-label') || event.target?.id,
        time: performance.now(), state: window.MEU?.readState?.()
      }), true);
    }
  });
});
test.afterEach(async ({ page }, info) => {
  if (release) return;
  const report = await page.evaluate(() => ({
    state: window.MEU?.readState?.(), inputs: window.__MEU_PILOT_INPUTS__,
    viewport: { width: innerWidth, height: innerHeight, ratio: devicePixelRatio }, userAgent: navigator.userAgent
  }));
  await info.attach('piloting-inputs-and-state', { body: JSON.stringify(report,null,2), contentType: 'application/json' });
  expect(errors).toEqual([]);
});
async function controls(page, isMobile) {
  await page.goto('./');
  await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  await expect(page.locator('body')).toHaveAttribute('data-rendered','true');
  const begin = page.getByRole('button', { name: 'Begin exploration', exact: true });
  if (isMobile) await begin.tap(); else await begin.click();
  const session = isMobile ? await page.context().newCDPSession(page) : null;
  let heldTouch = null;
  const tap = async locator => {
    if (!isMobile) return locator.click();
    const b = await locator.boundingBox();
    const point = {x:b.x+b.width/2,y:b.y+b.height/2,id:2};
    await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:heldTouch?[heldTouch,point]:[point]});
    await session.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
    heldTouch = null;
  };
  const keys = { 'Move forward':'KeyW', 'Move backward':'KeyS', 'Drive forward':'KeyW', 'Drive reverse':'KeyS', 'Turn left':'KeyA', 'Turn right':'KeyD' };
  const hold = async name => {
    if (isMobile) {
      const b = await page.getByRole('button', { name, exact: true }).boundingBox();
      heldTouch = {x:b.x+b.width/2,y:b.y+b.height/2,id:1};
      await session.send('Input.dispatchTouchEvent', { type:'touchStart', touchPoints:[heldTouch] });
    } else await page.keyboard.down(keys[name]);
  };
  const releaseInput = async name => {
    if (isMobile) { if (heldTouch) await session.send('Input.dispatchTouchEvent', { type:'touchEnd', touchPoints:[] }); heldTouch=null; }
    else await page.keyboard.up(keys[name]);
  };
  const until = async predicate => expect.poll(async () => predicate(await state(page)), { timeout:22000, intervals:[30,50,100] }).toBe(true);
  const approach = async () => {
    await hold('Move forward'); await until(s => s.use===5); await releaseInput('Move forward');
    await tap(page.getByRole('button', { name:'Take bridge helm · E', exact:true }));
    await expect(page.locator('body')).toHaveAttribute('data-helm','true');
    expect((await state(page)).helm).toBe(true);
  };
  const park = async () => {
    if (isMobile) await tap(page.getByRole('button', { name:'Park robot', exact:true }));
    else await page.keyboard.press('KeyP');
  };
  const leave = async () => {
    if (isMobile) await tap(page.getByRole('button', { name:'Release helm', exact:true }));
    else await page.keyboard.press('KeyE');
    await expect(page.locator('body')).toHaveAttribute('data-helm','false');
  };
  return { tap, session, hold, releaseInput, until, approach, park, leave };
}
test('pilots from the physical helm, looks independently, parks and releases safely', async ({ page, isMobile }, info) => {
  test.skip(release, 'Observes read-only C diagnostics.');
  test.setTimeout(80000);
  const c = await controls(page,isMobile);
  await c.approach();
  let s = await state(page);
  expect(s.robotX).toBe(0); expect(s.robotZ).toBe(0);
  const local = { x:s.x,y:s.y,z:s.z };
  for (const button of await page.locator('#pilot-actions button').all()) {
    const b = await button.boundingBox();
    expect(b.width).toBeGreaterThanOrEqual(48); expect(b.height).toBeGreaterThanOrEqual(48);
    expect(b.x).toBeGreaterThanOrEqual(0); expect(b.y+b.height).toBeLessThanOrEqual(page.viewportSize().height);
  }
  await c.hold('Drive forward'); await c.until(s => s.speed>5); await c.releaseInput('Drive forward');
  // Look while the machine is still braking: no heading command is issued.
  const beforeLook = await state(page);
  if (isMobile) {
    const b = await page.getByRole('group', { name:'Drag to look' }).boundingBox();
    const p = {x:b.x+20,y:b.y+50,id:2};
    await c.session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[p]});
    await c.session.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{...p,x:p.x+35,y:p.y-15}]});
    await c.session.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
  } else {
    const b=await page.locator('#look').boundingBox();
    await page.mouse.move(b.x+20,b.y+50); await page.mouse.down();
    await page.mouse.move(b.x+70,b.y+30,{steps:5}); await page.mouse.up();
  }
  s = await state(page);
  expect(s.yaw).toBeGreaterThan(beforeLook.yaw+.1);
  expect(s.robotYaw).toBeCloseTo(beforeLook.robotYaw,6);
  expect(s.x).toBe(local.x); expect(s.y).toBe(local.y); expect(s.z).toBe(local.z);
  await page.screenshot({ path:info.outputPath('helm-travel.png') });
  await c.park(); await c.until(s => s.parked);
  expect((await state(page)).speed).toBe(0);
  await c.hold('Turn right'); await c.until(s => s.robotYaw>.06); await c.releaseInput('Turn right');
  await c.park(); await c.until(s => s.parked);
  expect((await state(page)).robotYaw).toBeGreaterThan(.06);
  await c.hold('Drive forward'); await c.until(s => s.speed>4);
  await c.park();
  expect((await state(page)).parked).toBe(false);
  await c.releaseInput('Drive forward'); await c.until(s => s.parked);
  await page.screenshot({ path:info.outputPath('parked.png') });
  await c.hold('Drive forward'); await c.until(s => s.speed>4);
  await c.leave(); await c.releaseInput('Drive forward');
  s = await state(page); expect(s.held).toBe(0); expect(s.helm).toBe(false);
  await c.hold('Move backward'); await c.until(s => s.parked); await c.releaseInput('Move backward');
  expect((await state(page)).grounded).toBe(true);
  // A second acquisition tests interruption while genuine throttle input is held.
  await c.approach(); await c.hold('Drive forward'); await c.until(s => s.speed>3);
  await c.tap(page.getByRole('button', { name:'Pause', exact:true })); await c.releaseInput('Drive forward');
  const paused = await state(page); await page.waitForTimeout(200);
  expect((await state(page)).robotZ).toBe(paused.robotZ);
  expect(paused.held).toBe(0); expect(paused.helm).toBe(false);
  await c.tap(page.getByRole('button',{name:'Saved journeys',exact:true}));
  await c.tap(page.getByRole('button',{name:'Save journey',exact:true}));
  await expect(page.locator('body')).toHaveAttribute('data-save-state','error');
  await expect(page.locator('#save-status')).toContainText('Park the unit');
  await c.tap(page.getByRole('button',{name:'Close journeys',exact:true}));
  await c.tap(page.getByRole('button', { name:'Resume exploration', exact:true }));
  await c.until(s => s.parked);
  expect((await state(page)).travelSafe).toBe(true);
  if (c.session) await c.session.detach();
});
test('stops before a visible buttress despite a held travel command', async ({ page, isMobile }, info) => {
  test.skip(release, 'Observes read-only C diagnostics.');
  test.setTimeout(50000);
  const c = await controls(page,isMobile); await c.approach();
  const local = await state(page);
  await c.hold('Turn right'); await c.until(s => s.robotYaw>.16); await c.releaseInput('Turn right');
  await c.until(s => Math.abs(s.turnRate)<.00001);
  await c.hold('Drive forward');
  await c.until(s => s.travelReason===1 && s.parked);
  const stopped = await state(page);
  expect(stopped.robotZ).toBeLessThan(-70); expect(stopped.travelSafe).toBe(true);
  expect(stopped.x).toBe(local.x); expect(stopped.y).toBe(local.y); expect(stopped.z).toBe(local.z);
  await expect(page.locator('#travel-status')).toContainText('Structure in stopping path');
  await page.waitForTimeout(250);
  expect((await state(page)).robotZ).toBe(stopped.robotZ);
  await page.screenshot({ path:info.outputPath('blocked-route.png') });
  await c.releaseInput('Drive forward'); await c.leave();
  if (c.session) await c.session.detach();
});
test('brakes before backing off the supported causeway', async ({ page, isMobile }, info) => {
  test.skip(release, 'Observes read-only C diagnostics.');
  test.setTimeout(35000);
  const c = await controls(page,isMobile); await c.approach();
  await c.hold('Drive reverse'); await c.until(s => s.travelReason===2 && s.parked);
  const stopped = await state(page);
  expect(stopped.travelSafe).toBe(true);
  expect(stopped.robotZ).toBeGreaterThan(40); expect(stopped.robotZ).toBeLessThan(75.01);
  expect(stopped.speed).toBe(0);
  await expect(page.locator('#travel-status')).toContainText('Unsupported ground ahead');
  await page.waitForTimeout(250);
  expect((await state(page)).robotZ).toBe(stopped.robotZ);
  await page.screenshot({ path:info.outputPath('unsupported-route.png') });
  await c.releaseInput('Drive reverse');
  if (c.session) await c.session.detach();
});
