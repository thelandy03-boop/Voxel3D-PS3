import os
import subprocess

base_html_path = r"C:\Users\grezu\.gemini\antigravity\brain\aaf3c268-9ba2-4618-9ab6-ef68231aab25\clean_ui_render.html"
chrome_exe = r"C:\Program Files\Google\Chrome\Application\chrome.exe"
artifact_dir = r"C:\Users\grezu\.gemini\antigravity\brain\aaf3c268-9ba2-4618-9ab6-ef68231aab25"

with open(base_html_path, "r", encoding="utf-8") as f:
    base_html = f.read()

# Tab 2: Settings
set_html = base_html.replace('tab-btn tab-active mc-shadow">INSTALLED MODS</div>', 'tab-btn tab-inactive mc-shadow">INSTALLED MODS</div>')
set_html = set_html.replace('tab-btn tab-inactive mc-shadow">SETTINGS</div>', 'tab-btn tab-active mc-shadow">SETTINGS</div>')

# Replace panel-frame content with Settings grid
settings_panel_content = """
      <!-- PANEL FRAME: SETTINGS -->
      <div class="panel-frame" style="display:grid; grid-template-columns: 1fr 1fr; gap: 14px;">
        <div style="background:rgba(16,13,10,0.94); border:2px solid #0a0806; padding:12px 14px; display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-size:15px; font-weight:bold; color:#fff;">Autoload Mods on Boot</div>
            <div style="font-size:12px; color:#c9c3b8; margin-top:4px;">Inject active mods when game starts</div>
          </div>
          <div style="padding:6px 14px; background:#256621; color:#5ae84a; font-weight:bold; font-size:13px; border:2px solid #0a0806;">ON</div>
        </div>

        <div style="background:rgba(16,13,10,0.94); border:2px solid #0a0806; padding:12px 14px; display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-size:15px; font-weight:bold; color:#fff;">Skip Game Intros</div>
            <div style="font-size:12px; color:#c9c3b8; margin-top:4px;">Bypass Mojang / 4J Studios logos</div>
          </div>
          <div style="padding:6px 14px; background:#256621; color:#5ae84a; font-weight:bold; font-size:13px; border:2px solid #0a0806;">ON</div>
        </div>

        <div style="background:rgba(16,13,10,0.94); border:2px solid #f4e436; padding:12px 14px; display:flex; justify-content:space-between; align-items:center; box-shadow:0 0 10px rgba(244,228,54,0.25);">
          <div>
            <div style="font-size:15px; font-weight:bold; color:#f4e436;">RSX Performance Mode</div>
            <div style="font-size:12px; color:#c9c3b8; margin-top:4px;">Shader & clock speed profile</div>
          </div>
          <div style="padding:6px 14px; background:#2b241e; color:#f4e436; font-weight:bold; font-size:13px; border:2px solid #0a0806;">BOOST</div>
        </div>

        <div style="background:rgba(16,13,10,0.94); border:2px solid #0a0806; padding:12px 14px; display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-size:15px; font-weight:bold; color:#fff;">Debug HUD Overlay</div>
            <div style="font-size:12px; color:#c9c3b8; margin-top:4px;">Display FPS & temps in-game</div>
          </div>
          <div style="padding:6px 14px; background:#256621; color:#5ae84a; font-weight:bold; font-size:13px; border:2px solid #0a0806;">ON</div>
        </div>

        <div style="background:rgba(16,13,10,0.94); border:2px solid #0a0806; padding:12px 14px; display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-size:15px; font-weight:bold; color:#fff;">Voxel3D Weapon Models</div>
            <div style="font-size:12px; color:#c9c3b8; margin-top:4px;">First-person custom 3D models</div>
          </div>
          <div style="padding:6px 14px; background:#256621; color:#5ae84a; font-weight:bold; font-size:13px; border:2px solid #0a0806;">ON</div>
        </div>

        <div style="background:rgba(16,13,10,0.94); border:2px solid #0a0806; padding:12px 14px; display:flex; justify-content:space-between; align-items:center;">
          <div>
            <div style="font-size:15px; font-weight:bold; color:#fff;">Trophy Warning Bypass</div>
            <div style="font-size:12px; color:#c9c3b8; margin-top:4px;">Suppress trophy acquisition flags</div>
          </div>
          <div style="padding:6px 14px; background:#3d3731; color:#c9c3b8; font-weight:bold; font-size:13px; border:2px solid #0a0806;">OFF</div>
        </div>
      </div>
"""

# Replace between <!-- PANEL FRAME --> and </div>\n    </div>\n\n    <!-- BOTTOM PROMPT BAR -->
start_tag = '<!-- PANEL FRAME -->'
end_tag = '<!-- BOTTOM PROMPT BAR -->'
p1 = set_html.find(start_tag)
p2 = set_html.find(end_tag)

if p1 != -1 and p2 != -1:
    set_html_final = set_html[:p1] + settings_panel_content + "\n    </div>\n\n    " + set_html[p2:]
    out_set_html = os.path.join(artifact_dir, "render_settings.html")
    with open(out_set_html, "w", encoding="utf-8") as f:
        f.write(set_html_final)
    
    out_set_png = os.path.join(artifact_dir, "ps3_mc_loader_settings.png")
    cmd = [
        chrome_exe,
        "--headless=new",
        "--disable-gpu",
        "--no-sandbox",
        "--window-size=1280,720",
        f"--screenshot={out_set_png}",
        out_set_html
    ]
    subprocess.run(cmd, capture_output=True)
    print("Settings capture generated:", os.path.exists(out_set_png))

print("Done tab rendering!")
