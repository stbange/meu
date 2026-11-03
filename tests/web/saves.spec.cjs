const {test,expect}=require('@playwright/test');
const fs=require('node:fs/promises'),path=require('node:path');
const {state,button,tap,ready,menu,save,rows,setRecord,exported,imported}=require('./save-helpers.cjs');
const release=process.env.MEU_RELEASE==='1';
let errors;
const persisted=s=>Object.fromEntries(['ticks','x','y','z','yaw','pitch','robotX','robotZ','robotYaw','room','liftY',
  'sensitivity','invertY','walkBob','machineSway','volume','visited','paused','held','helm','parked','speed','turnRate'].map(k=>[k,s[k]]));
test.beforeEach(async({page,context})=>{
  errors=[];const observe=p=>p.on('pageerror',e=>errors.push(String(e)));
  observe(page);context.on('page',observe);
  await context.addInitScript(()=>{
    window.__MEU_SAVE_INPUTS__=[];
    for(const type of ['pointerdown','pointerup','keydown','keyup','visibilitychange','blur','focus'])
      window.addEventListener(type,e=>window.__MEU_SAVE_INPUTS__.push({type,code:e.code,
        target:e.target?.closest?.('button')?.id,time:performance.now(),state:window.MEU?.readState?.()}),true);
  });
});
test.afterEach(async({context},info)=>{
  if(release)return;
  const p=context.pages().filter(p=>!p.isClosed()).at(-1);
  if(p)await info.attach('save-state-and-inputs',{body:JSON.stringify(await p.evaluate(()=>({
    state:window.MEU?.readState?.(),inputs:window.__MEU_SAVE_INPUTS__,saveState:document.body.dataset.saveState,
    status:document.querySelector('#save-status')?.textContent,viewport:{width:innerWidth,height:innerHeight}
  })),null,2),contentType:'application/json'});
  expect(errors).toEqual([]);
});

test('recovers a previous snapshot, keeps damaged data and rejects incompatible imports atomically',async({page,context,isMobile},info)=>{
  test.skip(release,'Uses read-only C state and storage fault injection.');test.setTimeout(45000);
  await ready(page);await menu(page,isMobile);await save(page,isMobile,1);
  const first=await rows(page);await tap(button(page,'Close journeys'),isMobile);
  await tap(button(page,'Settings'),isMobile);await tap(page.locator('label[for="invert-y"]'),isMobile);
  await tap(button(page,'Close settings'),isMobile);await menu(page,isMobile);await save(page,isMobile,2);
  const second=await rows(page);expect(second.previous).toBe(first.latest);
  const bytes=Buffer.from(second.latest,'hex');bytes[56]^=1;const corrupt=bytes.toString('hex');
  await setRecord(page,'latest',corrupt);
  const next=await context.newPage();await page.close();page=next;await ready(page);
  await expect(page.locator('#save-note')).toContainText('damaged');
  expect((await state(page)).paused).toBe(true);expect((await state(page)).audioAttempts).toBe(0);
  await menu(page,isMobile);await tap(button(page,'Recover previous'),isMobile);
  await expect(page.locator('#save-status')).toContainText('Recovered journey 1');
  expect((await state(page)).invertY).toBe(false);expect((await rows(page)).latest).toBe(corrupt);
  expect(await exported(page,isMobile,'Export damaged save')).toEqual(bytes);
  const backup=await exported(page,isMobile);expect(backup.length).toBe(128);
  const unchanged=persisted(await state(page));
  const future=Buffer.from(backup);future.writeUInt32LE(99,8);
  await imported(page,isMobile,future,'future.meu');
  await expect(page.locator('body')).toHaveAttribute('data-save-state','error');
  await expect(page.locator('#save-status')).toContainText('Unsupported save version');
  expect(persisted(await state(page))).toEqual(unchanged);expect((await rows(page)).latest).toBe(corrupt);
  const malformed=Buffer.from(backup);malformed[80]^=1;
  await imported(page,isMobile,malformed,'broken.meu');
  await expect(page.locator('#save-status')).toContainText('damaged');
  expect(persisted(await state(page))).toEqual(unchanged);
  const reference=await fs.readFile(path.join(__dirname,'../fixtures/save-v1.meu'));
  await imported(page,isMobile,reference,'native-reference.meu');
  await expect(page.locator('body')).toHaveAttribute('data-save-state','imported');
  expect((await state(page)).ticks).toBe(12345);expect((await state(page)).paused).toBe(true);
  expect((await rows(page)).latest).toBe(corrupt); // Import does not publish over the local journal.
  await save(page,isMobile,2);const repaired=await rows(page);
  expect(repaired['damaged-latest']).toBe(corrupt);expect(repaired.previous).toBe(first.latest);
  await page.screenshot({path:info.outputPath('recovered-journey.png')});
  await page.reload();await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  expect((await state(page)).ticks).toBe(12345);expect((await state(page)).helm).toBe(false);
  const oversized='ab'.repeat(131072);
  await setRecord(page,'previous',repaired.latest);await setRecord(page,'latest',oversized);
  await page.reload();await expect(page.locator('body')).toHaveAttribute('data-ready','true');
  await menu(page,isMobile);await tap(button(page,'Recover previous'),isMobile);
  await expect(page.locator('#save-status')).toContainText('Recovered journey 2');
  expect((await state(page)).ticks).toBe(12345);
  expect((await exported(page,isMobile,'Export damaged save')).length).toBe(131072);
  expect((await rows(page)).latest).toBe(oversized);
});

test('waits for durable completion and preserves the confirmed snapshot through quota failure and interrupted publication',async({page,context,isMobile})=>{
  test.skip(release,'Exercises real IndexedDB transaction failure boundaries.');test.setTimeout(45000);
  await ready(page);await menu(page,isMobile);await save(page,isMobile,1);const confirmed=await rows(page);
  await page.evaluate(()=>{
    window.__originalPut=IDBObjectStore.prototype.put;
    IDBObjectStore.prototype.put=function(data,key){
      if(key==='latest')throw new DOMException('Injected storage quota failure','QuotaExceededError');
      return window.__originalPut.call(this,data,key);
    };
  });
  await tap(button(page,'Save journey'),isMobile);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','error');
  await expect(page.locator('#save-status')).toContainText('storage is full');
  expect(await rows(page)).toEqual(confirmed);
  await page.evaluate(()=>{
    IDBObjectStore.prototype.put=window.__originalPut;
    window.__requestSucceeded=false;
    IDBObjectStore.prototype.put=function(data,key){
      const request=window.__originalPut.call(this,data,key);
      if(key==='latest')request.addEventListener('success',()=>{
        window.__requestSucceeded=true;
        // Keep a real read/write transaction pending after put succeeds, then
        // close the page. Request success must never be displayed as Saved.
        const store=this,hold=()=>{const r=store.get('__hold__');r.onsuccess=hold;};hold();
      });
      return request;
    };
  });
  await tap(button(page,'Save journey'),isMobile);
  await page.waitForFunction(()=>window.__requestSucceeded);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','pending');
  await expect(page.locator('#save-status')).toContainText('waiting for storage confirmation');
  await expect(button(page,'Close journeys')).toBeDisabled();
  await page.keyboard.press('Shift+Tab');
  await expect(page.locator('#journeys-title')).toBeFocused();
  const next=await context.newPage();await page.close();page=next;await ready(page);
  const after=await rows(page);
  // Navigation may abort or finish a transaction; both outcomes must keep a
  // complete confirmed generation, never a partly published journal.
  expect(after.latest===confirmed.latest || after.previous===confirmed.latest).toBe(true);
  const probe=await page.evaluate(hex=>JSON.parse(Module.ccall('meu_web_probe','string',['string'],[hex])),after.latest);
  expect(probe.result).toBe(0);expect([1,2]).toContain(probe.generation);
  expect((await state(page)).paused).toBe(true);expect((await state(page)).held).toBe(0);
});

test('keeps play and portable backup available when local storage is unavailable',async({page,isMobile})=>{
  test.skip(release,'Injects browser storage denial without a shipping fault endpoint.');
  await page.addInitScript(()=>Object.defineProperty(window,'indexedDB',{get(){throw new DOMException('Storage disabled','SecurityError');}}));
  await ready(page);await expect(page.locator('#save-note')).toContainText('unavailable');
  await menu(page,isMobile);await tap(button(page,'Save journey'),isMobile);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','error');
  const backup=await exported(page,isMobile);expect(backup.length).toBe(128);
  const reference=await fs.readFile(path.join(__dirname,'../fixtures/save-v1.meu'));
  await imported(page,isMobile,reference);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','imported');
  expect((await state(page)).ticks).toBe(12345);
  await tap(button(page,'Close journeys'),isMobile);await tap(button(page,'Resume exploration'),isMobile);
  await expect(page.locator('body')).toHaveAttribute('data-paused','false');
});

test('serializes journal generations across two game tabs',async({page,context,isMobile})=>{
  test.skip(release || isMobile,'One desktop case exercises cross-tab IndexedDB serialization.');
  await ready(page);await menu(page,false);await save(page,false,1);
  const other=await context.newPage();await ready(other);await menu(other,false);
  await Promise.all([button(page,'Save journey').click(),button(other,'Save journey').click()]);
  await expect(page.locator('body')).toHaveAttribute('data-save-state','saved');
  await expect(other.locator('body')).toHaveAttribute('data-save-state','saved');
  const generations=[await page.locator('body').getAttribute('data-save-generation'),await other.locator('body').getAttribute('data-save-generation')].sort();
  expect(generations).toEqual(['2','3']);
  const result=await rows(other);
  const metadata=await other.evaluate(rows=>['latest','previous'].map(key=>JSON.parse(Module.ccall('meu_web_probe','string',['string'],[rows[key]]))),result);
  expect(metadata.map(m=>m.generation)).toEqual([3,2]);expect(metadata.every(m=>m.result===0)).toBe(true);
});

test('keeps journey actions within phone safe areas and restores keyboard focus on close',async({page,isMobile},info)=>{
  test.skip(release,'Save menu layout and keyboard regression check.');
  await ready(page);
  await page.addStyleTag({content:':root{--safe-top:20px;--safe-bottom:16px;--safe-left:24px;--safe-right:24px;}'});
  if(isMobile)await menu(page,true);else await page.keyboard.press('F2');
  await expect(page.getByRole('dialog',{name:'Saved journeys',exact:true})).toBeVisible();
  for(const selector of ['#journeys-panel','#close-journeys']){
    const b=await page.locator(selector).boundingBox();
    expect(b.x).toBeGreaterThanOrEqual(24);expect(b.y).toBeGreaterThanOrEqual(20);
    expect(b.x+b.width).toBeLessThanOrEqual(page.viewportSize().width-24);
    expect(b.y+b.height).toBeLessThanOrEqual(page.viewportSize().height-16);
  }
  for(const name of ['Save journey','Export journey','Import journey']){
    await button(page,name).scrollIntoViewIfNeeded();const b=await button(page,name).boundingBox();
    expect(b.width).toBeGreaterThanOrEqual(48);expect(b.height).toBeGreaterThanOrEqual(48);
  }
  await page.screenshot({path:info.outputPath('safe-journey-menu.png')});
  if(isMobile)await button(page,'Close journeys').tap();
  else{
    await button(page,'Close journeys').focus();await page.keyboard.press('Tab');
    await expect(button(page,'Save journey')).toBeFocused();
    await page.keyboard.press('Escape');await expect(button(page,'Begin exploration')).toBeFocused();
    await page.keyboard.press('Enter');await expect(page.locator('body')).toHaveAttribute('data-paused','false');
  }
});
