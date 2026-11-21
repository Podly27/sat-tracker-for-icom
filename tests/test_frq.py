"""Host tests for native FM text drawing gates; native GPU is not emulated."""
from pathlib import Path
import shutil, subprocess, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'), 'requires host C compiler')
class FrequencyTextTests(unittest.TestCase):
    def test_visibility_and_cleanup(self):
        with tempfile.TemporaryDirectory() as tmp:
            binary=Path(tmp)/'frq'
            subprocess.run(['cc','-std=c11','-I'+str(ROOT/'vendor/sgp4'),str(ROOT/'tests/frq_harness.c'),'-lm','-o',str(binary)],check=True)
            for case in ['success','missing','no-gps','expired','menu','tx','ssb','unsupported','stale']:
                with self.subTest(case=case):subprocess.run([str(binary),case],check=True)
