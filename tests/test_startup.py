"""Startup/STOP ordering without firmware or radio hardware."""
import pathlib, shutil, subprocess, tempfile, unittest
ROOT = pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'), 'requires host C compiler')
class StartupTests(unittest.TestCase):
    def test_startup_paths(self):
        with tempfile.TemporaryDirectory() as tmp:
            binary = pathlib.Path(tmp) / 'startup'
            subprocess.run(['cc', '-std=c11', '-I'+str(ROOT/'vendor/sgp4'), str(ROOT/'tests/startup_harness.c'), '-lm', '-o', str(binary)], check=True)
            for case in ['fresh', 'old', 'cancel', 'missing', 'no-fix', 'expired']:
                with self.subTest(case=case):
                    subprocess.run([str(binary), case], check=True)
