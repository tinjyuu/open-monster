# Open Monster v0.3.1 — Continuous scenery and original music

Moving previously disabled and rebuilt the entire LCD, producing bright flashes on real Chromatic hardware. Field movement now updates only the explorer and companion sprites when the hint is unchanged. Full scene changes prepare the other GBC VRAM bank and hidden map, then commit at VBlank without turning off the LCD. The approved artwork and exact transparency remain intact.

Two original eight-bar BGM loops, Trail Lanterns and Meet a Challenger, use the built-in pulse, wave and noise channels. SELECT mutes/resumes during play; title SELECT still changes language. The editable score, original waveform and sound driver are MIT. The MP4 now contains actual emulator audio.

The ROM grows to 128 KiB to hold the sound driver in bank 2. Save v1 and the 8 KiB RAM declaration are unchanged. A ROM layout check protects fixed-bank helpers and prevents overlapping sections or contributor data overflow.

Verification: 5,108 button-driven frames, including 48 movement steps, produce zero LCD-off calls after boot and zero blank white frames. Both tracks emit audio; mute/resume passes. English/Japanese full-adventure acceptance, the seventh JSON-only creature and exact transparency checks pass.

The owner confirmed v0.3.0 booting on a Chromatic DevDay Edition and reported the movement flicker. This revision addresses that report. Full hardware gameplay and musical acceptance of v0.3.1 still require checking the device.

日本語: 移動時の全面点滅を修正し、探索・戦闘の独自BGMを追加。5,108フレームの検査でLCD停止0・全面白フレーム0。プレイ中のSELECTで音楽をミュート／再開できます。セーブ形式v1を維持し、音付きの実際のプレイ動画と曲の試聴を添付しています。
