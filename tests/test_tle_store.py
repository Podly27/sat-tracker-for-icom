"""Startup refresh and recoverable two-slot SD cache, using mock ICAP I/O."""
from pathlib import Path
import shutil, subprocess, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'), 'requires host C compiler')
class TleStoreTests(unittest.TestCase):
    def test_refresh_and_recovery(self):
        with tempfile.TemporaryDirectory() as tmp:
            binary=Path(tmp)/'store'
            subprocess.run(['cc','-std=c11','-I'+str(ROOT/'src'),'-I'+str(ROOT/'vendor/sgp4'),str(ROOT/'tests/tle_store_harness.c'),str(ROOT/'src/tle_store.c'),'-lm','-o',str(binary)],check=True)
            for case in ['fresh','stale','missing','backup','closest','offline','expired-offline','fetch-fail','bad-download','old-download','write-fail','readback-fail','stop-download','stop-save']:
                with self.subTest(case=case):subprocess.run([str(binary),case],check=True)
