"""Validate contributor data and compile original pixels/font to Game Boy 2bpp."""
import json,re,unicodedata,argparse,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from localization import load_locales,symbol
from scene_codegen import scenes
from sprite_codegen import sprite_bytes,build_buttons
EFFECTS=['attack','guard','dodge','heal','focus','pierce','drain']
TRAITS=['steam','rain','warm','shell','spore','quick']
BASE_IDS=['puff','bash','spark','rain','guard','dodge','mend','rush','crush','focus','pierce','drain']
def content(root=ROOT):
 cfg,contexts,catalogs=load_locales(root)
 moves=json.loads((root/'data/moves.json').read_text()); mons=sorted([json.loads(p.read_text()) for p in (root/'data/monsters').glob('*.json')],key=lambda m:m['save_id'])
 assert [m['id'] for m in moves]==BASE_IDS,'Move order/IDs are save ABI; migration required to change'
 assert 6<=len(mons)<=24,'6..24 monsters supported by this ROM'
 assert len({m['id'] for m in mons})==len(mons),'Duplicate monster ID'
 assert len({m['save_id'] for m in mons})==len(mons),'Duplicate save_id'
 assert {m['save_id'] for m in mons}>=set(range(6)),'Keep baseline save IDs 0..5'
 assert [m['id'] for m in mons[:6]]==['chapo','amagumo','tomori','neji','kasamo','tikutaku'],'Baseline species IDs are save ABI'
 assert all(m['starter'] for m in mons[:3]) and sum(bool(m['starter']) for m in mons)==3,'Exactly first three are starters'
 for m in moves:
  assert type(m['power']) is int and type(m['priority']) is int,'Move values must be integers'
  assert m['name'] in contexts,'Unknown move name key'
  assert m['effect'] in EFFECTS and 0<=m['power']<=20 and -5<=m['priority']<=5,'Invalid move'
 for m in mons:
  assert type(m['save_id']) is int and type(m['starter']) is bool,'save_id must be integer, starter must be boolean'
  assert isinstance(m['name'],str) and isinstance(m['bio'],str),'Name/bio must be strings'
  assert all(type(m['stats'][k]) is int for k in ['hp','attack','defense','speed']),'Stats must be integers'
  assert re.fullmatch(r'[a-z][a-z0-9_-]*',m['id']) and 0<=m['save_id']<255
  assert m['name'] in contexts and m['bio'] in contexts,'Unknown monster translation key'
  assert contexts[m['name']]['max_cells']==8 and contexts[m['bio']]['max_cells']==18
  assert m['trait'] in TRAITS
  assert len(m['base_moves'])==3 and len(set(m['base_moves']))==3
  assert m['base_moves'][1:] == ['guard','dodge'],'Base slots 2/3 must be guard/dodge'
  assert moves[BASE_IDS.index(m['base_moves'][0])]['effect'] in ['attack','pierce','drain']
  assert set(m['training'])=={'attack','defense','technique'}
  assert all(x in BASE_IDS for x in m['base_moves']+list(m['training'].values()))
  assert 24<=m['stats']['hp']<=40 and all(5<=m['stats'][s]<=13 for s in ['attack','defense','speed'])
  assert 28<=sum(m['stats'].values())<=70
  assert len(m['palette'])==4 and all(re.fullmatch(r'#[0-9a-fA-F]{6}',x) for x in m['palette'])
  assert len(m['pixels']) in (16,32) and all(len(row)==len(m['pixels']) and set(row)<=set('.123') for row in m['pixels'])
  assert m['authors'] and all(isinstance(a,str) and a for a in m['authors'])
 return moves,mons
def tile_data(rows):
 out=[]
 for ty in range(0,len(rows),8):
  for tx in range(0,len(rows[0]),8):
   for y in range(8):
    lo=hi=0
    for x in range(8):
     v=rows[ty+y][tx+x];lo=(lo<<1)|(v&1);hi=(hi<<1)|((v>>1)&1)
    out.extend([lo,hi])
 return out
def art(rows,scale=1):return [[0 if c=='.' else int(c) for c in row for _ in range(scale)] for row in rows for _ in range(scale)]
def arr(name,data,typ='uint8_t'):return f'const {typ} {name}[] = {{'+','.join(map(str,data))+'};\n'
def color(s):r,g,b=[int(s[i:i+2],16)>>3 for i in (1,3,5)];return r|(g<<5)|(b<<10)
def generate():
 cfg,contexts,catalogs=load_locales(ROOT)
 moves,mons=content();font=json.loads((ROOT/'assets/font.json').read_text()); chars=list(font)
 extras='ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポァィゥェォッャュョ'
 chars+=list(extras)
 assert len(chars)<=128,'Font overlaps reserved graphic tiles'
 tiles=[]
 for c in chars:
  dec=unicodedata.normalize('NFD',c);base=dec[0];small=base in 'ァィゥェォッャュョ'
  if small:base=chr(ord(base)+1) if base not in 'ッャュョ' else {'ッ':'ツ','ャ':'ヤ','ュ':'ユ','ョ':'ヨ'}[base]
  src=font[base];pixels=[[0]*8 for _ in range(8)]
  for y,row in enumerate(src):
   for x,b in enumerate(row):
    if b=='1':pixels[min(7,y+1)][x+1]=1
  if len(dec)>1:
   for x,y in ([(6,0),(7,1),(6,2),(7,3)] if dec[1]=='\u3099' else [(6,0),(7,0),(6,1),(7,1)]):pixels[y][x]=1
  if small:
   old=pixels;pixels=[[0]*8 for _ in range(8)]
   for y in range(5):
    for x in range(4):pixels[y+3][x+2]=old[int(y*7/5)+1][int(x*5/4)+1]
  tiles+=tile_data(pixels)
 out='#include "core.h"\n#include "generated.h"\n'
 keys=list(contexts)
 header='#ifndef LOCALE_IDS_H\n#define LOCALE_IDS_H\n#define LOCALE_COUNT 2\n#define DEFAULT_LOCALE 1\nenum MessageId {\n'+',\n'.join('MSG_'+symbol(k)for k in keys)+',\nMSG_COUNT\n};\n#endif\n'
 (ROOT/'src/locale_ids.h').write_text(header)
 for lang in cfg['locales']:
  for key in keys:out+='const char text_'+lang+'_'+symbol(key)+'[]='+json.dumps(catalogs[lang][key],ensure_ascii=False)+';\n'
 out+='const char *const locale_text[LOCALE_COUNT][MSG_COUNT]={\n'+',\n'.join('{'+','.join('text_'+lang+'_'+symbol(k)for k in keys)+'}'for lang in cfg['locales'])+'};\n'
 def names(key):return '{'+','.join('text_'+lang+'_'+symbol(key)for lang in cfg['locales'])+'}'
 out+=arr('font_tiles',tiles)+arr('font_codes',[ord(c) for c in chars],'uint16_t')+f'const uint8_t font_count={len(chars)};\n'
 # Terrain tiles indices 160..171: meadow, path, tree, water, tall grass, lodge, trainer, berry, border, cards, HP full/empty
 patterns=[['22222222','22222222','22222222','22222222','22222222','22222222','22222222','22222222'],['33333333','33333333','33333333','33333333','33333333','33333333','33333333','33333333'],['22211222','22133122','21333312','13333331','21333312','22111122','22211222','22211222'],['22222222','23322233','22233222','22222222','33222332','22222222','22233222','22222222'],['22222222','22122122','21121122','22122122','12212212','21221222','22122122','22222222'],['22211222','22133122','21333312','11111111','23333332','23111132','23122132','23122132'],['22211222','22133122','22211222','22133122','21333312','22133122','22211222','22122122'],['22222222','22111222','21333122','21212122','21333122','22111222','22212222','22222222'],['11111111','13333331','13333331','13333331','13333331','13333331','13333331','11111111'],['33333333']*8,['22222222']*8,['11111111','13333331','13333331','13333331','13333331','13333331','13333331','11111111']]
 patterns += [['11111111']+['00000000']*7, ['00000000']*7+['11111111'], ['10000000']*8, ['00000001']*8, ['11111111']+['10000000']*7, ['11111111']+['00000001']*7, ['10000000']*7+['11111111'], ['00000001']*7+['11111111']]
 out+=arr('hp_tiles',sum((tile_data([[int(c) for c in r]for r in p]) for p in patterns[10:12]),[]))
 out+=arr('frame_tiles',sum((tile_data([[int(c) for c in r]for r in p]) for p in patterns[12:]),[]))
 for m in mons:
  out+=arr('small_'+m['id'],tile_data(art(m['pixels'] if len(m['pixels'])==16 else [''.join(row[::2])for row in m['pixels'][::2]])))
 out+=scenes(ROOT,tile_data,arr)
 out+=build_buttons(ROOT,tile_data,arr)
 for m in mons:out+=arr('obj_'+m['id'],sprite_bytes(art(m['pixels'],32//len(m['pixels'])),tile_data))
 # original overhead explorer
 player=['....111111......','...12222221.....','...11111111.....','....133331......','....131131......','....133331......','.....1111.......','....122221......','...12222221.....','...11222211.....','....122221......','....122221......','....111111......','....12..21......','....11..11......','................']
 out+=arr('player_tiles',tile_data(art(player)))
 out+='const Move moves[]={\n'+',\n'.join('{'+names(m['name'])+f",{m['power']},{m['priority']},{EFFECTS.index(m['effect'])}"+'}'for m in moves)+'};\n'
 out+='const Species species[]={\n'
 entries=[]
 for m in mons:
  st=m['stats'];values=[names(m['name']),names(m['bio']),str(m['save_id']),str(TRAITS.index(m['trait'])),str(int(m['starter']))]+[str(st[k])for k in ['hp','attack','defense','speed']]
  values+=['{'+','.join(str(BASE_IDS.index(x))for x in m['base_moves'])+'}','{'+','.join(str(BASE_IDS.index(m['training'][x]))for x in ['attack','defense','technique'])+'}','{'+','.join(str(color(x))for x in m['palette'])+'}','small_'+m['id'],'obj_'+m['id']]
  entries.append('{'+','.join(values)+'}')
 out+=',\n'.join(entries)+'};\n'+f'const uint8_t species_count={len(mons)},move_count={len(moves)};\n'
 # Validate all UI text has a glyph, preventing blank Japanese labels at runtime.
 for f in [ROOT/'src/main.c']:
  if f.exists():
   for literal in re.findall(r'"([^"\n]*)"',f.read_text()):
    if any(ord(c)>127 for c in literal):assert all(c in chars for c in literal),f'Unsupported glyph: {literal}'
 # Validate that every referenced interface key exists in both catalogs.
 source=(ROOT/'src/main.c').read_text()
 for msg in re.findall(r'L\((MSG_[A-Z0-9_]+)\)',source):assert msg in {'MSG_'+symbol(k)for k in keys},f'Unknown message ID {msg}'
 (ROOT/'src/generated.c').write_text(out)
 (ROOT/'src/generated.h').write_text('#ifndef GENERATED_H\n#define GENERATED_H\n#include <stdint.h>\n#include "locale_ids.h"\nextern const char *const locale_text[LOCALE_COUNT][MSG_COUNT];\nextern const uint8_t font_tiles[],font_count,hp_tiles[],frame_tiles[],player_tiles[];\nextern const uint8_t medal_0[],medal_1[],medal_2[];\nextern const uint8_t field_tiles[],field_map[],field_attrs[],field_count,arena_tiles[],arena_map[],arena_attrs[],arena_count,poster_tiles[],poster_map[],poster_attrs[],poster_count;\nextern const uint16_t font_codes[];\n#endif\n')
 print(f'Validated {len(mons)} monsters, {len(moves)} moves; {len(chars)} original font tiles')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');a=p.parse_args()
 if a.check:content();print('Content valid')
 else:generate()
