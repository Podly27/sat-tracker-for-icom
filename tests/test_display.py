"""Host-only native text-dialog probe flow; no native drawing is emulated."""
from pathlib import Path
import shutil, subprocess, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'), 'requires host C compiler')
class DisplayTests(unittest.TestCase):
    def test_dialog_flow(self):
        with tempfile.TemporaryDirectory() as tmp:
            binary=Path(tmp)/'display'
            subprocess.run(['cc','-std=c11','-I'+str(ROOT/'vendor/sgp4'),str(ROOT/'tests/display_harness.c'),'-lm','-o',str(binary)],check=True)
            for case in ['success','missing','no-gps','expired','busy']:
                with self.subTest(case=case):subprocess.run([str(binary),case],check=True)
