/* Test-only ROM: render every species through the production sprite path. */
#define main production_main
#include "../src/main.c"
#undef main
uint8_t visual_index=0;
void setup_visual(void){Monster m;new_game(&game,visual_index);game.language=1;init_fighter(&allies[0],&game.party[0]);init_mon(&m,visual_index,1);init_fighter(&enemy,&m);enemy_intent=m.moves[0];state=BATTLE;cursor=0;render();}
void main(void){uint8_t old=0,k;video_init();setup_visual();while(1){wait_vbl_done();k=joypad();if((k&J_A)&&!(old&J_A)){visual_index=(visual_index+1)%species_count;setup_visual();}old=k;}}
