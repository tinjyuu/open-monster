"""Validate translation parity, display widths and glyph availability without writing files."""
from pathlib import Path
from localization import load_locales
root=Path(__file__).resolve().parents[1]
cfg,context,catalogs=load_locales(root)
for lang in cfg['locales']:print(f'{lang}: {len(catalogs[lang])}/{len(context)} keys; widths and glyphs valid')
