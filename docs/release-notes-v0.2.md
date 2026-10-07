# Open Monster v0.2.0 — Bilingual companions

## Pixel art

All six creatures now use native 32×32 battle art, with four-color palettes and 16×16 field companions. Chapo A and Amagumo A, followed by Tomori, Nejimai, Kasamo and TickTock, were reviewed and approved before integration. Battle backgrounds preserve the creature silhouettes without rectangular color patches.

## English and Japanese

- Runtime language selection: press SELECT at the title. English is the default for a fresh game; START opens credits.
- Menus, messages, species names, descriptions and moves are localized through 115 stable semantic JSON keys.
- Translation context and hardware text-width/glyph checks run in CI.
- English-first README, contribution guide, specifications and issue templates, with linked Japanese versions.
- A complete multilingual contribution helper creates a species and both catalogs without changing the engine.
- Save v1 remains compatible: old saves load in Japanese, and the selected language is now stored in the reserved field.

Catalogs are compatible with Weblate's flat JSON format. No external translation account, credentials or synchronization were configured.

## Gameplay recording

`open-monster-gameplay-v0.2.mp4` is an 88-second recording of the actual ROM in PyBoy: language switching, starter selection, training, loadout, exploration, cards, dodge cooldown, a failed and successful scout, party selection and saving. It uses button input only, without gameplay RAM edits. 640×576, 30 fps, no audio. The recording script is included in source.

## Verification and limitations

English and Japanese acceptance runs both complete the three trainer challenges, including loss recovery, scouting, switching and save persistence/protection. Native move/save tests and 14 content/localization tests pass. An isolated seventh-species ROM is built, encountered and scouted without engine changes.

**Chromatic and original GBC hardware remain untested.** The prototype has no audio, detailed battle animations, multiplayer or full campaign. Use a compatible writable, save-capable cartridge.

Download `open-monster.gbc` to play. Original code, text, font and pixel sources remain MIT.

---

日本語: 6体の承認済みドット絵を反映し、日本語／英語の切り替えと翻訳ファイル・CI検査を追加しました。タイトルのSELECTで言語変更、STARTでクレジット。約88秒の実際のプレイ動画も添付しています。両言語で通しテスト済み、実機は未検証です。
