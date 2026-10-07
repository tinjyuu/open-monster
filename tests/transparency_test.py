"""Verify final emulator pixels, not just masks: scenery, OBJ alpha and scanline limits."""
from pathlib import Path
import json,tempfile,shutil,subprocess
from pyboy import PyBoy
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
SYMS={l.split()[1]:int(l.split()[2],16)for l in(ROOT/'dist/open-monster.noi').read_text().splitlines()if l.startswith('DEF ')}
results={}
with tempfile.TemporaryDirectory()as td:
 rom=Path(td)/'alpha.gbc';shutil.copy(ROOT/'dist/open-monster.gbc',rom)
 p=PyBoy(str(rom),window='null',sound_emulated=False);p.set_emulation_speed(0);p.tick(240)
 def press(k):p.button_press(k);p.tick(8);p.button_release(k);p.tick(90)
 def object_check(label,objects):
  full=p.screen.image.convert('RGB');control=p.memory[0xff40];p.memory[0xff40]=control&~2;p.tick(2);back=p.screen.image.convert('RGB');p.memory[0xff40]=control;p.tick(2);checked=0
  for rows,x,y,flip in objects:
   for yy,row in enumerate(rows):
    for xx,v in enumerate(row):
     if v not in ['.','0']:continue
     px=x+(len(row)-1-xx if flip else xx);py=y+yy
     assert full.getpixel((px,py))==back.getpixel((px,py)),f'{label}: alpha changed background at {px},{py}'
     checked+=1
  results[label]={'transparent_pixels':checked,'errors':0}
  return back
 mons={json.loads(f.read_text())['id']:json.loads(f.read_text())for f in(ROOT/'data/monsters').glob('*.json')}
 logo_scene=Image.open(ROOT/'dist/expected-poster-without-logo.ppm').convert('RGB');title_actual=p.screen.image.convert('RGB');logo_art=json.loads((ROOT/'assets/ui-art.json').read_text())['logo'];logo_checked=0
 for y,row in enumerate(logo_art):
  for x,value in enumerate(row):
   if value!='0':continue
   assert title_actual.getpixel((x+16,y+8))==logo_scene.getpixel((x+16,y+8)),f'Logo wiped pixel {x},{y}'
   logo_checked+=1
 results['logo_transparency']={'transparent_pixels':logo_checked,'errors':0}
 object_check('title_monsters',[(mons['chapo']['pixels'],24,72,False),(mons['amagumo']['pixels'],112,72,True)])
 for k in ['a','a','a']:press(k)
 assert p.memory[SYMS['_state']]==2
 p.memory[0xff40]&=~2;p.tick(2);actual=p.screen.image.convert('RGB');expected=Image.open(ROOT/'dist/expected-field.ppm').convert('RGB');before_hut=Image.open(ROOT/'dist/expected-field-without-hut.ppm').convert('RGB')
 checked=0
 for y in range(16,128):
  for x in range(160):
   if any(nx<=x<nx+8 and ny<=y<ny+8 for nx,ny in [(112,32),(120,64),(112,96)]):continue
   assert actual.getpixel((x,y))==expected.getpixel((x,y)),f'Scene roundtrip mismatch {x},{y}'
   checked+=1
 results['scenery_emulator_roundtrip']={'pixels':checked,'errors':0}
 ui=json.loads((ROOT/'assets/ui-art.json').read_text());zero=0
 for y,row in enumerate(ui['hut']):
  for x,v in enumerate(row):
   if v!='0':continue
   assert actual.getpixel((x+8,y+24))==before_hut.getpixel((x+8,y+24)),f'Hut erased underlying pixel {x},{y}'
   zero+=1
 results['building_transparency']={'transparent_pixels':zero,'errors':0}
 # Check holes/spaces around outlined text against the actual underlying scene too.
 codes=[]
 for i in range(p.memory[SYMS['_font_count']]):codes.append(p.memory[SYMS['_font_codes']+i*2]|p.memory[SYMS['_font_codes']+i*2+1]<<8)
 cat=json.loads((ROOT/'locales/en.json').read_text());text_zero=0
 for x0,y0,text,outline in [(14,4,'1',True),(15,8,'2',True),(14,12,'3',True),(0,16,cat['map.base'],True),(0,17,cat['hint.world'],False)]:
  for index,char in enumerate(text):
   glyph=codes.index(ord(char));rows=[p.memory[SYMS['_font_tiles']+glyph*16+i*2]for i in range(8)]
   for y in range(8):
    mask=rows[y]
    if outline:mask|=(rows[y]<<1)|(rows[y]>>1)|(rows[y-1]if y else 0)|(rows[y+1]if y<7 else 0)
    for x in range(8):
     if mask&(1<<(7-x)):continue
     px=(x0+index)*8+x;py=y0*8+y
     assert actual.getpixel((px,py))==expected.getpixel((px,py)),f'Text wiped scene pixel {px},{py}'
     text_zero+=1
 results['text_transparent_spaces']={'transparent_pixels':text_zero,'errors':0}

 p.memory[0xff40]|=2;p.tick(2)
 for k in ['down','down','right','right','right','right','right','a']:press(k)
 assert p.memory[SYMS['_state']]==8
 objects=[(mons['chapo']['pixels'],24,56,False),(mons['amagumo']['pixels'],104,56,True)]
 for i,rows in enumerate(ui['medals']):objects.append((rows,12+i*56,104,False))
 object_check('battle_monsters_and_commands',objects)
 occupancy=[]
 for y in range(144):
  n=0
  for i in range(40):
   top=p.memory[0xfe00+i*4]-16
   if top<=y<top+16:n+=1
  occupancy.append(n)
 assert max(occupancy)<=10,occupancy
 results['sprite_scanline_limit']={'max':max(occupancy),'limit':10,'errors':0}
 p.stop(save=False)
# Compile a test-only driver, leaving the playable ROM and gameplay state untouched.
subprocess.run([str(ROOT/'.tools/gbdk/bin/lcc'),'-msm83:gb','-Wm-yC','-Wm-yt0x1B','-Wm-yo8','-Wm-ya1','-Wl-j','-Wl-b_HOME=0x0200','-Wl-b_CODE=0x0400','-o',str(ROOT/'dist/visual-fixture.gbc'),str(ROOT/'tests/visual_fixture.c'),str(ROOT/'src/core.c'),str(ROOT/'src/generated.c'),str(ROOT/'src/music.c')],check=True,stdout=subprocess.DEVNULL)
q=PyBoy(str(ROOT/'dist/visual-fixture.gbc'),window='null',sound_emulated=False);q.set_emulation_speed(0);q.tick(240)
checked=0
for monster in sorted(mons.values(),key=lambda m:m['save_id']):
 full=q.screen.image.convert('RGB');ctl=q.memory[0xff40];q.memory[0xff40]=ctl&~2;q.tick(2);back=q.screen.image.convert('RGB');q.memory[0xff40]=ctl;q.tick(2)
 for x0,flip in [(24,False),(104,True)]:
  for y,row in enumerate(monster['pixels']):
   for x,value in enumerate(row):
    if value!='.':continue
    px=x0+(31-x if flip else x);py=56+y
    assert full.getpixel((px,py))==back.getpixel((px,py)),f'{monster["id"]}: alpha mismatch'
    checked+=1
 q.button_press('a');q.tick(8);q.button_release('a');q.tick(90)
q.stop(save=False)
results['all_six_species_both_directions']={'transparent_pixels':checked,'errors':0}
metrics=json.loads((ROOT/'dist/alpha-verification.json').read_text());metrics['emulator_checks']=results
(ROOT/'dist/alpha-verification.json').write_text(json.dumps(metrics,indent=2)+'\n')
print(json.dumps(results,indent=2))
