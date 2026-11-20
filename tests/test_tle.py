import datetime as dt,importlib.util,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('tle',ROOT/'tools/update_tle.py');tle=importlib.util.module_from_spec(spec);spec.loader.exec_module(tle)
class TLETests(unittest.TestCase):
 def setUp(self):
  self.text=(ROOT/'tests/fixtures/iss-historical.tle').read_text();self.rows=self.text.splitlines();self.a=next(s for s in self.rows if s.startswith('1 '));self.b=next(s for s in self.rows if s.startswith('2 '));self.now=tle.epoch(self.a)+dt.timedelta(hours=12)
 def test_valid_and_crlf(self):
  text,when,age=tle.extract(self.text.replace('\n','\r\n'),self.now);self.assertTrue(text.startswith('ISS (ZARYA)\n1 25544'));self.assertEqual(age,12);self.assertEqual(len(text.splitlines()[1]),69)
 def test_bad_checksum(self):
  bad=self.a[:-1]+str((int(self.a[-1])+1)%10)
  with self.assertRaises(tle.TLEError):tle.extract(bad+'\n'+self.b,self.now)
 def test_mismatched_line(self):
  with self.assertRaises(tle.TLEError):tle.extract(self.a+'\n'+self.b.replace('25544','27607'),self.now)
 def test_truncated_and_html(self):
  for value in [self.a,'<html>error</html>',self.a[:40]+'\n'+self.b]:
   with self.subTest(value=value),self.assertRaises(tle.TLEError):tle.extract(value,self.now)
 def test_duplicate(self):
  with self.assertRaises(tle.TLEError):tle.extract(self.text+'\n'+self.text,self.now)
 def test_age_past_future_and_strict(self):
  for now,maxage in [(self.now+dt.timedelta(days=8),168),(self.now-dt.timedelta(days=8),168),(self.now,6)]:
   with self.subTest(now=now),self.assertRaises(tle.TLEError):tle.extract(self.text,now,maxage)
 def test_multi_list_extract(self):
  text,_,_=tle.extract('OTHER SAT\n1 27607 unrelated\n2 27607 unrelated\n'+self.text,self.now);self.assertIn(self.a,text)
 def test_epoch_leap_and_invalid(self):
  def fixed(year,day):return self.a[:18]+f'{year:02d}'+f'{day:012.8f}'+self.a[32:]
  self.assertEqual(tle.epoch(fixed(24,366)).date(),dt.date(2024,12,31))
  for row in [fixed(25,366),fixed(24,367),fixed(26,0)]:
   with self.assertRaises(tle.TLEError):tle.epoch(row)
 def test_atomic_file(self):
  with tempfile.TemporaryDirectory()as d:
   p=Path(d)/'ISS.TLE';p.write_text('old');valid,_,_=tle.extract(self.text,self.now);tle.save_atomic(p,valid);self.assertEqual(p.read_text(),valid);self.assertEqual([x.name for x in Path(d).iterdir()],['ISS.TLE'])
 def test_failed_validation_keeps_existing(self):
  with tempfile.TemporaryDirectory()as d:
   p=Path(d)/'ISS.TLE';p.write_text('keep me')
   try:
    text,_,_=tle.extract('<html>unavailable</html>',self.now);tle.save_atomic(p,text)
   except tle.TLEError:pass
   self.assertEqual(p.read_text(),'keep me')
if __name__=='__main__':unittest.main()
