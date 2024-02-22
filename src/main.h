#ifndef MAIN_H
#define MAIN_H

#include <gpu.h>
#include <input.h>

#include "background.m3if.asset"
#include "blaster0.m3if.asset"
#include "blaster1.m3if.asset"
#include "blaster2.m3if.asset"
#include "blaster3.m3if.asset"
#include "blaster4.m3if.asset"
#include "error.m3if.asset"
#include "flash.m3if.asset"
#include "flipped.m3if.asset"
#include "graphics.h"
#include "player.m3if.asset"
#include "unflipped.m3if.asset"
#include "warning.m3if.asset"
#include <gpu.h>
#include <mes.h>
#include <rng.h>
#include <stdbool.h>
#include <stdint.h>
#include <timer.h>

#define FPS 10
#define FRAMETIME ((1.0 / FPS) * 1000)

#define AIR 0
#define UNFLIPPED 1
#define FLIPPED 2
#define BLASTER 3
#define ERR_BLOCK 4

#define ROWS 7
#define COLUMNS 6

#define GRAVITY (10 / 1000.0)
#define GLIDE_SPEED (16 / 1000.0)
#define JUMP_HEIGHT (85 / 1000.0)
#define STOMP_SPEED (7 / 1000.0)
#define HORIZONTAL_SPEED (65 / 1000.0)
#define MIN_NEW_BIT_DELAY 250
#define MAX_NEW_BIT_DELAY 4000
#define WARNING_DURATION 1000
#define BLASTER_CHANCE 0.1
#define FLASH_DURATION 50
#define DESTROIED_BIT_ANIMATION_DURATION 500
#define LASER_SHOT_DURATION 250
#define LASER_SHOT_DELAY 4000
#define DEATH_BLINK_SPEED 750
#define THREE_FALL_THRESHHOLD 25
#define RESPAWN_INTERVAL 12000

#define ALLOWED_FLOAT_ERROR 0.0001

#define BLACK 0
#define WHITE 1
#define YELLOW 2
#define RED 3
#define DARK_RED 4
#define GREEN 5
#define BLUE 6
#define GRAY 7

#define MAX(a, b) (a > b ? a : b)
// palette
uint16_t palette[8] = {
    COLOR_TO_GPIO(0b000, 0b000, 0b000), COLOR_TO_GPIO(0b111, 0b111, 0b111),
    COLOR_TO_GPIO(0b111, 0b111, 0b000), COLOR_TO_GPIO(0b111, 0b000, 0b000),
    COLOR_TO_GPIO(0b110, 0b001, 0b001), COLOR_TO_GPIO(0b000, 0b111, 0b000),
    COLOR_TO_GPIO(0b000, 0b111, 0b111), COLOR_TO_GPIO(0b011, 0b011, 0b011)};

struct point {
    float x;
    float y;
};

struct line {
    float x;
    float y;
    float a;
    float b;
};

struct line vertical_mesh[7];
int32_t vertical_mesh_count = 0;

struct line horizontal_mesh[7];
int32_t horizontal_mesh_count = 0;

struct player {
    float x;
    float y;
    float velx;
    float vely;
    int32_t stomped_obstacles;
    bool last_up_keystate;
    bool last_down_keystate;
    bool last_right_keystate;
    bool last_left_keystate;
    bool last_a_keystate;
    bool last_b_keystate;
    bool last_start_keystate;
    bool last_select_keystate;
    bool in_air;
    bool gliding;
    bool stomping;
    bool joined;
    bool dead;
    bool respawning;
    int32_t death_time;
};

// struct stage {
//     int32_t
// }

struct bit_descturction_animation {
    int32_t animation_start;
    int32_t row;
    int32_t column;
};

struct laser_shot {
    int32_t last_shot;
    int32_t row;
    int32_t column;
    int32_t texture;
    bool active;
    int32_t right_expansion;
    int32_t left_expansion;
    bool destroyed;
};

int32_t stop;
int32_t delta_start;

int32_t pause_menu_selected = 0;
int32_t paused_time = 0;

bool pause = false;
bool lost = false;
bool warning = false;
bool flash = false;
bool flash_spawned = false;
int32_t falling_blocks = 0;
int32_t last_warning_start = 0;
int32_t last_flash_start = 0;
int32_t interest_column = 0;
int32_t last_column = 0;
int32_t cycles = 0;
int32_t next_block = UNFLIPPED;

int32_t stomped_obstacle_score[6] = {100, 250, 500, 1000, 2500, 5000};
int32_t score = 0;
int32_t highscore = 0;
int32_t visualized_score = 0;
int32_t score_adder = 0;
int32_t sx;
struct player player0;
struct player player1;
struct player player2;
struct player player3;

struct bit_descturction_animation
    bit_descturction_animations[ROWS * COLUMNS + 1];
int32_t bit_destruction_animation_array_pointer = 0;

struct laser_shot laser_shots[ROWS * COLUMNS * 8 + 1];
int32_t laser_shots_poiter = 0;

int32_t bit_blocks[ROWS][COLUMNS];
int32_t delta;

Surface *prog_indic_0;
Surface *prog_indic_1;
Surface *prog_indic_2;
Surface *prog_indic_3;
Surface warning_texture;
Surface flash_texture;
Surface background_texture;
Surface blaster0_texture;
Surface blaster1_texture;
Surface blaster2_texture;
Surface blaster3_texture;
Surface blaster4_texture;
Surface unflipped_texture;
Surface flipped_texture;
Surface error_texture;
Surface player_texture;
Surface *box;

void handle_input(struct player *p1, int32_t player_id);

void update_player_status(struct player *p1, int32_t player_id);

void add_score(int32_t adder);

void update_score();

int32_t column_air_amount(int32_t column);

void spawn_warning();

void spawn_flash();

void end_flash();

bool check_for_limitreach(int32_t column);

void add_bit_descturction_animation(struct bit_descturction_animation ani);

void handle_new_laser(int32_t row, int32_t column);

void send_error_block(int32_t column);

void spawn_block();

void update_laser_shots();

void handle_box_collision(struct player *p1);

void correct_player_stomp_trajectory(struct player *p1);

void handle_stomp_obstacle(struct player *p1);

void handle_player_collision(struct player *p1);

void flip_blocks(struct player *p1);

void generate_mesh();

int32_t get_new_bit_delay();

void handle_block_collision(struct player *p1);

bool player_died(struct player *p1);

void draw_player(struct player p1, int32_t pindex);

void handle_player(struct player *p1, int32_t player_id);

bool has_lost();

void intPadder(int32_t number, int32_t padding, char *result);

void render();

uint8_t start(void);

#endif