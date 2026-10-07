/* Original music: Trail Lanterns / Meet a Challenger. MIT; see docs/music.md. */
#pragma bank 2
#include "music.h"
static uint8_t enabled,track,step,left,last_frame;
static const uint16_t tuning[]={44,157,263,363,457,547,631,711,786,856,923,986,1046,1102,1155,1205,1253,1297,1339,1379,1417,1452,1486,1517,1547,1575,1602,1627,1650,1673,1694,1714,1732,1750,1767,1783,1798,1812,1825,1837,1849,1860,1871,1881,1890,1899,1907,1915,1923};
static const uint8_t trail[]={76,79,0,81,79,76,74,72,76,0,81,79,76,74,72,0,77,81,79,77,76,72,0,74,74,79,81,79,77,74,71,0,72,76,79,0,81,79,76,74,77,0,81,84,81,79,77,76,74,77,81,79,77,74,72,0,71,74,79,77,74,71,72,0};
static const uint8_t challenge[]={72,0,75,79,80,79,75,74,72,75,80,0,79,77,75,72,74,77,82,80,77,74,72,0,71,74,79,0,77,74,71,0,72,75,79,84,82,79,75,74,72,0,75,80,79,75,72,0,74,77,82,0,80,77,74,72,71,74,79,83,79,77,74,71};
static const uint8_t trail_roots[]={48,45,41,43,48,41,38,43};
static const uint8_t challenge_roots[]={48,44,46,43,48,44,46,43};
static const uint16_t bass_tuning[]={263,363,457,547,631,711,786,856,923,986,1046};
static const uint8_t wave[]={0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10};
static void notes(void){uint8_t n,root,interval;uint16_t f;const uint8_t*melody=track?challenge:trail;
 n=melody[step];if(n){f=tuning[n-36];NR11_REG=0x80;NR12_REG=0x62;NR13_REG=(uint8_t)f;NR14_REG=0x80|(f>>8);}else NR12_REG=0;
 root=(track?challenge_roots:trail_roots)[step>>3];interval=(step&3)==1?7:((step&3)==2?12:0);f=tuning[root+interval-36];NR21_REG=0x40;NR22_REG=0x32;NR23_REG=(uint8_t)f;NR24_REG=0x80|(f>>8);
 if(!(step&3)){f=bass_tuning[root-38];NR31_REG=0;NR32_REG=0x60;NR33_REG=(uint8_t)f;NR34_REG=0x80|(f>>8);}
 if(!(step&1)){NR41_REG=0;NR42_REG=track?0x31:0x11;NR43_REG=(step&3)?0x12:0x34;NR44_REG=0x80;}
}
void music_init(void) BANKED{uint8_t i;enabled=1;track=255;step=0;left=0;last_frame=(uint8_t)sys_time;NR52_REG=0x80;NR50_REG=0x44;NR51_REG=0xff;NR10_REG=0;NR30_REG=0;for(i=0;i<16;i++)((volatile uint8_t*)0xff30)[i]=wave[i];NR30_REG=0x80;}
void music_toggle(void) BANKED{enabled^=1;NR30_REG=enabled?0x80:0;track=255;if(!enabled){NR12_REG=0;NR22_REG=0;NR32_REG=0;NR42_REG=0;}}
void music_tick(uint8_t battle,uint8_t frame) BANKED{uint8_t elapsed=frame-last_frame;last_frame=frame;if(!enabled)return;if(track!=battle){track=battle;step=0;left=track?12:16;notes();return;}if(elapsed<left){left-=elapsed;return;}elapsed-=left;left=track?12:16;step=(step+1)&63;while(elapsed>=left){elapsed-=left;step=(step+1)&63;}left-=elapsed;notes();}
