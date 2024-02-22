#ifndef UTILS_H
#define UTILS_H

#include "main.h"
#include "timer.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

bool is_point_inside_rectangle(float px, float py, float x, float y, float h,
                               float w) {
    if (px >= x && px <= x + w && py >= y && py <= y + h) {
        return true;
    }
    return false;
}

bool linesIntersect(struct line a, struct line b) {
    float x1 = a.x;
    float y1 = a.y;
    float x2 = a.a;
    float y2 = a.b;
    float x3 = b.x;
    float y3 = b.y;
    float x4 = b.a;
    float y4 = b.b;

    float den = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1);
    float ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / den;
    float ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / den;

    if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1) {
        return true;
    }
    return false;
}

bool rectanglesOverlap(float x1, float y1, float w1, float h1, float x2,
                       float y2, float w2, float h2) {
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
    printf("%d\n", delta);
}

#endif