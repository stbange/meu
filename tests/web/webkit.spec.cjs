const { test, expect } = require('@playwright/test');
const release=process.env.MEU_RELEASE==='1';
test('renders the C/OpenGL room and supports touch settings in WebKit',async ({ page },info) => {
  const errors=[]; page.on('pageerror',e=>errors.push(String(e)));
  await page.goto('./');
  await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  await expect(page.locator('body')).toHaveAttribute('data-rendered','true');
  if (release) expect(await page.evaluate(()=>typeof window.MEU.readState)).toBe('undefined');
  await page.getByRole('button',{name:'Settings',exact:true}).tap();
  const settings=page.getByRole('dialog',{name:'Settings',exact:true});
  await expect(settings).toBeVisible();
  if (!release) {
    const s=await page.evaluate(()=>window.MEU.readState());
    expect(s.audioAttempts).toBe(0); expect(s.paused).toBe(true);
    expect(s.scene).toBe('m1-tower-v1');
    if (process.env.GITHUB_SHA) expect(s.build).toContain(process.env.GITHUB_SHA);
  }
  await page.locator('label[for="walk-bob"]').tap();
  await page.getByRole('button',{name:'Close settings',exact:true}).tap();
  await expect(page.locator('body')).toHaveAttribute('data-paused','true');
  await page.getByRole('button',{name:'Begin exploration',exact:true}).tap();
  await expect(page.locator('body')).toHaveAttribute('data-paused','false');
  await page.getByRole('button',{name:'Pause',exact:true}).tap();
  await expect(page.locator('body')).toHaveAttribute('data-paused','true');
  await page.getByRole('button',{name:'Resume exploration',exact:true}).tap();
  await expect(page.locator('body')).toHaveAttribute('data-paused','false');
  await page.screenshot({path:info.outputPath('webkit-touch.png')});
  expect(errors).toEqual([]);
});
