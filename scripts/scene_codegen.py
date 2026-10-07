"""Exact alpha compositing of static scenery; no palette approximation is permitted."""
import json,re

def scenes(root,tile_data,arr):
 d=json.loads((root/'assets/ui-art.json').read_text())
 source=(root/'src/main.c').read_text();block=re.search(r'const palette_color_t palettes\[\]=\{(.*?)\};',source,re.S)[1]
 colors=[tuple(int(x)for x in group)for group in re.findall(r'RGB\((\d+),(\d+),(\d+)\)',block)]
 palettes=[colors[i:i+4]for i in range(0,32,4)];brand=[(3,7,9),(1,3,5),(30,22,9),(31,30,22)]
 metrics=[]
 def tile_rows(data,off=0,w=8,h=8):return [[int(c)for c in row]for row in data]
 def pack(name,canvas,pals):
  mapping=[];attributes=[];unique={};packed=[];decoded=[]
  for y in range(0,144,8):
   for x in range(0,160,8):
    pix=[row[x:x+8]for row in canvas[y:y+8]];needed={c for row in pix for c in row}
    choices=[i for i in [1,3,2,7,0,6,4,5]if needed<=set(pals[i])]
    assert choices,f'{name} tile {x},{y} exceeds palette colors: {needed}'
    pal=choices[0];rows=[[pals[pal].index(c)for c in row]for row in pix];data=tuple(tile_data(rows))
    if data not in unique:unique[data]=len(unique);packed+=data
    mapping.append(128+unique[data]);attributes.append(pal)
    assert all(pals[pal][rows[yy][xx]]==pix[yy][xx]for yy in range(8)for xx in range(8))
  assert len(unique)<=88,f'{name} leaves no room for UI font composites: {len(unique)}'
  metrics.append({'scene':name,'tiles':len(unique),'palette_roundtrip_errors':0})
  return arr(name+'_tiles',packed)+arr(name+'_map',mapping)+arr(name+'_attrs',attributes)+f'const uint8_t {name}_count={len(unique)};\n'
 def canvas():return [[palettes[1][0]]*160 for _ in range(144)]
 def stamp(c,rows,x,y,pal,alpha=False,label=''):
  count=0
  for yy,row in enumerate(rows):
   for xx,value in enumerate(row):
    px=x+xx;py=y+yy
    if not(0<=px<160 and 0<=py<144):continue
    before=c[py][px]
    if not(alpha and value==0):c[py][px]=palettes[pal][value]
    if alpha and value==0:assert c[py][px]==before;count+=1
  if alpha:metrics.append({'layer':label,'transparent_pixels_checked':count,'alpha_errors':0})
 grass=[[int(v)for v in r]for r in d['motifs'][0]]
 path=[[int(v)for v in r]for r in ['00000000','00000000','00222000','00332000','00000000','00002330','00002220','00000000']]
 water=[[int(v)for v in r]for r in ['22222222','22333222','22222222','22222332','22222222','33322222','22222222','22222222']]
 tall=[[int(v)for v in r]for r in ['00000000','00000000','00100100','01201210','12112121','01101100','00100100','00000000']]
 def base():
  c=canvas()
  for y in range(18):
   for x in range(20):stamp(c,grass,x*8,y*8,1)
  return c
 field=base()
 for y in range(14):
  for x in range(20):
   if x==5 and 2<y<12 and y!=7:stamp(field,water,x*8,(y+2)*8,2)
   elif 8<=x<=12 and 5<=y<=10:stamp(field,tall,x*8,(y+2)*8,1)
   elif 1<=x<=18 and 1<=y<=11 and (y==7 or x in [3,15]):stamp(field,path,x*8,(y+2)*8,3)
 for x in range(0,160,16):
  stamp(field,tile_rows(d['tree']),x,8,1,True,'world/tree/top');stamp(field,tile_rows(d['tree']),x,112,1,True,'world/tree/bottom')
 for y in range(24,112,16):
  stamp(field,tile_rows(d['tree']),-8,y,1,True,'world/tree/left');stamp(field,tile_rows(d['tree']),152,y,1,True,'world/tree/right')
 # Flowers stay clear of the solid hut footprint, while ground grain still runs beneath it.
 for x,y in [(7,3),(10,2),(13,9),(7,11),(17,4)]:stamp(field,tile_rows(d['motifs'][1]),x*8,(y+2)*8,1,True,'world/flowers')
 without_hut=[row[:]for row in field]
 stamp(field,tile_rows(d['hut']),8,24,3,True,'world/building')
 stamp(field,tile_rows(d['motifs'][4]),40,72,3,True,'world/bridge')
 for x,y in [(15,3),(16,7),(15,11)]:stamp(field,tile_rows(d['trainer']),x*8,(y+1)*8,3,True,'world/trainer')
 arena=base()
 for x in range(0,160,16):stamp(arena,tile_rows(d['tree']),x,8,1,True,'battle/tree')
 for y in range(24,128,16):
  stamp(arena,tile_rows(d['tree']),-8,y,1,True,'battle/tree/left');stamp(arena,tile_rows(d['tree']),152,y,1,True,'battle/tree/right')
 # A stone/clay walk and foreground leaves give a plane for the feet, not a menu panel.
 for x in range(20):stamp(arena,path,x*8,88,3)
 for x,y in [(5,6),(8,4),(17,5),(3,12),(10,14),(15,12)]:stamp(arena,tile_rows(d['motifs'][1]),x*8,y*8,1,True,'battle/flowers')
 title=base();tp=[list(p)for p in palettes];tp[7]=brand
 for y in range(48):title[y]=[brand[0]]*160
 for y in range(32,48):
  for x in range(160):
   if y>35+(x//12)%5:title[y][x]=brand[1]
 for x,y in [(8,7),(149,11),(132,23),(19,27),(118,5),(46,4)]:
  title[y][x]=brand[3]
  if x in [132,19]:title[y-1][x]=brand[2];title[y+1][x]=brand[2];title[y][x-1]=brand[2];title[y][x+1]=brand[2]
 before_logo=[row[:]for row in title];logo_zero=0
 for y,row in enumerate(d['logo']):
  for x,v in enumerate(row):
   if v!='0':title[y+8][x+16]=brand[int(v)]
   else:assert title[y+8][x+16]==before_logo[y+8][x+16];logo_zero+=1
 metrics.append({'layer':'title/logo','transparent_pixels_checked':logo_zero,'alpha_errors':0})
 for x in range(0,160,16):stamp(title,tile_rows(d['tree']),x,64,1,True,'title/tree')
 for x,y in [(2,11),(10,12),(17,11),(8,16)]:stamp(title,tile_rows(d['motifs'][1]),x*8,y*8,1,True,'title/flowers')
 def ppm(name,c):
  (root/f'dist/{name}.ppm').write_bytes(b'P6\n160 144\n255\n'+bytes(v*8 for row in c for color in row for v in color))
 ppm('expected-field',field);ppm('expected-field-without-hut',without_hut);ppm('expected-arena',arena);ppm('expected-poster',title);ppm('expected-poster-without-logo',before_logo)
 output=pack('field',field,palettes)+pack('arena',arena,palettes)+pack('poster',title,tp)
 (root/'dist/alpha-verification.json').write_text(json.dumps({'layers':metrics,'all_alpha_errors':0,'all_palette_roundtrip_errors':0},indent=2)+'\n')
 return output
