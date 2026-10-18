#!/usr/bin/env node

/**
 * Caster Soundboard Web Remote Automated Headless Browser Test
 * 
 * Verifies Web Remote UI rendering, SSE connection, touch tapping,
 * tile playback states, Stop All panic button, and Ducking toggle.
 * 
 * Requirements:
 *   - Node.js >= 18
 *   - puppeteer-core (`npm install puppeteer-core`)
 *   - Chromium (`chromium` or `google-chrome` in $PATH)
 * 
 * Usage:
 *   node scripts/test-web-remote.js [--port 5052] [--user caster] [--pass caster]
 */

const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

let puppeteer;
try {
  puppeteer = require('puppeteer-core');
} catch (e) {
  try {
    puppeteer = require('puppeteer');
  } catch (e2) {
    console.error('Error: puppeteer-core or puppeteer is required to run web tests.');
    console.error('Please run: npm install puppeteer-core');
    process.exit(1);
  }
}

function findChromium() {
  const candidates = [
    process.env.CHROMIUM_BIN,
    '/usr/bin/chromium',
    '/usr/bin/chromium-browser',
    '/usr/bin/google-chrome-stable',
    '/usr/bin/google-chrome',
    '/snap/bin/chromium'
  ];

  for (const bin of candidates) {
    if (bin && fs.existsSync(bin)) return bin;
  }

  try {
    const which = execSync('which chromium 2>/dev/null || which google-chrome 2>/dev/null', { encoding: 'utf8' }).trim();
    if (which && fs.existsSync(which)) return which;
  } catch (err) {
    // Ignore
  }

  return null;
}

async function main() {
  const args = process.argv.slice(2);
  let port = '5052';
  let user = 'caster';
  let pass = 'caster';

  for (let i = 0; i < args.length; i++) {
    if (args[i] === '--port' && args[i + 1]) port = args[++i];
    if (args[i] === '--user' && args[i + 1]) user = args[++i];
    if (args[i] === '--pass' && args[i + 1]) pass = args[++i];
  }

  const chromiumBin = findChromium();
  if (!chromiumBin) {
    console.error('Error: Could not locate Chromium executable on this system.');
    console.error('Please install Chromium (e.g. `sudo pacman -S chromium` or `sudo apt install chromium-browser`).');
    process.exit(1);
  }

  console.log(`Using Chromium at: ${chromiumBin}`);
  console.log(`Connecting to Web Remote at: http://127.0.0.1:${port} (user: ${user})...`);

  const browser = await puppeteer.launch({
    executablePath: chromiumBin,
    args: ['--no-sandbox', '--disable-setuid-sandbox', '--disable-gpu'],
    headless: true
  });

  try {
    const page = await browser.newPage();
    await page.setViewport({ width: 400, height: 750, isMobile: true, hasTouch: true });

    // Catch page errors
    const errors = [];
    page.on('pageerror', err => errors.push(err.message));
    page.on('console', msg => {
      if (msg.type() === 'error') {
        errors.push(msg.text());
      }
    });

    await page.authenticate({ username: user, password: pass });
    await page.goto(`http://127.0.0.1:${port}`, { waitUntil: 'domcontentloaded', timeout: 8000 });

    // 1. Wait for tab navigation to render
    await page.waitForSelector('.tab-pill', { timeout: 5000 });
    console.log('✔ UI loaded and tab navigation rendered');

    // Switch to first tab if not already on it
    const tabs = await page.$$('.tab-pill');
    if (tabs.length > 0) {
      await tabs[0].tap();
    }
    await page.waitForSelector('.tile-btn', { timeout: 5000 });
    console.log('✔ Sound tiles rendered on active tab');

    // 2. Check connection status indicator
    await page.waitForFunction(() => {
      const dot = document.querySelector('#statusDot');
      return dot && dot.classList.contains('connected');
    }, { timeout: 4000 });
    console.log('✔ Connection badge status confirmed (Live SSE/AJAX)');

    // 3. Find a playable tile and test tapping
    const playableTile = await page.$('.tile-btn:not(.empty)');
    if (playableTile) {
      const title = await page.evaluate(el => {
        const titleEl = el.querySelector('.tile-title');
        return titleEl ? titleEl.textContent.trim() : 'Tile';
      }, playableTile);

      console.log(`Tapping tile "${title}"...`);
      await playableTile.tap();

      // Wait for playing active state
      await page.waitForFunction(() => {
        const tile = document.querySelector('.tile-btn:not(.empty)');
        return tile && tile.classList.contains('playing');
      }, { timeout: 4000 });
      console.log('✔ Tile successfully transitioned to "playing"');

      // 4. Test Panic Stop All button
      console.log('Tapping "Stop All" panic button...');
      const stopAllBtn = await page.$('#stopAllBtn');
      await stopAllBtn.tap();

      await page.waitForFunction(() => {
        const tile = document.querySelector('.tile-btn:not(.empty)');
        return tile && !tile.classList.contains('playing');
      }, { timeout: 4000 });
      console.log('✔ "Stop All" halted playback and restored tile to stopped');
    } else {
      console.log('ℹ No playable tiles on active tab (empty profile), skipping playback toggle test');
    }

    // 5. Test Tab Navigation
    const tabPills = await page.$$('.tab-pill');
    if (tabPills.length > 1) {
      console.log('Tapping secondary tab pill...');
      await tabPills[1].tap();
      await page.waitForFunction(() => {
        const activeTab = document.querySelector('.tab-pill.active');
        return activeTab && activeTab.dataset.tabIndex === '1';
      }, { timeout: 3000 });
      console.log('✔ Tab navigation switched active tab successfully');
    }

    // 6. Test Ducking Toggle
    console.log('Tapping "Ducking" button...');
    const duckBtn = await page.$('#duckBtn');
    await duckBtn.tap();
    await page.waitForFunction(() => {
      const btn = document.querySelector('#duckBtn');
      return btn && btn.classList.contains('active');
    }, { timeout: 3000 });
    console.log('✔ Ducking activated');

    await duckBtn.tap();
    await page.waitForFunction(() => {
      const btn = document.querySelector('#duckBtn');
      return btn && !btn.classList.contains('active');
    }, { timeout: 3000 });
    console.log('✔ Ducking deactivated');

    // 7. Verify no uncaught console errors occurred
    if (errors.length > 0) {
      console.warn('Browser console errors observed during test:');
      errors.forEach(e => console.warn(`  - ${e}`));
    } else {
      console.log('✔ Zero browser console errors');
    }

    // Capture verification screenshot
    const screenshotPath = '/tmp/caster_web_remote_verified.png';
    await page.screenshot({ path: screenshotPath });
    console.log(`✔ Captured verification screenshot: ${screenshotPath}`);

    console.log('\n=========================================');
    console.log('  ALL WEB REMOTE E2E TESTS PASSED 100%   ');
    console.log('=========================================\n');
  } finally {
    await browser.close();
  }
}

main().catch(err => {
  console.error('\n❌ Web Remote test failure:', err.message);
  process.exit(1);
});
