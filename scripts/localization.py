"""Flat JSON catalogs compatible with Weblate; hardware-aware validation."""
import json,re
from pathlib import Path
EXTRAS='ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポァィゥェォッャュョ'
def load_locales(root):
 root=Path(root);cfg=json.loads((root/'locales/config.json').read_text());languages=cfg['locales']
 assert languages==['ja','en'],'v1 save locale indexes are stable: ja=0, en=1'
 assert cfg['source'] in languages and cfg['default'] in languages
 context=json.loads((root/'locales/context.json').read_text())
 catalogs={lang:json.loads((root/f'locales/{lang}.json').read_text())for lang in languages}
 keys=set(context)
 assert keys and all(re.fullmatch(r'[a-z][a-z0-9_.]*',k)for k in keys),'Invalid semantic key'
 allowed=set(json.loads((root/'assets/font.json').read_text()))|set(EXTRAS)
 for lang,cat in catalogs.items():
  assert set(cat)==keys,f'{lang}: missing/extra keys: {keys ^ set(cat)}'
  for key,value in cat.items():
   assert isinstance(value,str) and value,f'{lang}/{key}: empty/non-string text'
   assert len(value)<=context[key]['max_cells'],f'{lang}/{key}: {len(value)} cells exceeds {context[key]["max_cells"]}'
   assert set(value)<=allowed,f'{lang}/{key}: unsupported glyphs {set(value)-allowed}'
 return cfg,context,catalogs
def symbol(key):return key.upper().replace('.','_')
