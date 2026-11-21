#!/usr/bin/env python3
"""Fetch real configured satellite orbital elements and atomically save SAT.TLE.
Never modifies the radio, keys PTT or fabricates orbital elements.
"""
import csv,io,argparse,calendar,datetime as dt,math,os,re,sys,tempfile,urllib.request
from pathlib import Path
SOURCES=(('CelesTrak','https://celestrak.org/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE'),('AMSAT','https://www.amsat.org/tle/current/nasabare.txt'))
UTC=dt.timezone.utc
class TLEError(ValueError):pass
def checksum(line):return len(line)==69 and line[-1].isdigit() and sum(int(c)if c.isdigit()else c=='-'for c in line[:68])%10==int(line[-1])
def epoch(line):
 try:
  yy=int(line[18:20]);year=2000+yy if yy<57 else 1900+yy;day=float(line[20:32])
  days=366 if calendar.isleap(year)else 365
  if not math.isfinite(day)or not 1<=day<days+1:raise ValueError('day out of range')
  return dt.datetime(year,1,1,tzinfo=UTC)+dt.timedelta(days=day-1)
 except(ValueError,OverflowError)as e:raise TLEError('Invalid TLE epoch')from e
def extract(text,now=None,max_age_hours=168,norad=25544):
 identifier=f"{norad:05d}" if norad<100000 else "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ"[norad//10000]+f"{norad%10000:04d}"
 now=now or dt.datetime.now(UTC);rows=text.splitlines();found=[]
 for i,line in enumerate(rows):
  if line.startswith('1 '+identifier):
   if i+1>=len(rows):raise TLEError('Missing ISS line 2')
   other=rows[i+1]
   if not other.startswith('2 '+identifier):raise TLEError('Mismatched ISS line 2')
   if not checksum(line)or not checksum(other):raise TLEError('Invalid length/checksum; each orbital line must be69 columns')
   when=epoch(line);age=(now-when).total_seconds()/3600
   if abs(age)>max_age_hours:raise TLEError(f'TLE epoch is outside the allowed age window ({age:.1f} hours)')
   try:
    inc=float(other[8:16]);ecc=float('0.'+other[26:33]);motion=float(other[52:63])
    if not all(map(math.isfinite,(inc,ecc,motion)))or not(0<=inc<=180 and 0<=ecc<1 and 1440/110<=motion<=1440/80):raise ValueError('unsupported orbit')
   except ValueError as e:raise TLEError('Invalid or unsupported orbital fields')from e
   found.append((('ISS (ZARYA)' if norad==25544 else f'SAT {norad}')+'\n'+line+'\n'+other+'\n',when,age))
 if len(found)!=1:raise TLEError(f'Expected exactly one NORAD {norad} element pair')
 return found[0]
def save_atomic(path,text):
 path=Path(path);path.parent.mkdir(parents=True,exist_ok=True);temp=None
 try:
  with tempfile.NamedTemporaryFile('w',encoding='ascii',newline='\n',dir=path.parent,prefix='.SAT-TLE-',suffix='.tmp',delete=False)as f:
   temp=Path(f.name);f.write(text);f.flush();os.fsync(f.fileno())
  os.replace(temp,path)
 finally:
  if temp and temp.exists():temp.unlink()
def configuration(path):
 rows=[]
 for line in Path(path).read_text(encoding='ascii').splitlines():
  line=line.split('#',1)[0].strip()
  if not line or line.startswith(';'):continue
  values=[v.strip() for v in next(csv.reader([line]))]
  if len(values)!=5 or not re.fullmatch(r'[A-Z0-9-]{1,6}',values[0]):raise TLEError('Invalid SAT.CFG row')
  name,number,rx,tx,tone=values;number=int(number);rx=float(rx);tx=float(tx);tone=float(tone)
  band=lambda f:1 if 144<=f<=148 else 2 if 430<=f<=440 else 0
  tones={0,670,693,719,744,770,797,825,854,885,915,948,974,1000,1035,1072,1109,1148,1188,1230,1273,1318,1365,1413,1462,1514,1567,1598,1622,1655,1679,1713,1738,1773,1799,1835,1862,1899,1928,1966,1995,2035,2065,2107,2181,2257,2291,2336,2418,2503,2541}
  if not math.isfinite(tone) or round(tone*10)not in tones or abs(tone*10-round(tone*10))>.001:raise TLEError('Unsupported CTCSS tone')
  if not 1<=number<=339999 or not band(rx) or not band(tx) or band(rx)==band(tx) or not math.isfinite(tone) or tone<0 or tone>254.1:raise TLEError('Invalid satellite profile')
  if any(name==r[0] or number==r[1] for r in rows):raise TLEError('Duplicate profile')
  rows.append((name,number))
 if not 1<=len(rows)<=8:raise TLEError('Configure 1-8 satellites')
 return rows

def extract_csv(text,norad,now=None,max_age_hours=168):
 now=now or dt.datetime.now(UTC)
 if text.startswith('CSV,'):
  records=[line[4:] for line in text.splitlines() if line.startswith('CSV,')]
  header='OBJECT_NAME,OBJECT_ID,EPOCH,MEAN_MOTION,ECCENTRICITY,INCLINATION,RA_OF_ASC_NODE,ARG_OF_PERICENTER,MEAN_ANOMALY,EPHEMERIS_TYPE,CLASSIFICATION_TYPE,NORAD_CAT_ID,ELEMENT_SET_NO,REV_AT_EPOCH,BSTAR,MEAN_MOTION_DOT,MEAN_MOTION_DDOT'
  text=header+'\n'+'\n'.join(records)
 rows=list(csv.reader(io.StringIO(text)))
 if not rows or rows[0][:3]!=['OBJECT_NAME','OBJECT_ID','EPOCH']:raise TLEError('Invalid OMM CSV header')
 matches=[r for r in rows[1:]if len(r)==17 and r[11]==str(norad)]
 if len(matches)!=1:raise TLEError(f'Expected one OMM record for {norad}')
 row=matches[0]
 if not re.fullmatch(r'20[2-9][0-9]-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(?:\.[0-9]{1,7})?',row[2]):raise TLEError('Invalid UTC epoch')
 try:when=dt.datetime.fromisoformat(row[2]).replace(tzinfo=UTC)
 except ValueError as e:raise TLEError('Invalid UTC epoch')from e
 age=(now-when).total_seconds()/3600
 mm,ecc,inc,raan,arg,mean,bstar=[float(row[i])for i in (3,4,5,6,7,8,14)]
 if not all(map(math.isfinite,(mm,ecc,inc,raan,arg,mean,bstar)))or not (1440/110<=mm<=1440/80 and 0<=ecc<1 and 0<=inc<=180 and 0<=raan<360 and 0<=arg<360 and 0<=mean<360 and abs(bstar)<=1):raise TLEError('Unsupported OMM orbit')
 if abs(age)>max_age_hours:raise TLEError('OMM epoch outside age limit')
 out=io.StringIO();csv.writer(out,lineterminator='').writerow(row);record=out.getvalue()
 if len(record)+4>=320:raise TLEError('OMM record too long')
 return 'CSV,'+record+'\n',when,age

def main():
 base=Path(__file__).resolve().parents[1]/'sd-card/IC-705/Apps'
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--config',type=Path,default=base/'SAT.CFG');parser.add_argument('--input',type=Path,help='Existing combined SAT.TLE cache or TLE list; must contain all configured objects');parser.add_argument('--output',type=Path,default=base/'SAT.TLE');parser.add_argument('--max-age-hours',type=float,default=168);args=parser.parse_args()
 if not math.isfinite(args.max_age_hours)or args.max_age_hours<=0:parser.error('--max-age-hours must be positive and finite')
 try:
  profiles=configuration(args.config);local=args.input.read_text(encoding='ascii')if args.input else None;bundle=[];downloaded={}
  def fetch(url):
   if url not in downloaded:
    req=urllib.request.Request(url,headers={'User-Agent':'SAT-for-IC-705/0.23.0'})
    with urllib.request.urlopen(req,timeout=20)as response:raw=response.read(2_000_001)
    if len(raw)>2_000_000:raise TLEError('Response too large')
    downloaded[url]=raw.decode('ascii')
   return downloaded[url]
  for name,norad in profiles:
   if norad>=100000:
    raw=local if local is not None else fetch(f'https://celestrak.org/NORAD/elements/gp.php?CATNR={norad}&FORMAT=CSV')
    # A combined cache has ordinary TLEs followed by CSV, records.
    if local is not None:raw='\n'.join(r for r in raw.splitlines()if r.startswith('CSV,'))
    text,when,age=extract_csv(raw,norad,max_age_hours=args.max_age_hours)
   else:
    urls=[f'https://celestrak.org/NORAD/elements/gp.php?CATNR={norad}&FORMAT=TLE','https://www.amsat.org/tle/current/nasabare.txt']
    errors=[];result=None
    for url in ([None]if local is not None else urls):
     try:result=extract(local if local is not None else fetch(url),max_age_hours=args.max_age_hours,norad=norad);break
     except (OSError,ValueError,UnicodeError)as e:errors.append(str(e))
    if result is None:raise TLEError(f'{name}: '+ '; '.join(errors))
    text,when,age=result;text='\n'.join(text.splitlines()[1:])+'\n'
   bundle.append(name+'\n'+text);print(f'{name}: {when.isoformat()}, epoch age {age:.1f} h')
  save_atomic(args.output,''.join(bundle));print(f'Saved {len(profiles)} objects to {args.output}')
 except(OSError,ValueError,UnicodeError)as e:print(f'Orbit update failed; existing file was not replaced: {e}',file=sys.stderr);return 1
 return 0
if __name__=='__main__':raise SystemExit(main())
