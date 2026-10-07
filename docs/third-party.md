# Third-party dependencies

The game's source, setting data, pixels and original font are MIT. The concept images shown during planning are not embedded in this ROM.

- GBDK-2020 4.5.0: official release at https://github.com/gbdk-2020/gbdk-2020 . Linked GBDK library is GPLv2 with the linking exception reproduced in GBDK-LICENSE.txt. The compiler tools have their own upstream licenses distributed in the release's licenses directory. The local toolchain is not committed.
- PyBoy 2.6.1: LGPL-3.0, https://github.com/Baekalfen/PyBoy . Used only for emulator verification and screenshot capture, not embedded in the game.
- Pillow: HPND, https://github.com/python-pillow/Pillow . Used only for nearest-neighbor screenshot magnification.

Original font bitmap definitions are in assets/font.json. The project does not redistribute system fonts, commercial game graphics or third-party characters.
