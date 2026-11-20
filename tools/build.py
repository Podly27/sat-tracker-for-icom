#!/usr/bin/env python3
"""Build the standalone ISS tracker ICAP app; never packages radio firmware."""
from pathlib import Path
import argparse,hashlib,json,os,struct,subprocess,sys,zlib
from elftools.elf.elffile import ELFFile
ROOT=Path(__file__).resolve().parents[1]
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path,default=ROOT/'build');args=parser.parse_args();out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
 cache=ROOT/'.cache';env=os.environ.copy();env.update(ZIG_GLOBAL_CACHE_DIR=str(cache/'zig-global'),ZIG_LOCAL_CACHE_DIR=str(cache/'zig-local'))
 cc=[sys.executable,'-m','ziglang','cc','-target','arm-linux-musleabi','-mcpu=cortex_a9','-marm','-mfloat-abi=softfp','-mfpu=neon']
 flags=['-Oz','-ffunction-sections','-fdata-sections','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fno-pic','-fno-builtin','-I'+str(ROOT/'src'),'-I'+str(ROOT/'vendor/sgp4'),'-DISS_SAT_CONTROL=1','-DISS_TX_CONTROL=1']
 # Generate target libc/compiler archives in an isolated, repository-local cache.
 cache.mkdir(exist_ok=True);bootstrap=out/'toolchain-check.c';bootstrap.write_text('int main(void){return 0;}\n')
 subprocess.run(cc+flags+[str(bootstrap),'-static','-o',str(out/'toolchain-check.elf')],check=True,env=env)
 sources=['src/app.c','src/orbit.c','src/gps.c','src/civ_queue.c','src/tiny.c','src/sat_control.c','src/tx_control.c','src/radio_session.c','vendor/sgp4/SGP4.c'];objects=[]
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
  header=bytearray(64);struct.pack_into('<4sHH8I',header,0,b'ICAP',3,64,1,0x07051405,len(image),bss,elf['e_entry']-base,0,8192,zlib.crc32(image));struct.pack_into('<I',header,56,4);header[40:56]=b'ISS tracker\0'.ljust(16,b'\0');app=bytes(header)+image
  (out/'ISS.app').write_bytes(app);report={'version':'0.15.0','display_name':'ISS tracker','code_bytes':len(image),'bss_bytes':bss,'stack_bytes':8192,'target':'IC-705 705E140 / SQ6EMM 20261119v8, ICAP ABI3 build07051405','sha256':hashlib.sha256(app).hexdigest()};(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
if __name__=='__main__':main()
