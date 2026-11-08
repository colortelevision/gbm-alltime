#pragma bank 255
#include "gridduel.h"
static void put(uint8_t t,uint8_t iid,uint8_t card,uint8_t flags){gd.cell[t].iid=iid;gd.cell[t].flags=flags;gd.inst[iid].card=card;gd.inst[iid].level=1;gd.inst[iid].xp=0;gd.inst[iid].eq=0;gd.inst[iid].attack_override=0;}
void gd_fixture(uint8_t n) BANKED {uint8_t t;if(!n)return;for(t=0;t<49;t++){gd.cell[t].iid=255;gd.cell[t].flags=0;gd.traps[0][t]=gd.traps[1][t]=0;}put(45,0,1,GD_LEADER);put(3,25,2,GD_LEADER);gd.pick_cell=gd.pick_hand=255;gd.turn=0;gd.placed=1;gd.winner=255;gd.pane=GD_BOARD;gd.reveal=255;gd.lp[0]=gd.lp[1]=4000;gd.cursor=31;
if(n==1||n==14){put(31,1,7,0);if(n==1)gd.traps[1][24]=11;gd.cursor=31;} /* two-step walk through hidden snare */
if(n==2){put(32,1,6,0);gd.traps[1][24]=11;gd.cursor=32;} /* jump32->16 over24 */
if(n==3){put(31,1,3,0);put(17,26,6,0);gd.traps[1][24]=11;gd.cursor=31;} /* arrow31->17 over24 */
if(n>=4&&n<=6){put(31,1,3,0);gd.inst[1].attack_override=n==4?8:n==5?10:11;put(n==4?10:n==5?15:8,26,6,0);gd.cursor=31;}
if(n==7){put(10,1,7,0);gd.cursor=10;gd.lp[1]=100;}
if(n==8){put(38,26,7,0);gd.turn=1;gd.placed=1;gd.lp[0]=100;gd.cpu_wait=10;}
if(n==9||n==10){put(17,26,7,0);gd.turn=1;gd.placed=1;gd.cpu_wait=120;if(n==10)gd.traps[0][24]=11;}
if(n==11){put(31,1,7,0);gd.traps[1][17]=12;gd.cursor=31;}
if(n==12){put(32,1,6,0);gd.traps[1][16]=11;gd.cursor=32;}
if(n==13){put(31,1,3,0);gd.cursor=31;gd.placed=0;gd.hand[0][0]=2;gd.inst[2].card=15;}
if(n==15){put(31,1,7,0);gd.lp[0]=3800;gd.placed=0;gd.left[0]=gd.left[1]=0;for(t=0;t<5;t++){gd.hand[0][t]=t+2;gd.hand[1][t]=255;gd.inst[t+2].card=t==0?9:t==1?10:t==2?13:14;}gd.cursor=31;}
if(n==16){gd.left[0]=gd.left[1]=0;gd.lp[0]=500;for(t=0;t<5;t++)gd.hand[0][t]=gd.hand[1][t]=255;}
if(n==17){put(31,1,7,0);put(24,2,5,0);gd.cursor=31;}
if(n==18){put(31,1,8,0);gd.traps[1][23]=12;gd.cursor=31;}
if(n==19){put(31,1,3,0);put(24,2,2,0);put(17,26,6,0);gd.traps[1][24]=11;gd.cursor=31;}
gd.dirty=1;}
