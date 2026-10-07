#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include <string.h>
#include "core.h"
#include "generated.h"
#include "music.h"
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
uint8_t video_bank=0,video_sprite16=0,rendered_state=255,world_hint;
palette_color_t bg_colors[32],obj_colors[32];
void bg_palette(uint8_t first,uint8_t count,const palette_color_t*colors){memcpy(bg_colors+first*4,colors,count*8);}
void obj_palette(uint8_t first,uint8_t count,const palette_color_t*colors){memcpy(obj_colors+first*4,colors,count*8);}
const char *message1="",*message2="";
const uint8_t tx[]={15,16,15},ty[]={3,7,11};
const palette_color_t palettes[]={
 RGB(31,29,23),RGB(4,5,8),RGB(8,20,17),RGB(31,30,26),
 RGB(17,23,13),RGB(3,8,7),RGB(7,14,10),RGB(30,25,16),
 RGB(17,23,13),RGB(3,7,12),RGB(8,17,22),RGB(19,28,28),
 RGB(17,23,13),RGB(3,8,7),RGB(23,13,8),RGB(30,25,16),
 RGB(31,29,23),RGB(4,5,8),RGB(8,20,17),RGB(31,30,26),
 RGB(31,29,23),RGB(4,5,8),RGB(22,16,27),RGB(31,30,26),
 RGB(4,10,10),RGB(31,29,23),RGB(29,23,12),RGB(31,30,26),
 RGB(17,23,13),RGB(3,8,7),RGB(5,11,9),RGB(13,20,11)
};
const palette_color_t brand_palette[]={RGB(3,7,9),RGB(1,3,5),RGB(30,22,9),RGB(31,30,22)};
void cell(uint8_t x,uint8_t y,uint8_t tile,uint8_t pal){if(x<20&&y<18){uint16_t i=(uint16_t)y*20+x;screen[i]=tile;attrs[i]=pal;}}
void rect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t t,uint8_t p){uint8_t a,b;for(b=0;b<h;b++)for(a=0;a<w;a++)cell(x+a,y+b,t,p);}
uint8_t glyph(uint16_t c){uint8_t i;for(i=0;i<font_count;i++)if(font_codes[i]==c)return i;return 0;}
void text(uint8_t x,uint8_t y,const char*s,uint8_t p){uint16_t c;while(*s&&x<20){uint8_t b=(uint8_t)*s++;if(b<128)c=b;else if((b&0xf0)==0xe0){c=((uint16_t)(b&15)<<12);c|=((uint16_t)((uint8_t)*s++&63)<<6);c|=((uint8_t)*s++&63);}else c='?';cell(x++,y,glyph(c),p);}}
void number(uint8_t x,uint8_t y,uint8_t n,uint8_t p){char s[4];if(n>=100){s[0]='0'+n/100;s[1]='0'+n/10%10;s[2]='0'+n%10;s[3]=0;}else if(n>=10){s[0]='0'+n/10;s[1]='0'+n%10;s[2]=0;}else{s[0]='0'+n;s[1]=0;}text(x,y,s,p);}
void hint(const char*s){text(0,17,s,0);}
void frame(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t pal){uint8_t i,base=state==TITLE?228:172;for(i=1;i<w-1;i++){cell(x+i,y,base,pal);cell(x+i,y+h-1,base+1,pal);}for(i=1;i<h-1;i++){cell(x,y+i,base+2,pal);cell(x+w-1,y+i,base+3,pal);}cell(x,y,base+4,pal);cell(x+w-1,y,base+5,pal);cell(x,y+h-1,base+6,pal);cell(x+w-1,y+h-1,base+7,pal);}
void clear_sprites(void){uint8_t i;for(i=0;i<40;i++)move_sprite(i,0,0);}
void monster_obj(uint8_t s,uint8_t first,uint8_t tilebase,uint8_t x,uint8_t y,uint8_t flip){uint8_t i,col;video_sprite16=1;set_sprite_data(tilebase,16,species[s].battle_sprite);obj_palette(first?1:0,1,species[s].palette);for(i=0;i<8;i++){col=i%4;set_sprite_tile(first+i,tilebase+i*2);set_sprite_prop(first+i,(first?1:0)|(flip?S_FLIPX:0));move_sprite(first+i,x+8+(flip?3-col:col)*8,y+16+(i/4)*16);}}
void medal_obj(uint8_t slot,uint8_t kind,uint8_t selected){uint8_t i,first=16+slot*6,tile=32+slot*12;const uint8_t*data=kind==1?medal_1:(kind==2?medal_2:medal_0);set_sprite_data(tile,12,data);for(i=0;i<6;i++){set_sprite_tile(first+i,tile+i*2);set_sprite_prop(first+i,selected?3:2);move_sprite(first+i,20+slot*56+(i%3)*8,120+(i/3)*16);}}
uint8_t composited_count;
const uint8_t *active_scene;uint8_t composite_base;
void scene_load(uint8_t which){const uint8_t*map;const uint8_t*attributes;uint8_t count;active_scene=which==0?poster_tiles:(which==1?field_tiles:arena_tiles);map=which==0?poster_map:(which==1?field_map:arena_map);attributes=which==0?poster_attrs:(which==1?field_attrs:arena_attrs);count=which==0?poster_count:(which==1?field_count:arena_count);set_bkg_data(128,count,active_scene);memcpy(screen,map,360);memcpy(attrs,attributes,360);composite_base=128+count;composited_count=0;}
const uint8_t *background_bytes(uint8_t tile){if(tile>=128&&tile<composite_base)return active_scene+(tile-128)*16;return font_tiles+tile*16;}
void overlay_text(uint8_t x,uint8_t y,const char*s,uint8_t outlined){uint8_t tile,mask,shadow,i,pal,buf[16],fg[8];uint16_t code;const uint8_t*bg;while(*s&&x<20&&composite_base+composited_count<254){uint8_t b=(uint8_t)*s++;if(b<128)code=b;else{code=((uint16_t)(b&15)<<12);code|=((uint16_t)((uint8_t)*s++&63)<<6);code|=(uint8_t)*s++&63;}
 pal=attrs[y*20+x]&7;bg=background_bytes(screen[y*20+x]);tile=glyph(code);for(i=0;i<8;i++)fg[i]=font_tiles[tile*16+i*2];for(i=0;i<8;i++){mask=fg[i];shadow=outlined?(mask|(mask<<1)|(mask>>1)|(i?fg[i-1]:0)|(i<7?fg[i+1]:0)):mask;buf[i*2]=(bg[i*2]&~shadow)|shadow;buf[i*2+1]=(bg[i*2+1]&~shadow)|(outlined?mask:0);}
 tile=composite_base+composited_count;set_bkg_data(tile,1,buf);cell(x++,y,tile,pal);composited_count++;}}
uint8_t text_cells(const char*s){uint8_t n=0;while(*s){uint8_t b=(uint8_t)*s++;if(b>=128)s+=2;n++;}return n;}
void grove(void){scene_load(2);}
void portrait(uint8_t s){monster_obj(s,0,0,64,32,0);}
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
void title_draw(void){scene_load(0);bg_palette(7,1,brand_palette);rect(0,6,20,2,0,6);text(3,6,L(MSG_TITLE_TAGLINE1),6);text(1,7,L(MSG_TITLE_TAGLINE2),6);monster_obj(0,0,0,24,72,0);monster_obj(1,8,16,112,72,1);
 if(save_allowed){overlay_text(3,14,save_status==SAVE_OK?L(MSG_TITLE_CONTINUE):L(MSG_TITLE_START),1);overlay_text(3,16,L(MSG_TITLE_NEW),0);overlay_text(0,17,L(MSG_TITLE_CONTROLS),0);}else{rect(0,13,20,5,0,6);text(0,14,save_status==SAVE_INCOMPATIBLE?L(MSG_SAVE_WRONG_VERSION):L(MSG_SAVE_CORRUPT),6);text(0,16,L(MSG_SAVE_PLAY_WITHOUT),6);hint(L(MSG_TITLE_CONTROLS));}}
uint8_t map_context(void){if(near(3,4))return MSG_MAP_BASE;if(near(15,3))return MSG_MAP_TRAINER_ATTACK;if(near(16,7))return MSG_MAP_TRAINER_DEFENSE;if(near(15,11))return MSG_MAP_TRAINER_SPEED;if(terrain(game.x,game.y)==164)return MSG_MAP_SEARCH;return MSG_MAP_GRASS;}
void map_positions(void){uint8_t i;for(i=0;i<4;i++){move_sprite(i,game.x*8+8+(i%2)*8,game.y*8+32+(i/2)*8);move_sprite(i+4,last_x*8+8+(i%2)*8,last_y*8+32+(i/2)*8);}}
void map_draw(void){char digit[2];uint8_t i,s=game.party[game.active].species;scene_load(1);rect(0,0,20,2,0,6);text(1,0,species[s].name[game.language],6);text(13,0,"LV",6);number(16,0,game.party[game.active].level,6);text(1,1,L(MSG_BASE_NAME),6);for(i=0;i<3;i++){digit[0]='1'+i;digit[1]=0;overlay_text(tx[i]-1,ty[i]+1,digit,1);}
 set_sprite_data(0,4,player_tiles);set_sprite_data(4,4,species[s].small);obj_palette(0,1,palettes);obj_palette(1,1,species[s].palette);
 for(i=0;i<4;i++){set_sprite_tile(i,i);set_sprite_prop(i,0);set_sprite_tile(i+4,i+4);set_sprite_prop(i+4,1);}map_positions();world_hint=map_context();overlay_text(0,16,L(world_hint),1);overlay_text(0,17,L(MSG_HINT_WORLD),0);
}
void hp_bar(uint8_t x,uint8_t y,const Fighter*f,uint8_t pal){uint8_t i,n=(uint16_t)f->hp*6/max_hp(&f->mon);for(i=0;i<6;i++)cell(x+i,y,i<n?254:255,pal);}
void battle_draw(void){uint8_t i,n,kind;Fighter*a=&allies[game.active];const char*name;const char*labels[]={L(MSG_BATTLE_SWITCH),L(MSG_BATTLE_SCOUT),L(MSG_BATTLE_RUN)};const palette_color_t wood[]={RGB(0,0,0),RGB(6,4,6),RGB(22,14,8),RGB(30,26,16)},selected[]={RGB(0,0,0),RGB(3,7,9),RGB(29,21,8),RGB(31,30,24)};
 grove();set_bkg_data(254,2,hp_tiles);rect(0,0,20,2,0,6);text(1,0,species[a->mon.species].name[game.language],6);text(11,0,species[enemy.mon.species].name[game.language],6);hp_bar(1,1,a,6);hp_bar(13,1,&enemy,6);number(8,1,a->hp,6);
 monster_obj(a->mon.species,0,0,24,56,0);monster_obj(enemy.mon.species,8,16,104,56,1);obj_palette(2,1,wood);obj_palette(3,1,selected);
 composited_count=0;overlay_text(12,5,moves[enemy_intent].name[game.language],1);
 for(i=0;i<3;i++){uint8_t id=a->mon.moves[i];kind=moves[id].effect==FX_GUARD?1:(moves[id].effect==FX_DODGE?2:0);medal_obj(i,state==AUX?i:kind,i==cursor);name=state==AUX?labels[i]:moves[id].name[game.language];n=text_cells(name);overlay_text(i*7+(6-n)/2,16,name,i==cursor);}
 overlay_text(0,17,state==AUX?L(MSG_HINT_BATTLE_AUX):L(MSG_HINT_BATTLE),0);
}
void overlay_number(uint8_t x,uint8_t y,uint8_t value){char n[4];uint8_t k=0;if(value>=100)n[k++]='0'+value/100;if(value>=10)n[k++]='0'+(value/10)%10;n[k++]='0'+value%10;n[k]=0;overlay_text(x,y,n,0);}
void camp_header(const char*heading){scene_load(1);rect(0,0,20,2,0,6);text(0,0,heading,6);text(1,1,species[game.party[game.active].species].name[game.language],6);text(14,1,"LV",6);number(17,1,game.party[game.active].level,6);}
void command_palettes(void){const palette_color_t wood[]={RGB(0,0,0),RGB(6,4,6),RGB(22,14,8),RGB(30,26,16)},gold[]={RGB(0,0,0),RGB(3,7,9),RGB(29,21,8),RGB(31,30,24)};obj_palette(2,1,wood);obj_palette(3,1,gold);}
void camp_commands(uint8_t training){uint8_t i,n,x,id,kind;const char*label;Monster*m=&game.party[game.active];const char*styles[]={L(MSG_TRAINING_ATTACK),L(MSG_TRAINING_DEFENSE),L(MSG_TRAINING_TECHNIQUE)};camp_header(training?L(MSG_TRAINING_CHOOSE):L(MSG_LOADOUT_SLOT));monster_obj(m->species,0,0,64,48,0);command_palettes();if(training){text(10,1,moves[species[m->species].training[cursor]].name[game.language],6);text(17,1,"+2",6);}for(i=0;i<3;i++){id=m->moves[i];kind=training?i:(moves[id].effect==FX_GUARD?1:(moves[id].effect==FX_DODGE?2:0));medal_obj(i,kind,i==cursor);label=training?styles[i]:moves[id].name[game.language];n=text_cells(label);x=i*7+(n<6?(6-n)/2:0);overlay_text(x,16,label,i==cursor);}overlay_text(0,17,training?L(MSG_HINT_TRAINING):L(MSG_HINT_LOADOUT),0);}
void party_draw(void){uint8_t i,j;camp_header(battle_party?L(MSG_PARTY_SWITCH):L(MSG_PARTY_CHOOSE));text(14,1,"LV HP",6);for(i=0;i<game.count;i++){set_sprite_data(i*4,4,species[game.party[i].species].small);obj_palette(i,1,species[game.party[i].species].palette);for(j=0;j<4;j++){set_sprite_tile(i*4+j,i*4+j);set_sprite_prop(i*4+j,i);move_sprite(i*4+j,16+(j%2)*8,40+i*16+(j/2)*8);}overlay_text(4,3+i*2,species[game.party[i].species].name[game.language],i==cursor);overlay_number(14,3+i*2,game.party[i].level);if(battle_party)overlay_number(18,3+i*2,allies[i].hp);}overlay_text(0,3+cursor*2,">",1);overlay_text(0,17,L(MSG_HINT_PARTY),0);}
void base_draw(void){uint8_t i;const char*options[]={L(MSG_MENU_TRAIN),L(MSG_MENU_LOADOUT),L(MSG_MENU_PARTY),L(MSG_MENU_REST),L(MSG_MENU_SAVE),L(MSG_MENU_LEAVE)};camp_header(L(MSG_BASE_NAME));monster_obj(game.party[game.active].species,0,0,112,48,0);for(i=0;i<6;i++)overlay_text(1,4+i*2,options[i],i==cursor);overlay_text(0,4+cursor*2,">",1);overlay_text(0,17,L(MSG_HINT_MENU),0);}
void learned_draw(void){uint8_t i,id;camp_header(L(MSG_LOADOUT_KNOWN));monster_obj(game.party[game.active].species,0,0,112,48,0);for(i=0;i<6;i++){id=known_move(&game.party[game.active],i);if(id!=255){overlay_text(2,4+i*2,moves[id].name[game.language],i==cursor);overlay_number(10,4+i*2,moves[id].power);}}overlay_text(0,4+cursor*2,">",1);overlay_text(0,17,L(MSG_HINT_EQUIP),0);}
/* Prepare tiles in the other GBC VRAM bank and map while the old frame remains visible. */
void video_init(void){SWITCH_ROM(1);DISPLAY_OFF;HIDE_WIN;VBK_REG=0;set_bkg_data(0,font_count,font_tiles);VBK_REG=1;set_bkg_data(0,font_count,font_tiles);VBK_REG=0;}
void render(void){uint8_t s;uint16_t i;if(state==WORLD&&rendered_state==WORLD&&world_hint==map_context()){map_positions();return;}DISABLE_VBL_TRANSFER;video_bank^=1;VBK_REG=video_bank;video_sprite16=0;clear_sprites();memset(screen,0,sizeof(screen));memset(attrs,0,sizeof(attrs));
 bg_palette(0,8,palettes);set_bkg_data(172,8,frame_tiles);
 switch(state){
 case TITLE:title_draw();break;
 case CONFIRM_NEW:text(0,4,L(MSG_NEW_CONFIRM),0);text(0,7,L(MSG_NEW_WARNING1),0);text(0,8,L(MSG_NEW_WARNING2),0);hint(L(MSG_HINT_NEW));break;
 case STARTER:
  frame(1,3,18,9,0);text(0,1,L(MSG_STARTER_CHOOSE),0);s=cursor;portrait(s);text(6,10,species[s].name[game.language],0);text(1,13,species[s].bio[game.language],0);text(0,15,L(MSG_STARTER_ARROWS),0);hint(L(MSG_HINT_STARTER));break;
 case WORLD:map_draw();break;
 case BASE:base_draw();break;
 case TRAIN:camp_commands(1);break;
 case EQUIP:camp_commands(0);break;
 case MOVEPICK:learned_draw();break;
 case PARTY:party_draw();break;
 case BATTLE:case AUX:battle_draw();break;
 case MESSAGE:scene_load(return_state==BATTLE||return_state==AUX?2:1);monster_obj(game.party[game.active].species,0,0,64,48,0);rect(0,12,20,5,0,0);text(0,13,message1,0);text(0,15,message2,0);hint(L(MSG_HINT_CONTINUE));break;
 case ABOUT:text(0,1,"OPEN MONSTER",0);text(0,4,L(MSG_ABOUT_TAGLINE),0);text(0,7,"TINJYUU/OPEN-MONSTER",0);text(0,10,"CODE + PIXELS: MIT",0);text(0,13,L(MSG_ABOUT_CREDITS),0);hint(L(MSG_HINT_BACK));break;
 }
 for(i=0;i<360;i++)attrs[i]|=video_bank?S_BANK:0;for(i=0;i<40;i++)shadow_OAM[i].prop|=video_bank?S_BANK:0;
 if(LCDC_REG&LCDCF_BG9C00)LCDC_REG&=~LCDCF_WIN9C00;else LCDC_REG|=LCDCF_WIN9C00;
 VBK_REG=0;set_win_tiles(0,0,20,18,screen);VBK_REG=1;set_win_tiles(0,0,20,18,attrs);VBK_REG=0;
 if(LCDC_REG&LCDCF_ON)wait_vbl_done();
 set_bkg_palette(0,8,bg_colors);set_sprite_palette(0,8,obj_colors);LCDC_REG^=LCDCF_BG9C00;
 if(video_sprite16)SPRITES_8x16;else SPRITES_8x8;
 ENABLE_VBL_TRANSFER;refresh_OAM();SHOW_BKG;SHOW_SPRITES;DISPLAY_ON;rendered_state=state;
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
 if((k&J_SELECT)&&state!=TITLE){music_toggle();return;}
 if(state==MESSAGE){if(k&(J_A|J_B)){state=return_state;cursor=0;}return;}
 if(state==ABOUT){if(k&(J_A|J_B))state=TITLE;return;}
 if(state==TITLE){if(k&J_SELECT){game.language=(game.language+1)%LOCALE_COUNT;return;}if(k&J_START){state=ABOUT;return;}if(k&J_A){state=save_status==SAVE_OK?WORLD:STARTER;cursor=0;return;}if(k&J_B){state=save_status==SAVE_OK?CONFIRM_NEW:STARTER;cursor=0;}return;}
 if(state==CONFIRM_NEW){if(k&J_A){state=STARTER;cursor=0;}if(k&J_B)state=TITLE;return;}
 if(state==STARTER){if(k&J_LEFT)cursor=(cursor+2)%3;if(k&J_RIGHT)cursor=(cursor+1)%3;if(k&J_B)state=TITLE;if(k&J_A){n=game.language;new_game(&game,cursor);game.language=n;last_x=3;last_y=6;show_message(L(MSG_JOURNEY_BEGIN),L(MSG_HINT_JOURNEY),WORLD);}return;}
 if(state==WORLD){if(k&J_START){save_game();return;}if(k&J_B){battle_party=0;state=PARTY;cursor=game.active;return;}if(k&J_LEFT)move_world(-1,0);else if(k&J_RIGHT)move_world(1,0);else if(k&J_UP)move_world(0,-1);else if(k&J_DOWN)move_world(0,1);
 if(k&J_A){if(near(3,4)){state=BASE;cursor=0;}else{for(i=0;i<3;i++)if(near(tx[i],ty[i])){start_battle(i);return;}if(terrain(game.x,game.y)==164)start_battle(255);}}return;}
 if(state==BASE){if(k&J_UP)cursor=(cursor+5)%6;if(k&J_DOWN)cursor=(cursor+1)%6;if(k&J_B)state=WORLD;if(k&J_A){switch(cursor){case 0:state=TRAIN;break;case 1:state=EQUIP;break;case 2:battle_party=0;state=PARTY;break;case 3:show_message(L(MSG_REST_PARTY),L(MSG_REST_BEFORE),BASE);break;case 4:save_game();break;default:state=WORLD;}cursor=0;}return;}
 if(state==TRAIN){if(k&(J_UP|J_LEFT))cursor=(cursor+2)%3;if(k&(J_DOWN|J_RIGHT))cursor=(cursor+1)%3;if(k&J_B){state=BASE;cursor=0;}if(k&J_A){train_mon(m,cursor);show_message(L(MSG_TRAINING_DONE),L(MSG_TRAINING_EQUIP),BASE);}return;}
 if(state==EQUIP){if(k&(J_UP|J_LEFT))cursor=(cursor+2)%3;if(k&(J_DOWN|J_RIGHT))cursor=(cursor+1)%3;if(k&J_B){state=BASE;cursor=0;}if(k&J_A){equip_slot=cursor;state=MOVEPICK;cursor=0;}return;}
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
void main(void){uint8_t k,held,repeat=0;video_init();bg_palette(0,8,palettes);music_init();load_game();render();
 while(1){wait_vbl_done();music_tick(state==BATTLE||state==AUX||(state==PARTY&&battle_party)||(state==MESSAGE&&(return_state==BATTLE||return_state==AUX)),(uint8_t)sys_time);held=joypad();k=held&~prev;if(held && held==prev && state==WORLD){if(++repeat>=10){k=held&(J_UP|J_DOWN|J_LEFT|J_RIGHT);repeat=6;}}else repeat=0;prev=held;if(k){input(k);render();}}
}
