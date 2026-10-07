# Contributing to Open Monster

[English](CONTRIBUTING.md) · [日本語](CONTRIBUTING.ja.md)

Ideas, sketches, pixel art, writing, balance, code and testing are all welcome. You do not need to finish a creature alone.

## Start with an idea

Open a monster idea Issue. Describe its personality, habitat, fighting style and weaknesses. Attach a sketch if you have one, and explain where you would like help.

Agree on roles and credit in the Issue. Record idea, art and data authors in `authors`; add the same credits to CREDITS.md when merging.

## Add a playable species

1. Fork the repository and create a branch.
2. Run the helper to scaffold a monster and both language catalogs:

```sh
python3 scripts/new_monster.py sample \
  --name-en SAMPLE --name-ja サンプル --author 'Your name'
```

3. Replace the example pixel art, palette, habitat text, stats and move choices. Follow the [format](docs/monster-format.md).
4. Run `make test && make`, then test encountering, scouting, training and battling your creature.
5. Open a PR with screenshots, credits, design intent and verification results. State separately whether you tested real hardware.

The helper creates source data, not a new original design. New species enter the wild encounter pool automatically. The party limit is six; the data validation cap is 24, also subject to actual ROM capacity.

## Review criteria

- Original work that its creators can share under MIT.
- Personality and habitat connected to its combat identity.
- Stats within the published ranges, with strengths and weaknesses.
- Valid data, readable art and text at 160×144, and reproducible testing.
- Complete, agreed credits and a willingness to refine the design together.

Project owner tinjyuu and initial maintainers review changes publicly. Votes or creator popularity alone do not determine adoption. New moves, traits or regions start with a separate design Issue.

Never reuse or renumber permanent species IDs or reorder move IDs. Removing baseline species or changing the save format needs an explicit migration design. Do not replace existing artwork without the applicable review.

## Translate

Edit `locales/en.json` or `locales/ja.json`, preserve keys, consult the context catalog, and run `python3 scripts/check_locales.py`. Documentation translations use a language selector at the top. See [Localization](docs/localization.md).

Be respectful. Harassment, discrimination and personal attacks are not accepted. Report problems to a maintainer.
