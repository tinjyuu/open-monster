import importlib.util,json,shutil,tempfile,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('build_data',ROOT/'scripts/build_data.py');bd=importlib.util.module_from_spec(spec);spec.loader.exec_module(bd)
class ContentTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name);shutil.copytree(ROOT/'data',self.root/'data')
 def tearDown(self):self.tmp.cleanup()
 def sample(self,**changes):
  d=json.loads((self.root/'data/monsters/chapo.json').read_text());d.update(changes);return d
 def write(self,d,name='new'): (self.root/f'data/monsters/{name}.json').write_text(json.dumps(d,ensure_ascii=False))
 def test_seventh_monster_without_engine_change(self):
  self.write(self.sample(id='sample',save_id=6,starter=False,name='サンプル'));moves,mons=bd.content(self.root);self.assertEqual(len(mons),7);self.assertEqual(mons[-1]['save_id'],6)
 def test_duplicate_id_rejected(self):
  self.write(self.sample(save_id=6,starter=False));self.assertRaises(AssertionError,bd.content,self.root)
 def test_duplicate_save_id_rejected(self):
  self.write(self.sample(id='sample',starter=False));self.assertRaises(AssertionError,bd.content,self.root)
 def test_broken_pixels_rejected(self):
  self.write(self.sample(id='sample',save_id=6,starter=False,pixels=['xxx']));self.assertRaises(AssertionError,bd.content,self.root)
 def test_unknown_move_rejected(self):
  self.write(self.sample(id='sample',save_id=6,starter=False,base_moves=['missing','guard','dodge']));self.assertRaises(ValueError,bd.content,self.root)
 def test_extreme_stats_rejected(self):
  self.write(self.sample(id='sample',save_id=6,starter=False,stats=dict(hp=255,attack=10,defense=10,speed=10)));self.assertRaises(AssertionError,bd.content,self.root)
if __name__=='__main__':unittest.main()
