#!/usr/bin/env python3
"""Build the standalone ISS tracker ICAP app; never packages radio firmware."""
from pathlib import Path
import argparse,hashlib,json,os,struct,subprocess,sys,zlib
from elftools.elf.elffile import ELFFile
ROOT=Path(__file__).resolve().parents[1]
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--frq-probe',action='store_true',help='Experimental v8 native text drawing below frequency');parser.add_argument('--probe-ip',action='store_true',help='Use numeric IPv4 for the HTTPS network probe');parser.add_argument('--display-probe',action='store_true',help='Build isolated native text dialog probe');parser.add_argument('--probe',action='store_true',help='Build isolated HTTPS probe without radio-control modules');parser.add_argument('--output',type=Path,default=ROOT/'build');args=parser.parse_args();assert not args.probe_ip or args.probe,'--probe-ip requires --probe';assert sum([args.probe,args.display_probe,args.frq_probe])<=1,'Select only one probe';out=(args.output/('network-probe-ip' if args.probe_ip else 'network-probe' if args.probe else 'display-probe' if args.display_probe else 'frq-probe' if args.frq_probe else '')).resolve();out.mkdir(parents=True,exist_ok=True)
 cache=ROOT/'.cache';env=os.environ.copy();env.update(ZIG_GLOBAL_CACHE_DIR=str(cache/'zig-global'),ZIG_LOCAL_CACHE_DIR=str(cache/'zig-local'))
 cc=[sys.executable,'-m','ziglang','cc','-target','arm-linux-musleabi','-mcpu=cortex_a9','-marm','-mfloat-abi=softfp','-mfpu=neon']
 flags=['-Oz','-ffunction-sections','-fdata-sections','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fno-pic','-fno-builtin','-I'+str(ROOT/'src'),'-I'+str(ROOT/'vendor/sgp4'),'-DISS_SAT_CONTROL=1','-DISS_TX_CONTROL=1']
 if args.probe_ip:flags+=['-DTLE_FORCE_IP=1']
 # Generate target libc/compiler archives in an isolated, repository-local cache.
 cache.mkdir(exist_ok=True);bootstrap=out/'toolchain-check.c';bootstrap.write_text('int main(void){return 0;}\n')
 subprocess.run(cc+flags+[str(bootstrap),'-static','-o',str(out/'toolchain-check.elf')],check=True,env=env)
 sources=['src/app.c','src/satellite_display.c','src/aprs_status.c','src/pass_status.c','src/tle_store.c','src/tle_network.c','src/orbit.c','src/gps.c','src/civ_queue.c','src/tiny.c','src/sat_control.c','src/tx_control.c','src/radio_session.c','vendor/sgp4/SGP4.c'];objects=[]
 if args.probe:sources=['src/network_probe.c','src/tle_network.c','src/orbit.c','src/tiny.c','vendor/sgp4/SGP4.c']
 if args.display_probe:sources=['src/display_probe.c','src/gps.c','src/orbit.c','src/tiny.c','vendor/sgp4/SGP4.c']
 if args.frq_probe:sources=['src/frq_text_probe.c','src/aprs_status.c','src/gps.c','src/orbit.c','src/tiny.c','vendor/sgp4/SGP4.c']
 for name in sources:
  obj=out/(Path(name).stem+'.o');subprocess.run(cc+flags+['-c',str(ROOT/name),'-o',str(obj)],check=True,env=env);objects.append(str(obj))
 libraries=[]
 for name in ['libc.a','libzigc.a','libcompiler_rt.a']:
  candidates=list((cache/'zig-global').glob('o/*/'+name))
  if len(candidates)!=1:raise SystemExit(f'Expected one isolated ARM {name}, found {len(candidates)}. Remove .cache/ and rebuild.')
  libraries.append(str(candidates[0]))
 target=out/'ISS.elf';subprocess.run(cc+['-nostdlib','-static','-no-pie','-Wl,-T,'+str(ROOT/'src/app.ld'),'-Wl,-e,app_main','-Wl,--gc-sections','-Wl,--build-id=none']+objects+libraries+['-o',str(target)],check=True,env=env)
 with target.open('rb')as f:
  elf=ELFFile(f);symbols={s.name:s['st_value']for s in elf.get_section_by_name('.symtab').iter_symbols()};base=0x20831000;end=symbols['__image_end'];bss=symbols['__bss_end']-end;image=bytearray(end-base)
  for section in elf.iter_sections():
   if section['sh_flags']&2 and section['sh_type']!='SHT_NOBITS' and section['sh_size']:
    at=section['sh_addr']-base;assert at>=0 and at+section['sh_size']<=len(image);image[at:at+section['sh_size']]=section.data()
  assert len(image)+bss<0x14000,'ICAP image/BSS memory budget exceeded'
  header=bytearray(64);struct.pack_into('<4sHH8I',header,0,b'ICAP',3,64,1,0x07051405,len(image),bss,elf['e_entry']-base,0,8192,zlib.crc32(image));struct.pack_into('<I',header,56,4);header[40:56]=b'SAT\0'.ljust(16,b'\0');app=bytes(header)+image

  if args.probe:
   header[40:56]=(b'TLE HTTPS IP\0' if args.probe_ip else b'TLE HTTPS DNS\0').ljust(16,b'\0');app=bytes(header)+image
  if args.display_probe:
   header[40:56]=b'ISS DISPLAY\0'.ljust(16,b'\0');app=bytes(header)+image
  if args.frq_probe:
   header[40:56]=b'FRQ TEXT TEST\0'.ljust(16,b'\0');app=bytes(header)+image
  (out/('FRQTEXT.app' if args.frq_probe else 'DISPCHK.app' if args.display_probe else 'TLEIP.app' if args.probe_ip else 'TLEDNS.app' if args.probe else 'SAT.app')).write_bytes(app);report={'version':'frq-test-6-stop-debug' if args.frq_probe else '0.17.0-probe' if args.probe or args.display_probe else '0.22.0','display_name':'FRQ TEXT TEST' if args.frq_probe else 'ISS DISPLAY' if args.display_probe else 'TLE HTTPS IP' if args.probe_ip else 'TLE HTTPS DNS' if args.probe else 'SAT','code_bytes':len(image),'bss_bytes':bss,'stack_bytes':8192,'target':('IC-705 705E140 / SQ6EMM v8/v14 APRS display-cache probe' if args.frq_probe else 'IC-705 705E140 / SQ6EMM v8 or v14 native text dialog probe' if args.display_probe else 'IC-705 705E140 / SQ6EMM v8 or v14 network probe' if args.probe else 'IC-705 705E140 / SQ6EMM 20261119v8, ICAP ABI3 build07051405'),'sha256':hashlib.sha256(app).hexdigest()};(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
