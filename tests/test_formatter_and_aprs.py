from pathlib import Path
import shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'),'requires host C compiler')
class NativeHelpersTests(unittest.TestCase):
 def test_actual_tiny_formatter(self):
  with tempfile.TemporaryDirectory()as t:
   binary=Path(t)/'fmt'
   subprocess.run(['cc','-std=c11','-fno-builtin','-Dsnprintf=tiny_snprintf',str(ROOT/'src/tiny.c'),str(ROOT/'tests/formatter_harness.c'),'-o',str(binary)],check=True)
   subprocess.run([str(binary)],check=True)
 def test_aprs_display_cache_ownership(self):
  with tempfile.TemporaryDirectory()as t:
   binary=Path(t)/'cache'
   subprocess.run(['cc','-std=c11','-DISS_HOST_TEST','-I'+str(ROOT/'src'),str(ROOT/'src/aprs_status.c'),str(ROOT/'tests/aprs_status_harness.c'),'-o',str(binary)],check=True)
   subprocess.run([str(binary)],check=True)
