#pragma bank 255
/* Phoenix only. No engine replacement, hidden victory, fire power or save writes. */
#include <gbdk/platform.h>
#include <string.h>
#include "vm.h"
#include "actor.h"
#include "input.h"
#include "data/game_globals.h"
#include "gbs_types.h"
#include "game_time.h"
typedef struct {UWORD age,last;UBYTE oldjoy,warm,attack_cd,dodge_cd,dodge,pose,pose_ticks,iframes,windup,stagger,recover,regen,foe_pose;} PHState;
#define ph (*(PHState *)(&script_memory[MAX_GLOBAL_VARS]))
typedef char ph_heap_guard[(MAX_GLOBAL_VARS+(sizeof(PHState)+1)/2<=VM_HEAP_SIZE)?1:-1];
#define G(x) script_memory[VAR_PH_##x]
static UBYTE sub(UBYTE a,UBYTE b){return a>b?a-b:0;}
static WORD ab(WORD v){return v<0?-v:v;}
static void pose(UBYTE n){if(ph.pose!=n){ph.pose=n;G(ACTION)=n+1;}}
static void damage(void){if(!ph.iframes){if(G(HP))G(HP)--;G(HUD)=1;ph.iframes=50;ph.pose_ticks=18;pose(4);}}
UBYTE ph_init(void *THIS,UBYTE start,UWORD *stack_frame) OLDCALL BANKED {
 memset(&ph,0,sizeof(ph));ph.last=game_time;ph.oldjoy=joypads.joy0;ph.warm=8;ph.pose=255;
 G(RESULT)=0;G(CLOCK)=0;G(ACTION)=0;G(TELL)=0;G(HP)=4;G(STAM)=3;G(HUD)=1;*stack_frame=0;return TRUE;
}
UBYTE ph_tick(void *THIS,UBYTE start,UWORD *stack_frame) OLDCALL BANKED {
 UBYTE now,pressed,dt,mode,pose_now=0,foe_pose=0;WORD dx,dy;actor_t *foe;
 mode=(UBYTE)G(MODE);if(!mode){*stack_frame=0;return TRUE;}
 dt=(UBYTE)(game_time-ph.last);if(!dt){*stack_frame=G(RESULT);return TRUE;}ph.last=game_time;if(dt>4)dt=4;
 now=joypads.joy0;pressed=now&~ph.oldjoy;ph.oldjoy=now;
 if(ph.warm){ph.warm=sub(ph.warm,dt);*stack_frame=0;return TRUE;}
 ph.age+=dt;G(CLOCK)=ph.age;
 ph.attack_cd=sub(ph.attack_cd,dt);ph.dodge_cd=sub(ph.dodge_cd,dt);ph.dodge=sub(ph.dodge,dt);ph.iframes=sub(ph.iframes,dt);ph.pose_ticks=sub(ph.pose_ticks,dt);ph.stagger=sub(ph.stagger,dt);
 if(mode==1){ /* swim: real directional world movement; A provides a short stroke, B cannot run */
  if(pressed&J_A)ph.dodge=22;PLAYER.move_speed=ph.dodge?24:8;*stack_frame=0;return TRUE;
 }
 if(mode==2){if(ph.age>=180){G(RESULT)=1;G(MODE)=0;}*stack_frame=G(RESULT);return TRUE;}
 if(mode==4){ /* Conner can try repeatedly; no count can turn the fixed failed rescue into a win. */
  if((pressed&J_A)&&!ph.attack_cd){dx=(WORD)(PLAYER.pos.x>>5)-64;dy=(WORD)(PLAYER.pos.y>>5)-48;
   if(ab(dx)<=32&&ab(dy)<=32){G(PULLS)++;G(ACTION)=6;ph.attack_cd=30;PLAYER.dir=DIR_UP;}
  }
  if(ph.age>=600){G(RESULT)=1;G(MODE)=0;}*stack_frame=G(RESULT);return TRUE;
 }
 if(mode!=3){*stack_frame=0;return TRUE;}
 foe=&actors[(UBYTE)G(FOE)];dx=(WORD)(PLAYER.pos.x>>5)-(WORD)(foe->pos.x>>5);dy=(WORD)(PLAYER.pos.y>>5)-(WORD)(foe->pos.y>>5);
 if((pressed&J_B)&&!ph.dodge_cd&&G(STAM)){
  G(DODGES)++;G(STAM)--;G(HUD)=1;ph.dodge=18;ph.dodge_cd=40;ph.iframes=20;
 }
 if(ph.dodge){
  pose_now=2;/* short step, clamped to the actual unobstructed combat floor */
  if(PLAYER.dir==DIR_LEFT&&PLAYER.pos.x>512)PLAYER.pos.x-=48;
  if(PLAYER.dir==DIR_RIGHT&&PLAYER.pos.x<4096)PLAYER.pos.x+=48;
  if(PLAYER.dir==DIR_UP&&PLAYER.pos.y>2048)PLAYER.pos.y-=48;
  if(PLAYER.dir==DIR_DOWN&&PLAYER.pos.y<3584)PLAYER.pos.y+=48;
 }else if(now&J_SELECT&&G(STAM)){pose_now=3;PLAYER.move_speed=8;}
 else PLAYER.move_speed=16;
 if((pressed&J_A)&&!ph.attack_cd&&!ph.dodge&&!(now&J_SELECT)){
  G(ATTACKS)++;ph.attack_cd=28;ph.pose_ticks=12;pose_now=1;
  if(ab(dx)<=24&&ab(dy)<=20&&((PLAYER.dir==DIR_LEFT&&dx>=0)||(PLAYER.dir==DIR_RIGHT&&dx<=0)||(PLAYER.dir==DIR_UP&&dy>=0)||(PLAYER.dir==DIR_DOWN&&dy<=0))){ph.stagger=12;ph.windup=0;ph.recover=12;}
 }
 if(ph.pose_ticks){if(ph.pose==4)pose_now=4;else if(ph.pose==1)pose_now=1;}
 /* A 30-frame visible wind-up, a range-checked strike, recovery, pursuit.
    Hits can stagger her and defense changes survival/microtiming, never the chapter outcome. */
 if(ph.windup){
  foe_pose=1;ph.windup=sub(ph.windup,dt);
  if(!ph.windup){
   foe_pose=2;ph.recover=28;
   if(ab(dx)<=27&&ab(dy)<=24&&!ph.iframes){
    if((now&J_SELECT)&&G(STAM)){G(GUARDS)++;G(STAM)--;G(HUD)=1;ph.iframes=18;}
    else damage();
   }
  }
 }else if(ph.recover){ph.recover=sub(ph.recover,dt);foe_pose=ph.recover>15?2:0;}
 else if(!ph.stagger){
  if(ab(dx)<=24&&ab(dy)<=20){ph.windup=30;foe_pose=1;}
  else{
   if(ab(dx)>7){foe->dir=dx>0?DIR_RIGHT:DIR_LEFT;foe->pos.x+=dx>0?20:-20;}
   else if(ab(dy)>7){foe->dir=dy>0?DIR_DOWN:DIR_UP;foe->pos.y+=dy>0?20:-20;}
   actor_set_anim_idle(foe);
  }
 }
 /* slow stamina refill only outside guard/dodge; no infinite guaranteed safety. */
 if(!(now&J_SELECT)&&!ph.dodge){ph.regen+=dt;if(ph.regen>=100){ph.regen=0;if(G(STAM)<3){G(STAM)++;G(HUD)=1;}}}else ph.regen=0;
 if(ph.pose_ticks&&ph.pose==4)pose_now=4;pose(pose_now);
 if(ph.foe_pose!=foe_pose){ph.foe_pose=foe_pose;G(TELL)=foe_pose+1;}
 /* Story finisher: even perfect play ends here; health is not Maxwell's HP. */
 if(!G(HP)||ph.age>=1080){G(HP)=0;G(HUD)=1;G(RESULT)=1;G(MODE)=0;}
 *stack_frame=G(RESULT);return TRUE;
}
