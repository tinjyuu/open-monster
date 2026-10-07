"""Record genuine ROM gameplay with button inputs; requires ffmpeg and test dependencies."""
from pathlib import Path
import json,shutil,subprocess,tempfile,hashlib
from pyboy import PyBoy
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
SYMS={line.split()[1]:int(line.split()[2],16)for line in(ROOT/'dist/open-monster.noi').read_text().splitlines()if line.startswith('DEF ')}
OUT=ROOT/'dist/video';OUT.mkdir(exist_ok=True)
video=OUT/'open-monster-gameplay-v0.2.mp4'
encoder=subprocess.Popen(['ffmpeg','-hide_banner','-loglevel','error','-y','-f','rawvideo','-pixel_format','rgb24','-video_size','640x576','-framerate','30','-i','pipe:0','-an','-c:v','libx264','-preset','fast','-crf','18','-pix_fmt','yuv420p','-movflags','+faststart',str(video)],stdin=subprocess.PIPE)
frames=0;chapters=[]
with tempfile.TemporaryDirectory()as td:
 rom=Path(td)/'demo.gbc';shutil.copy(ROOT/'dist/open-monster.gbc',rom)
 p=PyBoy(str(rom),window='null',sound_emulated=False);p.set_emulation_speed(0);p.tick(180)
 def read(name,offset=0):return p.memory[SYMS['_'+name]+offset]
 def state():return read('state')
 def advance(n):
  global frames
  for _ in range((n+1)//2):
   p.tick(2);image=p.screen.image.convert('RGB').resize((640,576),Image.Resampling.NEAREST);encoder.stdin.write(image.tobytes());frames+=1
 def mark(name):chapters.append({'seconds':round(frames/30,2),'scene':name});print(name,round(frames/30,2),flush=True)
 def press(key,delay=36):p.button_press(key);advance(8);p.button_release(key);advance(delay)
 def ack():
  assert state()==10;advance(70);press('a')
 def choose(index):
  for _ in range(3):
   if read('cursor')==index:return
   press('right',20)
  assert read('cursor')==index
 def walk(axis,target):
  offset=3 if axis=='x' else 4
  for _ in range(30):
   value=read('game',offset)
   if value==target:return
   assert state()==2
   press(('right'if target>value else'left')if axis=='x'else('down'if target>value else'up'),20)
  raise AssertionError('Path blocked')
 mark('Title / language switching');advance(120);press('select');advance(100);press('select');advance(80)
 press('a');assert state()==1;mark('Choose your first companion');advance(100)
 press('right');advance(55);press('right');advance(55);press('left');press('left');advance(55);press('a');ack();assert state()==2
 mark('Explore with Chapo');advance(90);press('a');assert state()==3;advance(90);press('a');assert state()==4
 mark('Training styles');advance(100);press('down');advance(60);press('down');advance(60);press('up');press('up');advance(60);press('a');ack()
 mark('Equip a learned move');press('down');press('a');assert state()==5;advance(90);press('a');assert state()==6;advance(60)
 for _ in range(3):press('down',20)
 advance(70);press('a');advance(80);press('b');press('b');assert state()==2
 mark('Cross the river and search the grass');walk('y',7);walk('x',8);advance(90);press('a');assert state()==8
 mark('Battle cards and enemy intent');advance(140)
 choose(1);advance(65);press('a');assert state()==8;advance(90)
 choose(2);advance(65);press('a');assert state()==8;advance(90)
 mark('Dodge cooldown');choose(2);press('a');assert state()==10;ack();assert state()==8
 choose(0);advance(65);press('a');assert state()==8;advance(90)
 assert read('enemy',8)==12,'Expected genuine first attack damage'
 mark('Scout a weakened wild creature');press('b');choose(1);advance(65);press('a');ack();assert state()==8
 mark('Try again after a failed scout');press('b');choose(1);advance(65);press('a');assert read('game')==2;ack();assert state()==2
 mark('Meet your new companion');press('b');assert state()==7;advance(100);press('down');advance(90);press('a');assert state()==2;advance(120)
 mark('Save the journey');press('start');ack();advance(140)
 p.stop(save=False)
encoder.stdin.close();assert encoder.wait()==0
info={'file':video.name,'duration_seconds':round(frames/30,2),'fps':30,'resolution':[640,576],'source':'Actual ROM in PyBoy 2.6.1; button inputs only, no gameplay RAM changes','audio':False,'rom_sha256':hashlib.sha256((ROOT/'dist/open-monster.gbc').read_bytes()).hexdigest(),'chapters':chapters}
(OUT/'gameplay-info.json').write_text(json.dumps(info,indent=2)+'\n');print(json.dumps(info,indent=2))
