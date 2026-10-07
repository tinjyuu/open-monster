# Original Game Boy score

[English](music.md) · [日本語](ja/music.md)

The two original eight-bar loops are authored as editable MIDI note numbers, chord roots, tuning and waveform data in `src/music.c`. Code and musical material are MIT, composed with Codex for Open Monster. No commercial melodies or recorded samples are used.

| Track | Scene | Tempo |
| --- | --- | --- |
| Trail Lanterns | Title, camp and exploration | About 112 BPM |
| Meet a Challenger | Battle and its auxiliary menus | About 149 BPM |

The four built-in channels provide pulse melody, pulse arpeggio, a triangle wave bass and quiet noise percussion. The driver follows elapsed VBlank ticks so rendering does not freeze the musical clock. SELECT during play mutes/resumes; title SELECT still changes language. Mute is a session preference and does not change save v1.

The driver and score occupy ROM bank 2. The existing flat engine/data occupy banks 0/1; GBDK bank-call helpers and startup sections are explicitly placed in fixed bank zero. `scripts/check_rom_layout.py` rejects overlaps and base-data overflow, including contributor builds. `video_init()` selects bank 1 before banked calls, ensuring the flat data bank is restored correctly. ROM size is 128 KiB; save RAM remains 8 KiB.

`tests/flicker_audio_test.py` checks real button-driven movement, zero LCD-off calls after boot, zero fully white frames, audible exploration/battle tracks and mute/resume. It exports an actual APU recording as `dist/music-preview.wav`. `scripts/record_gameplay.py` includes actual emulator audio in the MP4 and aligns its native clock with 30 fps video without changing gameplay state.
