# Rendering and transparency

[English](rendering.md) · [日本語](ja/rendering.md)

The approved v0.3 interface keeps the forest behind the title, exploration, battle, training and loadout controls. A compact HUD holds status; floating command medallions use wood and gold palettes to show selection. All artwork is original, with editable indexed pixels in `assets/ui-art.json` and `data/monsters/*.json`.

## Hardware implementation

Monsters and command medallions are genuine Game Boy OBJ sprites: color index zero is transparent. `scripts/sprite_codegen.py` packs 8×16 sprite pairs. Enemy mirroring reverses columns and flips each tile. Rendering hides all 40 objects before assigning a new scene. The battle uses at most nine objects on one scanline, within the hardware limit of ten.

Static buildings, trees, trainers, bridges and the wordmark are composited at build time by `scripts/scene_codegen.py`. A zero pixel preserves the lower layer. Every 8×8 tile must fit one actual four-color GBC palette exactly; the build rejects palette conflicts rather than approximating colors. Text composites preserve the scenery behind glyph spaces. Intentional opaque status/dialog panels remain separate from artwork transparency. HP tiles 254/255 are reserved outside the dynamic text pool.

Normal builds use only Python's standard library. Edit the JSON pixel sources directly; no image generator or raster editor is required to reproduce the ROM.

## Pixel verification

Run `tests/transparency_test.py` after building and the other acceptance tests. It compares actual emulator pixels with independent lower-layer reference images and checks all six monsters facing both directions. The test fixture uses production rendering functions. The test temporarily disables hardware sprites to inspect their background; gameplay acceptance and video recording use button inputs without gameplay RAM changes.

`dist/alpha-verification.json` records exact palette roundtrips, alpha checks and the scanline budget. Generated reference PPM files and the test-only ROM are ignored. Rebuilding resets the report to static checks, so run the transparency test last before distribution.

## Design references

The composition study used Nintendo's official screenshots of [The Minish Cap](https://www.nintendo.co.jp/event/e3/gbasoft/legendofzelda/index.html), [Golden Sun battles](https://www.nintendo.co.jp/n08/agsj/sento/index.html) and [Mario & Luigi RPG](https://www.nintendo.co.jp/n08/a88j/game/index.html): full scenery, compact edge status, contextual command icons, depth and shadows. No reference graphics are included in the game.

## Continuous display

Since v0.3.1, the LCD is disabled only during boot. Scene transitions prepare graphics in the other GBC VRAM bank and use the hidden tile map, then commit palettes, sprite mode, OAM and the visible map at VBlank. OAM DMA is paused while preparing sprites. Font tiles are initialized in both VRAM banks. Field movement updates only sprite coordinates while the context hint remains unchanged. This preserves the approved scene pixels while eliminating full-screen blanking on every step.
