import os
import subprocess

html_path = r"C:\Users\grezu\.gemini\antigravity\brain\aaf3c268-9ba2-4618-9ab6-ef68231aab25\mc_loader_preview.html"
chrome_exe = r"C:\Program Files\Google\Chrome\Application\chrome.exe"
artifact_dir = r"C:\Users\grezu\.gemini\antigravity\brain\aaf3c268-9ba2-4618-9ab6-ef68231aab25"

# 1. Take screenshot of Tab 0 (Installed Mods)
out_tab0 = os.path.join(artifact_dir, "ui_installed_mods.png")
cmd0 = [
    chrome_exe,
    "--headless=new",
    "--disable-gpu",
    "--no-sandbox",
    "--window-size=1024,640",
    f"--screenshot={out_tab0}",
    html_path
]
subprocess.run(cmd0, capture_output=True)
print("Tab 0 screenshot created:", os.path.exists(out_tab0), os.path.getsize(out_tab0) if os.path.exists(out_tab0) else 0)

# 2. Modify active tab to Tab 1 (USB Install) and capture
with open(html_path, "r", encoding="utf-8") as f:
    orig_html = f.read()

# Tab 1: USB Install
usb_html = orig_html.replace('id="tab-0" class="tab-active', 'id="tab-0" class="tab-inactive')
usb_html = usb_html.replace('id="tab-1" class="tab-inactive', 'id="tab-1" class="tab-active')
usb_html = usb_html.replace('id="view-tab-0" class="flex', 'id="view-tab-0" class="hidden')
usb_html = usb_html.replace('id="view-tab-1" class="hidden', 'id="view-tab-1" class="flex')

temp_tab1_path = os.path.join(artifact_dir, "temp_tab1.html")
with open(temp_tab1_path, "w", encoding="utf-8") as f:
    f.write(usb_html)

out_tab1 = os.path.join(artifact_dir, "ui_usb_install.png")
cmd1 = [
    chrome_exe,
    "--headless=new",
    "--disable-gpu",
    "--no-sandbox",
    "--window-size=1024,640",
    f"--screenshot={out_tab1}",
    temp_tab1_path
]
subprocess.run(cmd1, capture_output=True)
print("Tab 1 screenshot created:", os.path.exists(out_tab1), os.path.getsize(out_tab1) if os.path.exists(out_tab1) else 0)

# Tab 2: Settings
set_html = orig_html.replace('id="tab-0" class="tab-active', 'id="tab-0" class="tab-inactive')
set_html = set_html.replace('id="tab-2" class="tab-inactive', 'id="tab-2" class="tab-active')
set_html = set_html.replace('id="view-tab-0" class="flex', 'id="view-tab-0" class="hidden')
set_html = set_html.replace('id="view-tab-2" class="hidden', 'id="view-tab-2" class="flex')

temp_tab2_path = os.path.join(artifact_dir, "temp_tab2.html")
with open(temp_tab2_path, "w", encoding="utf-8") as f:
    f.write(set_html)

out_tab2 = os.path.join(artifact_dir, "ui_settings.png")
cmd2 = [
    chrome_exe,
    "--headless=new",
    "--disable-gpu",
    "--no-sandbox",
    "--window-size=1024,640",
    f"--screenshot={out_tab2}",
    temp_tab2_path
]
subprocess.run(cmd2, capture_output=True)
print("Tab 2 screenshot created:", os.path.exists(out_tab2), os.path.getsize(out_tab2) if os.path.exists(out_tab2) else 0)

# Tab 3: System
sys_html = orig_html.replace('id="tab-0" class="tab-active', 'id="tab-0" class="tab-inactive')
sys_html = sys_html.replace('id="tab-3" class="tab-inactive', 'id="tab-3" class="tab-active')
sys_html = sys_html.replace('id="view-tab-0" class="flex', 'id="view-tab-0" class="hidden')
sys_html = sys_html.replace('id="view-tab-3" class="hidden', 'id="view-tab-3" class="flex')

temp_tab3_path = os.path.join(artifact_dir, "temp_tab3.html")
with open(temp_tab3_path, "w", encoding="utf-8") as f:
    f.write(sys_html)

out_tab3 = os.path.join(artifact_dir, "ui_system.png")
cmd3 = [
    chrome_exe,
    "--headless=new",
    "--disable-gpu",
    "--no-sandbox",
    "--window-size=1024,640",
    f"--screenshot={out_tab3}",
    temp_tab3_path
]
subprocess.run(cmd3, capture_output=True)
print("Tab 3 screenshot created:", os.path.exists(out_tab3), os.path.getsize(out_tab3) if os.path.exists(out_tab3) else 0)

# Clean up temporary htmls
for tp in [temp_tab1_path, temp_tab2_path, temp_tab3_path]:
    if os.path.exists(tp):
        os.remove(tp)
print("Screenshots successfully captured.")
