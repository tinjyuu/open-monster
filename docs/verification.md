# Verification

[English](verification.md) · [日本語](ja/verification.md)

The ROM is built with GBDK 4.5.0. Native C tests cover all move effects, defense, dodge cooldown, piercing, focus, recovery, drain, training, progression, scouting and save validation. Content/localization tests reject invalid IDs, stats, pixels, missing translations, unsupported glyphs and overflowing text.

PyBoy 2.6.1 acceptance runs use actual button input, without editing gameplay RAM. English and Japanese both cover:

- Boot, title language selection, starter selection, exploration and camp.
- Training and equipping a learned move.
- Wild battle, fleeing, scouting and party switching.
- Losing to a stronger trainer and recovering at camp.
- Progressing to level five and defeating all three CPU trainers.
- SRAM persistence across emulator restarts.
- Unsupported/corrupt saves remaining unchanged after a save attempt.

`tests/addition_test.py` builds an isolated ROM with a seventh creature, then encounters and recruits it without editing engine source. The distributed game remains the six-species prototype.

```sh
make test && make
.tools/venv/bin/python tests/emulator_test.py
OPEN_MONSTER_LOCALE=ja .tools/venv/bin/python tests/emulator_test.py
.tools/venv/bin/python tests/addition_test.py
.tools/venv/bin/python tests/transparency_test.py
```

Actual ROM frames are in `dist/*-en-160.png` and `*-ja-160.png`; 4× previews use nearest-neighbor scaling. Results are recorded separately in `verification-en.json` and `verification-ja.json`. These are emulator frames, not generated concept art.

**Chromatic and original Game Boy Color hardware have not been tested.** A hardware pass still needs boot, palettes, controls, writable cartridge compatibility, saving across power cycles and completing the three challenges.

Prototype limitations: no audio, detailed battle animation, multiplayer, storage boxes, duplicate-species raising or full campaign. Every battle heals the party at its start. Six creatures can be held; data capacity is subject to validation and ROM budget. Saves remain v1; incompatible files are protected, with no automatic destructive migration.

Transparency checks cover the building, wordmark, text spaces, command sprites and all six species in both directions, with zero pixel differences. See [rendering](rendering.md) and `dist/alpha-verification.json`.
