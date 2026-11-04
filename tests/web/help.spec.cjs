const {test,expect}=require('@playwright/test');
const fs=require('node:fs/promises'),path=require('node:path');
const {state,button,tap,ready,menu,save,imported}=require('./save-helpers.cjs');
const {helpTour}=require('./help-helpers.cjs');
const release=process.env.MEU_RELEASE==='1';
test('Pip explains the game and next steps before play',async({page,isMobile},info)=>helpTour(page,isMobile,info));
async function fixture(page,mobile,name){
  await menu(page,mobile);await imported(page,mobile,await fs.readFile(path.join(__dirname,'../fixtures/m1-'+name+'.meu')));
  await expect(page.locator('body')).toHaveAttribute('data-save-state','imported');
  await tap(button(page,'Close journeys'),mobile);
}
test('Pip pauses suit work and clears a held movement gesture',async({page,isMobile})=>{
  test.skip(release,'Reads production-loop diagnostics.');
  await ready(page);await fixture(page,isMobile,'airlock');await tap(button(page,'Resume exploration'),isMobile);
  const header=await page.locator('header').boundingBox(),mission=await page.locator('#mission-hud').boundingBox();
  expect(header.y+header.height).toBeLessThanOrEqual(mission.y);
  await tap(button(page,'Wrist airlock · E'),isMobile);
  for(const pattern of [/Scan pressure/,/Power shelter/,/Suit up:/])await tap(page.locator('#expedition-options button').filter({hasText:pattern}),isMobile);
  await tap(button(page,'Close station'),isMobile);
  let session;
  if(isMobile){
    session=await page.context().newCDPSession(page);
    const move=await button(page,'Move forward').boundingBox();
    await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:move.x+move.width/2,y:move.y+move.height/2,id:1}]});
    await expect.poll(async()=>(await state(page)).held).toBe(1);
    const help=await button(page,'Ask Pip for help').boundingBox();
    await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:move.x+move.width/2,y:move.y+move.height/2,id:1},{x:help.x+help.width/2,y:help.y+help.height/2,id:2}]});
  }else{
    await page.keyboard.down('KeyW');await expect.poll(async()=>(await state(page)).held).toBe(1);await page.keyboard.press('KeyH');
  }
  await expect(page.locator('body')).toHaveAttribute('data-context','help');
  const before=await state(page);expect(before.paused).toBe(true);expect(before.suit).toBe(true);expect(before.held).toBe(0);
  await expect(page.locator('#help-heading')).toHaveText('Follow the railings out');
  await page.waitForTimeout(200);const after=await state(page);
  for(const key of ['ticks','x','y','z','energy','heat','suitTicks','exposure','jobTicks'])expect(after[key]).toBe(before[key]);
  if(isMobile)await session.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});else await page.keyboard.up('KeyW');
  await tap(button(page,'Close help'),isMobile);expect((await state(page)).paused).toBe(true);
  await tap(button(page,'Resume exploration'),isMobile);expect((await state(page)).held).toBe(0);
  expect(await page.evaluate(()=>document.pointerLockElement)).toBeNull();
});
test('Pip follows imported workshop and completed journeys instead of tutorial history',async({page,isMobile})=>{
  test.skip(release,'Imports native journey checkpoints and checks progress.');
  await ready(page);await tap(button(page,'Ask Pip for help'),isMobile);await tap(button(page,'Next help topic'),isMobile);await tap(button(page,'Close help'),isMobile);
  await fixture(page,isMobile,'workshop');await tap(button(page,'Ask Pip for help'),isMobile);
  await expect(page.locator('#help-topic')).toHaveValue('0');await expect(page.locator('#help-heading')).toHaveText('Test the unfamiliar coupler');
  await tap(button(page,'Close help'),isMobile);await fixture(page,isMobile,'bridge');
  await menu(page,isMobile);await save(page,isMobile,1);await page.reload();
  await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  await tap(button(page,'Ask Pip for help'),isMobile);await expect(page.locator('#help-heading')).toHaveText('A new route is yours');
  expect((await state(page)).completed).toBe(true);
});
