const { defineConfig } = require('@playwright/test');
const release = process.env.MEU_RELEASE === '1';
const deployedURL = process.env.MEU_BASE_URL;
module.exports = defineConfig({
  testDir: './tests/web',
  timeout: 30000,
  expect: { timeout: 7000 },
  retries: 0,
  workers: 1,
  outputDir: release ? 'test-results/web-release' : 'test-results/web-development',
  reporter: [['list'], ['html', { open: 'never', outputFolder: release ? 'playwright-report/release' : 'playwright-report/development' }]],
  use: {
    baseURL: deployedURL || 'http://127.0.0.1:4174/',
    headless: true,
    trace: 'retain-on-failure',
    screenshot: 'only-on-failure',
    launchOptions: { args: ['--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'] }
  },
  projects: [
    { name: 'desktop', testIgnore: '**/webkit.spec.cjs', use: { viewport: { width: 1100, height: 700 } } },
    { name: 'phone-portrait', testIgnore: '**/webkit.spec.cjs', use: { viewport: { width: 360, height: 640 }, isMobile: true, hasTouch: true } },
    { name: 'phone-landscape', testIgnore: '**/webkit.spec.cjs', use: { viewport: { width: 640, height: 360 }, isMobile: true, hasTouch: true } },
    { name: 'webkit-portrait', testMatch: '**/webkit.spec.cjs', use: { browserName: 'webkit', launchOptions: { args: [] }, viewport: { width: 360, height: 640 }, isMobile: true, hasTouch: true } },
    { name: 'webkit-landscape', testMatch: '**/webkit.spec.cjs', use: { browserName: 'webkit', launchOptions: { args: [] }, viewport: { width: 640, height: 360 }, isMobile: true, hasTouch: true } }
  ],
  webServer: deployedURL ? undefined : {
    command: (process.platform === 'win32' ? 'python' : 'python3') + ' tools/serve.py' +
      (release ? ' --directory build/web-release/bin' : ''),
    url: 'http://127.0.0.1:4174',
    reuseExistingServer: false,
    timeout: 15000
  }
});
