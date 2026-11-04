const {test,expect}=require('@playwright/test');
const {state,ready,button,menu,imported}=require('./save-helpers.cjs');
const fs=require('node:fs/promises'),path=require('node:path');
const release=process.env.MEU_RELEASE==='1';
async function begin(page){await ready(page);await button(page,'Begin exploration').click();}
async function capture(page){await page.mouse.click(320,320);await expect.poll(()=>page.evaluate(()=>document.pointerLockElement?.id)).toBe('canvas');}

test('click captures the desktop mouse, free relative motion looks, Escape clears held input',async({page,isMobile})=>{
  test.skip(isMobile,'Phones retain touch look without pointer lock.');
  await begin(page);await capture(page);
  await page.mouse.move(1300,410); // outside the viewport; button already released
  const before=release?null:await state(page);
  await page.mouse.move(1310,412);
  if(!release){
    await expect.poll(async()=>{const after=await state(page);return Math.abs(after.yaw-before.yaw);}).toBeGreaterThan(.01);
    await page.keyboard.down('KeyW');await expect.poll(async()=>(await state(page)).held).toBe(1);
  }
  await page.keyboard.press('Escape');await page.keyboard.up('KeyW');
  await expect.poll(()=>page.evaluate(()=>document.pointerLockElement)).toBeNull();
  await expect(page.locator('body')).toHaveAttribute('data-paused','true');
  if(!release)expect((await state(page)).held).toBe(0);
  await button(page,'Resume exploration').click();
  expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
});
for(const [key,title] of [['F1','Close settings'],['F2','Close journeys']]){
  test(`desktop ${key} releases the mouse for menus and requires a fresh capture click`,async({page,isMobile})=>{
    test.skip(isMobile,'Desktop pointer lock lifecycle.');
    await begin(page);await capture(page);await page.keyboard.press(key);
    await expect.poll(()=>page.evaluate(()=>document.pointerLockElement)).toBeNull();
    await expect(button(page,title)).toBeVisible();await button(page,title).click();
    await button(page,'Resume exploration').click();
    expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
    await capture(page);await page.evaluate(()=>document.exitPointerLock());
    await expect(page.locator('body')).toHaveAttribute('data-paused','true');
  });
}
test('opening a station releases capture without pausing its active simulation',async({page,isMobile})=>{
  test.skip(isMobile || release,'Reads real station state on desktop.');
  await ready(page);await menu(page,false);
  await imported(page,false,await fs.readFile(path.join(__dirname,'../fixtures/m1-workshop.meu')));
  await button(page,'Close journeys').click();await button(page,'Resume exploration').click();
  await capture(page);await page.keyboard.press('KeyE');
  await expect(page.locator('#expedition-panel')).toBeVisible();
  await expect.poll(()=>page.evaluate(()=>document.pointerLockElement)).toBeNull();
  expect((await state(page)).paused).toBe(false);
  await button(page,'Close station').click();
  expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
  expect((await state(page)).paused).toBe(false);
});
test('a refused browser lock leaves usable drag look and menus',async({page,isMobile})=>{
  test.skip(isMobile || release,'Desktop browser refusal fallback.');
  await page.addInitScript(()=>{HTMLCanvasElement.prototype.requestPointerLock=function(){return Promise.reject(new Error('Lock unavailable'));};});
  await begin(page);const before=await state(page);
  await page.mouse.move(320,320);await page.mouse.down();await page.mouse.move(350,330);await page.mouse.up();
  await expect.poll(async()=>(await state(page)).yaw).toBeGreaterThan(before.yaw+.05);
  expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
  await button(page,'Settings').click();await expect(button(page,'Close settings')).toBeVisible();
});
