# Open Monster v0.3.0 — Forest interface

The approved visual revision replaces the title and exploration scenery and carries the forest behind battle, training, party and loadout controls. Compact status bars and floating wood/gold command medallions show actions and selection at 160×144. Training and loadout accept left/right as well as up/down.

Buildings and the wordmark now preserve their underlying pixels exactly. All six monsters and command buttons use hardware sprite transparency. Static scenes reject palette conflicts, text spaces retain scenery, and health-bar tiles are protected from text composites. Editable original pixel sources and standard-library build scripts are included under MIT.

Native gameplay/save tests and 14 content/localization tests pass. English and Japanese button-driven acceptance runs complete scouting, training, switching, loss recovery, three CPU challenges and save restart/protection. A seventh JSON-only species can be built and recruited without engine edits.

Emulator transparency checks report zero differences: building 427 pixels, wordmark 3,355, title monsters 1,047, battle monsters/buttons 1,971, text spaces 1,213 and all six species in both directions 6,166. Scene palette roundtrip verifies 17,728 pixels. Peak battle objects per scanline: nine of ten. CI runs these checks and attaches the report.

The attached 88-second MP4 records the actual ROM, using only buttons without gameplay RAM edits, at 640×576 / 30 fps. It covers language selection, starters, camp training, equipping, exploration, battle, scouting, party and save. The prototype has no audio.

Save format remains v1 and v0.1/v0.2 saves stay compatible. **Chromatic and original Game Boy Color hardware remain untested.** Download `open-monster.gbc`; use a compatible writable, save-capable cartridge or GBC emulator.

日本語: 承認版の背景と重ねる操作UIを反映。建物・ロゴ・全6体・操作アイコンの透過検査は差分0、日英の通しプレイも成功。実際の更新ROMによる約88秒の動画を添付。セーブ形式はv1を維持し、実機は未検証です。
