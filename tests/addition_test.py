"""Build a seventh monster in an isolated copy, encounter and scout it in the ROM."""
from pathlib import Path
import json,shutil,subprocess,tempfile
from pyboy import PyBoy
ROOT=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as td:
 root=Path(td)
 for name in ['src','scripts','data','assets','locales']:shutil.copytree(ROOT/name,root/name,ignore=shutil.ignore_patterns('__pycache__','generated.*'))
 shutil.copy(ROOT/'Makefile',root/'Makefile');(root/'dist').mkdir()
 monster=json.loads((root/'data/monsters/chapo.json').read_text());monster.update(id='sample',save_id=6,name='monsters.sample.name',bio='monsters.sample.bio',starter=False,authors=['Test fixture'])
 for lang in ['en','ja']:
  path=root/f'locales/{lang}.json';cat=json.loads(path.read_text());cat['monsters.sample.name']='SAMPLE' if lang=='en' else 'サンプル';cat['monsters.sample.bio']='A TRAVELING TEAPOT' if lang=='en' else 'タビヲ スル キュウス';path.write_text(json.dumps(cat))
 ctx=json.loads((root/'locales/context.json').read_text());ctx['monsters.sample.name']={'max_cells':8,'description':'Sample name'};ctx['monsters.sample.bio']={'max_cells':18,'description':'Sample bio'};(root/'locales/context.json').write_text(json.dumps(ctx))
 (root/'data/monsters/sample.json').write_text(json.dumps(monster,ensure_ascii=False))
 subprocess.run(['make','-C',str(root),'GBDK_HOME='+str(ROOT/'.tools/gbdk')],check=True,stdout=subprocess.DEVNULL)
 syms={l.split()[1]:int(l.split()[2],16)for l in(root/'dist/open-monster.noi').read_text().splitlines()if l.startswith('DEF ')}
 p=PyBoy(str(root/'dist/open-monster.gbc'),window='null',sound_emulated=False);p.set_emulation_speed(0);p.tick(180)
 def press(k):p.button_press(k);p.tick(8);p.button_release(k);p.tick(40)
 def state():return p.memory[syms['_state']]
 def g(off=0):return p.memory[syms['_game']+off]
 for k in ['a','a','a','down','down','right','right','right','right','right']:press(k)
 assert state()==2
 found=False
 for attempt in range(80):
  press('a');assert state()==8
  if p.memory[syms['_enemy']]==6:
   for _ in range(6):
    press('b');press('right');press('a')
    if g()==2:found=True;break
    if state()==10:press('a')
    if state()!=8:break
   if found:break
   if state()==10:press('a')
   if g(3)==3:
    for k in ['down','down','right','right','right','right','right']:press(k)
  else:
   press('b');press('right');press('right');press('a');press('a')
 assert found,'Could not scout seventh monster'
 assert g(7+8)==6
 p.stop(save=False)
 print('PASS: seventh JSON-only monster built, encountered and recruited, no engine source edits')
