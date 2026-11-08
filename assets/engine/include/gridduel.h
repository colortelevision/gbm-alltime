#ifndef GRIDDUEL_NATIVE_H
#define GRIDDUEL_NATIVE_H
#include <gbdk/platform.h>
#include <stdint.h>
#define GD_N 49
#define GD_EMPTY 255
#define GD_MOVED 1
#define GD_ATTACKED 2
#define GD_LEADER 4
#define GD_DOWN 8
#define GD_BOARD 0
#define GD_HAND 1
#define GD_PIECES 2
#define GD_INFO 3
#define GD_QUIT 4
#define GD_RESULT 5
#include "gridduel_counts.h"
/* Catalog primary schema lives in plugin data; unset taxonomy/UNO fields remain metadata. */
typedef struct {uint8_t id; char name[8];uint16_t atk,defense;uint8_t move,attack,jump,ranged,kind,fx,shape;} GDCard;
typedef struct {uint8_t card,level;uint16_t xp,eq;uint8_t attack_override;} GDInstance;
typedef struct {uint8_t iid,flags;} GDCell;
typedef struct {GDCell cell[49];uint8_t traps[2][49];GDInstance inst[50];uint8_t deck[2][25],left[2],hand[2][5];uint16_t lp[2];uint8_t turn,placed,winner;uint8_t pane,cursor,hand_cursor,piece_cursor,pick_cell,pick_hand,back_pane,info_card,info_iid;uint8_t dirty,exit_code,cpu_wait,msg,reveal,reveal_fx,reveal_ticks;uint16_t moves,attacks,triggers;uint8_t last_from,last_to,last_action;uint8_t theme[2],leader_pos[2];} GDState;
/* Borrow ONLY compiler-unallocated VM heap, never globals or context stacks.
   Existing engine has768 heap words; compiler exposes actual occupied prefix.
   No engine size changes and no extra WRAM allocation. Compile fails on overlap. */
#include "vm.h"
#include "data/game_globals.h"
#define GD_STATE_WORDS 322
#if (MAX_GLOBAL_VARS + GD_STATE_WORDS) > VM_HEAP_SIZE
#error Gridduel_scratch_overlaps_VM_context_stacks
#endif
typedef char gd_size_guard[(sizeof(GDState)==GD_STATE_WORDS*2)?1:-1];
#define gd (*(GDState *)(&script_memory[MAX_GLOBAL_VARS]))
extern GDCard gd_cards[GD_COUNT];
GDCard *gd_card(uint8_t id) BANKED;
#define gd_owner(iid) ((iid)>=25)
#define gd_view(viewer,pos) (gd.cell[(pos)].iid)
uint8_t gd_can_move(uint8_t p,uint8_t f,uint8_t t) BANKED;
uint8_t gd_can_attack(uint8_t p,uint8_t f,uint8_t t) BANKED;
uint8_t gd_move(uint8_t p,uint8_t f,uint8_t t) BANKED;
uint8_t gd_attack(uint8_t p,uint8_t f,uint8_t t) BANKED;
uint8_t gd_can_play(uint8_t p,uint8_t h,uint8_t t) BANKED;
uint8_t gd_play(uint8_t p,uint8_t h,uint8_t t) BANKED;
void gd_begin(void) BANKED;
void gd_end(void) BANKED;
void gd_cpu(void) BANKED;
void gd_art_init(void) BANKED;
void gd_portrait(uint8_t iid) BANKED;
uint8_t gd_token(uint8_t iid) BANKED;
void gd_name(uint8_t iid,char *dest) BANKED;
void gd_render_init(void) BANKED;
void gd_render(void) BANKED;
void gd_fixture(uint8_t n) BANKED;
uint16_t gd_atk(uint8_t iid) BANKED;
uint8_t gd_attack_mask(uint8_t iid) BANKED;
uint8_t gd_mask(uint8_t mask,int8_t dx,int8_t dy) BANKED;
#endif
