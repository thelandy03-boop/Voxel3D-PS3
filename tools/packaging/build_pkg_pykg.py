import os, sys, ctypes

dll_path = os.path.abspath('tools/packaging/PyKG/_internal/python310.dll')
py310 = ctypes.CDLL(dll_path)
no_site = ctypes.c_int.in_dll(py310, 'Py_NoSiteFlag')
no_site.value = 1
py310.Py_SetProgramName(ctypes.c_wchar_p(os.path.abspath('tools/packaging/PyKG/PyKG.exe')))
py310.Py_SetPythonHome(ctypes.c_wchar_p(os.path.abspath('tools/packaging/PyKG/_internal')))
py310.Py_Initialize()

code_str = """
import sys, os, marshal, types

meipass = os.path.abspath('tools/packaging/PyKG/_internal')
sys._MEIPASS = meipass
sys.path.insert(0, meipass)

# 0. Load struct
import _struct
with open('tools/packaging/struct.bin', 'rb') as f:
    s_co = marshal.loads(f.read())
s_mod = types.ModuleType('struct')
sys.modules['struct'] = s_mod
exec(s_co, s_mod.__dict__)

# 1. Load pyimod01_archive
with open('tools/packaging/pyimod01_archive.bin', 'rb') as f:
    mod1_co = marshal.loads(f.read())
mod1 = types.ModuleType('pyimod01_archive')
sys.modules['pyimod01_archive'] = mod1
exec(mod1_co, mod1.__dict__)

# 2. Load pyimod02_importers
with open('tools/packaging/pyimod02_importers.bin', 'rb') as f:
    mod2_co = marshal.loads(f.read())
mod2 = types.ModuleType('pyimod02_importers')
sys.modules['pyimod02_importers'] = mod2
exec(mod2_co, mod2.__dict__)

sys._pyinstaller_pyz = os.path.abspath('tools/packaging/PYZ.pyz.bin')
mod2.install()

# 3. Now that PYZ importer is installed, standard libraries like pathlib, queue, etc. are available!
import pathlib

# 4. Mock GUI libraries
class MockClass:
    def __init__(self, *args, **kwargs): pass
    def __getattr__(self, name): return MockClass
    def __call__(self, *args, **kwargs): return MockClass()
    def __mro_entries__(self, bases): return (object,)

class MockMod(types.ModuleType):
    def __getattr__(self, name): return MockClass

for m in ['customtkinter', 'tkinter', 'tkinter.filedialog', 'tkinter.messagebox', 'PIL', 'PIL.Image', 'PIL.ImageTk']:
    sys.modules[m] = MockMod(m)

# 5. Load PyKG
with open('tools/packaging/PyKG.bin', 'rb') as f:
    pykg_co = marshal.loads(f.read())
pykg = types.ModuleType('PyKG')
pykg.__file__ = os.path.abspath('tools/packaging/PyKG/PyKG.py')
sys.modules['PyKG'] = pykg
exec(pykg_co, pykg.__dict__)

print('[*] Motor PyKG inicializado al 100% con claves oficiales de Sony')

# Configure Settings
content_id = 'UP0001-MCPS3MODS_00-0000000000000000'
settings = pykg.PkgSettings(
    content_id=content_id,
    content_type=5, # Game_Exec
    package_flags=14,
    drm_type=3, # Free
    package_revision=4193,
    package_version=0,
    eboot_is_npdrm_self=False
)

src_dir = pathlib.Path('app_ps3/pkgfiles').resolve()
out_pkg = pathlib.Path('Minecraft_PS3_Mod_Loader_v1.4.2.pkg').resolve()

def log(msg):
    print('  [PyKG]', msg)

print(f'[*] Empaquetando {src_dir} -> {out_pkg}...')
try:
    pykg.repack_pkg(
        src_dir=src_dir,
        template_pkg=None,
        out_pkg=out_pkg,
        settings=settings,
        progress_cb=None,
        log_cb=log,
        cancel_flag=None,
        content_id=content_id,
        generated_sfo=None
    )
    print('[+] PAQUETE CREADO EXITOSAMENTE POR EL MOTOR OFICIAL PYKG!')
    import shutil
    shutil.copyfile(str(out_pkg), 'tools/packaging/' + out_pkg.name)
    print(f'[+] Copiado a tools/packaging/{out_pkg.name}')
except Exception as e:
    import traceback
    traceback.print_exc()
"""

res = py310.PyRun_SimpleString(code_str.encode('utf-8'))
py310.Py_Finalize()
if res != 0:
    sys.exit(1)
