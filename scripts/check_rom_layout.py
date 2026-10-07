"""Reject overlapping ROM sections and helpers placed outside fixed ROM."""
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
areas={}
for name,start,size in re.findall(r'^(_\w+)\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s+=', (ROOT/'dist/open-monster.map').read_text(),re.M):
 areas[name]=(int(start,16),int(size,16))
base,size=areas['_CODE'];assert base==0x400 and base+size<=0x8000,'Flat engine/data exceed ROM banks 0/1'
base,size=areas['_HOME'];assert 0x200<=base and base+size<0x400,'Bank-switch helpers must reside in fixed bank zero'
selected=['_HOME','_INITIALIZER','_GSINIT','_GSFINAL','_CODE','_CODE_2']
ranges=[]
for name in selected:
 if name not in areas:continue
 base,size=areas[name]
 if not size:continue
 bank=base>>16;address=base&0xffff;physical=bank*0x4000+address-0x4000 if bank else address
 assert physical>=0x200 and physical+size<=131072,(name,physical,size)
 ranges.append((physical,physical+size,name))
for previous,current in zip(sorted(ranges),sorted(ranges)[1:]):
 assert previous[1]<=current[0],f'ROM overlap: {previous[2]} / {current[2]}'
print(f'PASS: ROM regions do not overlap; fixed bank-call helpers; base ROM space remaining {0x8000-sum(areas["_CODE"])} bytes')
