#ifndef UTILS_H
#define UTILS_H

#include "main.h"
#include "timer.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

bool is_point_inside_rectangle(int32_t px, int32_t py, int32_t x, int32_t y,
                               int32_t h, int32_t w) {
    if (px >= x && px <= x + w && py >= y && py <= y + h) {
        return true;
    }
    return false;
}

bool linesIntersect(struct line a, struct line b) {
    int32_t x1 = a.x;
    int32_t y1 = a.y;
    int32_t x2 = a.a;
    int32_t y2 = a.b;
    int32_t x3 = b.x;
    int32_t y3 = b.y;
    int32_t x4 = b.a;
    int32_t y4 = b.b;

    float den = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    float ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / den;
    float ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / den;

    if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1) {
        return true;
    }
    return false;
}

bool rectanglesOverlap(int32_t x1, int32_t y1, int32_t w1, int32_t h1,
                       int32_t x2, int32_t y2, int32_t w2, int32_t h2) {
    if (x1 > x2 + w2 || x1 + w1 < x2 || y1 > y2 + h2 || y1 + h1 < y2) {
        return false;
    }
    return true;
}

uint32_t get_unpaused_ms() { return timer_get_ms() - paused_time; }

void timing() {
    // timing
    stop = timer_get_ms();
    delta = stop - delta_start;
    //        printf("%d\n", delta);
    if (delta < FRAMETIME) {
        timer_block_ms(FRAMETIME - delta);
    }
    delta_start = timer_get_ms();
}

#endif