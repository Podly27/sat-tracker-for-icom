"""Six-digit OMM input and generated near-earth SGP4 versus Python SGP4."""
from pathlib import Path
import csv,ctypes as C,io,math,shutil,subprocess,sys,tempfile,unittest
from test_orbit import Look
ROOT=Path(__file__).resolve().parents[1]
try:
 from sgp4.api import Satrec
 from sgp4 import omm
except ImportError:Satrec=None
@unittest.skipUnless(Satrec and shutil.which('cc'),'requires Python sgp4 and host compiler')
class OmmTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=tempfile.TemporaryDirectory();d=Path(cls.tmp.name);leo=d/'leo.c';text=(ROOT/'vendor/sgp4/SGP4.c').read_text();text=text.replace("if ((2 * pi / satrec->no_unkozai) >= 225.0)","if (0)").replace("if (satrec->method == 'd')","if (0)");leo.write_text(text)
  lib=d/'omm.so';subprocess.run(['cc','-std=c11','-O1','-fPIC','-dynamiclib'if sys.platform=='darwin'else'-shared','-I'+str(ROOT/'src'),'-I'+str(ROOT/'vendor/sgp4'),str(ROOT/'tests/catalog_bindings.c'),str(ROOT/'src/profile.c'),str(ROOT/'src/orbit.c'),str(leo),'-lm','-o',str(lib)],check=True)
  cls.lib=C.CDLL(str(lib));cls.lib.test_csv_orbit.argtypes=[C.c_void_p,C.c_char_p,C.c_uint32];cls.lib.orbit_look_profile.argtypes=[C.c_void_p,C.c_uint32,C.c_double,C.c_double,C.c_double,C.c_uint32,C.c_uint32,C.POINTER(Look)];cls.lib.orbit_init.argtypes=[C.c_void_p,C.c_char_p,C.c_char_p];cls.lib.orbit_init_id.argtypes=[C.c_void_p,C.c_char_p,C.c_char_p,C.c_char_p]
  cls.csv=(ROOT/'tests/fixtures/so131-historical.csv').read_text().splitlines()[1];cls.fields=next(csv.DictReader(io.StringIO((ROOT/'tests/fixtures/so131-historical.csv').read_text())))
 @classmethod
 def tearDownClass(cls):cls.tmp.cleanup()
 def compare(self,orbit,ref,down,up):
  epoch=(ref.jdsatepoch+ref.jdsatepochF-2440587.5)*86400
  for lat,lon in [(0,0),(50,14),(-34,151),(89,179)]:
   for offset in range(-86400,86401,3600):
    t=int(epoch)+offset;jd=2440587.5+t/86400;whole=math.floor(jd);err,r,v=ref.sgp4(whole,jd-whole);self.assertEqual(err,0)
    T=(jd-2451545)/36525;gm=((67310.54841+(876600*3600+8640184.812866)*T+.093104*T*T-6.2e-6*T*T*T)*math.pi/43200)%(2*math.pi);c,s=math.cos(gm),math.sin(gm);x=c*r[0]+s*r[1];y=-s*r[0]+c*r[1];z=r[2];vx=c*v[0]+s*v[1]+7.29211514670698e-5*y;vy=-s*v[0]+c*v[1]-7.29211514670698e-5*x
    sl,cl=math.sin(math.radians(lat)),math.cos(math.radians(lat));so,co=math.sin(math.radians(lon)),math.cos(math.radians(lon));N=6378.137/math.sqrt(1-.00669437999014*sl*sl);dx=x-N*cl*co;dy=y-N*cl*so;dz=z-N*(1-.00669437999014)*sl;distance=math.sqrt(dx*dx+dy*dy+dz*dz);rate=(dx*vx+dy*vy+dz*v[2])/distance
    q=Look();self.assertEqual(self.lib.orbit_look_profile(orbit,t,lat,lon,0,down,up,C.byref(q)),0);self.assertLess(abs(q.range-distance),.001);self.assertLess(abs(q.rate-rate),1e-5);self.assertLessEqual(abs(q.down-int(down*(1-rate/299792.458)+.5)),1);self.assertLessEqual(abs(q.up-int(up*(1+rate/299792.458)+.5)),1)
 def test_so131_csv_orbit(self):
  o=C.create_string_buffer(8192);self.assertEqual(self.lib.test_csv_orbit(o,self.csv.encode(),101101),0);ref=Satrec();omm.initialize(ref,self.fields);self.compare(o,ref,436888000,145925000)
 def test_iss_near_earth_build(self):
  a,b=[r for r in(ROOT/'tests/fixtures/iss-historical.tle').read_text().splitlines()if r.startswith(('1 ','2 '))];o=C.create_string_buffer(8192);self.assertEqual(self.lib.orbit_init(o,a.encode(),b.encode()),0);self.compare(o,Satrec.twoline2rv(a,b),437800000,145990000)
 def test_other_satellite_orbits(self):
  rows=(ROOT/'tests/fixtures/multisat-historical.tle').read_text().splitlines();profiles={'27607':(436795000,145850000),'43017':(145960000,435250000),'61781':(435400000,145850000)}
  for i,a in enumerate(rows):
   if a.startswith('1 ')and a[2:7]in profiles:
    b=rows[i+1];o=C.create_string_buffer(8192);self.assertEqual(self.lib.orbit_init_id(o,a.encode(),b.encode(),a[2:7].encode()),0);self.compare(o,Satrec.twoline2rv(a,b),*profiles[a[2:7]])
 def test_invalid_omm(self):
  original=self.csv.split(',')
  for field,value in [(11,'101102'),(3,'1'),(4,'nan'),(5,'181'),(2,'2026-02-31T00:00:00.000000'),(2,'2026-11-20T25:00:00.000000'),(14,'1e999')]:
   row=original.copy();row[field]=value;self.assertLess(self.lib.test_csv_orbit(C.create_string_buffer(8192),','.join(row).encode(),101101),0)
