#include "core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Monster mon(uint8_t s,uint8_t l){Monster m;init_mon(&m,s,l);return m;}
static Fighter f(uint8_t s){Monster m=mon(s,1);Fighter a;init_fighter(&a,&m);return a;}
static uint16_t sum(const uint8_t*p){uint16_t c=0x51a7;int i;for(i=0;i<SAVE_BYTES-2;i++)c=(uint16_t)((c<<1)|(c>>15))^p[i];return c;}
static void reseal(uint8_t*p){uint16_t c=sum(p);p[SAVE_BYTES-2]=c;p[SAVE_BYTES-1]=c>>8;}
int main(void){Fighter a,b,base;Game g,loaded;Monster m;uint8_t p[SAVE_BYTES],hit,i,result;uint16_t successes_hi=0,successes_low=0;
 a=f(0);b=f(1);base=b;use_move(&a,&b,0);hit=base.hp-b.hp;assert(hit>0);
 a=f(0);b=f(1);b.guard=1;use_move(&a,&b,0);assert(base.hp-b.hp==(hit+1)/2);
 a=f(0);b=f(1);b.dodge=1;assert(use_move(&a,&b,0)==1);assert(b.hp==base.hp);use_move(&a,&b,10);assert(b.hp<base.hp);
 a=f(0);b=f(1);resolve_turn(&a,&b,5,0);assert(a.hp==max_hp(&a.mon));assert(a.last_dodge);resolve_turn(&a,&b,5,0);assert(a.hp<max_hp(&a.mon));assert(!a.last_dodge);
 a=f(0);b=f(1);a.hp=1;use_move(&a,&b,6);assert(a.hp==8);a.hp=max_hp(&a.mon)-1;use_move(&a,&b,6);assert(a.hp==max_hp(&a.mon));
 a=f(0);b=f(1);use_move(&a,&b,9);assert(a.focused);use_move(&a,&b,0);assert(base.hp-b.hp==hit+6);assert(!a.focused);
 a=f(4);b=f(1);a.hp=5;use_move(&a,&b,11);assert(a.hp>5&&b.hp<base.hp);
 a=f(0);b=f(1);resolve_turn(&a,&b,4,0);assert(max_hp(&a.mon)-a.hp>0);assert(a.guard);
 for(i=0;i<move_count;i++){a=f(0);b=f(1);use_move(&a,&b,i);assert(a.hp<=max_hp(&a.mon));assert(b.hp<=max_hp(&b.mon));}
 m=mon(0,1);assert(train_mon(&m,2));assert(m.known==4);assert(known_move(&m,3)==10);assert(train_mon(&m,0));assert(m.known==5);assert(known_move(&m,3)==8);assert(known_move(&m,4)==10);assert(!train_mon(&m,3));reward(&m,255);assert(m.level==5&&m.xp==0);
 new_game(&g,0);b=f(1);for(i=0;i<250;i++){g.count=1;result=scout(&g,&b);if(result==1)successes_hi++;}b.hp=1;for(i=0;i<250;i++){g.count=1;result=scout(&g,&b);if(result==1)successes_low++;}assert(successes_low>successes_hi*2);
 g.count=6;assert(scout(&g,&b)==2);g.count=1;g.party[0].species=1;assert(scout(&g,&b)==3);
 new_game(&g,2);train_mon(&g.party[0],0);g.party[0].moves[0]=8;g.badges=7;encode_save(&g,p);assert(decode_save(&loaded,p)==SAVE_OK);assert(memcmp(&g,&loaded,sizeof(g))==0);
 p[2]=99;assert(decode_save(&loaded,p)==SAVE_INCOMPATIBLE);assert(memcmp(&g,&loaded,sizeof(g))==0);
 encode_save(&g,p);p[20]^=4;assert(decode_save(&loaded,p)==SAVE_CORRUPT);
 encode_save(&g,p);p[12]=250;reseal(p);assert(decode_save(&loaded,p)==SAVE_INCOMPATIBLE);
 encode_save(&g,p);p[13]=0;reseal(p);assert(decode_save(&loaded,p)==SAVE_CORRUPT);
 memset(p,255,sizeof(p));assert(decode_save(&loaded,p)==SAVE_EMPTY);memset(p,0,sizeof(p));assert(decode_save(&loaded,p)==SAVE_EMPTY);p[0]=1;assert(decode_save(&loaded,p)==SAVE_CORRUPT);
 puts("PASS: all 12 effects, guard, dodge cooldown, healing, focus, drain, training, scouting, progression, save protection");return 0;}
