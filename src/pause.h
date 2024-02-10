#ifndef PAUSE_H
#define PAUSE_H

#include "main.h"

#include <gpu.h>
#include <input.h>

#include "graphics.h"
#include "timer.h"
#include "utils.h"
#include <gpu.h>
#include <mes.h>
#include <rng.h>
#include <stdbool.h>
#include <stdint.h>

void handle_pause_menu_input(struct player *p1, int32_t player_id) {
    if (input_get_button(player_id, BUTTON_DOWN) && !p1->last_down_keystate) {
        pause_menu_selected++;
        if (pause_menu_selected > 1) {
            pause_menu_selected = 0;
        }
    }
    if (input_get_button(player_id, BUTTON_UP) && !p1->last_up_keystate) {
        pause_menu_selected--;
        if (pause_menu_selected < 0) {
            pause_menu_selected = 1;
        }
    }
    if ((input_get_button(player_id, BUTTON_A) && !p1->last_a_keystate) ||
        (input_get_button(player_id, BUTTON_B) && !p1->last_b_keystate)) {
        switch (pause_menu_selected) {
        case 0:
            pause = false;
            break;
        case 1:
            pause = false;
            lost = true;
            break;
        }
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

void render_pause_menu() {
    int32_t pause_start = timer_get_ms();
    render();
    while (pause) {
        handle_pause_menu_input(&player0, 0);
        handle_pause_menu_input(&player1, 1);
        handle_pause_menu_input(&player2, 2);
        handle_pause_menu_input(&player3, 3);

        gpu_print_text(FRONT_BUFFER, 63, 60, GRAY, BLACK, "RESUME");
        gpu_print_text(FRONT_BUFFER, 69, 68, GRAY, BLACK, "QUIT");
        switch (pause_menu_selected) {
        case 0:
            gpu_print_text(FRONT_BUFFER, 63, 60, WHITE, GRAY, "RESUME");
            break;
        case 1:
            gpu_print_text(FRONT_BUFFER, 69, 68, WHITE, GRAY, "QUIT");
            break;
        }
        timing();
    }
    int32_t pause_end = timer_get_ms();
    int32_t delta = pause_end - pause_start;
    paused_time = paused_time + delta;
}

#endif