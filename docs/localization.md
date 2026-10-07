# Localization

[English](localization.md) · [日本語](ja/localization.md)

## Architecture

The game uses keyed, flat JSON translation catalogs, not Japanese strings embedded in gameplay code. The source catalog is English. Each `MSG_*` symbol in C refers to one stable semantic key such as `battle.next` or `training.choose`.

`make` validates the catalogs and compiles them into const ROM tables. The handheld reads those tables directly; it does not parse JSON or run a web localization library. Species and moves store translation keys, so their labels use the selected language too.

`locales/config.json` currently fixes the persistent indexes **ja=0, en=1**, with English as the default for new games. SELECT switches languages at the title; START opens credits. The choice is stored in previously reserved byte 10 of save v1. Existing v0.1 files have byte 10 set to zero and therefore load in Japanese. Unknown language indexes are protected rather than overwritten.

## Translating text

1. Edit values in `locales/en.json` or `locales/ja.json`; retain the semantic keys.
2. Read `locales/context.json` for context and the field's display limit.
3. Use the original font's supported characters. English game text currently uses uppercase Latin letters; Japanese uses katakana. Repository prose uses normal English and Japanese.
4. Run `python3 scripts/check_locales.py` and `make test && make`.
5. Check the actual 160×144 screen, and include screenshots with your PR.

The validator rejects missing/extra keys, empty strings, unsupported glyphs and labels that exceed their cell budget. Cards allow six cells, species names eight, biographies eighteen and full screen rows twenty. A translation fitting its limit still needs visual review.

Do not rename keys just because wording changes. Add context when a short label is ambiguous. Never silently reuse an existing key for a different meaning.

## Optional Weblate integration

This repository uses the flat JSON format supported by [Weblate](https://github.com/WeblateOrg/weblate/blob/main/docs/formats/json.rst). A future maintainer can configure a component with:

- Repository: `https://github.com/tinjyuu/open-monster.git`, branch `main`.
- File mask: `locales/*.json`, excluding `config.json` and `context.json`.
- Base file: `locales/en.json`; language filter: English and Japanese.
- Format: JSON file.
- Contributions through reviewed pull requests; run the existing CI before merging.

These are setup instructions. No hosted Weblate project, account, token or webhook has been created. Context and hardware limits remain in the repository even if an external editor is used.

## More languages and documentation

Adding another catalog requires a font/VRAM budget, UI layout and persistent-index review before enabling it. The current C interface and validator deliberately support only Japanese and English. There is no promise that an arbitrary language can be enabled by adding a JSON file alone.

English documentation lives at the root and under `docs/`. Japanese uses README.ja.md, CONTRIBUTING.ja.md and `docs/ja/`. Each document links directly to its counterpart. The build, data, save and release instructions should remain consistent across languages; translation and technical changes are reviewed together.
