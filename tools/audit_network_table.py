#!/usr/bin/env python3
"""Read-only audit of an owner's decoded firmware; no firmware redistribution."""
import argparse, hashlib, json, struct
from pathlib import Path
PROFILES={
 '13b956a2b958be7ccfa51f4011584616':('v8',0x2037eb30,[0x2037b551,0x2037c455,0x2037bb29,0x2037bc3d,0x2037b611]),
 '290c76af32ecc54c23aa0b9a0aa797a3':('v14',0x2037ecd0,[0x2037b579,0x2037c601,0x2037bc21,0x2037bd35,0x2037b639]),
}
def main():
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('decoded_main',type=Path);args=parser.parse_args()
 data=args.decoded_main.read_bytes();digest=hashlib.md5(data).hexdigest()
 if digest not in PROFILES:raise SystemExit('Unrecognised image: audit stopped. No addresses inferred from build ID alone.')
 version,address,expected=PROFILES[digest];offset=address-0x20005000
 table=struct.unpack_from('<36I',data,offset)
 if table[1:3]!=(0x705,0x07051405):raise SystemExit('Unexpected radio/build fields')
 actual=list(table[0x6c//4:0x80//4])
 if actual!=expected:raise SystemExit('Unexpected transport callback table')
 print(json.dumps({'version':version,'image_md5':digest,'table':hex(address),'callbacks':dict(zip(['ready','connect','send','receive','close'],map(hex,actual))),'scope':'static table check only; not a runtime network or TLS test'},indent=2))
if __name__=='__main__':main()
