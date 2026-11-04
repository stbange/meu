const {test}=require('@playwright/test');
const {helpTour}=require('./help-helpers.cjs');
test('Pip explains the game and next steps before play',async({page},info)=>helpTour(page,true,info));
