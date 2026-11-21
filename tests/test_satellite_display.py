"""Actual display-cache publication, scrolling, TX/mode gating and paced STOP."""
from pathlib import Path
import shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'),'requires host C compiler')
class SatelliteDisplayTests(unittest.TestCase):
 def test_publication_and_cleanup(self):
  with tempfile.TemporaryDirectory() as temp:
   binary=Path(temp)/'display'
   subprocess.run(['cc','-std=c11','-DISS_HOST_TEST','-I'+str(ROOT/'src'),str(ROOT/'tests/satellite_display_harness.c'),str(ROOT/'src/satellite_display.c'),str(ROOT/'src/aprs_status.c'),'-o',str(binary)],check=True)
   subprocess.run([str(binary)],check=True)
