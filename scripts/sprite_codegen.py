"""Pack source pixels into 8x16 OBJ tiles with real color-zero transparency."""

def sprite_bytes(rows,tile_data):
 data=tile_data(rows);w=len(rows[0])//8;h=len(rows)//8;out=[]
 assert h%2==0
 for y in range(0,h,2):
  for x in range(w):
   for offset in [y*w+x,(y+1)*w+x]:out+=data[offset*16:(offset+1)*16]
 return out

def build_buttons(root,tile_data,arr):
 # Three 24x24 medallions, padded to 24x32 for 8x16 hardware sprites.
 import json
 d=json.loads((root/'assets/ui-art.json').read_text())
 assert len(d['medals'])==3, 'Three command medallions are required'
 output=''
 for i,rows in enumerate(d['medals']):
  assert len(rows)==32 and all(len(row)==24 and set(row)<=set('0123') for row in rows), 'Invalid medallion pixels'
  output+=arr('medal_'+str(i),sprite_bytes([[int(c) for c in row] for row in rows],tile_data))
 return output
