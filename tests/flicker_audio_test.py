"""Real button inputs: no LCD-off redraws, audible tracks, mute/resume, and audio preview."""
from pathlib import Path
import json, shutil, tempfile, wave
import numpy as np
from pyboy import PyBoy
ROOT=Path(__file__).resolve().parents[1]
SYMS={l.split()[1]:int(l.split()[2],16)for l in (ROOT/'dist/open-monster.noi').read_text().splitlines()if l.startswith('DEF ')}
report={'emulator':'PyBoy 2.6.1','gameplay_RAM_modified':False}
with tempfile.TemporaryDirectory()as td:
 rom=Path(td)/'audio.gbc';shutil.copy(ROOT/'dist/open-monster.gbc',rom)
 p=PyBoy(str(rom),window='null',sound_emulated=True);p.set_emulation_speed(0);p.tick(180)
 off=[];frames=0;white_frames=0
 address=SYMS['_display_off'];assert address<0x4000,'Bank-switch helpers must stay in fixed ROM'
 p.hook_register(0,address,lambda ctx:ctx.append(1),off)
 def read(name,off=0):return p.memory[SYMS['_'+name]+off]
 def advance(n,collect=False):
  global frames,white_frames
  out=[]
  for _ in range(n):
   p.tick(1);frames+=1
   assert p.memory[0xff40]&0x80,'LCD disabled after boot'
   if np.all(p.screen.ndarray[:,:,:3]==255):white_frames+=1
   if collect:out.append(p.sound.ndarray.copy())
  return np.concatenate(out)if out else None
 def press(k):p.button_press(k);advance(8);p.button_release(k);advance(40)
 for k in ['a','a','a']:press(k)
 assert read('state')==2
 for _ in range(24):press('down');press('up')
 assert read('state')==2
 # Sustain movement while the HUD context remains unchanged.
 p.button_press('right');advance(60);p.button_release('right');advance(20)
 assert read('state')==2
 trail=advance(360,True)
 assert np.max(np.abs(trail.astype(np.int16)))>4,'Exploration BGM is silent'
 press('select');muted=advance(90,True)
 assert np.max(np.abs(muted[-5000:].astype(np.int16)))==0,'Mute still emits audio'
 press('select');resumed=advance(90,True)
 assert np.max(np.abs(resumed.astype(np.int16)))>4,'BGM did not resume'
 # Return along the starting clearing, then enter the grass with actual movement.
 while read('game',3)>3:press('left')
 while read('game',4)<7:press('down')
 for _ in range(5):press('right')
 press('a');assert read('state')==8
 battle=advance(360,True)
 assert np.max(np.abs(battle.astype(np.int16)))>4,'Battle BGM is silent'
 assert not np.array_equal(trail[:10000],battle[:10000]),'Battle track did not change'
 for _ in range(12):press('right');press('left')
 assert not off,f'LCD turned off {len(off)} times'
 assert not white_frames,f'{white_frames} blank white frames'
 report.update(frames_checked=frames,movement_steps=48,lcd_off_calls=len(off),blank_white_frames=white_frames,tracks=['Trail Lanterns','Meet a Challenger'],mute_resume_passed=True,sample_rate=p.sound.sample_rate)
 with wave.open(str(ROOT/'dist/music-preview.wav'),'wb')as f:
  f.setnchannels(2);f.setsampwidth(2);f.setframerate(p.sound.sample_rate);f.writeframes((np.concatenate([trail,battle]).astype(np.int16)*256).tobytes())
 p.stop(save=False)
(ROOT/'dist/flicker-audio-verification.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
