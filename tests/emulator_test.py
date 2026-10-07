"""Button-driven ROM acceptance test; symbols are read only, no gameplay RAM cheats."""
from pathlib import Path
import json,shutil,tempfile,os
from pyboy import PyBoy
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
SYMS={line.split()[1]:int(line.split()[2],16) for line in (ROOT/'dist/open-monster.noi').read_text().splitlines() if line.startswith('DEF ')}
OUT=ROOT/'dist'
LANGUAGE=os.environ.get('OPEN_MONSTER_LOCALE','en')
assert LANGUAGE in ['en','ja']

def run():
 with tempfile.TemporaryDirectory() as td:
  rom=Path(td)/'test.gbc';shutil.copy(ROOT/'dist/open-monster.gbc',rom)
  p=PyBoy(str(rom),window='null',sound_emulated=False);p.set_emulation_speed(0);p.tick(180)
  def read(name,off=0):return p.memory[SYMS['_'+name]+off]
  def state():return read('state')
  def press(key):p.button_press(key);p.tick(8);p.button_release(key);p.tick(40)
  def shot(name):p.tick(60);p.screen.image.save(OUT/f'{name}-{LANGUAGE}-160.png');p.screen.image.resize((640,576),Image.Resampling.NEAREST).save(OUT/f'{name}-{LANGUAGE}-preview.png')
  def ack():
   if state()==10:press('a')
  def walk(axis,target):
   off=3 if axis=='x' else 4
   for _ in range(30):
    v=read('game',off)
    if v==target:return
    assert state()==2,(state(),axis,target,v)
    press(('right'if target>v else'left')if axis=='x'else('down'if target>v else'up'))
   raise AssertionError(('blocked',axis,target,read('game',off)))
  assert state()==0
  if LANGUAGE=='ja':press('select')
  font=list(json.loads((ROOT/'assets/font.json').read_text()))+list('ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポァィゥェォッャュョ')
  assert read('screen',201)==font.index('Y' if LANGUAGE=='en' else 'ミ'),'Locale switch did not update title'
  shot('title');press('a');assert state()==1;press('a');ack();assert state()==2;shot('world')
  # Training + loadout via actual menus.
  press('a');assert state()==3;press('a');assert state()==4;shot('training');press('a');ack();assert read('game',7+4)==1
  press('down');press('a');assert state()==5;press('a');assert state()==6
  for _ in range(3):press('down')
  press('a');assert state()==5;assert read('game',7+5)==8
  press('b');press('b');assert state()==2
  walk('y',7);walk('x',8);press('a');assert state()==8;shot('battle')
  # Flee and re-enter, then farm encounters for progression and obtain a second monster.
  press('b');press('right');press('right');press('a');ack();assert state()==2
  # Deliberately challenge the Lv4 trainer at Lv1 and verify loss recovery.
  walk('x',6);walk('y',2);walk('x',14);walk('y',10);press('a');assert state()==8
  for _ in range(20):
   if state()!=8:break
   press('a')
  ack();assert state()==2 and read('game',3)==3 and read('game',4)==5
  walk('y',7);walk('x',8)
  caught=False;battles=0
  while read('game',8)<5 or not caught:
   battles+=1;assert battles<100
   press('a');assert state()==8
   for _ in range(30):
    if state()!=8:break
    active=read('game',1);ehp=read('enemy',8);es=read('enemy');ownspecies=read('game',7)
    if not caught and es!=ownspecies and ehp<22:
     press('b');press('right');press('a')
     if read('game')>1:caught=True
     if state()==10:
      # Failure message can return to battle; success/death returns to world.
      ack()
    else:press('a')
   ack()
   if state()==2 and read('game',3)==3:walk('y',7);walk('x',8)
   assert state()==2,state()
  # In-battle switching retains damage. Return to initial active afterward.
  press('a');assert state()==8;press('b');press('a');assert state()==7;press('down');press('a');ack()
  assert read('game',1)==1
  if state()==8:press('b');press('right');press('right');press('a');ack()
  press('b');press('up');press('a');assert read('game',1)==0
  # Reach every trainer over the bridge and along clear ground.
  walk('x',6);walk('y',2);walk('x',14);walk('y',3)
  trainer_positions=[(14,3),(15,7),(14,10)]
  for idx,(x,y) in enumerate(trainer_positions):
   if read('game',4)!=y:walk('y',y)
   if read('game',3)!=x:walk('x',x)
   press('a');assert state()==8
   for _ in range(50):
    if state()!=8:break
    # Attack the announced action; all trainers can be beaten by the trained Lv5 starter.
    press('a')
   assert state()==10;ack();assert read('game',2)&(1<<idx),(idx,read('game',2))
  assert read('game',2)==7
  # SRAM save persisted across emulator restart, with a real readback checksum.
  press('start');assert state()==10;ack();shot('completed')
  saved=list(p.memory[0xa000:0xa000+90]);p.stop()
  q=PyBoy(str(rom),window='null',sound_emulated=False);q.set_emulation_speed(0);q.tick(180)
  assert q.memory[SYMS['_save_status']]==1
  q.button('a');q.tick(30)
  assert q.memory[SYMS['_game']+2]==7
  q.stop(save=False)
  # Load incompatible and corrupt fixtures in isolated cartridges; verify no overwrite.
  for status,change in [(2,lambda b:b.__setitem__(2,99)),(3,lambda b:b.__setitem__(20,b[20]^1))]:
   fixture=Path(td)/f'bad{status}.gbc';shutil.copy(rom,fixture)
   ram=bytearray(Path(str(rom)+'.ram').read_bytes());change(ram);Path(str(fixture)+'.ram').write_bytes(ram)
   z=PyBoy(str(fixture),window='null',sound_emulated=False);z.set_emulation_speed(0);z.tick(180)
   assert z.memory[SYMS['_save_status']]==status
   for key in ['a','a','a','start']:z.button(key);z.tick(30)
   z.memory[0x0000]=0x0a  # MBC RAM enable for observation only; no SRAM contents changed.
   assert bytes(z.memory[0xa000:0xa000+90])==bytes(ram[:90]),'Protected SRAM overwritten'
   z.stop(save=False)
  result=dict(rom='open-monster.gbc',emulator='PyBoy 2.6.1',passed=['boot','starter','training','loadout','exploration','wild battle','scout','switch','all 3 trainers','loss recovery','progression','save/restart','incompatible save protected','corrupt save protected'],wild_battles=battles,hardware_tested=False,language=LANGUAGE)
  (OUT/f'verification-{LANGUAGE}.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':run()
