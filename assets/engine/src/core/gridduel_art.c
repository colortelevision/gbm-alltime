#pragma bank 255
#include "gridduel.h"
#include <string.h>
#include "gridduel_art_data.h"
static const char gd_names[2][8][8]={{"CBOT","BBOT","DBOT","GENBOT","MILTON","LBOT","MBOT","ROEBOT"},{"HASH A","HASH B","HASH C","HASH D","HASH E","HASH F","HASH G","HASH H"}};
void gd_art_init(void) BANKED {set_bkg_data(64,64,gd_token_tiles);}
void gd_portrait(uint8_t iid) BANKED {uint8_t c=gd.inst[iid].card;if(c>=1&&c<=8)set_bkg_data(128,16,gd_portraits[gd.theme[gd_owner(iid)]*8+c-1]);}
uint8_t gd_token(uint8_t iid) BANKED {uint8_t c=gd.inst[iid].card;return(c>=1&&c<=8)?64+gd.theme[gd_owner(iid)]*32+(c-1)*4:0;}
void gd_name(uint8_t iid,char *dest) BANKED {uint8_t c=gd.inst[iid].card;if(c>=1&&c<=8)memcpy(dest,gd_names[gd.theme[gd_owner(iid)]][c-1],8);else memcpy(dest,gd_card(c)->name,8);}
