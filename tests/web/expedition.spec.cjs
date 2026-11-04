const {test,expect}=require('@playwright/test');
const fs=require('node:fs/promises');
const path=require('node:path');
const {state,button,tap,ready,menu,save,imported}=require('./save-helpers.cjs');
const release=process.env.MEU_RELEASE==='1';
test.afterEach(async({page},info)=>{
  if(info.status!==info.expectedStatus && !release)console.error('M1 failure state:',JSON.stringify(await state(page).catch(()=>null)));
});
async function aimCoupler(page,mobile,session){
  for(let i=0;i<16;i++){
    const s=await state(page);
    const yaw=Math.atan2(27.8-s.x,-(-10-s.z)),pitch=Math.atan2(113.3-s.y,Math.hypot(27.8-s.x,-10-s.z));
    const angle=Math.atan2(Math.sin(yaw-s.yaw),Math.cos(yaw-s.yaw));
    if(Math.abs(angle)<.02 && Math.abs(pitch-s.pitch)<.02)return;
    const dx=Math.max(-32,Math.min(32,angle/.004/s.sensitivity));
    const dy=Math.max(-32,Math.min(32,-(pitch-s.pitch)/.004/s.sensitivity));
    const b=await page.locator('#look').boundingBox(),x=b.x+b.width/2,y=b.y+b.height/2;
    if(mobile){
      await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x,y,id:2}]});
      await session.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{x:x+dx,y:y+dy,id:2}]});
      await session.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
    }else{await page.mouse.move(x,y);await page.mouse.down();await page.mouse.move(x+dx,y+dy);await page.mouse.up();}
  }
  throw new Error('Coupler aim did not converge: '+JSON.stringify(await state(page)));
}
async function fixture(page,mobile,name){
  await ready(page);await menu(page,mobile);
  await imported(page,mobile,await fs.readFile(path.join(__dirname,'../fixtures/m1-'+name+'.meu')));
  await expect(page.locator('body')).toHaveAttribute('data-save-state','imported');
  await tap(button(page,'Close journeys'),mobile);await tap(button(page,'Resume exploration'),mobile);
}
async function station(page,mobile,name){await tap(button(page,name+' · E'),mobile);await expect(page.locator('#expedition-panel')).toBeVisible();}
async function option(page,mobile,pattern){const choice=page.locator('#expedition-options button').filter({hasText:pattern});await expect(choice).toHaveCount(1);await tap(choice,mobile);}

test('uses real desktop or touch airlock controls, walks the conduit, bypasses and persists the recovered coupler',async({page,isMobile},info)=>{
  test.skip(release,'Observes read-only production-loop diagnostics.');test.setTimeout(60000);
  await fixture(page,isMobile,'airlock');
  await station(page,isMobile,'Wrist airlock');
  await option(page,isMobile,/Suit up:/);await expect(page.locator('#expedition-message')).toContainText('scan');
  await option(page,isMobile,/Scan pressure/);await expect(page.locator('#expedition-message')).toContainText('80 kPa');
  await option(page,isMobile,/Power shelter/);await option(page,isMobile,/Suit up:/);
  expect((await state(page)).suit).toBe(true);
  for(const control of await page.locator('#expedition-options button:visible').all()){
    const b=await control.boundingBox();expect(b.height).toBeGreaterThanOrEqual(48);
  }
  await page.evaluate(()=>document.addEventListener('pointerdown',event=>{
    if(event.target.closest?.('#close-expedition'))window.__stationClosed={hidden:document.querySelector('#expedition-panel').hidden,context:document.body.dataset.context};
  }));
  await tap(button(page,'Close station'),isMobile);
  expect(await page.evaluate(()=>window.__stationClosed)).toEqual({hidden:true,context:'foot'});
  let session;
  if(isMobile){
    session=await page.context().newCDPSession(page);
    const b=await button(page,'Move forward').boundingBox();
    await session.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[{x:b.x+b.width/2,y:b.y+b.height/2,id:1}]});
  }else await page.keyboard.down('KeyW');
  try{await expect.poll(async()=>{const s=await state(page);return s.x>25.1;},{timeout:15000,intervals:[30,50]}).toBe(true);}
  finally{if(isMobile)await session.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});else await page.keyboard.up('KeyW');}
  // Stop at the physical approach, then aim through real look gestures.
  // Polling an interaction while still walking can miss its viewing cone under a busy software renderer.
  await aimCoupler(page,isMobile,session);
  await expect.poll(async()=>(await state(page)).use).toBe(10);
  await station(page,isMobile,'Phase coupler and bypass');
  await option(page,isMobile,/Install shelter bypass/);await option(page,isMobile,/Extract phase coupler/);
  expect((await state(page)).coupler).toBe(true);expect((await state(page)).bypass).toBe(true);
  await page.screenshot({path:info.outputPath('m1-conduit.png')});
  await menu(page,isMobile);await save(page,isMobile,1);const saved=await state(page);
  await page.reload();await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  const loaded=await state(page);expect(loaded.paused).toBe(true);expect(loaded.suit).toBe(true);
  expect(loaded.coupler).toBe(true);expect(loaded.bypass).toBe(true);expect(loaded.suitTicks).toBe(saved.suitTicks);
  expect(loaded.x).toBeCloseTo(saved.x,4);expect(loaded.robotZ).toBeCloseTo(-140,2);
});

test('tests and adapts recovered technology through native-parity station buttons and persists installed access',async({page,isMobile})=>{
  test.skip(release,'Observes read-only production-loop diagnostics.');
  await fixture(page,isMobile,'workshop');await station(page,isMobile,'Salvage cradle and test bench');
  await option(page,isMobile,/Controlled coupler test/);await option(page,isMobile,/Adapter: alloy/);
  await option(page,isMobile,/Install stabilizer:/);expect((await state(page)).installed).toBe(true);
  await option(page,isMobile,/Install stabilizer:/);await expect(page.locator('#expedition-message')).toContainText('empty installation socket');
  await menu(page,isMobile);await save(page,isMobile,1);await page.reload();
  await expect(page.locator('body')).toHaveAttribute('data-ready','true');expect((await state(page)).installed).toBe(true);
});

test('shows usable helm coordinates without mission/navigation overlap',async({page,isMobile})=>{
  test.skip(release,'Uses the native completed-journey fixture.');
  await fixture(page,isMobile,'bridge');
  await tap(button(page,'Take bridge helm · E'),isMobile);
  await expect(page.locator('#coordinates')).toContainText('X -163');
  await expect(page.locator('#coordinates')).toContainText('Z -90');
  await expect(page.locator('body')).toHaveAttribute('data-helm','true');
  const mission=await page.locator('#mission-hud').boundingBox(),navigation=await page.locator('#navigation').boundingBox();
  expect(mission.x>=navigation.x+navigation.width || mission.x+mission.width<=navigation.x ||
    mission.y>=navigation.y+navigation.height || mission.y+mission.height<=navigation.y).toBe(true);
  expect(mission.x).toBeGreaterThanOrEqual(0);expect(mission.y+mission.height).toBeLessThanOrEqual(page.viewportSize().height);
});
