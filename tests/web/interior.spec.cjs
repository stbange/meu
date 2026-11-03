const { test, expect } = require('@playwright/test');
const release = process.env.MEU_RELEASE === '1';
const state = page => page.evaluate(() => window.MEU.readState());

test.beforeEach(async ({ page }) => {
  await page.addInitScript(() => {
    window.__MEU_INTERIOR_INPUTS__ = [];
    for (const type of ['keydown','keyup','pointerdown','pointerup','pointercancel']) {
      window.addEventListener(type, event => {
        window.__MEU_INTERIOR_INPUTS__.push({
          type, code: event.code, target: event.target?.getAttribute?.('aria-label') || event.target?.id,
          time: performance.now(), state: window.MEU?.readState?.()
        });
      }, true);
    }
  });
});
test.afterEach(async ({ page }, info) => {
  if (release) return;
  const report = await page.evaluate(() => ({
    state: window.MEU?.readState?.(), inputs: window.__MEU_INTERIOR_INPUTS__,
    viewport: { width: innerWidth, height: innerHeight, ratio: devicePixelRatio }
  }));
  await info.attach('interior-state-and-inputs', { body: JSON.stringify(report,null,2), contentType: 'application/json' });
});

test('walks the connected decks using the real keyboard or touch lift controls', async ({ page, isMobile }, info) => {
  test.skip(release, 'The full route observes read-only C diagnostics.');
  test.setTimeout(100000);
  const errors = [];
  page.on('pageerror', error => errors.push(String(error)));
  await page.goto('./');
  await expect(page.locator('body')).toHaveAttribute('data-ready', 'true');
  const press = async locator => isMobile ? locator.tap() : locator.click();
  await press(page.getByRole('button', { name: 'Begin exploration', exact: true }));
  expect((await state(page)).scene).toBe('m0b-unit-v1');
  const session = isMobile ? await page.context().newCDPSession(page) : null;
  const replay = [];
  const move = async (forward, until) => {
    const key = forward ? 'KeyW' : 'KeyS';
    replay.push({ action: key, before: await state(page) });
    if (isMobile) {
      const b = await page.getByRole('button', { name: forward ? 'Move forward' : 'Move backward', exact: true }).boundingBox();
      await session.send('Input.dispatchTouchEvent', { type: 'touchStart', touchPoints: [{ x: b.x+b.width/2, y: b.y+b.height/2, id: 1 }] });
    } else await page.keyboard.down(key);
    try {
      await expect.poll(async () => until(await state(page)), { timeout: 14000, intervals: [30,30,50] }).toBe(true);
    } finally {
      if (isMobile) await session.send('Input.dispatchTouchEvent', { type: 'touchEnd', touchPoints: [] });
      else await page.keyboard.up(key);
    }
    const after = await state(page);
    replay.push({ action: 'release', after });
    expect(after.grounded).toBe(true);
  };
  const choose = async deck => {
    const name = ['Lift to bridge','Lift to living quarters','Lift to engineering'][deck];
    const button = page.getByRole('button', { name, exact: true });
    await expect(button).toBeEnabled();
    const b = await button.boundingBox();
    expect(b.width).toBeGreaterThanOrEqual(48);
    expect(b.height).toBeGreaterThanOrEqual(48);
    if (isMobile) await button.tap(); else await page.keyboard.press('Digit' + (deck+1));
    await expect.poll(async () => (await state(page)).liftMoving).toBe(true);
    await expect(button).toBeDisabled();
    replay.push({ action: 'lift ' + deck, state: await state(page) });
  };
  const arrive = async height => {
    await expect.poll(async () => (await state(page)).liftMoving, { timeout: 16000 }).toBe(false);
    expect((await state(page)).y).toBeCloseTo(height+1.7, 2);
    expect((await state(page)).onLift).toBe(true);
  };
  await expect(page.locator('#room')).toHaveText('Bridge');
  await page.screenshot({ path: info.outputPath('bridge.png') });
  if (!isMobile) {
    await move(true, s => s.z < -21.4);
    await page.keyboard.down('KeyD');
    await expect.poll(async () => (await state(page)).x, { intervals: [30,30,50] }).toBeGreaterThan(1.2);
    await page.keyboard.up('KeyD');
    await move(true, s => s.z < -22.6);
    const look = async (yaw, pitch) => {
      const before = await state(page);
      await page.mouse.move(500,300); await page.mouse.down();
      await page.mouse.move(500+(yaw-before.yaw)/.004,300-(pitch-before.pitch)/.004,{steps: 8});
      await page.mouse.up();
    };
    await look(.95,-.65);
    await page.screenshot({ path: info.outputPath('robot-shoulders.png') });
    await look(0,.34);
    await page.screenshot({ path: info.outputPath('alien-doorway.png') });
    await look(0,-.08);
    await move(false, s => s.z > -21);
    await page.keyboard.down('KeyA');
    await expect.poll(async () => (await state(page)).x, { intervals: [30,30,50] }).toBeLessThan(.05);
    await page.keyboard.up('KeyA');
  }
  await move(false, s => s.z > -.65);
  if (!isMobile) {
    await page.keyboard.down('KeyD');
    await expect.poll(async () => (await state(page)).x, { intervals: [30,30,50] }).toBeGreaterThan(1.67);
    await page.keyboard.up('KeyD');
    await expect(page.locator('#lift-status')).toHaveText('Step into the centre to choose a deck');
    await expect(page.getByRole('button', { name: 'Lift to living quarters', exact: true })).toBeDisabled();
    await page.keyboard.down('KeyA');
    await expect.poll(async () => (await state(page)).x, { intervals: [30,30,50] }).toBeLessThan(.2);
    await page.keyboard.up('KeyA');
  }
  await choose(1);
  await press(page.getByRole('button', { name: 'Pause', exact: true }));
  const stopped = await state(page);
  await page.waitForTimeout(200);
  expect((await state(page)).liftY).toBe(stopped.liftY);
  expect((await state(page)).held).toBe(0);
  await press(page.getByRole('button', { name: 'Resume exploration', exact: true }));
  await page.screenshot({ path: info.outputPath('service-lift.png') });
  await arrive(156);
  await move(true, s => s.z < -8);
  await expect(page.locator('#room')).toHaveText('Living quarters');
  await page.screenshot({ path: info.outputPath('quarters.png') });
  await move(false, s => s.z > -.65);
  await choose(2); await arrive(112);
  await move(true, s => s.z < -9);
  await expect(page.locator('#room')).toHaveText('Engineering hall');
  await page.screenshot({ path: info.outputPath('engineering-hall.png') });
  await move(true, s => s.z < -24.4);
  expect((await state(page)).y).toBeCloseTo(114.9, 2);
  await expect(page.getByRole('button', { name: 'Toggle inspection lights · E', exact: true })).toBeVisible();
  await press(page.getByRole('button', { name: 'Toggle inspection lights · E', exact: true }));
  await page.screenshot({ path: info.outputPath('reactor-catwalk.png') });
  await move(false, s => s.z > -.65);
  await choose(0); await arrive(196);
  await move(true, s => s.z < -8);
  expect((await state(page)).visited).toBe(15);
  expect((await state(page)).room).toBe(0);
  expect(errors).toEqual([]);
  if (session) await session.detach();
  await info.attach('interior-input-replay', { body: JSON.stringify(replay,null,2), contentType: 'application/json' });
});
