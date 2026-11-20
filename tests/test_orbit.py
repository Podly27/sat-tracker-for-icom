"""Host C implementation versus independent Python SGP4 propagation."""
import ctypes as C,datetime,math,os,shutil,subprocess,sys,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
try:from sgp4.api import Satrec,WGS72
except ImportError:Satrec=None
class Look(C.Structure):_fields_=[('az',C.c_double),('el',C.c_double),('range',C.c_double),('rate',C.c_double),('down',C.c_int32),('up',C.c_int32)]
@unittest.skipUnless(Satrec and shutil.which('cc'),'requires optional Python sgp4 and a host C compiler')
class OrbitTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=tempfile.TemporaryDirectory();path=Path(cls.tmp.name)/('orbit.dylib'if sys.platform=='darwin'else'orbit.so');flag='-dynamiclib'if sys.platform=='darwin'else'-shared'
  subprocess.run(['cc','-O2',flag,'-fPIC','-I'+str(ROOT/'vendor/sgp4'),str(ROOT/'src/orbit.c'),str(ROOT/'vendor/sgp4/SGP4.c'),'-lm','-o',str(path)],check=True)
  cls.lib=C.CDLL(str(path));cls.lib.orbit_init.argtypes=[C.c_void_p,C.c_char_p,C.c_char_p];cls.lib.orbit_look.argtypes=[C.c_void_p,C.c_uint32,C.c_double,C.c_double,C.c_double,C.POINTER(Look)]
  cls.a,cls.b=[x for x in(ROOT/'tests/fixtures/iss-historical.tle').read_text().splitlines()if x.startswith(('1 ','2 '))];cls.ref=Satrec.twoline2rv(cls.a,cls.b,WGS72)
 @classmethod
 def tearDownClass(cls):cls.tmp.cleanup()
 def test_sgp4_range_and_doppler(self):
  orbit=C.create_string_buffer(8192);self.assertEqual(self.lib.orbit_init(orbit,self.a.encode(),self.b.encode()),0)
  epoch=(self.ref.jdsatepoch+self.ref.jdsatepochF-2440587.5)*86400
  for latitude,longitude in [(0,0),(50,14),(-34,151),(89,179)]:
   for offset in range(-86400,86401,3600):
    t=int(epoch)+offset;jd=2440587.5+t/86400;whole=math.floor(jd);err,r,v=self.ref.sgp4(whole,jd-whole);self.assertEqual(err,0)
    T=(jd-2451545)/36525;gm=((67310.54841+(876600*3600+8640184.812866)*T+.093104*T*T-6.2e-6*T*T*T)*math.pi/43200)%(2*math.pi);c,s=math.cos(gm),math.sin(gm);x=c*r[0]+s*r[1];y=-s*r[0]+c*r[1];z=r[2]
    vx=c*v[0]+s*v[1]+7.29211514670698e-5*y;vy=-s*v[0]+c*v[1]-7.29211514670698e-5*x;sl,cl=math.sin(math.radians(latitude)),math.cos(math.radians(latitude));so,co=math.sin(math.radians(longitude)),math.cos(math.radians(longitude));N=6378.137/math.sqrt(1-.00669437999014*sl*sl);dx=x-N*cl*co;dy=y-N*cl*so;dz=z-N*(1-.00669437999014)*sl;distance=math.sqrt(dx*dx+dy*dy+dz*dz);rate=(dx*vx+dy*vy+dz*v[2])/distance
    q=Look();self.assertEqual(self.lib.orbit_look(orbit,t,latitude,longitude,0,C.byref(q)),0);self.assertLess(abs(q.range-distance),.001);self.assertLess(abs(q.rate-rate),1e-5);self.assertLessEqual(abs(q.down-int(437800000*(1-rate/299792.458)+.5)),1);self.assertLessEqual(abs(q.up-int(145990000*(1+rate/299792.458)+.5)),1)
 def test_bad_checksum_rejected(self):
  bad=self.a[:-1]+str((int(self.a[-1])+1)%10);self.assertLess(self.lib.orbit_init(C.create_string_buffer(8192),bad.encode(),self.b.encode()),0)
if __name__=='__main__':unittest.main()
