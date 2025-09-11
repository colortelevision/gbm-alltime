/*
State Change Bugs:
- Wall, Ground, and Fall all use the basic y collision, which means that they keep reseting to Fall state and THEN back to Init.

Future notes on things to do:
- Limit air dashes before touching the ground
- write event plugin for dash_interrupt
    - Test this for the specific dash problem of being pushed into walls
- Write an event plugin for the state machine for 'on state change'
- Add a jump-type variable so that it's easy to test which jump the player is doing.
- Add an option for wall jump that only allows alternating walls.


- Note, the way I've written the cascading state switch logic, if a player hits jump, they will not do a ladder check the same frame. That seems fine, but keep an eye on it.

NOTES on GBStudio Quirks
- 256 velocities per position, 16 'positions' per pixel, 8 pixels per tile
- Player bounds: for an ordinary 16x16 sprite, bounds.left starts at 0 and bounds.right starts at 16. If it's smaller, bounds.left is POSITIVE
- For bounds.top, however, Y starts counting from the middle of a sprite. bounds.top is negative and bounds.bottom is positive 

//Steps before uploading to Github
//1. Comment better



*/
#pragma bank 3

#include "data/states_defines.h"
#include "states/platform.h"

#include "actor.h"
#include "camera.h"
#include "collision.h"
#include "data_manager.h"
#include "game_time.h"
#include "input.h"
#include "math.h"
#include "scroll.h"
#include "trigger.h"
#include "vm.h"

#ifndef INPUT_PLATFORM_JUMP
#define INPUT_PLATFORM_JUMP        INPUT_A
#endif
#ifndef INPUT_PLATFORM_RUN
#define INPUT_PLATFORM_RUN         INPUT_B
#endif
#ifndef INPUT_PLATFORM_INTERACT
#define INPUT_PLATFORM_INTERACT    INPUT_A
#endif
#ifndef PLATFORM_CAMERA_DEADZONE_X
#define PLATFORM_CAMERA_DEADZONE_X 4
#endif
#ifndef PLATFORM_CAMERA_DEADZONE_Y
#define PLATFORM_CAMERA_DEADZONE_Y 16
#endif


//DEFAULT ENGINE VARIABLES
WORD plat_min_vel;
WORD plat_walk_vel;
WORD plat_run_vel;
WORD plat_climb_vel;
WORD plat_walk_acc;
WORD plat_run_acc;
WORD plat_dec;
WORD plat_jump_vel;
WORD plat_grav;
WORD plat_hold_grav;
WORD plat_max_fall_vel;

//PLATFORMER PLUS ENGINE VARIABLES
UBYTE plat_drop_through; //Drop-through control
UBYTE plat_mp_group;
UBYTE plat_solid_group;
WORD plat_jump_min;
UBYTE plat_hold_jump_max; //Maximum number for frames for continuous input
UBYTE plat_extra_jumps; //Number of jumps while in the air
WORD plat_jump_reduction; //Reduce height each double jump
UBYTE plat_coyote_max; //Coyote Time maximum frames
UBYTE plat_buffer_max; //Jump Buffer maximum frames
UBYTE plat_wall_jump_max; //Number of wall jumps in a row
UBYTE plat_wall_slide;
WORD plat_wall_grav; //Gravity while clinging to the wall
WORD plat_wall_kick;
UBYTE plat_float_input;
WORD plat_float_grav;
UBYTE plat_air_control;
WORD plat_air_dec; // air deceleration rate
UBYTE plat_run_type;
WORD plat_turn_acc;
WORD plat_run_boost;
UBYTE plat_dash;
UBYTE plat_dash_style;
UBYTE plat_dash_momentum;
UBYTE plat_dash_through;
WORD plat_dash_dist;
UBYTE plat_dash_frames;
UBYTE plat_dash_ready_max;

enum pStates {
    FALL_INIT = 0,
    FALL_STATE,
    GROUND_INIT,
    GROUND_STATE,
    JUMP_INIT,
    JUMP_STATE,
    DASH_INIT,
    DASH_STATE,
    LADDER_INIT,
    LADDER_STATE,
    WALL_INIT,
    WALL_STATE
}; 
enum pStates plat_state;
UBYTE nocontrol_h; 
UBYTE nocontrol_v;
UBYTE nocollide;

//COUNTER variables
UBYTE ct_val; //Coyote Time Variable
UBYTE jb_val; //Jump Buffer Variable
UBYTE wc_val; //Wall Coyote Time Variable
UBYTE plat_hold_jump_val; //Jump input hold variable
UBYTE dj_val; //Current double jump
UBYTE wj_val; //Current wall jump

//WALL variables 
BYTE last_wall; //tracks the last wall the player touched
BYTE col; //tracks if there is a block left or right

//DASH VARIABLES
UBYTE dash_ready_val;
WORD dash_dist;
UBYTE plat_dash_currentframe;
BYTE tap_val;
BYTE dash_dir;
UBYTE dash_end_clear;

//COLLISION VARS
actor_t *last_mp;
UBYTE mp_attached;
WORD mp_last_x;
WORD mp_last_y;
WORD deltaX;
WORD deltaY;
WORD actorColX;
WORD actorColY;

//JUMPING VARIABLES
WORD plat_jump_reduction_val; //Holds a temporary jump velocity reduction
WORD plat_jump_val;
WORD jump_reduction;

//WALKING AND RUNNING VARIABLES
WORD pl_vel_x;
WORD pl_vel_y;

//VARIABLES FOR EVENT PLUGINS
UBYTE grounded;
BYTE run_stage;
UBYTE jump_type;
UBYTE dash_interrupt;


void platform_init() BANKED {
    //Make sure jumping doesn't overflow variables
    //First, check for jumping based on Frames and Initial Jump Min
    while (32000 - (plat_jump_vel/MIN(15,plat_hold_jump_max)) - plat_jump_min < 0){
        plat_hold_jump_max += 1;
    }

    //This ensures that, by itself, the plat run boost active on any single frame cannot overflow a WORD.
    //It is complemented by another check in the jump itself that works with the actual velocity. 
    if (plat_run_boost != 0){
        while((32000/plat_run_boost) < (plat_run_vel/(plat_hold_jump_max))){
            plat_run_boost--;
        }
    }

    //Normalize variables by number of frames
    plat_jump_val = plat_jump_vel / MIN(15, plat_hold_jump_max);
    jump_reduction = plat_jump_reduction / plat_hold_jump_max;
    dash_dist = plat_dash_dist/plat_dash_frames;

    if (PLAYER.dir == DIR_UP || PLAYER.dir == DIR_DOWN) {
        PLAYER.dir = DIR_RIGHT;
    }

    mp_attached = FALSE;

    //Initialize State
    plat_state = FALL_INIT;
    grounded = FALSE;
    run_stage = 0;
    nocontrol_h = 0;
    nocontrol_v = 0;
    nocollide = 0;
    actorColX = 0;
    actorColY = 0;

    //Initialize other vars
    camera_offset_x = 0;
    camera_offset_y = 0;
    camera_deadzone_x = PLATFORM_CAMERA_DEADZONE_X;
    camera_deadzone_y = PLATFORM_CAMERA_DEADZONE_Y;
    game_time = 0;
    pl_vel_x = 0;
    pl_vel_y = plat_grav << 2;
    last_wall = 0;
    col = 0;
    plat_hold_jump_val = plat_hold_jump_max;
    dj_val = 0;
    wj_val = plat_wall_jump_max;
    dash_end_clear = FALSE;
    dash_interrupt = FALSE;
    jump_type = 0;
}

void platform_update() BANKED {
    //INITIALIZE VARS
    UBYTE tile_start, tile_end, tile_current;
    UBYTE p_half_width = (PLAYER.bounds.right - PLAYER.bounds.left) >> 1;
    actor_t *hit_actor;

    WORD temp_y = 0;
    deltaX = 0;
    deltaY = 0;

    //INPUT CHECK=================================================================================================
    //Dash Input Check
    UBYTE dash_press = FALSE;
    if (plat_dash == 1){
        if (INPUT_PRESSED(INPUT_PLATFORM_INTERACT)){
            dash_press = TRUE;
        }
    } else if (plat_dash == 2){
        if (INPUT_PRESSED(INPUT_LEFT)){
            if(tap_val < 0){
                dash_press = TRUE;
            } else{
                tap_val = -15;
            }
        } else if (INPUT_PRESSED(INPUT_RIGHT)){
            if(tap_val > 0){
                dash_press = TRUE;
            } else{
                tap_val = 15;
            }
        }
    } else if (plat_dash == 3){
        if ((INPUT_PRESSED(INPUT_DOWN) && INPUT_PLATFORM_JUMP) || (INPUT_DOWN && INPUT_PRESSED(INPUT_PLATFORM_JUMP))){
            dash_press = TRUE;
        }
    }
    //Drop Through Press
    UBYTE drop_press = FALSE;
    if ((plat_drop_through == 1 && INPUT_DOWN ) || (plat_drop_through == 2 && INPUT_DOWN && INPUT_PLATFORM_JUMP)){
        drop_press = TRUE;
    }
    //FLOAT INPUT
    UBYTE float_press = FALSE;
    if ((plat_float_input == 1 && INPUT_PLATFORM_JUMP) || (plat_float_input == 2 && INPUT_UP)){
        float_press = TRUE;
    }

    // STATE MACHINE==================================================================================================
    // Change to Switch Statement
    switch(plat_state){
        case LADDER_INIT:
            plat_state = LADDER_STATE;
            jump_type = 0;
        case LADDER_STATE:{
            // PLAYER.pos.x = 0;
            UBYTE tile_x_mid = ((PLAYER.pos.x >> 4) + PLAYER.bounds.left + p_half_width) >> 3; 
            pl_vel_y = 0;
            if (INPUT_UP) {
                // Climb laddder
                UBYTE tile_y = ((PLAYER.pos.y >> 4) + PLAYER.bounds.top + 1) >> 3;
                if (tile_at(tile_x_mid, tile_y) & TILE_PROP_LADDER) {
                    pl_vel_y = -plat_climb_vel;
                }
            } else if (INPUT_DOWN) {
                // Descend ladder
                UBYTE tile_y = ((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom + 1) >> 3;
                if (tile_at(tile_x_mid, tile_y) & TILE_PROP_LADDER) {
                    pl_vel_y = plat_climb_vel;
                }
            } else if (INPUT_LEFT) {
                plat_state = FALL_INIT;
                // Check if able to leave ladder on left
                tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
                tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;
                while (tile_start != tile_end) {
                    if (tile_at(tile_x_mid - 1, tile_start) & COLLISION_RIGHT) {
                        plat_state = LADDER_INIT;
                        break;
                    }
                    tile_start++;
                }            
            } else if (INPUT_RIGHT) {
                plat_state = FALL_INIT;
                // Check if able to leave ladder on right
                tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
                tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;
                while (tile_start != tile_end) {
                    if (tile_at(tile_x_mid + 1, tile_start) & COLLISION_LEFT) {
                        plat_state = LADDER_INIT;
                        break;
                    }
                    tile_start++;
                }
            }
            PLAYER.pos.y += (pl_vel_y >> 8);

            //State Change-------------------------------------------------------------------------------------------------
            //Collision logic provides options for exiting to Neutral
            //No other states can be reached from ladder.
        }
        break;
    //================================================================================================================
        case DASH_INIT:
            plat_state = DASH_STATE;
            jump_type = 0;
        case DASH_STATE: 
        {
            //Dash Interrupt for Editor Event.-------------------------------------------------------------------------------
            if(dash_interrupt){
                dash_dir = 0;
                plat_dash_currentframe = 0;
            }

            //Movement & Collision Combined----------------------------------------------------------------------------------
            tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
            tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;        
            col = 0;

            //Right Dash Movement & Collision
            if (dash_dir == 1){
                //Get tile x-coord of player position
                tile_current = ((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3;
                //Get tile x-coord of final position
                UWORD new_x = PLAYER.pos.x + (dash_dist);
                UBYTE tile_x = (((new_x >> 4) + PLAYER.bounds.right) >> 3) + 1;
                //CHECK EACH SPACE FROM START TO END
                while (tile_current != tile_x){
                    //CHECK TOP AND BOTTOM
                    while (tile_start != tile_end) {
                        //Check for Collisions
                        if(plat_dash_through != 3 || dash_end_clear == FALSE){ //If you collide with walls
                            if (tile_at(tile_current, tile_start) & COLLISION_LEFT) {
                                new_x = ((((tile_current) << 3) - PLAYER.bounds.right) << 4) -1;
                                col = 1;
                                last_wall = -1;
                                wc_val = plat_coyote_max;
                                plat_dash_currentframe == 0;
                                goto endRcol;
                            }   
                        }
                        //Check for Triggers
                        if (plat_dash_through < 2){
                            if (trigger_at_tile(tile_current, tile_start) != NO_TRIGGER_COLLISON) {
                                new_x = ((((tile_current+1) << 3) - PLAYER.bounds.right) << 4);
                            }
                        }
                        tile_start++;
                    }
                    tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top) >> 3);
                    tile_current += 1;
                }
                endRcol: 
                if(plat_dash_momentum == 1 || plat_dash_momentum == 3){            
                    pl_vel_x = plat_run_vel;
                } else{
                    pl_vel_x = 0;
                }
                PLAYER.pos.x = MIN((image_width - 16) << 4, new_x);
            }

            //Left Dash Movement & Collision
            else  if (dash_dir == -1){
                //Get tile x-coord of player position
                tile_current = ((PLAYER.pos.x >> 4) + PLAYER.bounds.left) >> 3;
                //Get tile x-coord of final position
                WORD new_x = PLAYER.pos.x - (dash_dist);
                UBYTE tile_x = (((new_x >> 4) + PLAYER.bounds.left) >> 3)-1;
                //CHECK EACH SPACE FROM START TO END
                while (tile_current != tile_x){
                    //CHECK TOP AND BOTTOM
                    while (tile_start != tile_end) {   
                        //check for walls
                        if(plat_dash_through != 3 || dash_end_clear == FALSE){//If you collide with walls
                            if (tile_at(tile_current, tile_start) & COLLISION_RIGHT) {
                                new_x = ((((tile_current + 1) << 3) - PLAYER.bounds.left) << 4)+1;
                                col = -1;
                                last_wall = 1;
                                plat_dash_currentframe == 0;
                                wc_val = plat_coyote_max;
                                goto endLcol;
                            }
                        }
                        //Check for triggers
                        if (plat_dash_through  < 2){
                            if (trigger_at_tile(tile_current, tile_start) != NO_TRIGGER_COLLISON) {
                                new_x = ((((tile_current - 1) << 3) - PLAYER.bounds.left) << 4);
                                goto endLcol;
                            }
                        }  
                        tile_start++;
                    }
                    tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top) >> 3);
                    tile_current -= 1;
                }
                endLcol: 
                if(plat_dash_momentum == 1 || plat_dash_momentum == 3){            
                    pl_vel_x = -plat_run_vel;
                } else{
                    pl_vel_x = 0;
                }
                PLAYER.pos.x = MAX(0, new_x);
            }

            //Vertical Movement & Collision-------------------------------------------------------------------------
            if(plat_dash_momentum >= 2){
                //Add Normal gravity
                pl_vel_y += plat_hold_grav;

                //Add Jump force
                if (INPUT_PRESSED(INPUT_PLATFORM_JUMP)){
                    //We're going to use CT as a proxy for being grounded. This doesn't work as well with jump --> dash...
                    if (ct_val != 0){
                    //Coyote Time Jump
                        mp_attached = FALSE;
                        pl_vel_y = -(plat_jump_min + (plat_jump_vel/2));
                        jb_val = 0;
                        ct_val = 0;
                        jump_type = 1;
                    } else if (dj_val != 0){
                    //Double Jump
                        dj_val -= 1;
                        plat_jump_reduction_val += jump_reduction;
                        mp_attached = FALSE;
                        pl_vel_y = -(plat_jump_min + (plat_jump_vel/2));
                        jb_val = 0;
                        ct_val = 0;
                        jump_type = 2;
                    }
                } 

                //Vertical Collisions
                temp_y = PLAYER.pos.y;    
                deltaY = pl_vel_y >> 8;
                deltaY += actorColY;
                deltaY = CLAMP(deltaY, -127, 127);
                tile_start = (((PLAYER.pos.x >> 4) + PLAYER.bounds.left)  >> 3);
                tile_end   = (((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3) + 1;
                if (deltaY > 0) {

                //Moving Downward
                    WORD new_y = PLAYER.pos.y + deltaY;
                    UBYTE tile_yz = ((new_y >> 4) + PLAYER.bounds.bottom) >> 3;
                    while (tile_start != tile_end) {
                        if (tile_at(tile_start, tile_yz) & COLLISION_TOP) {                    
                            //Land on Floor
                            new_y = ((((tile_yz) << 3) - PLAYER.bounds.bottom) << 4) - 1;
                            mp_attached = FALSE; //Detach when MP moves through a solid tile.                                   
                            pl_vel_y = 0;
                            ground_reset();
                            break;
                        }
                        tile_start++;
                    }
                    PLAYER.pos.y = new_y;
                } else if (deltaY < 0) {

                    //Moving Upward
                    WORD new_y = PLAYER.pos.y + deltaY;
                    UBYTE tile_yz = (((new_y >> 4) + PLAYER.bounds.top) >> 3);
                    while (tile_start != tile_end) {
                        if (tile_at(tile_start, tile_yz) & COLLISION_BOTTOM) {
                            new_y = ((((UBYTE)(tile_yz + 1) << 3) - PLAYER.bounds.top) << 4) + 1;
                            pl_vel_y = 0;
                            break;
                        }
                        tile_start++;
                    }
                    PLAYER.pos.y = new_y;
                }
                // Clamp Y Velocity
                pl_vel_y = CLAMP(pl_vel_y,-plat_max_fall_vel, plat_max_fall_vel);
            }

            //STATE CHANGE: No exits above.------------------------------------------------------------------------------------
            //DASH -> NEUTRAL Check
            //Colliding with a wall sets the currentframe to 0 above.
            if (plat_dash_currentframe == 0){
                plat_state = FALL_INIT;
                dash_dir = 0;
            }
        }
        break;  
    //================================================================================================================
        case GROUND_INIT:
            plat_state = GROUND_STATE;
            jump_type = 0;
        case GROUND_STATE:{
            //Horizontal Motion---------------------------------------------------------------------------------------------
            if (INPUT_LEFT) {
                acceleration(-1);
            } else if (INPUT_RIGHT) {
                acceleration(1);
            } else {
                deceleration();
                deltaX = pl_vel_x;
            }

            //Add X motion from moving platforms
            deltaX = deltaX >> 8;
            if (mp_attached){
                //BOUNDS NEED TO BE CONVERTED TO FROM PIXELS TO POSITIONS
                if (PLAYER.pos.x + (PLAYER.bounds.left << 4) > last_mp->pos.x + (last_mp->bounds.right<< 4)) {
                    plat_state = FALL_INIT;
                    mp_attached = FALSE;
                }
                else if (PLAYER.pos.x + (PLAYER.bounds.right << 4) < last_mp->pos.x + (last_mp->bounds.left << 4)){
                    plat_state = FALL_INIT;
                    mp_attached = FALSE;
                }
                else{
                    deltaX += (last_mp->pos.x - mp_last_x);
                    mp_last_x = last_mp->pos.x;
                }
            }
            deltaX += actorColX;

            // Vertical Motion-------------------------------------------------------------------------------------------------
            if (mp_attached){
                pl_vel_y = 0;
            } else if (nocollide != 0){
                pl_vel_y += 2500; //magic number, rough minimum for actually having the player descend through a platform
            } 
            else {
                //Normal gravity
                pl_vel_y += plat_grav;
            }

            // Add Y motion from moving platforms
            deltaY = pl_vel_y >> 8;
            deltaY += actorColY;
            if (mp_attached){
                deltaY += last_mp->pos.y - mp_last_y;
                mp_last_y = last_mp->pos.y;
                temp_y = last_mp->pos.y;
            }
            else{
                temp_y = PLAYER.pos.y;
            }
      

         

            //Collision-----------------------------------------------------------------------------------------------------
            //Horizontal Collision Checks
            basic_x_col();

            //Vertical Collision Checks
            basic_y_col(drop_press);

            //STATE CHANGE: Above, basic_y_col can shift to FALL_STATE.--------------------------------------------------
            //GROUND -> DASH Check
            if((plat_state == GROUND_STATE || plat_state == GROUND_INIT) && dash_press){
                if(plat_dash_style != 1){
                    dash_check();
                }
            }
            //GROUND -> JUMP Check
            if (plat_state != DASH_STATE){    //If we started dashing, don't do other checks.
                if (nocontrol_v !=0){
                    //Do Nothing
                } else if (INPUT_PRESSED(INPUT_PLATFORM_JUMP)){
                    //Standard Jump
                    jump_type = 1;
                    jump_init();
                } else if (jb_val !=0){
                    //Jump Buffered Jump
                    jump_type = 1;
                    jump_init();
                }
                else{
            //GROUND -> LADDER Check
                    ladder_check();
                }
            }
        }
        break;
    //================================================================================================================
        case JUMP_INIT:
            plat_state = JUMP_STATE;
        case JUMP_STATE:
        {
            //Horizontal Movement-----------------------------------------------------------------------------------------
            if (nocontrol_h != 0){
                //No horizontal input
                deltaX = pl_vel_x;
            } else if (plat_air_control == 0){
                deltaX = pl_vel_x;
                //No accel or decel in mid-air
            } else if (INPUT_LEFT) {
                acceleration(-1);
            } else if (INPUT_RIGHT) {
                acceleration(1);
            } else {
                deceleration();
                deltaX = pl_vel_x;
            }

            //Vertical Movement-------------------------------------------------------------------------------------------

            //Add Jump Force
            if (plat_hold_jump_val !=0 && INPUT_PLATFORM_JUMP){
                pl_vel_y -= plat_jump_val;
                //Reduce subsequent jump amounts
                if (plat_jump_vel >= plat_jump_reduction_val){
                    pl_vel_y += plat_jump_reduction_val;
                } else {
                    pl_vel_y = 0;
                }
            //Add jump boost from horizontal movement
                if (pl_vel_x > 0){
                    //This defines the amount of boost. It's important to devide pl_vel_x rather than plat_run_boost because otherwise we just get 1.
                    WORD tempBoost = plat_run_boost*(pl_vel_x/plat_hold_jump_max);
                    //This is a test to see if the results will overflow pl_vel_y. Note, pl_vel_y is negative here.
                    if(tempBoost > 32767 + pl_vel_y){
                        pl_vel_y = -32767;
                    }
                    else{
                        pl_vel_y -= tempBoost;
                    }
                }
                else if (pl_vel_x < 0){
                    WORD tempBoost = plat_run_boost*(-pl_vel_x/plat_hold_jump_max);
                    if(tempBoost > 32767 + pl_vel_y){
                        pl_vel_y = -32767;
                    }
                    else{
                        pl_vel_y -= tempBoost;
                    }
                }
                plat_hold_jump_val -=1;
            } else if (INPUT_PLATFORM_JUMP && pl_vel_y < 0){
                ct_val = 0;
                pl_vel_y += plat_hold_grav;
            } else {
                //Shift out of jumping if the player stops pressing OR if they run out of jump frames
                ct_val = 0;
                plat_state = FALL_INIT;
            }

            //Collision Checks ---------------------------------------------------------------------------------------
            //Horizontal Collision
            deltaX = deltaX >> 8;
            deltaX += actorColX;
            basic_x_col();

            //Vertical Collision
            deltaY = pl_vel_y >> 8;
            deltaY += actorColY;
            deltaY = CLAMP(deltaY,-127,127);
            temp_y = PLAYER.pos.y;    
            tile_start = (((PLAYER.pos.x >> 4) + PLAYER.bounds.left)  >> 3);
            tile_end   = (((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3) + 1;
            if (deltaY < 0) {
                //Moving Upward
                WORD new_y = PLAYER.pos.y + deltaY;
                UBYTE tile_y = (((new_y >> 4) + PLAYER.bounds.top) >> 3);
                while (tile_start != tile_end) {
                    if (tile_at(tile_start, tile_y) & COLLISION_BOTTOM) {
                        new_y = ((((UBYTE)(tile_y + 1) << 3) - PLAYER.bounds.top) << 4) + 1;
                        pl_vel_y = 0;
                        ct_val = 0;
                        plat_state = FALL_INIT;
                        break;
                    }
                    tile_start++;
                }
                PLAYER.pos.y = new_y;
            }
            // Clamp Y Velocity
            pl_vel_y = CLAMP(pl_vel_y,-plat_max_fall_vel, plat_max_fall_vel);

            //STATE CHANGE------------------------------------------------------------------------------------------------
            //Above: JUMP-> NEUTRAL when a) player starts descending, b) player hits roof, c) player stops pressing, d)jump frames run out.
            //JUMP -> WALL check
            wall_check();
            //JUMP -> DASH check
            if(dash_press){
                if(plat_dash_style != 0 || ct_val != 0){
                    dash_check();
                }
            }
            //JUMP -> LADDER check
            if (plat_state != DASH_STATE){
                ladder_check();
            }
        }
        break;
    //================================================================================================================
        case WALL_INIT:
            plat_state = WALL_STATE;
        case WALL_STATE:{
            //Horizontal Movement----------------------------------------------------------------------------------------
            if (INPUT_LEFT) {
                acceleration(-1);
            } else if (INPUT_RIGHT) {
                acceleration(1);
            } else {
                deceleration();
                deltaX = pl_vel_x;
            }

            //Vertical Movement------------------------------------------------------------------------------------------
            //WALL SLIDE
            if (nocollide != 0){
                pl_vel_y += 3000; //magic number, rough minimum for actually having the player descend through a platform
            } else if (pl_vel_y < 0){
                pl_vel_y += plat_grav;
            } else if (plat_wall_slide) {
                pl_vel_y = plat_wall_grav;
                }
            else{
                pl_vel_y += plat_grav;
            }

            //Collision--------------------------------------------------------------------------------------------------
            //Horizontal Collision Checks
            deltaX = deltaX >> 8;
            deltaX += actorColX;
            basic_x_col();

            //Vertical Collision Checks
            deltaY = pl_vel_y >> 8;
            deltaY += actorColY;
            temp_y = PLAYER.pos.y;    
            basic_y_col(drop_press);

            //STATE CHANGE------------------------------------------------------------------------------------------------
            //Above, basic_y_col can cause WALL -> GROUNDED.
            //Exit state as baseline
            wall_check();
            //WALL -> DASH Check
            if(dash_press){
                if (plat_state == WALL_STATE && plat_dash_style != 0){
                    dash_check();
                }
                else if (plat_state == GROUND_INIT && plat_dash_style != 1){
                    dash_check();
                }
            }
            //WALL -> JUMP Check
            if (plat_state != DASH_STATE){
                if (nocontrol_v !=0){
                    //Do Nothing
                } else if (INPUT_PRESSED(INPUT_PLATFORM_JUMP)){
                    //Wall Jump
                    if(wj_val != 0){
                        wj_val -= 1;
                        nocontrol_h = 5;
                        pl_vel_x += (plat_wall_kick + plat_walk_vel)*-last_wall;
                        jump_type = 3;
                        jump_init();

                    }
                } else {
            //WALL -> LADDER Check
                    ladder_check();
                }
            }
        }
        break;
    //================================================================================================================
        case FALL_INIT:
            jump_type = 0;
            plat_state = FALL_STATE;
        case FALL_STATE: {
            //Horizontal Movement----------------------------------------------------------------------------------------
            if (nocontrol_h != 0){
                //No horizontal input
                deltaX = pl_vel_x;
            } else if (plat_air_control == 0){
                deltaX = pl_vel_x;
                //No accel or decel in mid-air
            } else if (INPUT_LEFT) {
                acceleration(-1);
            } else if (INPUT_RIGHT) {
                acceleration(1);
            } else {
                deceleration();
                deltaX = pl_vel_x;
            }
        
            //Vertical Movement--------------------------------------------------------------------------------------------
            if (nocollide != 0){
                pl_vel_y += 2500; //magic number, rough minimum for actually having the player descend through a platform
            } 
            else if (INPUT_PLATFORM_JUMP && pl_vel_y < 0) {
                //Gravity while holding jump
                pl_vel_y += plat_hold_grav;
            } else if (float_press && pl_vel_y > 0){
                pl_vel_y = plat_float_grav;
            } else {
                //Normal gravity
                pl_vel_y += plat_grav;
            }
        

            //Collision ---------------------------------------------------------------------------------------------------
            //Horizontal Collision Checks
            deltaX = deltaX >> 8;
            deltaX += actorColX;
            basic_x_col();

            //Vertical Collision Checks
            deltaY = pl_vel_y >> 8;
            deltaY += actorColY;
            temp_y = PLAYER.pos.y;    
            basic_y_col(drop_press); 
            
            //STATE CHANGE------------------------------------------------------------------------------------------------
            //Above: NEUTRAL -> GROUND in basic_y_col()
            //NEUTRAL -> WALL check
            wall_check();
            //NEUTRAL -> DASH check
            if(dash_press){
                if ((plat_state == FALL_STATE || plat_state == FALL_INIT) && plat_dash_style != 0){
                    dash_check();
                }
                else if (plat_state == GROUND_INIT && plat_dash_style != 1){
                    dash_check();
                }
            }
            //NEUTRAL -> JUMP check
            if(plat_state != DASH_STATE){
                if (nocontrol_v !=0){
                    //Do Nothing
                } else if (INPUT_PRESSED(INPUT_PLATFORM_JUMP)){
                    //Wall Jump
                    if(wc_val != 0){
                        if(wj_val != 0){
                            jump_type = 3;
                            wj_val -= 1;
                            nocontrol_h = 5;
                            pl_vel_x += (plat_wall_kick + plat_walk_vel)*-last_wall;
                            jump_init();
                        }
                    }
                    if (ct_val != 0){
                    //Coyote Time Jump
                        jump_type = 1;
                        jump_init();
                    } else if (dj_val != 0){
                    //Double Jump
                        jump_type = 2;
                        dj_val -= 1;
                        plat_jump_reduction_val += jump_reduction;
                        jump_init();
                    } else {
                    // Setting the Jump Buffer when jump is pressed while not on the ground
                    jb_val = plat_buffer_max; 
                    }
                } else{
            //NEUTRAL -> LADDER check
                    ladder_check();
                } 
            }
        }
        break;
    }
    
    //Where to put this?
    //Ground Player when on MP  
    if (mp_attached){
        if(last_mp->disabled == TRUE){
            mp_attached = FALSE;
        }
        else{
            plat_state = GROUND_INIT;
        }
    }

    // Check for trigger collisions
    if(plat_state != DASH_STATE || plat_dash_through < 2){
        if (trigger_activate_at_intersection(&PLAYER.bounds, &PLAYER.pos, INPUT_UP_PRESSED)) {
            // Landed on a trigger
            return;
        }
    }

    //Don't hit actors while dashing
    actorColX = 0;
    actorColY = 0;
    if(plat_state != DASH_STATE || plat_dash_through == 0){
        //Actor Collisions
        //Q: Does this only ever return a single actor?
        hit_actor = actor_overlapping_player(FALSE);
        if (hit_actor != NULL && hit_actor->collision_group) {
            //Solid Actors
            if (hit_actor->collision_group == plat_solid_group){
                if(!mp_attached || hit_actor != last_mp){
                    if (temp_y < hit_actor->pos.y + (hit_actor->bounds.top << 4) && pl_vel_y >= 0){
                        //Attach to MP
                        last_mp = hit_actor;
                        mp_last_x = hit_actor->pos.x;
                        mp_last_y = hit_actor->pos.y;
                        PLAYER.pos.y = hit_actor->pos.y + (hit_actor->bounds.top << 4) - (PLAYER.bounds.bottom << 4) - 4;
                        //Other cleanup
                        pl_vel_y = 0;
                        mp_attached = TRUE;                        
                        plat_state = GROUND_INIT;
                        ground_reset();
                        //PLAYER bounds top seems to be 0 and counting down...
                    } else if (temp_y + (PLAYER.bounds.top<<4) > hit_actor->pos.y + (hit_actor->bounds.bottom<<4)){
                        actorColY += (hit_actor->pos.y - PLAYER.pos.y) + ((-PLAYER.bounds.top + hit_actor->bounds.bottom)<<4) + 32;
                        pl_vel_y = 0;
                        if (plat_state == JUMP_STATE){
                            plat_state = FALL_STATE;
                        }
                    } else if (PLAYER.pos.x < hit_actor->pos.x){
                        actorColX = (hit_actor->pos.x - PLAYER.pos.x) - ((PLAYER.bounds.right + hit_actor->bounds.left)<<4);
                        pl_vel_x = 0;
                        if(plat_state == DASH_STATE){
                            plat_state = FALL_STATE;
                        }
                    } else if (PLAYER.pos.x > hit_actor->pos.x){
                        actorColX = (hit_actor->pos.x - PLAYER.pos.x) + ((-PLAYER.bounds.left + hit_actor->bounds.right)<<4)+16;
                        pl_vel_x = 0;
                        if(plat_state == DASH_STATE){
                            plat_state = FALL_STATE;
                        }
                    }
                }
            } else if (hit_actor->collision_group == plat_mp_group){
                //Platform Actors
                if(!mp_attached){
                    if (temp_y < hit_actor->pos.y + (hit_actor->bounds.top << 4) && pl_vel_y >= 0){
                        //Attach to MP
                        last_mp = hit_actor;
                        mp_last_x = hit_actor->pos.x;
                        mp_last_y = hit_actor->pos.y;
                        PLAYER.pos.y = hit_actor->pos.y + (hit_actor->bounds.top << 4) - (PLAYER.bounds.bottom << 4);
                        //Other cleanup
                        pl_vel_y = 0;
                        mp_attached = TRUE;                        
                        plat_state = GROUND_INIT;
                        ground_reset();
                    }
                }
            }
            //All Other Collisions
            player_register_collision_with(hit_actor);
        } else if (INPUT_PRESSED(INPUT_PLATFORM_INTERACT)) {
            if (!hit_actor) {
                hit_actor = actor_in_front_of_player(8, TRUE);
            }
            if (hit_actor && !hit_actor->collision_group && hit_actor->script.bank) {
                script_execute(hit_actor->script.bank, hit_actor->script.ptr, 0, 1, 0);
            }
        }
    }

    //For compatability with mods that check 'grounded'
    if(plat_state == GROUND_STATE || plat_state == GROUND_INIT){
        grounded = true;
    } else{
        grounded = false;
    }


    //COUNTERS===============================================================
    // Counting Down Dash Frames
    if (plat_dash_currentframe != 0){
        plat_dash_currentframe -= 1;
    }
 
	// Counting down Jump Buffer Window
	if (jb_val != 0){
		jb_val -= 1;
	}
	
	// Counting down Coyote Time Window
	if (ct_val != 0){
		ct_val -= 1;
	}
    //Counting down Wall Coyote Time
    if (wc_val !=0 && col == 0){
        wc_val -= 1;
    }

    // Counting down No Control frames
    if (nocontrol_h != 0){
        nocontrol_h -= 1;
    }
    if (nocontrol_v != 0){
        nocontrol_v -= 1;
    }
    if (nocollide != 0){
        nocollide -= 1;
    }
    if (dash_ready_val != 0){
        dash_ready_val -=1;
    }
    if (tap_val > 0){
        tap_val -= 1;
    } else if (tap_val < 0){
        tap_val += 1;
    }

    //Hone Camera
    if (camera_deadzone_x > PLATFORM_CAMERA_DEADZONE_X){
        camera_deadzone_x -= 1;
    }


    // Player animation=========================================================================
    // These should be moved within the state machine to get rid of these IFs
    if (plat_state == LADDER_STATE || plat_state == LADDER_INIT) {
        actor_set_anim(&PLAYER, ANIM_CLIMB);
        if (pl_vel_y == 0) {
            actor_stop_anim(&PLAYER);
        }
    } else if (plat_state == GROUND_STATE || plat_state == GROUND_INIT) {
        if (pl_vel_x < 0) {
            actor_set_dir(&PLAYER, DIR_LEFT, TRUE);
        } else if (pl_vel_x > 0) {
            actor_set_dir(&PLAYER, DIR_RIGHT, TRUE);
        } else {
            actor_set_anim_idle(&PLAYER);
        }
    } else if (plat_state == WALL_STATE || plat_state == WALL_INIT) {
    //Face away from walls
        if (col == 1){
            actor_set_dir(&PLAYER, DIR_RIGHT, TRUE);
            //actor_set_anim(&PLAYER, ANIM_JUMP_RIGHT);
        } else if (col == -1){
            actor_set_dir(&PLAYER, DIR_LEFT, TRUE);
            //actor_set_anim(&PLAYER, ANIM_JUMP_LEFT);
    //Flip in the air
        } 
    }
    else{
        //This needs to be turned into a function that can be placed at the bottom of the other state machine states.
        //Other states
        if (pl_vel_x < 0 && PLAYER.dir != DIR_LEFT) {
            actor_set_dir(&PLAYER, DIR_LEFT, TRUE);
        } else if (pl_vel_x > 0 && PLAYER.dir != DIR_RIGHT) {
            actor_set_dir(&PLAYER, DIR_RIGHT, TRUE);
        } 
        if (PLAYER.dir == DIR_LEFT){
            actor_set_anim(&PLAYER, ANIM_JUMP_LEFT);
        } else {
            actor_set_anim(&PLAYER, ANIM_JUMP_RIGHT);
        }
    } 
}

void acceleration(BYTE dir) BANKED {
    //Right now, the clamp limits the carry-over from slipperiness. I'm not sure it actually adds anything to the simulation though. 
    //If I want to add a turn around speed, the clamp is it. Two options: 1, turn 'minimum_vel' into a signed number, that way it can have neg velocities for setting the MAX slipperiness
    //That is probably enough, but I could also add a specific turn around acceleration.
    //Ideally, I should probably make smooth acceleration use walk speed until it's at full too.


    //Can I make this work by multiplying pl_vel_x by dir, so it becomes positive, and then multiplying it by dir again at the end to 
    WORD tempSpd = pl_vel_x * dir;
    WORD mid_run = 0;

    if (INPUT_PLATFORM_RUN){
        switch(plat_run_type) {
            case 0:
            //Ordinay Walk as below
                run_stage = 0;
                tempSpd += plat_walk_acc;
                tempSpd = CLAMP(tempSpd, plat_min_vel, plat_walk_vel);
            break;
            case 1:
            //Type 1: Smooth Acceleration as the Default in GBStudio
                tempSpd += plat_run_acc;
                tempSpd = CLAMP(tempSpd, plat_min_vel, plat_run_vel);
                run_stage = 1;
            break;
            case 2:
            //Type 2: Enhanced Smooth Acceleration
                if(tempSpd < 0){
                    tempSpd += plat_turn_acc;
                    tempSpd = MIN(tempSpd, plat_min_vel);
                    run_stage = -1;
                }
                else if (tempSpd < plat_walk_vel){
                    tempSpd += plat_walk_acc;
                    run_stage = 1;
                }
                else{
                    tempSpd += plat_run_acc;
                    tempSpd = MIN(tempSpd, plat_run_vel);
                    run_stage = 2;
                }
            break;
            case 3:
            //Type 3: Instant acceleration to full speed
                tempSpd = plat_run_vel;
                run_stage = 1;
            break;
            case 4:
            //Type 4: Tiered acceleration with 2 speeds
                //If we're below the walk speed, use walk acceleration
                if (tempSpd < 0){
                    tempSpd += plat_turn_acc;
                    tempSpd = MIN(tempSpd, plat_min_vel);
                    run_stage = -1;
                } else if(tempSpd < plat_walk_vel){
                    tempSpd += plat_walk_acc;
                    run_stage = 1;
                } else if (tempSpd < plat_run_vel){
                //If we're above walk, but below the run speed, use run acceleration
                    tempSpd += plat_run_acc;
                    tempSpd = CLAMP(tempSpd, plat_min_vel, plat_run_vel);
                    pl_vel_x = tempSpd*dir; //We need to have this here because this part returns a different value than other sections
                    run_stage = 2;
                    deltaX = dir*plat_walk_vel;
                    return;
                } else{
                //If we're at run speed, stay there
                    run_stage = 3;
                }
            break;
            case 5:
                mid_run = (plat_run_vel - plat_walk_vel)/2;
                mid_run += plat_walk_vel;
            //Type 4: Tiered acceleration with 3 speeds
                if (tempSpd < 0){
                    tempSpd += plat_turn_acc;
                    tempSpd = MIN(tempSpd, plat_min_vel);
                    run_stage = -1;
                }else if(tempSpd < plat_walk_vel){
                    tempSpd += plat_walk_acc;
                    run_stage = 1;
                } else if (tempSpd < mid_run){
                //If we're above walk, but below the mid-run speed, use run acceleration
                    tempSpd += plat_run_acc;
                    pl_vel_x = tempSpd*dir; //We need to have this here because this part returns a different value than other sections
                    run_stage = 2;
                    deltaX = dir*plat_walk_vel;
                    return;
                } else if (tempSpd < plat_run_vel){
                //If we're above walk, but below the run speed, use run acceleration
                    tempSpd += plat_run_acc;
                    tempSpd = CLAMP(tempSpd, plat_min_vel, plat_run_vel);
                    pl_vel_x = tempSpd*dir; //We need to have this here because this part returns a different value than other sections
                    run_stage = 3;
                    deltaX = dir*mid_run;
                    return;
                } else{
                //If we're at run speed, stay there
                    run_stage = 4;
                }
            break;
        }
    } else {
        //Ordinay Walk
            run_stage = 0;
            tempSpd += plat_walk_acc;
            tempSpd = CLAMP(tempSpd, plat_min_vel, plat_walk_vel); 
    }
            
    pl_vel_x = tempSpd*dir;
    deltaX = pl_vel_x;
}

void deceleration() BANKED {
    //b. Deceleration (ground and air)
    if (pl_vel_x < 0) {
        if (plat_state == GROUND_STATE){
            pl_vel_x += plat_dec;
        } else { 
            pl_vel_x += plat_air_dec;
        }
        if (pl_vel_x > 0) {
            pl_vel_x = 0;
        }
    } else if (pl_vel_x > 0) {
        if (plat_state == GROUND_STATE){
            pl_vel_x -= plat_dec;
            }
        else { 
            pl_vel_x -= plat_air_dec;
            }
        if (pl_vel_x < 0) {
            pl_vel_x = 0;
        }
    }
    run_stage = 0;
}

void dash_check() BANKED {
    //Initialize Dash
    //Pre-Check for input and if we're already in a dash
    UBYTE tile_start, tile_end;

    //Pre-check for recharge and script interrupt
    if (dash_ready_val == 0 && !dash_interrupt){
        //Start Dashing!
        plat_state = DASH_INIT;
        if (INPUT_RIGHT){
            dash_dir = 1;
        }
        else if(INPUT_LEFT){
            dash_dir = -1;
        }
        else if (PLAYER.dir == DIR_LEFT){
            dash_dir = -1;
        }
        else {
            dash_dir = 1;
        }
        
        //Dash through walls extra checks
        if(plat_dash_through == 3 && plat_dash_momentum < 2){
            dash_end_clear = true;
            tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
            tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;     
            WORD new_x = PLAYER.pos.x + (dash_dir*(dash_dist*(plat_dash_frames)));

            //Check for a landing spot on the right
            if (dash_dir == 1){
                if (PLAYER.pos.x + (PLAYER.bounds.right <<4) + (dash_dist*(plat_dash_frames)) > (image_width -16) <<4){   
                    dash_end_clear = false;                                     //Don't dash off the screen to the right
                }
                else{
                    UBYTE tile_xr = ((new_x >> 4) + PLAYER.bounds.right) >> 3;  
                    UBYTE tile_xl = ((new_x >> 4) + PLAYER.bounds.left) >> 3;
                    while (tile_start != tile_end) {
                        if (tile_at(tile_xr, tile_start) & COLLISION_LEFT) {
                                dash_end_clear = false;
                                break;
                        }
                        else if (tile_at(tile_xl, tile_start) & COLLISION_RIGHT) {
                                dash_end_clear = false;
                                break;
                        }
                        tile_start++;
                    }
                }
            }
            //Check for a landing spot on the left
            else if(dash_dir == -1) {
                if (PLAYER.pos.x <= ((dash_dist*(plat_dash_frames))+(PLAYER.bounds.left << 4))+(8<<4)){
                    dash_end_clear = false;         //To get around unsigned position, test if the player's current position is less than the total dist.
                }
                else{
                    UBYTE tile_xr = ((new_x >> 4) + PLAYER.bounds.right) >> 3;
                    UBYTE tile_xl = ((new_x >> 4) + PLAYER.bounds.left) >> 3;
                    while (tile_start != tile_end) {
                        if (tile_at(tile_xr, tile_start) & COLLISION_LEFT) {
                                dash_end_clear = false;
                                break;
                        }
                        else if (tile_at(tile_xl, tile_start) & COLLISION_RIGHT) {
                                dash_end_clear = false;
                                break;
                        }
                        tile_start++;
                    }
                }
            }
        }

        //INITIALIZE DASH
        mp_attached = FALSE;
        camera_deadzone_x = 32;
        dash_ready_val = plat_dash_ready_max + plat_dash_frames;
        if(plat_dash_momentum < 2){
            nocontrol_v = plat_dash_frames;
            pl_vel_y = 0;
        }
        plat_dash_currentframe = plat_dash_frames;
        tap_val = 0;
    }
}

void jump_init() BANKED {
    //Initialize Jumping
    plat_hold_jump_val = plat_hold_jump_max; 
    mp_attached = FALSE;
    pl_vel_y = -plat_jump_min;
    jb_val = 0;

    plat_state = JUMP_INIT;

}

void ladder_check() BANKED {
    UBYTE p_half_width = (PLAYER.bounds.right - PLAYER.bounds.left) >> 1;
    if (INPUT_UP) {
        // Grab upwards ladder
        UBYTE tile_x_mid = ((PLAYER.pos.x >> 4) + PLAYER.bounds.left + p_half_width) >> 3;
        UBYTE tile_y   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top) >> 3);
        if (tile_at(tile_x_mid, tile_y) & TILE_PROP_LADDER) {
            PLAYER.pos.x = (((tile_x_mid << 3) + 4 - (PLAYER.bounds.left + p_half_width) << 4));
            plat_state = LADDER_INIT;
            pl_vel_x = 0;
        }
    } else if (INPUT_DOWN) {
        // Grab downwards ladder
        UBYTE tile_x_mid = ((PLAYER.pos.x >> 4) + PLAYER.bounds.left + p_half_width) >> 3;
        UBYTE tile_y   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;
        if (tile_at(tile_x_mid, tile_y) & TILE_PROP_LADDER) {
            PLAYER.pos.x = (((tile_x_mid << 3) + 4 - (PLAYER.bounds.left + p_half_width) << 4));
            plat_state = LADDER_INIT;
            pl_vel_x = 0;
        }
    }
}

void wall_check() BANKED {
    //Wall-State Check
    if(plat_state != GROUND_STATE && pl_vel_y >= 0 && plat_wall_slide){
        if ((col == 1 && INPUT_LEFT) || (col == -1 && INPUT_RIGHT)){
            if (plat_state != WALL_STATE){plat_state = WALL_INIT;}
        }
        else if (plat_state == WALL_STATE){plat_state = FALL_INIT;}
    }
}

void basic_x_col() BANKED {
    UBYTE tile_start = (((PLAYER.pos.y >> 4) + PLAYER.bounds.top)    >> 3);
    UBYTE tile_end   = (((PLAYER.pos.y >> 4) + PLAYER.bounds.bottom) >> 3) + 1;        
    col = 0;
    deltaX = CLAMP(deltaX, -127, 127);
    if (deltaX > 0) {
        UWORD new_x = PLAYER.pos.x + deltaX;
        UBYTE tile_x = ((new_x >> 4) + PLAYER.bounds.right) >> 3;
        while (tile_start != tile_end) {
            if (tile_at(tile_x, tile_start) & COLLISION_LEFT) {
                new_x = (((tile_x << 3) - PLAYER.bounds.right) << 4) - 1;
                pl_vel_x = 0;
                col = -1;
                last_wall = 1;
                wc_val = plat_coyote_max;
                break;
            }
            tile_start++;
        }
        PLAYER.pos.x = MIN((image_width - 16) << 4, new_x);
    } else if (deltaX < 0) {      
        WORD new_x = PLAYER.pos.x + deltaX;
        UBYTE tile_x = ((new_x >> 4) + PLAYER.bounds.left) >> 3;
        while (tile_start != tile_end) {
            if (tile_at(tile_x, tile_start) & COLLISION_RIGHT) {
                new_x = ((((tile_x + 1) << 3) - PLAYER.bounds.left) << 4) + 1;
                pl_vel_x = 0;
                col = 1;
                last_wall = -1;
                wc_val = plat_coyote_max;
                break;
            }
            tile_start++;
        }
        PLAYER.pos.x = MAX(0, new_x);
    }
}

void basic_y_col(UBYTE drop_press) BANKED {
    UBYTE tile_start, tile_end;
    deltaY = CLAMP(deltaY, -127, 127);
    UBYTE tempState = plat_state;
    tile_start = (((PLAYER.pos.x >> 4) + PLAYER.bounds.left)  >> 3);
    tile_end   = (((PLAYER.pos.x >> 4) + PLAYER.bounds.right) >> 3) + 1;
    if (deltaY > 0) {
        //Moving Downward
        WORD new_y = PLAYER.pos.y + deltaY;
        UBYTE tile_yz = ((new_y >> 4) + PLAYER.bounds.bottom) >> 3;
        if (nocollide == 0){
            while (tile_start != tile_end) {
                if (tile_at(tile_start, tile_yz) & COLLISION_TOP) {
                    //Drop-Through Floor Check 
                    UBYTE drop_attempt = FALSE;
                    if (drop_press == TRUE){
                        drop_attempt = TRUE;
                        while (tile_start != tile_end) {
                            if (tile_at(tile_start, tile_yz) & COLLISION_BOTTOM){
                                drop_attempt = FALSE;
                                break;
                            }
                        tile_start++;
                        }
                    }
                    if (drop_attempt == TRUE){
                        nocollide = 10; //Magic Number, how many frames to steal vertical control
                        pl_vel_y += plat_grav; 
                    } else {
                        //Land on Floor
                        new_y = ((((tile_yz) << 3) - PLAYER.bounds.bottom) << 4) - 1;
                        mp_attached = FALSE; //Detach when MP moves through a solid tile.
                        if(plat_state != GROUND_STATE){plat_state = GROUND_INIT;}
                        pl_vel_y = 0;
                        //Various things that reset when Grounded
                        ground_reset();
                        PLAYER.pos.y = new_y;
                        pl_vel_y = CLAMP(pl_vel_y,-plat_max_fall_vel, plat_max_fall_vel);
                        return;
                    }
                }
                tile_start++;
            }
            if(plat_state == GROUND_STATE){plat_state = FALL_INIT;}
        }
        PLAYER.pos.y = new_y;
    } else if (deltaY < 0) {
        //Moving Upward
        WORD new_y = PLAYER.pos.y + deltaY;
        UBYTE tile_yz = (((new_y >> 4) + PLAYER.bounds.top) >> 3);
        while (tile_start != tile_end) {
            if (tile_at(tile_start, tile_yz) & COLLISION_BOTTOM) {
                new_y = ((((UBYTE)(tile_yz + 1) << 3) - PLAYER.bounds.top) << 4) + 1;
                pl_vel_y = 0;
                //MP Test: Attempting stuff to stop the player from continuing upward
                if(mp_attached){
                    mp_attached = FALSE;
                    new_y = last_mp->pos.y;
                }
                break;
            }
            tile_start++;
        }
        PLAYER.pos.y = new_y;
    }
    else if (mp_attached){
        plat_state = GROUND_INIT;
    }
    // Clamp Y Velocity
    pl_vel_y = CLAMP(pl_vel_y,-plat_max_fall_vel, plat_max_fall_vel);
}

void ground_reset() NONBANKED{
    //Various things that reset when Grounded
    ct_val = plat_coyote_max; 
    dj_val = plat_extra_jumps; 
    wj_val = plat_wall_jump_max;
    plat_jump_reduction_val = 0;
}

