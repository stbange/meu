const { spawnSync } = require('node:child_process');
const executable = process.env.MEU_PYTHON || (process.platform === 'win32' ? 'python' : 'python3');
const result = spawnSync(executable, process.argv.slice(2), { stdio: 'inherit' });
if (result.error) console.error(result.error.message);
process.exit(result.status ?? 1);
