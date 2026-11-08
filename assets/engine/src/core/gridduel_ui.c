#pragma bank 255
#include "gridduel.h"
#include <gb/cgb.h>
#include "gridduel_font.h"
#include "palette.h"
#include <string.h>
static uint8_t row[20],attrs[20];
static char label[8];
static uint8_t tile(char c){return(c>=32&&c<96)?c-32:0;}
static void text(uint8_t x,const char*s,uint8_t pal){while(*s&&x<20){row[x]=tile(*s++);attrs[x++]=pal;}}
static void num(uint8_t x,uint16_t v,uint8_t n,uint8_t pal){while(n){uint8_t col=x+--n;row[col]=tile('0'+v%10);attrs[col]=pal;v/=10;}}
static void name(uint8_t x,uint8_t iid,uint8_t pal){gd_name(iid,label);text(x,label,pal);}
static void tokenrow(uint8_t x,uint8_t iid,uint8_t part,uint8_t pal){uint8_t t=gd_token(iid);if(t){row[x]=t+(part?2:0);row[x+1]=t+(part?3:1);attrs[x]=attrs[x+1]=pal;}else if(!part){gd_name(iid,label);row[x]=tile(label[0]);row[x+1]=tile(gd_owner(iid)?'2':'1');attrs[x]=attrs[x+1]=pal;}}
static void write(uint8_t y){VBK_REG=0;set_bkg_tiles(0,y,20,1,row);VBK_REG=1;set_bkg_tiles(0,y,20,1,attrs);VBK_REG=0;}
void gd_render_init(void) BANKED {
static const palette_color_t colors[32]={RGB(30,30,27),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(23,25,22),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(18,25,31),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(31,20,19),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(16,29,17),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(31,17,13),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(31,28,10),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3),RGB(17,17,19),RGB(20,20,18),RGB(10,10,9),RGB(1,2,3)};
VBK_REG=0;set_bkg_data(0,64,gd_font);gd_art_init();memcpy(BkgPalette,colors,sizeof(colors));DMG_palette[0]=0xE4;set_bkg_palette(0,8,colors);BGP_REG=0xE4;move_bkg(0,0);HIDE_WIN;gd.dirty=1;}
static uint8_t piece(uint8_t n){uint8_t t;for(t=0;t<49;t++)if(gd.cell[t].iid!=255&&gd_owner(gd.cell[t].iid)==0){if(!n)return t;n--;}return 255;}
static void cellrow(uint8_t y){uint8_t x,t,cx,cy,part=(y-1)&1,base,i;if(y<1||y>14)return;cy=(y-1)/2;for(cx=0;cx<7;cx++){t=cy*7+cx;x=1+cx*2;i=gd_view(0,t);base=i!=255?(gd_owner(i)?3:2):((cx+cy)&1);if(gd.pick_cell!=255){if(gd_can_move(0,gd.pick_cell,t))base=4;if(gd_can_attack(0,gd.pick_cell,t))base=5;}if(gd.pick_hand!=255&&gd_can_play(0,gd.pick_hand,t))base=4;if(t==gd.cursor)base=6;attrs[x]=attrs[x+1]=base;
if(i!=255){if(gd_owner(i)&&gd.cell[t].flags&GD_DOWN){if(!part)row[x]=tile('?');}else tokenrow(x,i,part,base);}else if(!part){row[x]=tile(gd.traps[0][t]?'T':'.');}if(gd.reveal_ticks&&gd.reveal==t&&!part){row[x]=tile('!');attrs[x]=5;}}
}
void gd_render(void) BANKED {uint8_t y,x,i,t,k,mask;GDCard*c=gd_card(gd.info_card);if(gd.pane==GD_INFO&&gd.info_iid!=255)gd_portrait(gd.info_iid);for(y=0;y<18;y++){for(x=0;x<20;x++){row[x]=0;attrs[x]=0;}
if(gd.pane==GD_BOARD){if(!y){text(0,"GRIDDUEL",0);text(11,gd.turn?"CPU TURN":"YOUR TURN",gd.turn?3:2);}cellrow(y);if(y==1)text(16,"P1",2);if(y==2)num(16,gd.lp[0],4,2);if(y==4)text(16,"P2",3);if(y==5)num(16,gd.lp[1],4,3);if(y==7)text(16,"MOVE",0);if(y==8){i=gd.pick_cell;if(i!=255){num(17,gd.cell[i].flags&GD_LEADER?1:gd_card(gd.inst[gd.cell[i].iid].card)->move,2,4);}}if(y==13)text(16,"USED",0);if(y==14&&gd.pick_cell!=255){if(gd.cell[gd.pick_cell].flags&GD_MOVED)text(16,"M",0);if(gd.cell[gd.pick_cell].flags&GD_ATTACKED)text(18,"A",0);}if(y==10)text(16,"ATK",0);if(y==11&&gd.pick_cell!=255)num(17,gd.cell[gd.pick_cell].flags&GD_LEADER?0:gd_attack_mask(gd.cell[gd.pick_cell].iid),2,5);
if(y==15){if(gd.msg==1)text(0,"NOT LEGAL / USED",5);else if(gd.msg==2)text(0,"CARD PLAYED",2);else if(gd.msg==3)text(0,"BATTLE!",5);else if(gd.msg==4)text(0,"SNARE! UNIT LOST",5);else if(gd.msg==5)text(0,"TRAP! LP DAMAGE",5);else if(gd.msg==6)text(0,"FACE-DOWN MONSTER",0);else if(gd.msg==7)text(0,"EMPTY DECK: -500 LP",5);else text(0,"SEL: HAND / PIECES",0);}if(y==16)text(0,"A:PICK B:INFO/CANCEL",0);if(y==17)text(0,"START:END  EMPTY B:Q",0);
}else if(gd.pane==GD_HAND){if(!y)text(0,"HAND   A:PLAY B:INFO",0);for(k=0;k<5;k++)if(y==2+k*3||y==3+k*3){i=gd.hand[0][k];if(y==2+k*3){if(k==gd.hand_cursor)text(0,">",6);num(2,k+1,1,0);if(i!=255){c=gd_card(gd.inst[i].card);name(4,i,2);text(13,c->kind==0?"MON":c->kind==1?"SPELL":c->kind==2?"TRAP":"EQUIP",0);tokenrow(17,i,0,2);}else text(4,"EMPTY",0);}else if(i!=255){c=gd_card(gd.inst[i].card);text(4,"ATK",0);num(8,c->atk,4,0);text(12,"L",0);num(14,gd.inst[i].level,1,0);tokenrow(17,i,1,2);}}if(y==17)text(0,"SEL:PIECES START:END",0);
}else if(gd.pane==GD_PIECES){if(!y)text(0,"PIECES A:PICK B:INFO",0);for(k=0;k<6;k++)if(y==2+k*2){t=piece(k);if(t!=255){i=gd.cell[t].iid;if(k==gd.piece_cursor)text(0,">",6);if(gd.cell[t].flags&GD_LEADER)text(2,"LEADER",2);else name(2,i,2);text(10,"X",0);num(11,t%7+1,1,0);text(13,"Y",0);num(14,t/7+1,1,0);if(gd.cell[t].flags&GD_MOVED)text(15,"M",0);if(gd.cell[t].flags&GD_ATTACKED)text(16,"A",0);tokenrow(17,i,0,2);}}if(y>=3&&y<=13&&(y&1)){t=piece((y-3)/2);if(t!=255)tokenrow(17,gd.cell[t].iid,1,2);}if(y==16)text(0,"M:MOVED A:ATTACKED",0);if(y==17)text(0,"SELECT:BOARD",0);
}else if(gd.pane==GD_INFO){c=gd_card(gd.info_card);if(!y){if(gd.info_iid!=255)name(0,gd.info_iid,2);else text(0,c->name,2);}if(y==1){text(0,"SHAPE",0);text(6,c->shape==0?"CIRCLE":c->shape==1?"SQUARE":"TRIANGLE",0);}if(y==2){text(0,"ATK",0);num(4,gd.info_iid==255?c->atk:gd_atk(gd.info_iid),4,0);}if(y==3){text(0,"DEF",0);num(4,c->defense,4,0);}if(y==4){text(0,"LV",0);num(3,gd.info_iid==255?1:gd.inst[gd.info_iid].level,2,0);text(7,"XP",0);num(10,gd.info_iid==255?0:gd.inst[gd.info_iid].xp,4,0);}if(y==5){text(0,"MOVE",4);num(5,c->move,2,4);text(9,c->jump?"JUMP":"WALK",4);}if(y==6){text(0,"ATTACK",5);num(7,(gd.info_iid==0||gd.info_iid==25)?0:gd.info_iid==255?c->attack:gd_attack_mask(gd.info_iid),2,5);text(11,c->ranged||c->attack>2?"RANGE":"MELEE",5);}if(y==8){text(1,"MOVE",4);text(11,"ATTACK",5);}if(y>=9&&y<=15){int8_t dy=y-12;for(x=0;x<7;x++){int8_t dx=x-3;mask=gd_mask(c->move,dx,dy);row[1+x]=tile(!dx&&!dy?'O':mask?'#':'.');attrs[1+x]=!dx&&!dy?6:4;mask=gd_mask((gd.info_iid==0||gd.info_iid==25)?0:gd.info_iid==255?c->attack:gd_attack_mask(gd.info_iid),dx,dy);row[11+x]=tile(!dx&&!dy?'O':mask?'#':'.');attrs[11+x]=!dx&&!dy?6:5;}}if(gd.info_iid==0||gd.info_iid==25){
 if(y==1||y==2||y==3||y==6){for(x=0;x<16;x++){row[x]=0;attrs[x]=0;}}
 if(y==1)text(0,"LEADER: NO ATTACK",0);
 if(y==2){text(0,"LP",0);num(4,gd.lp[gd_owner(gd.info_iid)],4,0);}
 if(y==3)text(0,"MOVE ONLY",0);
 if(y==6)text(0,"ATTACK 00 / NONE",5);
}if(c->kind){
 if((y>=1&&y<=6)||((c->kind!=3||c->fx!=6)&&y>=8&&y<=15)){for(x=0;x<20;x++){row[x]=0;attrs[x]=0;}}
 if(y==1)text(0,c->kind==1?"SPELL":c->kind==2?"HIDDEN FLOOR TRAP":"EQUIPMENT",0);
 if(y==2){if(c->fx==1){text(0,"ENEMY LP -",5);num(10,c->atk,4,5);}else if(c->fx==2){text(0,"HEAL LP +",4);num(9,c->atk,4,4);}else if(c->fx==3)text(0,"KO ENTERING MONSTER",5);else if(c->fx==4){text(0,"OWNER LP -",5);num(10,c->atk,4,5);}else if(c->fx==5){text(0,"ATK +",4);num(5,c->atk,4,4);}else if(c->fx==6){text(0,"ATTACK MASK",5);num(12,c->attack,2,5);}}
 if(y==4)text(0,c->kind==1?"PLAY ON ANY TILE":c->kind==2?"SET BY YOUR LEADER":"TARGET OWN PIECE",0);
 if(y==5){if(c->kind==2)text(0,"INVISIBLE TO ENEMY",0);else if(c->kind==3&&c->fx==5)text(0,"EQUIP CAP: +1500",0);else if(c->kind==3)text(0,"WALK MASK UNCHANGED",0);}
 if(y==6&&c->kind==2)text(0,"CONSUMED ON TRIGGER",0);
 if(y==9&&c->kind==2)text(0,"JUMPS: LANDING ONLY",0);
 if(y==10&&c->kind==2)text(0,"ARROWS IGNORE FLOOR",0);
}if(gd.info_iid!=255&&gd.inst[gd.info_iid].card<=8&&y<4){for(x=0;x<4;x++){row[16+x]=128+y*4+x;attrs[16+x]=2;}}if(y==17)text(0,"A/B:BACK  O=ORIGIN",0);
}else if(gd.pane==GD_QUIT){if(y==5)text(1,"QUIT THIS DUEL?",0);if(y==8)text(1,"A:YES  B:KEEP PLAYING",0);if(y==11)text(1,"NO ANTE / LEVEL LOSS",0);
}else if(gd.pane==GD_RESULT){if(y==5)text(3,gd.winner==0?"YOU WIN!":"YOU LOSE",gd.winner==0?2:3);if(y==8)text(1,"A/B:BACK TO METRO",0);if(y==11)text(1,"NO ANTE / LEVEL LOSS",0);}
write(y);}gd.dirty=0;}
