import json,shutil,tempfile,unittest,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'scripts'))
from localization import load_locales
class LocaleTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name)
  for f in ['locales','assets']:shutil.copytree(ROOT/f,self.root/f)
 def tearDown(self):self.tmp.cleanup()
 def change(self,key,value):
  p=self.root/'locales/en.json';d=json.loads(p.read_text())
  if value is None:del d[key]
  else:d[key]=value
  p.write_text(json.dumps(d))
 def test_complete_catalogs(self):
  cfg,ctx,cat=load_locales(self.root);self.assertEqual(set(cat['en']),set(cat['ja']));self.assertGreater(len(ctx),100)
 def test_missing_translation(self):
  self.change('menu.train',None);self.assertRaises(AssertionError,load_locales,self.root)
 def test_screen_overflow(self):
  self.change('battle.scout','THIS LABEL IS TOO LONG');self.assertRaises(AssertionError,load_locales,self.root)
 def test_unsupported_glyph(self):
  self.change('menu.train','train');self.assertRaises(AssertionError,load_locales,self.root)
 def test_empty_translation(self):
  self.change('menu.train','');self.assertRaises(AssertionError,load_locales,self.root)
if __name__=='__main__':unittest.main()
