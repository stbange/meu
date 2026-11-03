const { test, expect } = require('@playwright/test');
const release = process.env.MEU_RELEASE === '1';
const state = page => page.evaluate(() => window.MEU.readState());
let errors;

test.beforeEach(async ({ page }) => {
  errors = [];
  page.on('pageerror', error => errors.push(String(error)));
  page.on('console', message => { if (message.type() === 'error') errors.push(message.text()); });
  await page.addInitScript(() => {
    window.__MEU_INPUT_LOG__ = [];
    for (const kind of ['keydown','keyup','pointerdown','pointerup','pointercancel','resize','visibilitychange']) {
      window.addEventListener(kind, event => {
        window.__MEU_INPUT_LOG__.push({
          type: event.type, time: performance.now(), code: event.code,
          target: event.target?.getAttribute?.('aria-label') || event.target?.id,
          pointerType: event.pointerType, x: event.clientX, y: event.clientY,
          tick: window.MEU?.readState?.().ticks
        });
      }, true);
    }
  });
  await page.goto('./');
  await expect(page.locator('body')).toHaveAttribute('data-ready', 'true');
  await expect(page.locator('body')).toHaveAttribute('data-rendered', 'true');
});

test.afterEach(async ({ page }, info) => {
  const report = await page.evaluate(() => ({
    state: window.MEU?.readState?.(), ready: document.body.dataset.ready,
    inputs: window.__MEU_INPUT_LOG__, userAgent: navigator.userAgent,
    viewport: { width: innerWidth, height: innerHeight, ratio: devicePixelRatio }
  }));
  await info.attach('state-and-input-replay', { body: JSON.stringify(report, null, 2), contentType: 'application/json' });
  expect(errors).toEqual([]);
});

test('renders the C/OpenGL room and begins only after a gesture', async ({ page, isMobile }, info) => {
  await expect(page.locator('body')).not.toHaveAttribute('data-error', 'true');
  if (!release) {
    const before = await state(page);
    expect(before.scene).toBe('m0b-unit-v1');
    if (process.env.GITHUB_SHA) expect(before.build).toContain(process.env.GITHUB_SHA);
    expect(before.frames).toBeGreaterThanOrEqual(3);
    expect(before.variation).toBeGreaterThan(8);
    expect(before.ticks).toBe(0);
    expect(before.audioAttempts).toBe(0);
  } else {
    expect(await page.evaluate(() => typeof window.MEU.readState)).toBe('undefined');
  }
  const begin = page.getByRole('button', { name: 'Begin exploration', exact: true });
  if (isMobile) await begin.tap(); else await begin.click();
  await expect(page.locator('body')).toHaveAttribute('data-paused', 'false');
  if (!release) {
    await expect.poll(async () => (await state(page)).ticks).toBeGreaterThan(1);
    expect((await state(page)).audioAttempts).toBe(1);
    expect((await state(page)).audioReady).toBe(true);
  }
  for (const button of await page.locator('#controls button:visible').all()) {
    const box = await button.boundingBox();
    expect(box.width).toBeGreaterThanOrEqual(48);
    expect(box.height).toBeGreaterThanOrEqual(48);
    expect(box.x).toBeGreaterThanOrEqual(0);
    expect(box.y + box.height).toBeLessThanOrEqual(page.viewportSize().height);
  }
  await page.screenshot({ path: info.outputPath('room.png') });
});

test('desktop input moves the real C simulation and pause clears held actions', async ({ page, isMobile }) => {
  test.skip(isMobile || release, 'Desktop keyboard scenario uses the read-only diagnostics build.');
  await page.getByRole('button', { name: 'Begin exploration', exact: true }).click();
  const before = await state(page);
  await page.keyboard.down('KeyW');
  await expect.poll(async () => (await state(page)).z).toBeLessThan(before.z - 0.2);
  await page.getByRole('button', { name: 'Pause', exact: true }).click();
  await page.keyboard.up('KeyW');
  const paused = await state(page);
  expect(paused.held).toBe(0);
  expect(paused.paused).toBe(true);
  await page.waitForTimeout(200);
  expect((await state(page)).ticks).toBe(paused.ticks);
  await page.getByRole('button', { name: 'Resume exploration', exact: true }).click();
  await expect.poll(async () => (await state(page)).ticks).toBeGreaterThan(paused.ticks);
  expect((await state(page)).z).toBeCloseTo(paused.z, 4);
  await page.keyboard.down('ArrowRight');
  await expect.poll(async () => (await state(page)).yaw).toBeGreaterThan(0.1);
  await page.keyboard.up('ArrowRight');
});

test('phone touch moves and looks without a keyboard or pointer lock', async ({ page, isMobile }) => {
  test.skip(!isMobile || release, 'Touch scenario uses an emulated phone and the read-only diagnostics build.');
  await page.getByRole('button', { name: 'Begin exploration', exact: true }).tap();
  const before = await state(page);
  const forward = await page.getByRole('button', { name: 'Move forward', exact: true }).boundingBox();
  const session = await page.context().newCDPSession(page);
  const point = { x: forward.x + forward.width/2, y: forward.y + forward.height/2, id: 1 };
  await session.send('Input.dispatchTouchEvent', { type: 'touchStart', touchPoints: [point] });
  await expect.poll(async () => (await state(page)).z).toBeLessThan(before.z - 0.2);
  await session.send('Input.dispatchTouchEvent', { type: 'touchEnd', touchPoints: [] });
  await expect.poll(async () => (await state(page)).held).toBe(0);
  const look = await page.getByRole('group', { name: 'Drag to look' }).boundingBox();
  const origin = { x: look.x + 20, y: look.y + 60, id: 2 };
  await session.send('Input.dispatchTouchEvent', { type: 'touchStart', touchPoints: [origin] });
  await session.send('Input.dispatchTouchEvent', { type: 'touchMove', touchPoints: [{ ...origin, x: origin.x+35, y: origin.y-20 }] });
  await session.send('Input.dispatchTouchEvent', { type: 'touchEnd', touchPoints: [] });
  await expect.poll(async () => (await state(page)).yaw).toBeGreaterThan(0.1);
  expect(await page.evaluate(() => document.pointerLockElement)).toBeNull();
  await session.detach();
});

test('viewport interruption cancels movement and requires explicit resume', async ({ page }) => {
  test.skip(release, 'State assertions use the diagnostics build.');
  await page.getByRole('button', { name: 'Begin exploration', exact: true }).click();
  await page.keyboard.down('KeyW');
  await expect.poll(async () => (await state(page)).held).toBe(1);
  const viewport = page.viewportSize();
  await page.setViewportSize({ width: viewport.height, height: viewport.width });
  await expect(page.locator('body')).toHaveAttribute('data-paused', 'true');
  await page.keyboard.up('KeyW');
  const paused = await state(page);
  expect(paused.held).toBe(0);
  await page.waitForTimeout(200);
  expect((await state(page)).ticks).toBe(paused.ticks);
  await page.getByRole('button', { name: 'Resume exploration', exact: true }).click();
  await expect.poll(async () => (await state(page)).ticks).toBeGreaterThan(paused.ticks);
  expect((await state(page)).z).toBeCloseTo(paused.z, 4);
});
