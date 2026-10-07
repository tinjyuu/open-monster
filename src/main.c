#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include <string.h>
#include "core.h"
#include "generated.h"
#define L(id) locale_text[game.language][id]
#define TITLE 0
#define STARTER 1
#define WORLD 2
#define BASE 3
#define TRAIN 4
#define EQUIP 5
#define MOVEPICK 6
#define PARTY 7
#define BATTLE 8
#define AUX 9
#define MESSAGE 10
#define CONFIRM_NEW 11
#define ABOUT 12
Game game;
Fighter allies[PARTY_MAX],enemy;
uint8_t screen[360],attrs[360],savebuf[SAVE_BYTES];
uint8_t state=TITLE,cursor=0,prev=0,return_state=WORLD;
uint8_t save_status,save_allowed,enemy_intent,trainer_id=255,battle_party=0;
uint8_t equip_slot=0,grass_steps=0,last_x=3,last_y=6;
const char *message1="",*message2="";
const uint8_t tx[]={15,16,15},ty[]={3,7,11};
const palette_color_t palettes[]={
 RGB(31,29,23),RGB(5,4,8),RGB(9,24,20),RGB(31,30,26),
 RGB(31,29,23),RGB(5,10,9),RGB(10,20,13),RGB(25,26,16),
 RGB(31,29,23),RGB(4,9,15),RGB(7,17,23),RGB(16,27,29),
 RGB(31,29,23),RGB(10,4,8),RGB(28,14,9),RGB(31,25,13),
 RGB(31,29,23),RGB(5,4,8),RGB(9,24,20),RGB(31,30,26),
 RGB(31,29,23),RGB(5,4,8),RGB(22,16,27),RGB(31,30,26),
 RGB(9,24,20),RGB(4,5,8),RGB(26,28,19),RGB(31,30,26),
 RGB(31,29,23),RGB(5,4,8),RGB(9,24,20),RGB(31,30,26)
};
void cell(uint8_t x,uint8_t y,uint8_t tile,uint8_t pal){if(x<20&&y<18){uint16_t i=(uint16_t)y*20+x;screen[i]=tile;attrs[i]=pal;}}
void rect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t t,uint8_t p){uint8_t a,b;for(b=0;b<h;b++)for(a=0;a<w;a++)cell(x+a,y+b,t,p);}
uint8_t glyph(uint16_t c){uint8_t i;for(i=0;i<font_count;i++)if(font_codes[i]==c)return i;return 0;}
void text(uint8_t x,uint8_t y,const char*s,uint8_t p){uint16_t c;while(*s&&x<20){uint8_t b=(uint8_t)*s++;if(b<128)c=b;else if((b&0xf0)==0xe0){c=((uint16_t)(b&15)<<12);c|=((uint16_t)((uint8_t)*s++&63)<<6);c|=((uint8_t)*s++&63);}else c='?';cell(x++,y,glyph(c),p);}}
void number(uint8_t x,uint8_t y,uint8_t n,uint8_t p){char s[4];if(n>=100){s[0]='0'+n/100;s[1]='0'+n/10%10;s[2]='0'+n%10;s[3]=0;}else if(n>=10){s[0]='0'+n/10;s[1]='0'+n%10;s[2]=0;}else{s[0]='0'+n;s[1]=0;}text(x,y,s,p);}
void hint(const char*s){text(0,17,s,0);}
void art_at(uint8_t x,uint8_t y,uint8_t start,uint8_t pal){uint8_t a,b;for(b=0;b<4;b++)for(a=0;a<4;a++)cell(x+a,y+b,start+b*4+a,pal);}
void frame(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t pal){uint8_t i;for(i=1;i<w-1;i++){cell(x+i,y,172,pal);cell(x+i,y+h-1,173,pal);}for(i=1;i<h-1;i++){cell(x,y+i,174,pal);cell(x+w-1,y+i,175,pal);}cell(x,y,176,pal);cell(x+w-1,y,177,pal);cell(x,y+h-1,178,pal);cell(x+w-1,y+h-1,179,pal);}
void load_art(uint8_t s,uint8_t second){palette_color_t pal[4];memcpy(pal,species[s].palette,sizeof(pal));pal[0]=palettes[0];set_bkg_data(second?208:192,16,species[s].art);set_bkg_palette(second?5:4,1,pal);}
void clear_sprites(void){uint8_t i;for(i=0;i<8;i++)move_sprite(i,0,0);}
void portrait(uint8_t s){load_art(s,0);art_at(8,4,192,4);}
void show_message(const char*a,const char*b,uint8_t back){message1=a;message2=b;return_state=back;state=MESSAGE;cursor=0;}
void save_game(void){uint8_t i;volatile uint8_t *ram=(volatile uint8_t*)0xa000;
 if(!save_allowed){show_message(L(MSG_SAVE_PROTECTED),L(MSG_SAVE_OTHER_FILE),WORLD);return;}
 encode_save(&game,savebuf);ENABLE_RAM;for(i=0;i<SAVE_BYTES;i++)ram[i]=savebuf[i];DISABLE_RAM;
 show_message(L(MSG_SAVE_SUCCESS),L(MSG_HINT_SAVE),WORLD);
}
void load_game(void){uint8_t i;volatile uint8_t *ram=(volatile uint8_t*)0xa000;ENABLE_RAM;for(i=0;i<SAVE_BYTES;i++)savebuf[i]=ram[i];DISABLE_RAM;save_status=decode_save(&game,savebuf);save_allowed=(save_status==SAVE_OK||save_status==SAVE_EMPTY);if(save_status!=SAVE_OK)game.language=DEFAULT_LOCALE;}
uint8_t terrain(uint8_t x,uint8_t y){
 if(x==0||x==19||y==0||y==13)return 162;
 if(x==5 && y>2&&y<12&&y!=7)return 163;
 if(x>=8&&x<=12&&y>=5&&y<=10)return 164;
 if(x==3&&y==4)return 165;
 if((x==15&&y==3)||(x==16&&y==7)||(x==15&&y==11))return 166;
 if((x==7&&y==3)||(x==14&&y==10))return 167;
 if(y==7||x==3||x==15)return 161;
 return 160;
}
uint8_t near(uint8_t x,uint8_t y){int8_t dx=game.x-x,dy=game.y-y;return dx>=-1&&dx<=1&&dy>=-1&&dy<=1;}
void map_draw(void){uint8_t x,y,t,i,s=game.party[game.active].species;
 text(0,0,"OPEN MONSTER",0);text(14,0,"LV",0);number(17,0,game.party[game.active].level,0);
 for(y=0;y<14;y++)for(x=0;x<20;x++){t=terrain(x,y);cell(x,y+2,t,t==163?2:(t==165||t==166?3:1));}
 set_sprite_data(0,4,player_tiles);set_sprite_data(4,4,species[s].small);set_sprite_palette(0,1,palettes);set_sprite_palette(1,1,species[s].palette);
 for(i=0;i<4;i++){set_sprite_tile(i,i);set_sprite_prop(i,0);move_sprite(i,game.x*8+8+(i%2)*8,game.y*8+32+(i/2)*8);set_sprite_tile(i+4,i+4);set_sprite_prop(i+4,1);move_sprite(i+4,last_x*8+8+(i%2)*8,last_y*8+32+(i/2)*8);}
 if(near(3,4))text(0,16,L(MSG_MAP_BASE),0);
 else if(near(15,3))text(0,16,L(MSG_MAP_TRAINER_ATTACK),0);
 else if(near(16,7))text(0,16,L(MSG_MAP_TRAINER_DEFENSE),0);
 else if(near(15,11))text(0,16,L(MSG_MAP_TRAINER_SPEED),0);
 else if(terrain(game.x,game.y)==164)text(0,16,L(MSG_MAP_SEARCH),0);
 else text(0,16,L(MSG_MAP_GRASS),0);
 hint(L(MSG_HINT_WORLD));
}
void hp_bar(uint8_t x,uint8_t y,const Fighter*f,uint8_t pal){uint8_t i,n=(uint16_t)f->hp*6/max_hp(&f->mon);for(i=0;i<6;i++)cell(x+i,y,i<n?170:171,pal);}
void battle_draw(void){palette_color_t p4[4],p5[4];uint8_t i;Fighter*a=&allies[game.active];const char*labels[]={L(MSG_BATTLE_SWITCH),L(MSG_BATTLE_SCOUT),L(MSG_BATTLE_RUN)};
 load_art(a->mon.species,0);load_art(enemy.mon.species,1);memcpy(p4,species[a->mon.species].palette,sizeof(p4));memcpy(p5,species[enemy.mon.species].palette,sizeof(p5));p4[0]=p5[0]=palettes[6];set_bkg_palette(4,1,p4);set_bkg_palette(5,1,p5);
 text(0,0,species[a->mon.species].name[game.language],0);text(11,0,species[enemy.mon.species].name[game.language],0);
 hp_bar(0,1,a,4);hp_bar(13,1,&enemy,5);text(8,1,"VS",0);
 number(0,2,a->hp,0);text(3,2,"/",0);number(4,2,max_hp(&a->mon),0);number(13,2,enemy.hp,0);
 rect(0,4,20,7,160,1);for(i=0;i<20;i++){cell(i,4,162,1);cell(i,10,161,1);}
 art_at(2,6,192,4);for(i=0;i<16;i++)cell(14+3-i%4,6+i/4,208+i,5|32);
 text(0,11,L(MSG_BATTLE_NEXT),0);text(6,11,moves[enemy_intent].name[game.language],0);
 if(a->last_dodge)text(13,11,L(MSG_BATTLE_DODGE_LOCKED),0);
 for(i=0;i<3;i++){
  uint8_t x=i*7,p=(cursor==i?6:0);rect(x,12,6,5,0,p);frame(x,12,6,5,p);
  if(state==AUX){text(x+1,14,labels[i],p);}else{
   uint8_t mid=a->mon.moves[i],ic=moves[mid].effect==FX_GUARD?1:(moves[mid].effect==FX_DODGE?2:0);cell(x+2,12,128+ic*4,p);cell(x+3,12,129+ic*4,p);cell(x+2,13,130+ic*4,p);cell(x+3,13,131+ic*4,p);text(x,14,moves[mid].name[game.language],p);number(x+1,15,moves[mid].power,p);
   if(moves[mid].effect==FX_GUARD)text(x,15,"1/2",p);
   if(moves[mid].effect==FX_DODGE)text(x,15,a->last_dodge?"X":L(MSG_BATTLE_EVADE),p);
  }
 }
 hint(state==AUX?L(MSG_HINT_BATTLE_AUX):L(MSG_HINT_BATTLE));
}
void render(void){uint8_t i,s;DISPLAY_OFF;clear_sprites();memset(screen,0,sizeof(screen));memset(attrs,0,sizeof(attrs));
 switch(state){
 case TITLE:
  text(3,2,"OPEN MONSTER",0);text(2,3,L(MSG_TITLE_NAME),0);portrait(0);
  text(1,10,L(MSG_TITLE_TAGLINE1),0);text(1,11,L(MSG_TITLE_TAGLINE2),0);
  text(1,13,save_status==SAVE_OK?L(MSG_TITLE_CONTINUE):L(MSG_TITLE_START),0);
  text(1,14,L(MSG_TITLE_NEW),0);
  if(!save_allowed){text(0,15,save_status==SAVE_INCOMPATIBLE?L(MSG_SAVE_WRONG_VERSION):L(MSG_SAVE_CORRUPT),0);text(0,16,L(MSG_SAVE_PLAY_WITHOUT),0);}
  hint(L(MSG_TITLE_CONTROLS));break;
 case CONFIRM_NEW:text(0,4,L(MSG_NEW_CONFIRM),0);text(0,7,L(MSG_NEW_WARNING1),0);text(0,8,L(MSG_NEW_WARNING2),0);hint(L(MSG_HINT_NEW));break;
 case STARTER:
  text(0,1,L(MSG_STARTER_CHOOSE),0);s=cursor;portrait(s);text(6,10,species[s].name[game.language],0);text(0,12,species[s].bio[game.language],0);text(0,14,L(MSG_STARTER_ARROWS),0);hint(L(MSG_HINT_STARTER));break;
 case WORLD:map_draw();break;
 case BASE:{const char*opt[]={L(MSG_MENU_TRAIN),L(MSG_MENU_LOADOUT),L(MSG_MENU_PARTY),L(MSG_MENU_REST),L(MSG_MENU_SAVE),L(MSG_MENU_LEAVE)};text(0,1,L(MSG_BASE_NAME),0);text(0,3,species[game.party[game.active].species].name[game.language],0);text(13,3,"LV",0);number(16,3,game.party[game.active].level,0);for(i=0;i<6;i++){text(1,5+i*2,opt[i],0);if(i==cursor)text(0,5+i*2,">",0);}hint(L(MSG_HINT_MENU));break;}
 case TRAIN:{const char*style[]={L(MSG_TRAINING_ATTACK),L(MSG_TRAINING_DEFENSE),L(MSG_TRAINING_TECHNIQUE)};Monster*m=&game.party[game.active];text(0,1,L(MSG_TRAINING_CHOOSE),0);for(i=0;i<3;i++){text(2,4+i*3,style[i],i==cursor?6:0);text(10,4+i*3,moves[species[m->species].training[i]].name[game.language],0);}text(0,14,L(MSG_TRAINING_EFFECT),0);text(0,15,L(MSG_TRAINING_CHANGE),0);hint(L(MSG_HINT_TRAINING));break;}
 case EQUIP:text(0,1,L(MSG_LOADOUT_SLOT),0);for(i=0;i<3;i++)text(1,5+i*3,moves[game.party[game.active].moves[i]].name[game.language],i==cursor?6:0);hint(L(MSG_HINT_LOADOUT));break;
 case MOVEPICK:text(0,1,L(MSG_LOADOUT_KNOWN),0);for(i=0;i<6;i++){s=known_move(&game.party[game.active],i);if(s!=255)text(1,4+i*2,moves[s].name[game.language],i==cursor?6:0);}hint(L(MSG_HINT_EQUIP));break;
 case PARTY:text(0,1,battle_party?L(MSG_PARTY_SWITCH):L(MSG_PARTY_CHOOSE),0);for(i=0;i<game.count;i++){text(1,3+i*2,species[game.party[i].species].name[game.language],i==cursor?6:0);text(11,3+i*2,"LV",0);number(14,3+i*2,game.party[i].level,0);if(battle_party)number(17,3+i*2,allies[i].hp,0);}hint(L(MSG_HINT_PARTY));break;
 case BATTLE:case AUX:battle_draw();break;
 case MESSAGE:text(0,6,message1,0);text(0,9,message2,0);hint(L(MSG_HINT_CONTINUE));break;
 case ABOUT:text(0,1,"OPEN MONSTER",0);text(0,4,"MINNA DE TSUKURU SEKAI",0);text(0,7,"TINJYUU/OPEN-MONSTER",0);text(0,10,"CODE + PIXELS: MIT",0);text(0,13,L(MSG_ABOUT_CREDITS),0);hint(L(MSG_HINT_BACK));break;
 }
 VBK_REG=0;set_bkg_tiles(0,0,20,18,screen);VBK_REG=1;set_bkg_tiles(0,0,20,18,attrs);VBK_REG=0;SHOW_BKG;SHOW_SPRITES;DISPLAY_ON;
}
void start_battle(uint8_t trainer){uint8_t i,s,l;Monster m;trainer_id=trainer;cursor=0;state=BATTLE;for(i=0;i<game.count;i++)init_fighter(&allies[i],&game.party[i]);
 if(trainer==255){s=random_byte(&game)%species_count;l=game.party[game.active].level;if(l>1&&random_byte(&game)<128)l--;}
 else{s=trainer+3;l=trainer+2;}
 init_mon(&m,s,l);
 if(trainer!=255){train_mon(&m,trainer);m.moves[0]=species[s].training[trainer];if(moves[m.moves[0]].effect!=FX_ATTACK&&moves[m.moves[0]].effect!=FX_PIERCE&&moves[m.moves[0]].effect!=FX_DRAIN)m.moves[0]=species[s].base[0];}
 init_fighter(&enemy,&m);enemy_intent=enemy_move(&game,&enemy,trainer);
}
void battle_end_check(void){uint8_t i;
 if(!enemy.hp){reward(&game.party[game.active],trainer_id==255?10:24);if(trainer_id!=255){game.badges|=1<<trainer_id;if(game.badges==7)show_message(L(MSG_VICTORY_ALL),L(MSG_VICTORY_JOURNEY),WORLD);else show_message(L(MSG_VICTORY_BADGE),L(MSG_VICTORY_GROWTH),WORLD);}else show_message(L(MSG_VICTORY_WILD),L(MSG_VICTORY_GROWTH),WORLD);return;}
 if(!allies[game.active].hp){for(i=0;i<game.count;i++)if(allies[i].hp){game.active=i;break;}if(i==game.count){game.x=3;game.y=5;last_x=3;last_y=6;show_message(L(MSG_REST_CAMP),L(MSG_REST_PARTY),WORLD);return;}}
 enemy_intent=enemy_move(&game,&enemy,trainer_id);state=BATTLE;cursor=0;
}
void enemy_free_turn(void){Fighter*a=&allies[game.active];a->guard=a->dodge=a->last_dodge=0;enemy.guard=enemy.dodge=0;use_move(&enemy,a,enemy_intent);enemy.last_dodge=enemy.dodge;battle_end_check();}
void move_world(int8_t dx,int8_t dy){uint8_t nx=game.x+dx,ny=game.y+dy,t;if(nx<1||nx>18||ny<1||ny>12)return;t=terrain(nx,ny);if(t==163||t==165||t==166)return;last_x=game.x;last_y=game.y;game.x=nx;game.y=ny;if(t==164 && ++grass_steps>=8){grass_steps=0;start_battle(255);}}
void input(uint8_t k){uint8_t i,n,r;Monster*m=&game.party[game.active];
 if(state==MESSAGE){if(k&(J_A|J_B)){state=return_state;cursor=0;}return;}
 if(state==ABOUT){if(k&(J_A|J_B))state=TITLE;return;}
 if(state==TITLE){if(k&J_SELECT){game.language=(game.language+1)%LOCALE_COUNT;return;}if(k&J_START){state=ABOUT;return;}if(k&J_A){state=save_status==SAVE_OK?WORLD:STARTER;cursor=0;return;}if(k&J_B){state=save_status==SAVE_OK?CONFIRM_NEW:STARTER;cursor=0;}return;}
 if(state==CONFIRM_NEW){if(k&J_A){state=STARTER;cursor=0;}if(k&J_B)state=TITLE;return;}
 if(state==STARTER){if(k&J_LEFT)cursor=(cursor+2)%3;if(k&J_RIGHT)cursor=(cursor+1)%3;if(k&J_B)state=TITLE;if(k&J_A){n=game.language;new_game(&game,cursor);game.language=n;last_x=3;last_y=6;show_message(L(MSG_JOURNEY_BEGIN),L(MSG_HINT_JOURNEY),WORLD);}return;}
 if(state==WORLD){if(k&J_START){save_game();return;}if(k&J_B){battle_party=0;state=PARTY;cursor=game.active;return;}if(k&J_LEFT)move_world(-1,0);else if(k&J_RIGHT)move_world(1,0);else if(k&J_UP)move_world(0,-1);else if(k&J_DOWN)move_world(0,1);
 if(k&J_A){if(near(3,4)){state=BASE;cursor=0;}else{for(i=0;i<3;i++)if(near(tx[i],ty[i])){start_battle(i);return;}if(terrain(game.x,game.y)==164)start_battle(255);}}return;}
 if(state==BASE){if(k&J_UP)cursor=(cursor+5)%6;if(k&J_DOWN)cursor=(cursor+1)%6;if(k&J_B)state=WORLD;if(k&J_A){switch(cursor){case 0:state=TRAIN;break;case 1:state=EQUIP;break;case 2:battle_party=0;state=PARTY;break;case 3:show_message(L(MSG_REST_PARTY),L(MSG_REST_BEFORE),BASE);break;case 4:save_game();break;default:state=WORLD;}cursor=0;}return;}
 if(state==TRAIN){if(k&J_UP)cursor=(cursor+2)%3;if(k&J_DOWN)cursor=(cursor+1)%3;if(k&J_B){state=BASE;cursor=0;}if(k&J_A){train_mon(m,cursor);show_message(L(MSG_TRAINING_DONE),L(MSG_TRAINING_EQUIP),BASE);}return;}
 if(state==EQUIP){if(k&J_UP)cursor=(cursor+2)%3;if(k&J_DOWN)cursor=(cursor+1)%3;if(k&J_B){state=BASE;cursor=0;}if(k&J_A){equip_slot=cursor;state=MOVEPICK;cursor=0;}return;}
 if(state==MOVEPICK){n=3;for(i=0;i<3;i++)if(m->known&(1<<i))n++;if(k&J_UP)cursor=(cursor+n-1)%n;if(k&J_DOWN)cursor=(cursor+1)%n;if(k&J_B){state=EQUIP;cursor=equip_slot;}if(k&J_A){r=known_move(m,cursor);for(i=0;i<3;i++)if(i!=equip_slot&&m->moves[i]==r){show_message(L(MSG_LOADOUT_DUPLICATE),L(MSG_LOADOUT_OTHER),EQUIP);return;}m->moves[equip_slot]=r;state=EQUIP;cursor=equip_slot;}return;}
 if(state==PARTY){if(k&J_UP)cursor=(cursor+game.count-1)%game.count;if(k&J_DOWN)cursor=(cursor+1)%game.count;if(k&J_B){state=battle_party?AUX:WORLD;cursor=0;}if(k&J_A){if(battle_party){if(cursor==game.active||!allies[cursor].hp){show_message(L(MSG_PARTY_INVALID),L(MSG_PARTY_OTHER),PARTY);return;}game.active=cursor;enemy_free_turn();}else{game.active=cursor;state=WORLD;}}return;}
 if(state==BATTLE||state==AUX){if(k&J_LEFT)cursor=(cursor+2)%3;if(k&J_RIGHT)cursor=(cursor+1)%3;if(k&J_B){state=state==BATTLE?AUX:BATTLE;cursor=0;return;}if(k&J_A){
 if(state==BATTLE){i=allies[game.active].mon.moves[cursor];if(moves[i].effect==FX_DODGE&&allies[game.active].last_dodge){show_message(L(MSG_BATTLE_COOLDOWN),L(MSG_LOADOUT_OTHER),BATTLE);return;}resolve_turn(&allies[game.active],&enemy,i,enemy_intent);battle_end_check();}
 else if(cursor==0){battle_party=1;state=PARTY;cursor=game.active;}
 else if(cursor==2)show_message(L(MSG_BATTLE_LEFT),L(MSG_BATTLE_TRAIN_TIP),WORLD);
 else if(trainer_id!=255)show_message(L(MSG_SCOUT_TRAINER),L(MSG_SCOUT_WILD_ONLY),AUX);
 else{r=scout(&game,&enemy);if(r==1)show_message(L(MSG_SCOUT_SUCCESS),L(MSG_SCOUT_PARTY_HINT),WORLD);else if(r==2)show_message(L(MSG_SCOUT_FULL),L(MSG_SCOUT_UNAVAILABLE),AUX);else if(r==3)show_message(L(MSG_SCOUT_DUPLICATE),L(MSG_SCOUT_OTHER),AUX);else{enemy_free_turn();if(state==BATTLE)show_message(L(MSG_SCOUT_FAILED),L(MSG_SCOUT_WEAKEN),BATTLE);}}
 }return;}
}
void main(void){uint8_t k,held,repeat=0;DISPLAY_OFF;SPRITES_8x8;HIDE_WIN;set_bkg_palette(0,8,palettes);set_bkg_data(0,font_count,font_tiles);set_bkg_data(128,12,action_icons);set_bkg_data(160,20,terrain_tiles);load_game();render();
 while(1){wait_vbl_done();held=joypad();k=held&~prev;if(held && held==prev && state==WORLD){if(++repeat>=10){k=held&(J_UP|J_DOWN|J_LEFT|J_RIGHT);repeat=6;}}else repeat=0;prev=held;if(k){input(k);render();}}
}
