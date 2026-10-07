#ifndef OPEN_MONSTER_MUSIC_H
#define OPEN_MONSTER_MUSIC_H
#include <gb/gb.h>
void music_init(void) BANKED;
void music_tick(uint8_t battle,uint8_t frame) BANKED;
void music_toggle(void) BANKED;
#endif
