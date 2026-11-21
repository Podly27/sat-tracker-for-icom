"""A foreign CI-V busy response is retried without clearing its state."""
from pathlib import Path
import shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'),'requires host C compiler')
class RadioSessionTests(unittest.TestCase):
 def test_bounded_startup_busy(self):
  with tempfile.TemporaryDirectory()as directory:
   path=Path(directory)/'session';subprocess.run(['cc','-std=c11','-I'+str(ROOT/'src'),str(ROOT/'tests/radio_session_harness.c'),'-o',str(path)],check=True)
   for case in ['clear','read-busy','permanent','cancel','unsupported','restore-busy']:
    with self.subTest(case=case):subprocess.run([str(path),case],check=True)
