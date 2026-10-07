"""Create a monster plus complete English/Japanese catalog entries."""
import argparse,json,re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('id');p.add_argument('--name-en',required=True);p.add_argument('--name-ja',required=True);p.add_argument('--bio-en',default='A NEW COMPANION');p.add_argument('--bio-ja',default='アタラシイ ナカマ');p.add_argument('--author',required=True)
a=p.parse_args()
if not re.fullmatch(r'[a-z][a-z0-9_-]*',a.id):p.error('ID must start with a lowercase letter and contain only lowercase letters, digits, _ or -')
path=ROOT/f'data/monsters/{a.id}.json'
if path.exists():p.error('Monster already exists')
from localization import load_locales
cfg,context,catalogs=load_locales(ROOT)
monster=json.loads((ROOT/'data/monsters/chapo.json').read_text());monster.update(id=a.id,save_id=max(json.loads(f.read_text())['save_id']for f in (ROOT/'data/monsters').glob('*.json'))+1,starter=False,authors=[a.author])
for field,en,ja,limit in [('name',a.name_en,a.name_ja,8),('bio',a.bio_en,a.bio_ja,18)]:
 key=f'monsters.{a.id}.{field}';monster[field]=key;context[key]={'max_cells':limit,'description':f'{a.id} {field}'};catalogs['en'][key]=en;catalogs['ja'][key]=ja
 # Validate input before writing anything.
 allowed=set(json.loads((ROOT/'assets/font.json').read_text()))|set(__import__('localization').EXTRAS)
 if any(not text or len(text)>limit or not set(text)<=allowed for text in [en,ja]):p.error(f'{field}: use supported glyphs and at most {limit} cells')
if monster['save_id']>254:p.error('No save IDs remaining')
for language,catalog in catalogs.items():(ROOT/f'locales/{language}.json').write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n')
(ROOT/'locales/context.json').write_text(json.dumps(context,indent=2)+'\n');path.write_text(json.dumps(monster,indent=2)+'\n')
print(f'Created {path.relative_to(ROOT)} and bilingual text. Replace the example art/stats, then run make test && make.')
