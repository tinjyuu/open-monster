#include "core.h"
#include <string.h>
static uint8_t stat(const Monster *m,uint8_t which){
 const Species *s=&species[m->species]; uint8_t v=which==0?s->attack:(which==1?s->defense:s->speed);
 return v+(m->level-1)*2+(m->training==which?2:0);
}
void init_mon(Monster *m,uint8_t s,uint8_t level){uint8_t i;memset(m,0,sizeof(*m));m->species=s;m->level=level;m->training=255;for(i=0;i<3;i++)m->moves[i]=species[s].base[i];}
void new_game(Game *g,uint8_t starter){memset(g,0,sizeof(*g));g->count=1;g->x=3;g->y=5;g->rng=0x7193;init_mon(&g->party[0],starter,1);}
uint8_t max_hp(const Monster *m){return species[m->species].hp+(m->level-1)*4;}
void init_fighter(Fighter *f,const Monster *m){memset(f,0,sizeof(*f));f->mon=*m;f->hp=max_hp(m);}
uint8_t random_byte(Game *g){g->rng=(uint16_t)(g->rng*25173u+13849u);return (uint8_t)(g->rng>>8);}
uint8_t use_move(Fighter *a,Fighter *b,uint8_t id){
 const Move *mv=&moves[id];uint8_t v,limit;uint8_t trait=species[a->mon.species].trait;
 if(mv->effect==FX_GUARD){a->guard=1;return 0;}
 if(mv->effect==FX_DODGE){if(a->last_dodge)return 2;a->dodge=1;return 0;}
 if(mv->effect==FX_FOCUS){a->focused=1;return 0;}
 if(mv->effect==FX_HEAL){v=mv->power+(trait==TRAIT_WARM?3:0);limit=max_hp(&a->mon);a->hp=(a->hp+v>limit)?limit:a->hp+v;return 0;}
 if(b->dodge && mv->effect!=FX_PIERCE)return 1;
 v=mv->power+stat(&a->mon,0)/2;
 if(trait==TRAIT_STEAM && mv->effect==FX_PIERCE)v+=2;
 if(trait==TRAIT_RAIN && id==3)v+=2;
 if(a->focused){v+=6;a->focused=0;}
 limit=stat(&b->mon,1)/3;v=v>limit?v-limit:1;
 if(species[b->mon.species].trait==TRAIT_SHELL && v>1)v--;
 if(b->guard && mv->effect!=FX_PIERCE)v=(v+1)/2;
 b->hp=b->hp>v?b->hp-v:0;
 if(mv->effect==FX_DRAIN){limit=max_hp(&a->mon);v=(v+1)/2+(trait==TRAIT_SPORE?2:0);a->hp=(a->hp+v>limit)?limit:a->hp+v;}
 return 0;
}
void resolve_turn(Fighter *a,Fighter *b,uint8_t am,uint8_t bm){
 int16_t av,bv;uint8_t ad=0,bd=0;
 a->guard=b->guard=a->dodge=b->dodge=0;
 if(moves[am].effect==FX_GUARD || moves[am].effect==FX_DODGE){use_move(a,b,am);ad=1;}
 if(moves[bm].effect==FX_GUARD || moves[bm].effect==FX_DODGE){use_move(b,a,bm);bd=1;}
 av=stat(&a->mon,2)+moves[am].priority*8+(species[a->mon.species].trait==TRAIT_QUICK?3:0);
 bv=stat(&b->mon,2)+moves[bm].priority*8+(species[b->mon.species].trait==TRAIT_QUICK?3:0);
 if(av>=bv){if(!ad)use_move(a,b,am);if(b->hp&&!bd)use_move(b,a,bm);}else{if(!bd)use_move(b,a,bm);if(a->hp&&!ad)use_move(a,b,am);}
 a->last_dodge=a->dodge;b->last_dodge=b->dodge;
}
void reward(Monster *m,uint8_t n){uint16_t xp=m->xp+n;while(m->level<5 && xp>=m->level*12){xp-=m->level*12;m->level++;}m->xp=m->level==5?0:(uint8_t)xp;}
uint8_t train_mon(Monster *m,uint8_t style){if(style>2)return 0;m->training=style;m->known|=1<<style;return 1;}
uint8_t known_move(const Monster *m,uint8_t index){uint8_t i,n=3;if(index<3)return species[m->species].base[index];for(i=0;i<3;i++)if(m->known&(1<<i)){if(n==index)return species[m->species].training[i];n++;}return 255;}
uint8_t scout_chance(const Fighter *e){return 40+(uint16_t)(max_hp(&e->mon)-e->hp)*180/max_hp(&e->mon);}
uint8_t scout(Game *g,const Fighter *e){uint8_t i;if(g->count==PARTY_MAX)return 2;for(i=0;i<g->count;i++)if(g->party[i].species==e->mon.species)return 3;if(random_byte(g)>=scout_chance(e))return 0;g->party[g->count++]=e->mon;return 1;}
uint8_t enemy_move(Game *g,const Fighter *e,uint8_t trainer){uint8_t idx;
 if(trainer==1 && random_byte(g)<90)idx=1;
 else if(trainer==2 && !e->last_dodge && random_byte(g)<80)idx=2;
 else idx=0;
 return e->mon.moves[idx];
}
static uint16_t checksum(const uint8_t *p){uint8_t i;uint16_t c=0x51a7;for(i=0;i<SAVE_BYTES-2;i++)c=(uint16_t)((c<<1)|(c>>15))^p[i];return c;}
void encode_save(const Game *g,uint8_t *p){uint8_t i,j,o=12;uint16_t c;memset(p,0,SAVE_BYTES);p[0]='O';p[1]='M';p[2]=SAVE_VERSION;p[3]=g->count;p[4]=g->active;p[5]=g->badges;p[6]=g->x;p[7]=g->y;p[8]=g->rng;p[9]=g->rng>>8;
 for(i=0;i<g->count;i++){const Monster*m=&g->party[i];p[o++]=species[m->species].save_id;p[o++]=m->level;p[o++]=m->xp;p[o++]=m->training;p[o++]=m->known;for(j=0;j<3;j++)p[o++]=m->moves[j];}
 c=checksum(p);p[SAVE_BYTES-2]=c;p[SAVE_BYTES-1]=c>>8;
}
uint8_t decode_save(Game *g,const uint8_t*p){uint8_t i,j,s,o=12;uint16_t c;Game temp;
 if(p[0]!='O'||p[1]!='M'){for(i=0;i<SAVE_BYTES;i++)if(p[i]!=0&&p[i]!=255)return SAVE_CORRUPT;return SAVE_EMPTY;}
 if(p[2]!=SAVE_VERSION)return SAVE_INCOMPATIBLE;
 c=p[SAVE_BYTES-2]|((uint16_t)p[SAVE_BYTES-1]<<8);if(c!=checksum(p))return SAVE_CORRUPT;
 memset(&temp,0,sizeof(temp));temp.count=p[3];temp.active=p[4];temp.badges=p[5];temp.x=p[6];temp.y=p[7];temp.rng=p[8]|((uint16_t)p[9]<<8);
 if(!temp.count||temp.count>PARTY_MAX||temp.active>=temp.count||temp.badges>7||temp.x>18||temp.x<1||temp.y>12||temp.y<1)return SAVE_CORRUPT;
 for(i=0;i<temp.count;i++){Monster*m=&temp.party[i];for(s=0;s<species_count;s++)if(species[s].save_id==p[o])break;if(s==species_count)return SAVE_INCOMPATIBLE;o++;m->species=s;m->level=p[o++];m->xp=p[o++];m->training=p[o++];m->known=p[o++];if(!m->level||m->level>5||(m->training>2&&m->training!=255)||m->known>7||m->xp>=m->level*12)return SAVE_CORRUPT;
 for(j=0;j<3;j++){uint8_t k,found=0;m->moves[j]=p[o++];for(k=0;k<6;k++)if(known_move(m,k)==m->moves[j])found=1;if(m->moves[j]>=move_count||!found)return SAVE_CORRUPT;}}
 *g=temp;return SAVE_OK;
}
