#ifndef CORE_H
#define CORE_H
#include <stdint.h>
#include "locale_ids.h"
#define PARTY_MAX 6
#define SAVE_VERSION 1
#define SPECIES_LIMIT 24
#define MOVE_SLOTS 3
#define FX_ATTACK 0
#define FX_GUARD 1
#define FX_DODGE 2
#define FX_HEAL 3
#define FX_FOCUS 4
#define FX_PIERCE 5
#define FX_DRAIN 6
#define TRAIT_STEAM 0
#define TRAIT_RAIN 1
#define TRAIT_WARM 2
#define TRAIT_SHELL 3
#define TRAIT_SPORE 4
#define TRAIT_QUICK 5
#define SAVE_EMPTY 0
#define SAVE_OK 1
#define SAVE_INCOMPATIBLE 2
#define SAVE_CORRUPT 3
#define SAVE_BYTES 90

typedef struct { const char *name[LOCALE_COUNT]; uint8_t power; int8_t priority; uint8_t effect; } Move;
typedef struct { const char *name[LOCALE_COUNT]; const char *bio[LOCALE_COUNT]; uint8_t save_id, trait, starter; uint8_t hp,attack,defense,speed; uint8_t base[3],training[3]; uint16_t palette[4]; const uint8_t *small; const uint8_t *battle_sprite; } Species;
typedef struct { uint8_t species,level,xp,training,known; uint8_t moves[3]; } Monster;
typedef struct { uint8_t count,active,badges,x,y; uint16_t rng; Monster party[PARTY_MAX]; uint8_t language; } Game;
typedef struct { Monster mon; uint8_t hp,guard,dodge,focused,last_dodge; } Fighter;
extern const Move moves[];
extern const Species species[];
extern const uint8_t species_count,move_count;
void new_game(Game *g,uint8_t starter);
void init_mon(Monster *m,uint8_t s,uint8_t level);
uint8_t max_hp(const Monster *m);
void init_fighter(Fighter *f,const Monster *m);
uint8_t random_byte(Game *g);
uint8_t use_move(Fighter *actor,Fighter *target,uint8_t move);
void resolve_turn(Fighter *a,Fighter *b,uint8_t am,uint8_t bm);
void reward(Monster *m,uint8_t amount);
uint8_t train_mon(Monster *m,uint8_t style);
uint8_t known_move(const Monster *m,uint8_t index);
uint8_t scout_chance(const Fighter *enemy);
uint8_t scout(Game *g,const Fighter *enemy);
uint8_t enemy_move(Game *g,const Fighter *enemy,uint8_t trainer);
void encode_save(const Game *g,uint8_t *data);
uint8_t decode_save(Game *g,const uint8_t *data);
#endif
