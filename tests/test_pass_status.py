"""UTC rollover and remaining-minute semantics, using the actual tiny formatter."""
from pathlib import Path
import ctypes as C,datetime,shutil,subprocess,sys,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
@unittest.skipUnless(shutil.which('cc'),'requires host C compiler')
class PassStatusTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=tempfile.TemporaryDirectory();binary=Path(cls.tmp.name)/'status.so'
  subprocess.run(['cc','-std=c11','-fno-builtin','-Dsnprintf=iss_test_snprintf','-dynamiclib'if sys.platform=='darwin'else'-shared','-fPIC',str(ROOT/'src/pass_status.c'),str(ROOT/'src/tiny.c'),'-o',str(binary)],check=True)
  cls.lib=C.CDLL(str(binary));cls.lib.pass_status_format.argtypes=[C.c_void_p,C.c_size_t,C.c_char_p,C.c_uint32,C.c_uint32,C.c_uint32,C.c_int,C.c_int]
 @classmethod
 def tearDownClass(cls):cls.tmp.cleanup()
 def status(self,now,aos=0,los=0,visible=0,complete=0,name=b'ISS'):
  b=C.create_string_buffer(32);self.lib.pass_status_format(b,32,name,now,aos,los,visible,complete);return b.value.decode()
 def utc(self,s):return int(datetime.datetime.fromisoformat(s).replace(tzinfo=datetime.timezone.utc).timestamp())
 def test_waiting_date(self):
  self.assertEqual(self.status(self.utc('2026-11-21T07:00'),self.utc('2026-11-21T12:48')),'Waiting for ISS 21.11. 12:48')
 def test_next_year_and_leap_day(self):
  for now,aos,want in [('2026-12-31T23:59','2027-01-01T00:05','Waiting for ISS 01.01. 00:05'),('2028-02-28T23:59','2028-02-29T02:03','Waiting for ISS 29.02. 02:03'),('2029-02-28T23:59','2029-03-01T02:03','Waiting for ISS 01.03. 02:03')]:
   self.assertEqual(self.status(self.utc(now),self.utc(aos)),want)
 def test_minutes_round_up(self):
  for remaining,want in [(1,1),(59,1),(60,1),(61,2),(420,7)]:self.assertEqual(self.status(1000,los=1000+remaining,visible=1),f'ISS pass: {want} min left')
 def test_end_search_and_absence(self):
  self.assertEqual(self.status(1000,los=1000,visible=1),'ISS pass ending')
  self.assertEqual(self.status(1000,visible=1),'ISS pass / finding end')
  self.assertEqual(self.status(1000),'Finding next ISS pass')
  self.assertEqual(self.status(1000,complete=1),'No ISS pass in next 24 hours')
 def test_past_aos_and_name_parameter(self):
  self.assertEqual(self.status(1000,aos=900),'Finding next ISS pass')
  self.assertEqual(self.status(1000,los=1060,visible=1,name=b'SO-50'),'SO-50 pass: 1 min left')

 def test_fixed_native_pages(self):
  fn=self.lib.pass_status_pages;fn.argtypes=[C.c_void_p,C.c_char_p,C.c_uint32,C.c_uint32,C.c_uint32,C.c_int,C.c_int];fn.restype=C.c_uint
  def pages(name,now,aos=0,los=0,visible=0,complete=1):
   out=C.create_string_buffer(28);count=fn(out,name,now,aos,los,visible,complete);result=[out.raw[i*14:(i+1)*14].split(b'\0')[0].decode()for i in range(count)];self.assertTrue(all(len(s)<=13 for s in result));return result
  now=self.utc('2026-11-21T12:00');aos=self.utc('2026-11-21T15:20')
  self.assertEqual(pages(b'SO-131',now,aos),['WAIT SO-131','21.11 15:20Z'])
  self.assertEqual(pages(b'SO-131',now,los=now+61,visible=1),['SO-131 PASS','LEFT 2 MIN'])
  self.assertEqual(pages(b'AO-123',now,visible=1),['AO-123 PASS','FINDING LOS'])
  self.assertEqual(pages(b'ISS',now,los=now,visible=1),['ISS PASS','PASS ENDING'])
  self.assertEqual(pages(b'ISS',now),['NO PASS 24H'])
  self.assertEqual(pages(b'ISS',now,complete=0),['FIND PASSES'])
