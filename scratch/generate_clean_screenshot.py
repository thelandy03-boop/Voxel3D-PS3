import os
import subprocess

html_content = """<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <title>Minecraft: PlayStation 3 Edition — Mod Loader & Manager v1.4.2</title>
  <style>
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
    }
    body {
      background-color: #0b0c10;
      color: #e0d6cb;
      font-family: 'Segoe UI', Tahoma, Arial, sans-serif;
      width: 1280px;
      height: 720px;
      overflow: hidden;
      position: relative;
    }

    /* BACKDROP WITH HEAVY BLUR SO NO GHOST TEXT REMAINS */
    .backdrop {
      position: absolute;
      top: 0; left: 0; width: 100%; height: 100%;
      overflow: hidden;
      z-index: 0;
      background: #111422;
    }
    .backdrop img {
      width: 100%; height: 100%;
      object-fit: cover;
      transform: scale(1.15);
      filter: blur(26px) brightness(0.55) saturate(1.25);
    }
    .backdrop::after {
      content: '';
      position: absolute;
      inset: 0;
      background: linear-gradient(180deg, rgba(0,0,0,0.45) 0%, rgba(0,0,0,0.15) 30%, rgba(0,0,0,0.25) 70%, rgba(0,0,0,0.7) 100%);
    }

    .tv-frame {
      position: relative;
      z-index: 1;
      width: 1280px;
      height: 720px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      padding: 24px 44px 20px 44px;
    }

    .mc-shadow {
      text-shadow: 2px 2px 0px #000, -1px -1px 0px #000, 1px -1px 0px #000, -1px 1px 0px #000;
    }
    .mc-title-shadow {
      text-shadow: 4px 4px 0px #000, -2px -2px 0px #000, 2px -2px 0px #000, -2px 2px 0px #000;
    }
    .mc-gold-shadow {
      text-shadow: 2px 2px 0px #382c03, -1px -1px 0px #382c03;
    }

    /* TOP ROW */
    .top-bar {
      display: flex;
      justify-content: space-between;
      align-items: flex-start;
      height: 52px;
    }
    .ps3-brand {
      display: flex;
      align-items: center;
      gap: 10px;
    }
    .ps3-wordmark {
      font-size: 26px;
      font-weight: 700;
      color: #ffffff;
      letter-spacing: -0.5px;
    }
    .ps3-wordmark sup {
      font-size: 11px;
      vertical-align: super;
    }
    .telemetry-hud {
      text-align: right;
      font-family: 'Consolas', 'Courier New', monospace;
      font-size: 14px;
      line-height: 1.4;
      color: #e2e8f0;
    }

    /* CENTER TITLE */
    .center-title {
      text-align: center;
      margin-top: -12px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .mc-logo {
      font-family: 'Arial Black', Impact, sans-serif;
      font-size: 54px;
      font-weight: 900;
      color: #ded9d2;
      letter-spacing: 2.5px;
      line-height: 1;
    }
    .edition-banner {
      background: linear-gradient(180deg, #3c3631 0%, #1c1916 100%);
      border: 2px solid #090807;
      box-shadow: inset 0 1px 0 #6e645b;
      padding: 3px 16px;
      font-size: 12px;
      font-weight: 900;
      letter-spacing: 0.25em;
      color: #f2f0ea;
      margin-top: 4px;
    }
    .mod-subtitle {
      color: #f4e436;
      font-family: 'Consolas', monospace;
      font-size: 19px;
      font-weight: 700;
      letter-spacing: 1px;
      margin-top: 6px;
    }

    /* MAIN CONTAINER */
    .main-container {
      width: 100%;
      max-width: 1140px;
      margin: 0 auto;
    }

    /* TABS */
    .tab-row {
      display: flex;
      align-items: flex-end;
      gap: 8px;
      padding-left: 4px;
    }
    .bumper-btn {
      background: #2b2520;
      border: 2px solid #0a0806;
      border-radius: 4px 4px 0 0;
      color: #c9c3b8;
      font-family: monospace;
      font-size: 13px;
      font-weight: bold;
      padding: 6px 12px;
    }
    .tab-btn {
      flex: 1;
      text-align: center;
      padding: 9px 12px;
      font-size: 15px;
      font-weight: bold;
      border-radius: 6px 6px 0 0;
      border: 2px solid #0a0806;
      letter-spacing: 0.6px;
    }
    .tab-active {
      background: linear-gradient(180deg, #4b4036 0%, #2d251e 100%);
      color: #ffffff;
      border-bottom: 2px solid #2d251e;
      margin-bottom: -2px;
      z-index: 5;
    }
    .tab-inactive {
      background: linear-gradient(180deg, #3d342c 0%, #1e1915 100%);
      color: #8c786a;
      border-bottom: 2px solid #0a0806;
    }

    /* PANELS */
    .panel-frame {
      background: linear-gradient(180deg, rgba(45, 37, 30, 0.96) 0%, rgba(24, 19, 15, 0.96) 100%);
      border: 3px solid #0a0806;
      box-shadow: inset 2px 2px 0 rgba(160, 144, 128, 0.28), inset -2px -2px 0 rgba(0, 0, 0, 0.65), 0 16px 40px rgba(0,0,0,0.8);
      padding: 16px;
      height: 410px;
      display: flex;
      gap: 18px;
    }

    /* LEFT MOD LIST */
    .mod-list-box {
      flex: 1.05;
      background: rgba(16, 13, 10, 0.94);
      border: 2px solid #0a0806;
      display: flex;
      overflow: hidden;
    }
    .mod-list-scroll {
      flex: 1;
      padding: 8px;
      display: flex;
      flex-direction: column;
      gap: 5px;
    }
    .mod-row {
      display: flex;
      align-items: center;
      gap: 12px;
      padding: 7px 10px;
      background: rgba(34, 29, 25, 0.5);
      border: 2px solid transparent;
      min-height: 42px;
    }
    .mod-row.selected {
      background: linear-gradient(180deg, rgba(108, 102, 88, 0.7) 0%, rgba(72, 67, 57, 0.75) 100%);
      border: 2px solid #f4e436;
      box-shadow: 0 0 10px rgba(244, 228, 54, 0.25);
    }
    .mod-checkbox {
      width: 26px;
      height: 26px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 15px;
      font-weight: bold;
      border: 2px solid #090807;
      flex-shrink: 0;
    }
    .chk-on {
      background: #256621;
      color: #5ae84a;
    }
    .chk-off {
      background: #6e2424;
      color: #ea4335;
    }
    .mod-row-title {
      font-size: 15px;
      font-weight: bold;
      letter-spacing: 0.5px;
      color: #f4f2ec;
      white-space: nowrap;
      overflow: hidden;
      text-overflow: ellipsis;
    }
    .mod-row.selected .mod-row-title {
      color: #f4e436;
    }

    /* SCROLLBAR */
    .mc-scrollbar {
      width: 22px;
      background: #14100e;
      border-left: 2px solid #0a0806;
      display: flex;
      flex-direction: column;
    }
    .mc-scroll-arrow {
      height: 22px;
      background: #4b443e;
      color: #111;
      font-size: 11px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: bold;
    }
    .mc-scroll-track {
      flex: 1;
      position: relative;
      background: #1a1613;
    }
    .mc-scroll-thumb {
      position: absolute;
      left: 2px;
      right: 2px;
      top: 68px;
      height: 70px;
      background: #736c64;
      border: 1px solid #0c0a08;
    }

    /* RIGHT DETAILS BOX */
    .mod-detail-box {
      flex: 1.15;
      background: rgba(16, 13, 10, 0.94);
      border: 2px solid #0a0806;
      padding: 18px 22px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
    }
    .detail-top {
      display: flex;
      gap: 16px;
    }
    .mod-thumb {
      width: 90px;
      height: 90px;
      border: 2px solid #090807;
      background: #243b55;
      display: flex;
      align-items: center;
      justify-content: center;
      flex-shrink: 0;
      box-shadow: 0 0 0 1px rgba(255,255,255,0.15);
      overflow: hidden;
    }
    .detail-heading {
      flex: 1;
    }
    .detail-title {
      font-size: 21px;
      font-weight: bold;
      color: #ffffff;
      margin-bottom: 6px;
    }
    .detail-author {
      font-size: 14px;
      color: #c9c3b8;
    }
    .detail-desc {
      margin-top: 16px;
      font-size: 13.5px;
      line-height: 1.6;
      color: #ece9e0;
    }
    .detail-meta {
      margin-top: 14px;
      display: flex;
      flex-direction: column;
      gap: 8px;
      font-size: 13.5px;
    }
    .status-enabled {
      color: #5ae84a;
      font-weight: bold;
    }
    .warning-text {
      color: #ea4335;
      font-weight: 600;
    }

    /* BOTTOM CONTROLLER PROMPT BAR */
    .bottom-bar {
      background: linear-gradient(180deg, rgba(0,0,0,0.5) 0%, rgba(0,0,0,0.95) 100%);
      padding: 10px 24px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 32px;
      border-radius: 4px;
      border-top: 1px solid rgba(255,255,255,0.1);
    }
    .prompt-item {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      font-size: 14px;
      font-weight: bold;
      font-family: monospace;
    }
    .prompt-btn-icon {
      width: 22px;
      height: 22px;
      border-radius: 50%;
      background: #1f1f1f;
      border: 2px solid #090909;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      font-size: 12px;
    }
    .prompt-cross { color: #6ab7ff; }
    .prompt-circle { color: #f28b82; }
    .prompt-square { color: #f48fb1; }
    .prompt-triangle { color: #81e699; }
  </style>
</head>
<body>
  <!-- BACKGROUND WITHOUT TEXT GHOSTS -->
  <div class="backdrop">
    <img src="reference_ui.jpg" alt="Minecraft Sunset Backdrop">
  </div>

  <div class="tv-frame">
    <!-- TOP BAR -->
    <div class="top-bar">
      <div class="ps3-brand">
        <svg width="34" height="26" viewBox="0 0 24 24" fill="#ffffff">
          <path d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-1 15h-2v-6h2v6zm4 0h-2v-6h2v6z"/>
        </svg>
        <span class="ps3-wordmark mc-shadow">PlayStation<sup>®</sup>3</span>
      </div>

      <div class="telemetry-hud mc-shadow">
        <div>CPU: <span style="color:#5ae84a; font-weight:bold;">56°C</span> | RSX: <span style="color:#5ae84a; font-weight:bold;">51°C</span></div>
        <div>RAM: <span style="color:#6ab7ff; font-weight:bold;">174MB</span>/256MB</div>
        <div style="color:#f4e436; font-weight:bold;">CFW 4.84 DEX</div>
      </div>
    </div>

    <!-- CENTER TITLE STACK -->
    <div class="center-title">
      <div class="mc-logo mc-title-shadow">MINECRAFT</div>
      <div class="edition-banner mc-shadow">PLAYSTATION®3 EDITION</div>
      <div class="mod-subtitle mc-gold-shadow">MOD LOADER & MANAGER v1.4.2</div>
    </div>

    <!-- MAIN CONTAINER -->
    <div class="main-container">
      <!-- TABS -->
      <div class="tab-row">
        <div class="bumper-btn mc-shadow">[L1]</div>
        <div class="tab-btn tab-active mc-shadow">INSTALLED MODS</div>
        <div class="tab-btn tab-inactive mc-shadow">USB INSTALL</div>
        <div class="tab-btn tab-inactive mc-shadow">SETTINGS</div>
        <div class="tab-btn tab-inactive mc-shadow">SYSTEM</div>
        <div class="bumper-btn mc-shadow">[R1]</div>
      </div>

      <!-- PANEL FRAME -->
      <div class="panel-frame">
        <!-- LEFT: MOD LIST -->
        <div class="mod-list-box">
          <div class="mod-list-scroll">
            <div class="mod-row">
              <div class="mod-checkbox chk-on">✓</div>
              <div class="mod-row-title mc-shadow">TOO MANY ITEMS</div>
            </div>
            <div class="mod-row">
              <div class="mod-checkbox chk-on">✓</div>
              <div class="mod-row-title mc-shadow">X-RAY VISION</div>
            </div>
            <div class="mod-row selected">
              <div class="mod-checkbox chk-on">✓</div>
              <div class="mod-row-title mc-shadow">FLY MOD v2.1</div>
            </div>
            <div class="mod-row">
              <div class="mod-checkbox chk-on">✓</div>
              <div class="mod-row-title mc-shadow">DYNAMIC LIGHTS</div>
            </div>
            <div class="mod-row">
              <div class="mod-checkbox chk-on">✓</div>
              <div class="mod-row-title mc-shadow">BETTER FPS (PS3)</div>
            </div>
            <div class="mod-row">
              <div class="mod-checkbox chk-on">✓</div>
              <div class="mod-row-title mc-shadow">MINIMAP MOD v3.0</div>
            </div>
            <div class="mod-row">
              <div class="mod-checkbox chk-off">✗</div>
              <div class="mod-row-title mc-shadow">CUSTOM CAPES</div>
            </div>
          </div>

          <div class="mc-scrollbar">
            <div class="mc-scroll-arrow">▲</div>
            <div class="mc-scroll-track"><div class="mc-scroll-thumb"></div></div>
            <div class="mc-scroll-arrow">▼</div>
          </div>
        </div>

        <!-- RIGHT: MOD DETAILS -->
        <div class="mod-detail-box">
          <div>
            <div class="detail-top">
              <div class="mod-thumb">
                <!-- Steve Flying Avatar SVG -->
                <svg width="74" height="74" viewBox="0 0 80 80">
                  <rect width="80" height="80" fill="#2b4a6f"/>
                  <rect x="28" y="18" width="24" height="24" fill="#d49e77"/>
                  <rect x="28" y="18" width="24" height="8" fill="#452d1a"/>
                  <rect x="34" y="26" width="4" height="4" fill="#224499"/>
                  <rect x="42" y="26" width="4" height="4" fill="#224499"/>
                  <rect x="24" y="42" width="32" height="28" fill="#00aaaa"/>
                </svg>
              </div>
              <div class="detail-heading">
                <div class="detail-title mc-shadow">FLY MOD v2.1</div>
                <div class="detail-author mc-shadow">Author: PS3Modder99</div>
              </div>
            </div>

            <div class="detail-desc mc-shadow">
              Description: Toggle flight in Survival mode. Double-tap jump to fly. Adjust speeds in mod options.
            </div>
          </div>

          <div class="detail-meta mc-shadow">
            <div>Status: <span class="status-enabled">ENABLED</span></div>
            <div>File Size: 240 KB</div>
            <div class="warning-text">Warning: Disables Trophy acquisition.</div>
          </div>
        </div>
      </div>
    </div>

    <!-- BOTTOM PROMPT BAR -->
    <div class="bottom-bar mc-shadow">
      <div class="prompt-item prompt-cross">
        <span class="prompt-btn-icon">✕</span> TOGGLE/SELECT
      </div>
      <div class="prompt-item prompt-circle">
        <span class="prompt-btn-icon">○</span> BACK
      </div>
      <div class="prompt-item prompt-square">
        <span class="prompt-btn-icon">□</span> OPTIONS
      </div>
      <div class="prompt-item prompt-triangle">
        <span class="prompt-btn-icon">△</span> REFRESH
      </div>
      <div class="prompt-item" style="color:#c9c3b8;">
        L3/R3 SCROLL
      </div>
    </div>
  </div>
</body>
</html>
"""

artifact_dir = r"C:\Users\grezu\.gemini\antigravity\brain\aaf3c268-9ba2-4618-9ab6-ef68231aab25"
out_html = os.path.join(artifact_dir, "clean_ui_render.html")
with open(out_html, "w", encoding="utf-8") as f:
    f.write(html_content)

chrome_exe = r"C:\Program Files\Google\Chrome\Application\chrome.exe"
out_png = os.path.join(artifact_dir, "ps3_mc_loader_final.png")

cmd = [
    chrome_exe,
    "--headless=new",
    "--disable-gpu",
    "--no-sandbox",
    "--window-size=1280,720",
    f"--screenshot={out_png}",
    out_html
]

res = subprocess.run(cmd, capture_output=True, text=True)
print("Return code:", res.returncode)
print("Final screenshot updated successfully:", os.path.exists(out_png))
