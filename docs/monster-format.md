# Monster data v1

[English](monster-format.md) · [日本語](ja/monster-format.md)

Ordinary species additions are content-only. Each JSON uses existing moves/traits and source pixel rows. All labels are localization keys.

| Field | Meaning |
| --- | --- |
| `id` | Unique lowercase identifier; starts with a letter, then letters, digits, `_` or `-` |
| `save_id` | Permanent integer 0–254. Baseline 0–5 are reserved; never renumber |
| `name`, `bio` | Keys such as `monsters.sample.name` and `monsters.sample.bio`, present in both catalogs and context |
| `starter` | `false` for additions; the first three baseline species remain starters |
| `stats` | Integer HP 24–40; attack/defense/speed 5–13; total at most 70 |
| `base_moves` | Three IDs: an attack/drain/pierce move, `guard`, `dodge` |
| `training` | Existing move ID for each of `attack`, `defense`, `technique` |
| `trait` | `steam`, `rain`, `warm`, `shell`, `spore` or `quick` |
| `palette` | Four `#RRGGBB` colors: background/transparent, outline, body, highlight |
| `pixels` | 16×16 or native 32×32 rows. `.` background, `1` outline, `2` body, `3` highlight |
| `authors` | Proposal, pixel and content creator names |

## Example

```sh
python3 scripts/new_monster.py sample \
  --name-en SAMPLE --name-ja サンプル \
  --bio-en 'A TRAVELING TEAPOT' --bio-ja 'タビヲ スル キュウス' \
  --author 'Your name'
make test && make
```

The helper scaffolds the example art and data from Chapo, assigns an unused save ID and creates complete catalog entries. Replace the example with your own design before proposing it. The new species becomes a wild encounter candidate automatically; no engine edit is needed.

The battle uses 32×32 hardware sprites with color index zero transparent. Existing 16×16 art is enlarged at an integer scale. Native 32×32 art is rendered directly; the field companion uses a 16×16 sampling. Review both sizes. Palette colors are quantized to GBC 5-bit RGB.

## Moves and traits

Move IDs and their order are save ABI; do not reorder `data/moves.json`. Existing twelve moves cover attacks, guard, dodge, recovery, speed, focus, pierce and drain.

Traits: steam gives pierce +2; rain gives Rain +2; warm gives healing +3; shell reduces damage by one; spore gives drain healing +2; quick gives action speed +3. Training gives its selected stat +2 and learns the corresponding move. Changing styles retains learned moves; equip up to three at camp.

Validation currently caps species data at 24 and checks baseline IDs, bounds, localization, palettes and square pixel rows. Actual ROM capacity is also a limit. Full credits live in source and CREDITS; the game points to GitHub for them.
