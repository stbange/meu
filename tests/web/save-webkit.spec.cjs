const {test,expect}=require('@playwright/test');
const {state,button,tap,ready,menu,save,rows,exported,imported}=require('./save-helpers.cjs');
const release=process.env.MEU_RELEASE==='1';
test('persists settings across a fresh WebKit page and exports/imports the same C snapshot',async({page,context},info)=>{
  const errors=[];page.on('pageerror',e=>errors.push(String(e)));
  await ready(page);await tap(button(page,'Settings'),true);
  await page.locator('label[for="invert-y"]').tap();
  await page.locator('label[for="walk-bob"]').tap();
  await button(page,'Close settings').tap();await menu(page,true);await save(page,true,1);
  const written=await rows(page);expect(written.latest).toHaveLength(256);
  const backup=await exported(page,true);expect(backup.length).toBe(128);
  expect(backup.subarray(0,8).toString('hex')).toBe('4d45555341564500');
  const next=await context.newPage();next.on('pageerror',e=>errors.push(String(e)));await page.close();page=next;
  await ready(page);
  if(!release){
    const loaded=await state(page);
    expect(loaded.paused).toBe(true);expect(loaded.audioAttempts).toBe(0);
    expect(loaded.invertY).toBe(true);expect(loaded.walkBob).toBe(true);expect(loaded.helm).toBe(false);
  }else expect(await page.evaluate(()=>typeof window.MEU.readState)).toBe('undefined');
  await button(page,'Settings').tap();
  await expect(page.locator('#invert-y')).toBeChecked();await expect(page.locator('#walk-bob')).toBeChecked();
  await button(page,'Close settings').tap();
  await expect(button(page,'Resume exploration')).toBeEnabled();
  await menu(page,true);await imported(page,true,backup);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','imported');
  expect((await rows(page)).latest).toBe(written.latest);
  await page.screenshot({path:info.outputPath('webkit-saved-journeys.png')});
  expect(errors).toEqual([]);
});
