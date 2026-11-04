const {test,expect}=require('@playwright/test');
const fs=require('node:fs/promises');const path=require('node:path');
const {state,button,tap,ready,menu,save,imported}=require('./save-helpers.cjs');
test('operates suit recovery console with real touch and reloads the recovered coupler',async({page})=>{
  test.skip(process.env.MEU_RELEASE==='1','Observes read-only diagnostics.');
  await ready(page);await menu(page,true);
  await imported(page,true,await fs.readFile(path.join(__dirname,'../fixtures/m1-coupler.meu')));
  await tap(button(page,'Close journeys'),true);await tap(button(page,'Resume exploration'),true);
  await tap(button(page,'Phase coupler and bypass · E'),true);
  await tap(page.locator('#expedition-options button').filter({hasText:/Install shelter bypass/}),true);
  await tap(page.locator('#expedition-options button').filter({hasText:/Extract phase coupler/}),true);
  expect((await state(page)).coupler).toBe(true);expect((await state(page)).exposure).toBe(0);
  await menu(page,true);await save(page,true,1);await page.reload();
  await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  expect((await state(page)).coupler).toBe(true);expect((await state(page)).suit).toBe(true);
});
