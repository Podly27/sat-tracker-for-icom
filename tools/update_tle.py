#!/usr/bin/env python3
"""Fetch or extract real ISS orbital elements and atomically save SAT.TLE.
Never modifies the radio, keys PTT or fabricates orbital elements.
"""
import argparse,calendar,datetime as dt,math,os,re,sys,tempfile,urllib.request
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
def extract(text,now=None,max_age_hours=168):
 now=now or dt.datetime.now(UTC);rows=text.splitlines();found=[]
 for i,line in enumerate(rows):
  if line.startswith('1 25544'):
   if i+1>=len(rows):raise TLEError('Missing ISS line 2')
   other=rows[i+1]
   if not other.startswith('2 25544'):raise TLEError('Mismatched ISS line 2')
   if not checksum(line)or not checksum(other):raise TLEError('Invalid length/checksum; each orbital line must be69 columns')
   when=epoch(line);age=(now-when).total_seconds()/3600
   if abs(age)>max_age_hours:raise TLEError(f'TLE epoch is outside the allowed age window ({age:.1f} hours)')
   try:
    inc=float(other[8:16]);ecc=float('0.'+other[26:33]);motion=float(other[52:63])
    if not all(map(math.isfinite,(inc,ecc,motion)))or not(0<=inc<=180 and 0<=ecc<1 and 1440/110<=motion<=1440/80):raise ValueError('unsupported orbit')
   except ValueError as e:raise TLEError('Invalid or unsupported orbital fields')from e
   found.append(('ISS (ZARYA)\n'+line+'\n'+other+'\n',when,age))
 if len(found)!=1:raise TLEError('Expected exactly one ISS (NORAD25544) element pair')
 return found[0]
def save_atomic(path,text):
 path=Path(path);path.parent.mkdir(parents=True,exist_ok=True);temp=None
 try:
  with tempfile.NamedTemporaryFile('w',encoding='ascii',newline='\n',dir=path.parent,prefix='.SAT-TLE-',suffix='.tmp',delete=False)as f:
   temp=Path(f.name);f.write(text);f.flush();os.fsync(f.fileno())
  os.replace(temp,path)
 finally:
  if temp and temp.exists():temp.unlink()
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--input',type=Path,help='Extract ISS from an already downloaded text TLE list instead of using the network');parser.add_argument('--output',type=Path,default=Path(__file__).resolve().parents[1]/'sd-card/IC-705/Apps/SAT.TLE');parser.add_argument('--max-age-hours',type=float,default=168);args=parser.parse_args()
 if not math.isfinite(args.max_age_hours)or args.max_age_hours<=0:parser.error('--max-age-hours must be positive and finite')
 try:
  if args.input:result=extract(args.input.read_text(encoding='ascii'),max_age_hours=args.max_age_hours);source=str(args.input)
  else:
   result=None;errors=[]
   for name,url in SOURCES:
    try:
     req=urllib.request.Request(url,headers={'User-Agent':'SAT-for-IC-705/0.21.0 (manual TLE update)'})
     with urllib.request.urlopen(req,timeout=20)as f:raw=f.read(2_000_001)
     if len(raw)>2_000_000:raise TLEError('TLE response too large')
     result=extract(raw.decode('ascii'),max_age_hours=args.max_age_hours);source=name;break
    except(Exception)as e:errors.append(f'{name}: {e}')
   if result is None:raise TLEError('; '.join(errors))
  text,when,age=result;save_atomic(args.output,text);print(f'Saved {args.output}\nSource: {source}\nEpoch: {when.isoformat()}\nEpoch age: {age:.1f} hours')
  if abs(age)>24:print('Note: epoch exceeds24 hours; refresh before a planned pass if fresher data is available.',file=sys.stderr)
 except(OSError,ValueError,UnicodeError)as e:print(f'TLE update failed; existing file was not replaced: {e}',file=sys.stderr);return 1
 return 0
if __name__=='__main__':raise SystemExit(main())
