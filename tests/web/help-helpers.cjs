const {expect}=require('@playwright/test');
const {ready,button,tap,state}=require('./save-helpers.cjs');
const release=process.env.MEU_RELEASE==='1';
async function helpTour(page,mobile,info){
  const errors=[];page.on('pageerror',error=>errors.push(error.message));
  await ready(page);await tap(button(page,'Ask Pip for help'),mobile);
  const dialog=page.getByRole('dialog',{name:'Pip · Flight guide',exact:true});
  await expect(dialog).toBeVisible();await expect(page.locator('body')).toHaveAttribute('data-context','help');
  await expect(page.locator('#help-heading')).toHaveText("Let's start on the bridge");
  if(!release){const s=await state(page);expect(s.paused).toBe(true);expect(s.started).toBe(false);expect(s.audioAttempts).toBe(0);expect(s.ticks).toBe(0);}
  else expect(await page.evaluate(()=>typeof MEU.readState)).toBe('undefined');
  const box=await dialog.boundingBox(),size=page.viewportSize();
  expect(box.x).toBeGreaterThanOrEqual(0);expect(box.y).toBeGreaterThanOrEqual(0);
  expect(box.x+box.width).toBeLessThanOrEqual(size.width+.5);expect(box.y+box.height).toBeLessThanOrEqual(size.height+.5);
  for(const selector of ['#help','#help-topic','#help-previous','#help-next','#close-help']){
    const b=await page.locator(selector).boundingBox();expect(b.height).toBeGreaterThanOrEqual(48);
  }
  if(!mobile){await page.keyboard.press('Shift+Tab');await expect(button(page,'Close help')).toBeFocused();await page.keyboard.press('Tab');await expect(page.locator('#help-topic')).toBeFocused();}
  await tap(button(page,'Next help topic'),mobile);await expect(page.locator('#help-action')).toContainText('sole human Commander');
  await expect(page.locator('#help-action')).toContainText('240-metre');
  await tap(button(page,'Next help topic'),mobile);await expect(page.locator('#help-tip')).toContainText('lift');
  await expect(page.locator('#help-action')).toContainText('capture the mouse');
  await tap(button(page,'Next help topic'),mobile);await expect(page.locator('#help-action')).toContainText('independently');
  await page.locator('#help-topic').selectOption('6');await expect(page.locator('#help-heading')).toHaveText('Keep a durable journey');
  await expect(page.locator('#help-tip')).toContainText('Import replaces');
  await tap(button(page,'Previous help topic'),mobile);await expect(page.locator('#help-tip')).toContainText('BEFORE');
  const article=page.locator('#help-article');
  if(await article.evaluate(node=>node.scrollHeight>node.clientHeight+1)){
    await article.press('End');
    await expect.poll(()=>article.evaluate(node=>node.scrollTop)).toBeGreaterThan(0);
  }
  await page.locator('#help-topic').selectOption('0');await expect(page.locator('#help-heading')).toHaveText("Let's start on the bridge");
  await expect.poll(()=>article.evaluate(node=>node.scrollTop)).toBe(0);
  await page.emulateMedia({reducedMotion:'reduce'});
  expect(await page.locator('#help-panel .pip-flight').evaluate(node=>getComputedStyle(node).animationName)).toBe('none');
  await page.screenshot({path:info.outputPath('pip-help.png')});
  await tap(button(page,'Close help'),mobile);await expect(dialog).toBeHidden();
  await expect(page.locator('body')).toHaveAttribute('data-paused','true');
  await tap(button(page,'Begin exploration'),mobile);await tap(button(page,'Ask Pip for help'),mobile);
  await expect(page.locator('#help-heading')).toHaveText('Park at the tower socket');
  await tap(button(page,'Close help'),mobile);await expect(page.locator('body')).toHaveAttribute('data-paused','true');
  expect(errors).toEqual([]);
}
module.exports={helpTour};
