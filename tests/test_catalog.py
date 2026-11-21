from pathlib import Path
import shutil,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'),'requires host C compiler')
class CatalogTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=tempfile.TemporaryDirectory();cls.binary=Path(cls.tmp.name)/'catalog'
  subprocess.run(['cc','-O1','-std=c11','-I'+str(ROOT/'src'),'-I'+str(ROOT/'vendor/sgp4'),str(ROOT/'tests/catalog_harness.c'),str(ROOT/'src/profile.c'),str(ROOT/'src/orbit.c'),str(ROOT/'vendor/sgp4/SGP4.c'),'-lm','-o',str(cls.binary)],check=True)
 @classmethod
 def tearDownClass(cls):cls.tmp.cleanup()
 def test_config_and_priority_cache(self):
  for case in ['config','fresh','stale','offline','partial','write-fail','readback-fail','cancel-download','cancel-save']:
   with self.subTest(case=case):subprocess.run([str(self.binary),case,str(ROOT/'sd-card/IC-705/Apps/SAT.CFG'),str(ROOT/'tests/fixtures/multisat-historical.tle')],check=True)
