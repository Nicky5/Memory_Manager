#include "main.h"
#include "gpu.h"
#include "gpu_internal.h"
#include "graphics.h"
#include "input.h"
#include "timer.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>

#include "pause.h"

void handle_input(struct player *p1, int32_t player_id) {

    if (input_get_button(player_id, BUTTON_B) && p1->in_air &&
        !p1->last_b_keystate) {
        p1->stomping = true;
    }
    if (input_get_button(player_id, BUTTON_LEFT) && !p1->stomping) {
        p1->velx += (int32_t)(-HORIZONTAL_SPEED * MAX(FRAMETIME, delta));
    }
    if (input_get_button(player_id, BUTTON_RIGHT) && !p1->stomping) {
        p1->velx += (int32_t)(HORIZONTAL_SPEED * MAX(FRAMETIME, delta));
        ;
    }

    //        if ((! p1.in_air or  p1.gliding) &&event.type == SDL_KEYDOWN &&
    //        event.key.keysym.sym == SDLK_UP) {
    //             p1.vely = -1.3;
    //        }
    if (input_get_button(player_id, BUTTON_A) && (!p1->in_air || p1->gliding) &&
        !p1->last_a_keystate) {
        if (p1->vely == 0) {
            p1->vely = -JUMP_HEIGHT;
        } else {
            p1->vely = -JUMP_HEIGHT / 1.6;
        }
        //        p1.y -= 0.05;
    }

    if (input_get_button(player_id, BUTTON_START)) {
        pause = true;
    }
    p1->last_up_keystate = input_get_button(player_id, BUTTON_UP);
    p1->last_down_keystate = input_get_button(player_id, BUTTON_DOWN);
    p1->last_right_keystate = input_get_button(player_id, BUTTON_RIGHT);
    p1->last_left_keystate = input_get_button(player_id, BUTTON_LEFT);
    p1->last_a_keystate = input_get_button(player_id, BUTTON_A);
    p1->last_b_keystate = input_get_button(player_id, BUTTON_B);
    p1->last_start_keystate = input_get_button(player_id, BUTTON_START);
    p1->last_select_keystate = input_get_button(player_id, BUTTON_SELECT);
}

void update_player_status(struct player *p1, int32_t player_id) {

    bool prev = p1->joined;
    p1->joined = input_is_available(player_id);
    if (!prev && p1->joined) {
        // spawn_player()
    }
}

void add_score(int32_t adder) {
    score += adder;
    score_adder = (int32_t)(((score - visualized_score) * 0.01) + 1);
}

void update_score() {
    if (visualized_score + score_adder >= score) {
        visualized_score = score;
    } else {
        visualized_score += score_adder;
    }
}

int32_t column_air_amount(int32_t column) {
    for (int32_t i = 0; i < ROWS; ++i) {
        if (bit_blocks[i][column] != AIR) {
            return i;
        }
    }
    return ROWS;
}

void spawn_warning() {
    cycles++;
    last_warning_start = get_unpaused_ms();
    interest_column = rng_u32() % 6;
    while (interest_column == last_column) {
        interest_column = rng_u32() % 6;
    }
    last_column = interest_column;
    warning = true;
    flash = false;
    if (rng_u32() % (int)(1 / BLASTER_CHANCE) == 0) {
        next_block = BLASTER;
    } else {
        next_block = UNFLIPPED;
    }
    if (falling_blocks < 2 && cycles > THREE_FALL_THRESHHOLD &&
        rng_u32() % 5 == 0) {
        falling_blocks = 4;
    } else {
        falling_blocks = 1;
    }
}

void spawn_flash() {
    flash_spawned = true;
    last_flash_start = get_unpaused_ms();
    warning = false;
    flash = true;
}

void end_flash() {
    warning = false;
    flash = false;
}

bool check_for_limitreach(int32_t column) {
    return bit_blocks[1][column] != AIR;
}

void add_bit_descturction_animation(struct bit_descturction_animation ani) {
    bit_descturction_animations[bit_destruction_animation_array_pointer] = ani;
    bit_destruction_animation_array_pointer++;
    if (bit_destruction_animation_array_pointer >= ROWS * COLUMNS)
        bit_destruction_animation_array_pointer = 0;
}

void handle_new_laser(int32_t row, int32_t column) {
    struct laser_shot shot;
    shot.row = row;
    shot.column = column;
    shot.last_shot = LASER_SHOT_DELAY + LASER_SHOT_DURATION;
    shot.destroyed = false;
    laser_shots[laser_shots_poiter] = shot;
    laser_shots_poiter++;
    if (laser_shots_poiter >= ROWS * COLUMNS * 8) {
        laser_shots_poiter = 0;
    }
}

void send_error_block(int32_t column) {
    bit_blocks[0][column] = AIR;
    for (int32_t i = 1; i < ROWS - 1; ++i) {
        struct bit_descturction_animation ani;
        ani.animation_start = get_unpaused_ms();
        ani.column = column;
        ani.row = i;
        if (bit_blocks[i + 1][column] == ERR_BLOCK) {
            bit_blocks[i][column] = ERR_BLOCK;
            return;
        }
        add_bit_descturction_animation(ani);
        bit_blocks[i][column] = AIR;
    }
    bit_blocks[6][column] = ERR_BLOCK;
}

void spawn_block() {
    falling_blocks--;
    for (int32_t i = 1; i < ROWS + 1; ++i) {
        if (bit_blocks[i][interest_column] != AIR) {
            bit_blocks[i - 1][interest_column] = next_block;
            if (next_block == BLASTER) {
                handle_new_laser(i - 1, interest_column);
            }
            return;
        }
    }
    bit_blocks[6][interest_column] = next_block;
    if (next_block == BLASTER) {
        handle_new_laser(6, interest_column);
    }
}

void update_laser_shots() {

    for (int i = 0; i < ROWS * COLUMNS * 8; i++) {
        if (laser_shots[i].destroyed) {
            continue;
        }
        if (bit_blocks[laser_shots[i].row][laser_shots[i].column] != BLASTER) {
            laser_shots[i].destroyed = true;
            return;
        }
        if (get_unpaused_ms() - laser_shots[i].last_shot > LASER_SHOT_DELAY) {
            laser_shots[i].active = true;
            laser_shots[i].texture = 4;
        } else {
            laser_shots[i].active = false;
            laser_shots[i].texture = 0;
            if (get_unpaused_ms() - laser_shots[i].last_shot >
                LASER_SHOT_DELAY * 0.55) {
                laser_shots[i].texture = 1;
            }
            if (get_unpaused_ms() - laser_shots[i].last_shot >
                LASER_SHOT_DELAY * 0.7) {
                laser_shots[i].texture = 2;
            }
            if (get_unpaused_ms() - laser_shots[i].last_shot >
                LASER_SHOT_DELAY * 0.85) {
                laser_shots[i].texture = 3;
            }
        }
        if (get_unpaused_ms() - laser_shots[i].last_shot >
            LASER_SHOT_DELAY + LASER_SHOT_DURATION) {
            laser_shots[i].active = false;
            laser_shots[i].last_shot = get_unpaused_ms();
            laser_shots[i].texture = 4;
        }
        int right = 0;
        int left = 0;
        for (int j = laser_shots[i].column + 1; j < COLUMNS; ++j) {
            if (bit_blocks[laser_shots[i].row][j] == AIR) {
                right++;
            } else
                break;
        }
        for (int j = laser_shots[i].column - 1; j >= 0; --j) {
            if (bit_blocks[laser_shots[i].row][j] == AIR) {
                left++;
            } else
                break;
        }
        laser_shots[i].right_expansion = right;
        laser_shots[i].left_expansion = left;
    }
}

void handle_box_collision(struct player *p1) {
    p1->gliding = false;
    if (p1->velx > 0) {
        if (p1->x + p1->velx + 7 * 1000 > 119 * 1000) {
            p1->velx = 0;
            p1->x = 119 * 1000 - 7 * 1000;
            p1->gliding = true;
        }
    } else if (p1->velx < 0) {
        if (p1->x + p1->velx < 43 * 1000) {
            p1->velx = 0;
            p1->x = 42 * 1000 + 2;
            p1->gliding = true;
        }
    }
    if (p1->vely >= 0 && p1->y >= 110000) {
        p1->vely = 0;
        p1->y = 119 * 1000 - 9 * 1000;
        p1->in_air = false;
    } else {
        p1->in_air = true;
    }
    if (p1->vely <= 0 && p1->y <= 32 * 1000) {
        p1->vely = 0;
    }
}

void correct_player_stomp_trajectory(struct player *p1) {
    int32_t x = ((int)round(((float)p1->x / 1000) + 4.5) - 42) % 13;
    if (x > 13 - 2.5) {
        p1->velx = (int32_t)(-STOMP_SPEED / 2 * MAX(FRAMETIME, delta));
        p1->vely = (int32_t)(STOMP_SPEED / 20 * MAX(FRAMETIME, delta));
    } else if (x < 3.5) {
        p1->velx = (int32_t)(STOMP_SPEED / 2 * MAX(FRAMETIME, delta));
        p1->vely = (int32_t)(STOMP_SPEED / 20 * MAX(FRAMETIME, delta));
    }
}

void handle_stomp_obstacle(struct player *p1) {
    if (p1->vely == 0) {
        return;
    }
    for (int32_t i = 0; i < ROWS; ++i) {
        for (int32_t j = 0; j < COLUMNS; ++j) {
            int32_t x = 42000 + (j * 13000);
            int32_t y = 35000 + (i * 12000);
            int32_t w = 12000;
            int32_t h = 12000;

            struct point p;
            p.y = p1->y + 9000 + p1->vely;
            p.x = p1->x + 3500;
            if (is_point_inside_rectangle(p.x, p.y, x, y, h, w)) {
                if (bit_blocks[i][j] == UNFLIPPED) {
                    bit_blocks[i][j] = FLIPPED;
                    add_score(10);
                    p1->vely = -JUMP_HEIGHT;
                    p1->in_air = true;
                    p1->stomping = false;
                    return;
                } else if (bit_blocks[i][j] == FLIPPED ||
                           bit_blocks[i][j] == BLASTER) {
                    bit_blocks[i][j] = AIR;
                    struct bit_descturction_animation ani;
                    ani.animation_start = get_unpaused_ms();
                    ani.column = j;
                    ani.row = i;
                    add_bit_descturction_animation(ani);
                    add_score(stomped_obstacle_score[p1->stomped_obstacles]);
                    p1->stomped_obstacles++;

                } else if (bit_blocks[i][j] == ERR_BLOCK) {
                    p1->vely = -JUMP_HEIGHT;
                    p1->in_air = true;
                    p1->stomping = false;
                    return;
                }
            }
        }
    }
}

void handle_player_collision(struct player *p1) {
    if ((!player0.stomping && !player0.dead && player0.joined) &&
        (player0.y > p1->y) &&
        rectanglesOverlap(p1->x, p1->y, 7000, 9000, player0.x, player0.y, 7000,
                          9000)) {
        player0.dead = true;
        player0.death_time = get_unpaused_ms();
        p1->vely = -JUMP_HEIGHT;
        p1->in_air = true;
        p1->stomping = false;
    }
    if ((!player1.stomping && !player1.dead && player1.joined) &&
        (player1.y > p1->y) &&
        rectanglesOverlap(p1->x, p1->y, 7000, 9000, player1.x, player1.y, 7000,
                          9000)) {
        player1.dead = true;
        player1.death_time = get_unpaused_ms();
        p1->vely = -JUMP_HEIGHT;
        p1->in_air = true;
        p1->stomping = false;
    }
    if ((!player2.stomping && !player2.dead && player2.joined) &&
        (player2.y > p1->y) &&
        rectanglesOverlap(p1->x, p1->y, 7000, 9000, player2.x, player2.y, 7000,
                          9000)) {
        player2.dead = true;
        player2.death_time = get_unpaused_ms();
        p1->vely = -JUMP_HEIGHT;
        p1->in_air = true;
        p1->stomping = false;
    }
    if ((!player3.stomping && !player3.dead && player3.joined) &&
        (player3.y > p1->y) &&
        rectanglesOverlap(p1->x, p1->y, 7000, 9000, player3.x, player3.y, 7000,
                          9000)) {
        player3.dead = true;
        player3.death_time = get_unpaused_ms();
        p1->vely = -JUMP_HEIGHT;
        p1->in_air = true;
        p1->stomping = false;
    }
}

void flip_blocks(struct player *p1) {
    if (p1->velx == 0) {
        return;
    }
    for (int32_t i = 0; i < ROWS; ++i) {
        for (int32_t j = 0; j < COLUMNS; ++j) {
            if (bit_blocks[i][j] != UNFLIPPED) {
                continue;
            }

            int32_t x = 42000 + (j * 13000);
            int32_t y = 35000 + (i * 12000);
            int32_t w = 12000;
            int32_t h = 12000;

            struct point p;
            p.y = p1->y + 4500;
            p.x = p1->x + p1->velx;
            if (p1->velx > 0)
                p.x += 7000;
            if (is_point_inside_rectangle(p.x, p.y, x, y, h, w)) {
                bit_blocks[i][j] = FLIPPED;
                add_score(10);
            }
        }
    }
}

int32_t column_height(int32_t column) {
    int32_t counter = 0;
    for (int32_t i = ROWS - 1; i >= 0; --i) {
        if (bit_blocks[i][column] == AIR) {
            return counter;
        }
        counter++;
    }
    return ROWS;
}

void generate_mesh() {

    //    struct line vertical_mesh[6];
    vertical_mesh_count = 0;

    //    struct line horizontal_mesh[5];
    horizontal_mesh_count = 0;

    for (int32_t i = 0; i < COLUMNS - 1; ++i) {
        int32_t left_height = column_height(i);
        int32_t right_height = column_height(i + 1);

        if (left_height < right_height) {
            struct line new_line;
            new_line.x = (42 + ((i + 1) * 13) + 1) * 1000 - 3500;
            new_line.a = new_line.x;

            new_line.y = 120000;
            new_line.b = ((119 - (right_height * 12)) * 1000) - 4500 + 1;
            vertical_mesh[vertical_mesh_count] = new_line;
            vertical_mesh_count++;
        } else if (left_height > right_height) {
            struct line new_line;
            new_line.x = (41 + (i + 1) * 13) * 1000 + 3500;
            new_line.a = new_line.x;

            new_line.y = ((119 - (left_height * 12)) * 1000) - 4500 + 1;
            new_line.b = 120000;
            vertical_mesh[vertical_mesh_count] = new_line;
            vertical_mesh_count++;
        }
    }

    for (int32_t i = 0; i < COLUMNS; ++i) {
        int32_t left_height = column_height(i - 1);
        int32_t current_height = column_height(i);
        int32_t right_height = column_height(i + 1);
        if (current_height == 0) {
            continue;
        }
        if (i != 0 && horizontal_mesh_count != 0 &&
            current_height == left_height) {
            horizontal_mesh[horizontal_mesh_count - 1].a += 13000;
        } else {
            struct line new_line;
            new_line.x = (42 + (i * 13)) * 1000 - 3500;
            new_line.a = new_line.x + 12000 + 7000;

            new_line.y = (119 - (current_height * 12)) * 1000 - 4500;
            new_line.b = new_line.y;
            horizontal_mesh[horizontal_mesh_count] = new_line;
            horizontal_mesh_count++;
        }
        //        if ((i != COLUMNS - 1 && right_height > current_height)) {
        //            horizontal_mesh[horizontal_mesh_count].a += 3000;
        //        }
        //        if (i == COLUMNS - 1 && horizontal_mesh_count != 0) {
        //            horizontal_mesh[horizontal_mesh_count].a += 2000;
        //        }
        //
        //        if ((i != 0 && left_height > current_height)) {
        //            horizontal_mesh[horizontal_mesh_count].x -= 3000;
        //        }
        //        if (i == 0 && horizontal_mesh_count != 0) {
        //            horizontal_mesh[horizontal_mesh_count].x -= 2000;
        //        }
    }
}

int32_t get_new_bit_delay() {
    return MAX(MAX_NEW_BIT_DELAY - (cycles * 75 - cycles), MIN_NEW_BIT_DELAY);
}

void handle_block_collision(struct player *p1) {
    struct point center;
    center.x = p1->x + 3500;
    center.y = p1->y + 4500;

    struct point center_vel;
    center_vel.x = center.x + p1->velx;
    center_vel.y = center.y + p1->vely;

    struct line center_line;
    center_line.x = center.x;
    center_line.y = center.y;
    center_line.a = center_vel.x;
    center_line.b = center_vel.y;
    // redner mesh

    if (p1->vely >= -40) {
        for (int32_t i = 0; i < horizontal_mesh_count; i++) {
            //            printf("1, %d\n", i);
            bool g = linesIntersect(horizontal_mesh[i], center_line);
            //            printf("2, %d\n", i);
            if (g) {
                p1->vely = 0;
                p1->y = horizontal_mesh[i].b - 4500;
                p1->in_air = false;
            }
        }
    }
    if (p1->vely == 0) {
        struct point gravity_center_vel;
        gravity_center_vel.x = center.x + p1->velx;
        gravity_center_vel.y =
            center.y + (int32_t)(GRAVITY * MAX(FRAMETIME, delta));
        struct line gravity_center_line;
        gravity_center_line.x = center.x;
        gravity_center_line.y = center.y;
        gravity_center_line.a = gravity_center_vel.x;
        gravity_center_line.b = gravity_center_vel.y;
        for (int32_t i = 0; i < horizontal_mesh_count; i++) {
            if (linesIntersect(horizontal_mesh[i], gravity_center_line)) {
                p1->in_air = false;
            }
        }
    }

    for (int32_t i = 0; i < vertical_mesh_count; i++) {
        if (linesIntersect(vertical_mesh[i], center_line)) {
            p1->gliding = true;
            if (p1->x > 0) {
                // p1->x = vertical_mesh[i].x - 3400;
            } else if (p1->x < 0) {
                // p1->x = vertical_mesh[i].x + 3600;
            }
            // if (p1->x - vertical_mesh[i].x > )
            p1->velx = 0;
        }
    }
}

bool player_died(struct player *p1) {

    if (flash_spawned) {
        if (rectanglesOverlap((p1->x + 1) / 1000 - 41, (p1->y + 1) / 1000 - 34,
                              5, 7, (interest_column)*13, 0, 12, 120)) {
            return true;
        }
    }
    for (int i = 0; i < ROWS * COLUMNS * 8; i++) {
        if (laser_shots[i].active && !laser_shots[i].destroyed) {
            int32_t x =
                ((laser_shots[i].column - laser_shots[i].left_expansion) * 13) -
                1;
            int32_t y = (laser_shots[i].row * 12) + 5;
            int32_t w = ((laser_shots[i].left_expansion +
                          laser_shots[i].right_expansion + 1) *
                         13) +
                        1;
            int32_t h = 4;
            if (rectanglesOverlap((p1->x + 1) / 1000 - 41,
                                  (p1->y + 1) / 1000 - 34, 7, 9, x, y, w, h)) {
                return true;
            }
        }
    }
    return false;
}

void draw_player(struct player p1, int32_t pindex) {
    sx = 2 * 7;
    if (!p1.dead) {
        if (p1.stomping) {
            sx = (3 + ((get_unpaused_ms() % 1200) / 300)) *
                 7; // index 4 to 7 are for he rotating motion when stomping
            // printf("%d\n", 4+((get_unpaused_ms()%400)/100));
        } else if (input_get_button(pindex, BUTTON_RIGHT)) {
            if (p1.gliding)
                sx = 7 * 7; // index 7 is for gliding on a wall on the right
            else
                sx = 0 * 7; // index 0 is for going right
        } else if (input_get_button(pindex, BUTTON_LEFT)) {
            if (p1.gliding)
                sx = 8 * 7; // index 8 is for gliding on a wall on the left
            else
                sx = 1 * 7; // index 1 is for going left
        } else {
            sx = 2 * 7; // index 2 is for looking at straight ahead
        }
    }
    surf_draw_masked_subsurf(box, &player_texture, (p1.x) / 1000 - 41,
                             (p1.y) / 1000 - 34, sx, pindex * 9, 7, 9, BLACK);
}

void handle_player(struct player *p1, int32_t player_id) {
    update_player_status(p1, player_id);
    if (p1->respawning && p1->dead &&
        get_unpaused_ms() - p1->death_time > RESPAWN_INTERVAL) {
        p1->dead = false;
    }
    if (!p1->joined || p1->dead) {
        if (p1->death_time + DEATH_BLINK_SPEED * 4 < get_unpaused_ms()) {
            p1->y = 0;
        }
        return;
    }

    p1->velx = 0;
    if (p1->in_air) {
        p1->vely += (int32_t)(GRAVITY * MAX(FRAMETIME, delta));
        ;
    } else {
        p1->vely = 0;
    }
    handle_input(p1, player_id);
    if (p1->stomping) {
        p1->vely = (int32_t)((STOMP_SPEED - (p1->stomped_obstacles + 3) *
                                                (STOMP_SPEED / (5 + 3.8))) *
                             MAX(FRAMETIME, delta));
        handle_box_collision(p1);
        if (p1->vely == 0) {

            p1->vely = -JUMP_HEIGHT;
            p1->in_air = true;
            p1->stomping = false;
            return;
        }
        correct_player_stomp_trajectory(p1);
        handle_stomp_obstacle(p1);
        handle_player_collision(p1);

    } else {
        p1->stomped_obstacles = 0;
        flip_blocks(p1);

        handle_box_collision(p1);
        handle_block_collision(p1);
        if (p1->gliding &&
            p1->vely > (int32_t)(GLIDE_SPEED * MAX(FRAMETIME, delta))) {
            p1->vely = (int32_t)(GLIDE_SPEED * MAX(FRAMETIME, delta));
        }
    }
    p1->dead = player_died(p1);
    if (p1->dead) {
        p1->death_time = get_unpaused_ms();
    }
    p1->x += p1->velx;
    p1->y += p1->vely;
}

bool has_lost() {
    if (lost) {
        return true;
    }

    if (player0.dead || player1.dead || player2.dead || player3.dead) {
        if ((!player0.joined || player0.dead) &&
            (!player1.joined || player1.dead) &&
            (!player2.joined || player2.dead) &&
            (!player3.joined || player3.dead)) {
            return true;
        }
    }

    if (!player0.joined && !player1.joined && !player2.joined &&
        !player3.joined) {
        pause = true;
    }

    for (int32_t i = 0; i < COLUMNS; i++) {
        if (bit_blocks[1][i] == ERR_BLOCK) {
            return true;
        }
    }
    return false;
}

// void intPadder(int32_t number, int32_t padding, char *result) {
////    sprintf(result, "%d", number);
//
//    // Calculate the length of the resulting string
//    size_t length = strlen(result);
//
//    // Check if padding is necessary
//    if (length < padding) {
//        // Create a temporary string with the padded zeros
//        char temp[padding + 1];
//        memset(temp, '0', padding - length);
//        temp[padding - length] = '\0';
//
//        // Concatenate the padded zeros with the original string
//        strcat(temp, result);
//        strcpy(result, temp);
//    }
//}

void draw_player_respawn_progress(struct player p1, int32_t player_id) {
    struct point positions[24] = {
        {0, 5}, {1, 5}, {2, 5}, {3, 5}, {3, 4}, {2, 4}, {1, 4}, {0, 4},
        {0, 3}, {1, 3}, {2, 3}, {3, 3}, {3, 2}, {2, 2}, {1, 2}, {0, 2},
        {0, 1}, {1, 1}, {2, 1}, {3, 1}, {3, 0}, {2, 0}, {1, 0}, {0, 0},
    };
    switch (player_id) {
    case 0:
        surf_fill(prog_indic_0, DARK_RED);
        for (int32_t i = 0; i < ((get_unpaused_ms() - p1.death_time) /
                                 (RESPAWN_INTERVAL / 24));
             i++) {
            if (i < 0 || i > 25) {
                continue;
            }
            surf_set_pixel(prog_indic_0, positions[i].x, positions[i].y, GRAY);
        }
    case 1:
        surf_fill(prog_indic_1, DARK_RED);
        for (int32_t i = 0; i < ((get_unpaused_ms() - p1.death_time) /
                                 (RESPAWN_INTERVAL / 24));
             i++) {
            if (i < 0 || i > 24) {
                continue;
            }
            surf_set_pixel(prog_indic_1, positions[i].x, positions[i].y, GRAY);
        }
    case 2:
        surf_fill(prog_indic_2, DARK_RED);
        for (int32_t i = 0; i < ((get_unpaused_ms() - p1.death_time) /
                                 (RESPAWN_INTERVAL / 24));
             i++) {
            if (i < 0 || i > 24) {
                continue;
            }
            surf_set_pixel(prog_indic_2, positions[i].x, positions[i].y, GRAY);
        }
    case 3:
        surf_fill(prog_indic_3, DARK_RED);
        for (int32_t i = 0; i < ((get_unpaused_ms() - p1.death_time) /
                                 (RESPAWN_INTERVAL / 24));
             i++) {
            if (i < 0 || i > 24) {
                continue;
            }
            surf_set_pixel(prog_indic_3, positions[i].x, positions[i].y, GRAY);
        }
    }
    // break;
    // case 1:
    //     surf_fill(prog_indic_1, DARK_RED);
    //     printf("m%d\n",
    //            ((get_unpaused_ms() - p1.death_time) % RESPAWN_INTERVAL) /
    //            24);
    //     for (int32_t i = -1;
    //          i < ((get_unpaused_ms() - p1.death_time) % RESPAWN_INTERVAL)
    //          / 24; i++) {
    //         printf("i%d\n", i);
    //         if (i == -1) {
    //             continue;
    //         }
    //         surf_set_pixel(prog_indic_1, positions[i].x, positions[i].y,
    //         GRAY);
    //     }
    //     break;
    // case 2:
    //     surf_fill(prog_indic_2, DARK_RED);
    //     printf("m%d\n",
    //            ((get_unpaused_ms() - p1.death_time) % RESPAWN_INTERVAL) /
    //            24);
    //     for (int32_t i = -1;
    //          i < ((get_unpaused_ms() - p1.death_time) % RESPAWN_INTERVAL)
    //          / 24; i++) {
    //         printf("i%d\n", i);
    //         if (i == -1) {
    //             continue;
    //         }
    //         surf_set_pixel(prog_indic_2, positions[i].x, positions[i].y,
    //         GRAY);
    //     }
    //     break;
    // case 3:
    //     surf_fill(prog_indic_3, DARK_RED);
    //     printf("m%d\n",
    //            ((get_unpaused_ms() - p1.death_time) % RESPAWN_INTERVAL) /
    //            24);
    //     for (int32_t i = -1;
    //          i < ((get_unpaused_ms() - p1.death_time) % RESPAWN_INTERVAL)
    //          / 24; i++) {
    //         printf("i%d\n", i);
    //         if (i == -1) {
    //             continue;
    //         }
    //         surf_set_pixel(prog_indic_3, positions[i].x, positions[i].y,
    //         GRAY);
    //     }
    //     break;
}

void render() {
    surf_fill(box, BLACK);
    // render Background
    char paddedScore[14];
    char paddedHighScore[14];
    sprintf(paddedScore, "%07i", visualized_score);
    sprintf(paddedHighScore, "%07i", highscore);

    gpu_print_text(FRONT_BUFFER, 38 - 3, 3, WHITE, BLACK, paddedScore);
    gpu_print_text(FRONT_BUFFER, (160 / 2) - 2, 3, WHITE, BLACK, "/");
    gpu_print_text(FRONT_BUFFER, (160 / 2) + 4, 3, WHITE, BLACK,
                   paddedHighScore);

    surf_draw_masked_surf(box, &background_texture, 0, 0, BLACK);

    for (int32_t bit = 0; bit < ROWS * COLUMNS; bit++) {
        if (get_unpaused_ms() -
                    bit_descturction_animations[bit].animation_start <
                DESTROIED_BIT_ANIMATION_DURATION &&
            get_unpaused_ms() > DESTROIED_BIT_ANIMATION_DURATION) {
            surf_set_pixel(
                box, 1 + (bit_descturction_animations[bit].column * 13) + 5,
                1 + (bit_descturction_animations[bit].row * 12) + 5, WHITE);
            surf_set_pixel(
                box, 1 + (bit_descturction_animations[bit].column * 13) + 5,
                1 + (bit_descturction_animations[bit].row * 12) + 6, WHITE);
            surf_set_pixel(
                box, 1 + (bit_descturction_animations[bit].column * 13) + 6,
                1 + (bit_descturction_animations[bit].row * 12) + 5, WHITE);
            surf_set_pixel(
                box, 1 + (bit_descturction_animations[bit].column * 13) + 6,
                1 + (bit_descturction_animations[bit].row * 12) + 6, WHITE);
        }
    }

    if (warning) {
        //        SDL_Rect destination_rect;
        int32_t x = 1 + (interest_column * 13);
        int32_t y = 1;
        //        int32_t w = 12;
        //        int32_t h = 12 * (column_air_amount(interest_column));
        surf_draw_masked_surf(box, &warning_texture, x, y, BLACK);
        switch (next_block) {
        case UNFLIPPED:
            surf_draw_surf(box, &unflipped_texture, x, y);
            break;
        case BLASTER:
            surf_draw_surf(box, &blaster0_texture, interest_column * 13 + 1, 1);
            break;
        }
    } else if (flash) {
        //        SDL_Rect destination_rect;
        int32_t x = 1 + (interest_column * 13);
        int32_t y = 1 - (12 * (COLUMNS - column_air_amount(interest_column)));
        //        int32_t w = 12;
        //        int32_t h = 12 * (COLUMNS);

        //        SDL_Rect source_rect;
        //        source_rect.x = 0;
        //        source_rect.y = 84 - (12 *
        //        (column_air_amount(interest_column))); source_rect.w = 12;
        //        source_rect.h = 12 * (column_air_amount(interest_column));
        surf_draw_masked_surf(box, &flash_texture, x, y, BLACK);
    }

    for (int i = 0; i < ROWS * COLUMNS * 8; i++) {
        if (laser_shots[i].active && !laser_shots[i].destroyed) {
            int32_t x =
                ((laser_shots[i].column - laser_shots[i].left_expansion) * 13) -
                1;
            int32_t y = (laser_shots[i].row * 12) + 5;
            int32_t w = ((laser_shots[i].left_expansion +
                          laser_shots[i].right_expansion + 1) *
                         13) +
                        1;
            for (int j = x; j < x + w; j++) {
                surf_set_pixel(box, j, y, DARK_RED);
                surf_set_pixel(box, j, y + 1, RED);
                surf_set_pixel(box, j, y + 2, RED);
                surf_set_pixel(box, j, y + 3, DARK_RED);
            }
        }
        if (!laser_shots[i].destroyed) {
            int32_t x = 1 + (laser_shots[i].column * 13);
            int32_t y = 1 + (laser_shots[i].row * 12);
            //            printf("%d\n", laser_shots[i].texture);
            if (laser_shots[i].texture == 0) {
                surf_draw_surf(box, &blaster0_texture, x, y);
            } else if (laser_shots[i].texture == 1) {
                surf_draw_surf(box, &blaster1_texture, x, y);
            } else if (laser_shots[i].texture == 2) {
                surf_draw_surf(box, &blaster2_texture, x, y);
            } else if (laser_shots[i].texture == 3) {
                surf_draw_surf(box, &blaster3_texture, x, y);
            } else if (laser_shots[i].texture == 4) {
                surf_draw_surf(box, &blaster4_texture, x, y);
            }
        }
    }

    // render blocks
    for (int32_t i = 0; i < ROWS; ++i) {
        for (int32_t j = 0; j < COLUMNS; ++j) {
            int32_t x = 1 + (j * 13);
            int32_t y = 1 + (i * 12);
            int32_t w = 12;
            int32_t h = 12;

            //            std::cout << zero_texture.get_surface_rect(count,
            //            0)->x << ", " << zero_texture.get_surface_rect(count,
            //            0)->y << "\n";
            switch (bit_blocks[i][j]) {
            case UNFLIPPED:
                surf_draw_surf(box, &unflipped_texture, x, y);
                break;
            case FLIPPED:
                surf_draw_surf(box, &flipped_texture, x, y);
                break;
            case ERR_BLOCK:
                surf_draw_surf(box, &error_texture, x, y);
                break;
            }
        }
    }
    char box_char[2] = {'!' - 64, 0};
    if (player0.dead) {
        gpu_print_transparent_text(FRONT_BUFFER, 58, 13, RED, box_char);
        if (player0.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
            (player0.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
                DEATH_BLINK_SPEED) {
            draw_player(player0, 0);
        }
        if (player0.respawning) {
            draw_player_respawn_progress(player0, 0);
        }
    } else if (player0.joined) {
        gpu_print_text(FRONT_BUFFER, 58, 13, RED, WHITE, box_char);
        draw_player(player0, 0);
    } else {
        gpu_print_text(FRONT_BUFFER, 58, 13, RED, GRAY, box_char);
    }

    if (player1.dead) {
        gpu_print_transparent_text(FRONT_BUFFER, 71, 13, BLUE, box_char);
        if (player1.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
            (player1.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
                DEATH_BLINK_SPEED) {
            draw_player(player1, 1);
        }
        if (player1.respawning) {
            draw_player_respawn_progress(player1, 1);
        }
    } else if (player1.joined) {
        gpu_print_text(FRONT_BUFFER, 71, 13, BLUE, WHITE, box_char);
        draw_player(player1, 1);
    } else {
        gpu_print_text(FRONT_BUFFER, 71, 13, BLUE, GRAY, box_char);
    }

    if (player2.dead) {
        gpu_print_transparent_text(FRONT_BUFFER, 84, 13, GREEN, box_char);
        if (player2.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
            (player2.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
                DEATH_BLINK_SPEED) {
            draw_player(player2, 2);
        }
        if (player2.respawning) {
            draw_player_respawn_progress(player2, 2);
        }
    } else if (player2.joined) {
        gpu_print_text(FRONT_BUFFER, 84, 13, GREEN, WHITE, box_char);
        draw_player(player2, 2);
    } else {
        gpu_print_text(FRONT_BUFFER, 84, 13, GREEN, GRAY, box_char);
    }

    if (player3.dead) {
        gpu_print_transparent_text(FRONT_BUFFER, 97, 13, YELLOW, box_char);
        if (player3.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
            (player3.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
                DEATH_BLINK_SPEED) {
            draw_player(player3, 3);
        }
        if (player3.respawning) {
            draw_player_respawn_progress(player3, 3);
        }
    } else if (player3.joined) {
        gpu_print_text(FRONT_BUFFER, 97, 13, YELLOW, WHITE, box_char);
        draw_player(player3, 3);
    } else {
        gpu_print_text(FRONT_BUFFER, 97, 13, YELLOW, GRAY, box_char);
    }

    // //debug player contour lines
    // if (player0.joined && !player0.dead)
    //     surf_draw_rectangle(box, (player0.x - 1) / 1000 - 41,
    //                         (player0.y - 1) / 1000 - 34, 7, 9, WHITE);
    // if (player1.joined && !player1.dead)
    //     surf_draw_rectangle(box, (player1.x - 1) / 1000 - 41,
    //                         (player1.y - 1) / 1000 - 34, 7, 9, WHITE);
    // if (player2.joined && !player2.dead)
    //     surf_draw_rectangle(box, (player2.x - 1) / 1000 - 41,
    //                         (player2.y - 1) / 1000 - 34, 7, 9, WHITE);
    // if (player3.joined && !player3.dead)
    //     surf_draw_rectangle(box, (player3.x - 1) / 1000 - 41,
    //                         (player3.y - 1) / 1000 - 34, 7, 9, WHITE);

    // if (player0.joined && player0.dead &&
    //     player0.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
    //     (player0.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
    //         DEATH_BLINK_SPEED)
    //     surf_draw_rectangle(box, (player0.x - 1) / 1000 - 41,
    //                         (player0.y - 1) / 1000 - 34, 7, 9, WHITE);

    // if (player1.joined && player1.dead &&
    //     player1.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
    //     (player1.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
    //         DEATH_BLINK_SPEED)
    //     surf_draw_rectangle(box, (player1.x - 1) / 1000 - 41,
    //                         (player1.y - 1) / 1000 - 34, 7, 9, WHITE);

    // if (player2.joined && player2.dead &&
    //     player2.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
    //     (player2.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
    //         DEATH_BLINK_SPEED)
    //     surf_draw_rectangle(box, (player2.x - 1) / 1000 - 41,
    //                         (player2.y - 1) / 1000 - 34, 7, 9, WHITE);

    // if (player3.joined && player3.dead &&
    //     player3.death_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms() &&
    //     (player3.death_time - get_unpaused_ms()) % DEATH_BLINK_SPEED * 2 <
    //         DEATH_BLINK_SPEED)
    //     surf_draw_rectangle(box, (player3.x - 1) / 1000 - 41,
    //                         (player3.y - 1) / 1000 - 34, 7, 9, WHITE);

    // // debug collision contour lines
    //    for (int32_t i = 0; i < horizontal_mesh_count; i++) {
    //        surf_draw_line(box, horizontal_mesh[i].x / 1000 - 41,
    //        horizontal_mesh[i].y / 1000 - 34,
    //                       horizontal_mesh[i].a / 1000 - 41,
    //                       horizontal_mesh[i].b / 1000 - 34, BLUE);
    //    }
    // for (int32_t i = 0; i < vertical_mesh_count; i++) {
    //     surf_draw_line(box, vertical_mesh[i].x / 1000 - 41,
    //                    vertical_mesh[i].y / 1000 - 34,
    //                    vertical_mesh[i].a / 1000 - 41,
    //                    vertical_mesh[i].b / 1000 - 34, BLUE);
    // }

    gpu_block_frame();
    gpu_send_buf(BACK_BUFFER, box->w, box->h, 41, 34, box->d);
    if (player0.respawning) {
        gpu_send_buf(BACK_BUFFER, prog_indic_0->w, prog_indic_0->h, 59, 14,
                     prog_indic_0->d);
    }
    if (player1.respawning) {
        gpu_send_buf(BACK_BUFFER, prog_indic_1->w, prog_indic_1->h, 72, 14,
                     prog_indic_1->d);
    }
    if (player2.respawning) {
        gpu_send_buf(BACK_BUFFER, prog_indic_2->w, prog_indic_2->h, 85, 14,
                     prog_indic_2->d);
    }
    if (player3.respawning) {
        gpu_send_buf(BACK_BUFFER, prog_indic_3->w, prog_indic_3->h, 98, 14,
                     prog_indic_3->d);
    }
    // gpu_send_buf(BACK_BUFFER, prog_indic_1->w, prog_indic_1->h, 41, 34,
    //              prog_indic_1->d);
    // gpu_send_buf(BACK_BUFFER, prog_indic_2->w, prog_indic_2->h, 41, 34,
    //              prog_indic_2->d);
    // gpu_send_buf(BACK_BUFFER, prog_indic_3->w, prog_indic_3->h, 41, 34,
    //              prog_indic_3->d);
    gpu_swap_buf();

    //    SDL_SetRenderDrawColor(*renderer_pointer, 0, 255, 0, 255);
    //    for (int32_t i = 0; i < vertical_mesh_count; i++) {
    //        SDL_RenderDrawLine(*renderer_pointer, vertical_mesh[i].x * 9,
    //        vertical_mesh[i].y * 9, vertical_mesh[i].a * 9, vertical_mesh[i].b
    //        * 9);
    //    }
    //    SDL_SetRenderDrawColor(*renderer_pointer, 0, 255, 0, 255);
    //    for (int32_t i = 0; i < horizontal_mesh_count; i++) {
    //        SDL_RenderDrawLine(*renderer_pointer, horizontal_mesh[i].x * 9,
    //        horizontal_mesh[i].y * 9, horizontal_mesh[i].a * 9,
    //        horizontal_mesh[i].b * 9);
    //    }
}

uint8_t start(void) {
    Surface temp0 = surf_create(4, 6);
    Surface temp1 = surf_create(4, 6);
    Surface temp2 = surf_create(4, 6);
    Surface temp3 = surf_create(4, 6);
    Surface temp = surf_create(1 + (COLUMNS * 13), 2 + (ROWS * 12));
    prog_indic_0 = &temp0;
    prog_indic_1 = &temp1;
    prog_indic_2 = &temp2;
    prog_indic_3 = &temp3;
    box = &temp;
    warning_texture = surf_create_from_memory(12, 84, ASSET_WARNING_M3IF);
    flash_texture = surf_create_from_memory(12, 84, ASSET_FLASH_M3IF);
    background_texture = surf_create_from_memory(
        1 + (COLUMNS * 13), 2 + (ROWS * 12), ASSET_BACKGROUND_M3IF);
    blaster0_texture = surf_create_from_memory(12, 12, ASSET_BLASTER0_M3IF);
    blaster1_texture = surf_create_from_memory(12, 12, ASSET_BLASTER1_M3IF);
    blaster2_texture = surf_create_from_memory(12, 12, ASSET_BLASTER2_M3IF);
    blaster3_texture = surf_create_from_memory(12, 12, ASSET_BLASTER3_M3IF);
    blaster4_texture = surf_create_from_memory(12, 12, ASSET_BLASTER4_M3IF);
    unflipped_texture = surf_create_from_memory(12, 12, ASSET_UNFLIPPED_M3IF);
    flipped_texture = surf_create_from_memory(12, 12, ASSET_FLIPPED_M3IF);
    error_texture = surf_create_from_memory(12, 12, ASSET_ERROR_M3IF);
    player_texture = surf_create_from_memory(70, 36, ASSET_PLAYER_M3IF);
    rng_init();
    gpu_update_palette(palette);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            bit_blocks[i][j] = 0;
        }
    }
    // bit_blocks[ROWS][COLUMNS] = {
    //     {AIR, AIR, AIR, AIR, AIR, AIR}, {AIR, AIR, AIR, AIR, AIR, AIR},
    //     {AIR, AIR, AIR, AIR, AIR, AIR}, {AIR, AIR, AIR, AIR, AIR, AIR},
    //     {AIR, AIR, AIR, AIR, AIR, AIR}, {AIR, AIR, AIR, AIR, AIR, AIR},
    //     {AIR, AIR, AIR, AIR, AIR, AIR},
    // };
    player0.x = 96000;
    player0.y = 0;
    player0.velx = 0;
    player0.vely = -100;
    player0.dead = false;
    player0.joined = false;
    player0.respawning = true;
    player1.x = 96000;
    player1.y = 0;
    player1.velx = 0;
    player1.vely = -100;
    player1.dead = false;
    player1.joined = false;
    player1.respawning = true;
    player2.x = 96000;
    player2.y = 0;
    player2.velx = 0;
    player2.vely = -100;
    player2.dead = false;
    player2.joined = false;
    player2.respawning = true;
    player3.x = 96000;
    player3.y = 0;
    player3.velx = 0;
    player3.vely = -100;
    player3.dead = false;
    player3.joined = false;
    player3.respawning = true;

    for (int i = 0; i < ROWS * COLUMNS * 8; i++) {
        struct laser_shot shot;
        shot.row = 0;
        shot.column = 0;
        shot.last_shot = LASER_SHOT_DELAY + LASER_SHOT_DURATION;
        shot.destroyed = true;
        laser_shots[i] = shot;
    }

    gpu_blank(FRONT_BUFFER, 0);
    gpu_blank(BACK_BUFFER, 0);

    while (!lost) {
        if (pause) {
            // printf("%i\n", get_unpaused_ms());
            render_pause_menu();
            pause = false;
            // printf("%i\n", get_unpaused_ms());
            continue;
        }
        //        printf("%d\n", falling_blocks);
        if (falling_blocks > 1) {
            if (!warning && !flash) {
                last_warning_start = get_unpaused_ms();
                warning = true;
                flash = false;
            } else if (warning && get_unpaused_ms() - last_warning_start >
                                      WARNING_DURATION) {
                spawn_flash();
                spawn_block();
            } else if (flash &&
                       get_unpaused_ms() - last_flash_start > FLASH_DURATION) {
                end_flash();
            }
        } else if (falling_blocks >= 0) {
            if ((get_unpaused_ms() > get_new_bit_delay() * cycles) &&
                !warning && !flash) {
                spawn_warning();
            } else if (warning && get_unpaused_ms() - last_warning_start >
                                      WARNING_DURATION) {
                spawn_flash();
                spawn_block();
            } else if (flash &&
                       get_unpaused_ms() - last_flash_start > FLASH_DURATION) {
                end_flash();
            }
        } else {
            printf("Error\n");
        }
        update_laser_shots();
        for (int32_t i = 0; i < COLUMNS; ++i) {
            if (check_for_limitreach(i)) {
                send_error_block(i);
            }
        }
        generate_mesh();
        handle_player(&player0, 0);
        handle_player(&player1, 1);
        handle_player(&player2, 2);
        handle_player(&player3, 3);
        flash_spawned = false;
        update_score();
        lost = has_lost();
        render();

        timing();
    }
    player0.respawning = false;
    player1.respawning = false;
    player2.respawning = false;
    player3.respawning = false;
    int32_t lost_time = get_unpaused_ms();
    player0.dead = true;
    player0.death_time = get_unpaused_ms();
    player1.dead = true;
    player1.death_time = get_unpaused_ms();
    player2.dead = true;
    player2.death_time = get_unpaused_ms();
    player3.dead = true;
    player3.death_time = get_unpaused_ms();
    while (lost_time + DEATH_BLINK_SPEED * 4 > get_unpaused_ms()) {
        render();
        timing();
    }
    return CODE_EXIT;
}
