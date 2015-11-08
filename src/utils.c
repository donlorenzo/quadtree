#include "utils.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "bool.h"

long cross_product(long x1, long y1, long x2, long y2);
bool point_in_rectangle(int px, int py, int rx, int ry, int w, int h);
bool lines_intersect(int line1[2][2], int line2[2][2]);

unsigned long next_power_of_2(unsigned long n) {
    int shifts = 0;
    if (n == 0) {
        return 1l;
    }
    --n;
    while (n) {
        n >>= 1;
        ++shifts;
    }
    return 1l << shifts;
}

int collide_polygon_rectangle(int n, int *xs, int *ys, int rx, int ry, int w, int h) {
    assert(n > 0);
    int polygon_line[2][2];
    int rectangle_lines[4][2][2] = { { {rx,   ry},   {rx+w, ry} },
                                     { {rx+w, ry},   {rx+w, ry+h} },
                                     { {rx+w, ry+h}, {rx,   ry+h} },
                                     { {rx,   ry+h}, {rx,   ry} } };
    int polygon_index, rectangle_index;
    int old_polygon_index = n - 1;
    for (polygon_index = 0; polygon_index < n; ++polygon_index) {
        polygon_line[0][0] = xs[old_polygon_index];
        polygon_line[0][1] = ys[old_polygon_index];
        polygon_line[1][0] = xs[polygon_index];
        polygon_line[1][1] = ys[polygon_index];
        for (rectangle_index = 0; rectangle_index < 4; ++rectangle_index) {
            if (lines_intersect(polygon_line, rectangle_lines[rectangle_index])) {
                return COLLISION;
            }
        }
        old_polygon_index = polygon_index;
    }
    if (point_in_rectangle(xs[0], ys[0], rx, ry, w, h)) {
        return COLLISION;
    }
    if (point_in_polygon(rx, ry, n, xs, ys)) {
        return COLLISION;
    }
    return NO_COLLISION;
}

int rectangle_inside_polygon(int rx, int ry, int w, int h, int number_of_points, int *xs, int *ys) {
    return (point_in_polygon(rx,   ry,   number_of_points, xs, ys) &&
            point_in_polygon(rx+w-1, ry,   number_of_points, xs, ys) &&
            point_in_polygon(rx,   ry+h-1, number_of_points, xs, ys) &&
            point_in_polygon(rx+w-1, ry+h-1, number_of_points, xs, ys));
}


long cross_product(long x1, long y1, long x2, long y2) {
    return ((x1 * y2) - (y1 * x2));
}

long dot_product(long x1, long y1, long x2, long y2) {
    return ((x1 * x2) + (y1 * y2));
}

bool point_in_rectangle(int px, int py, int rx, int ry, int w, int h) {
    int rectangle_xs[4] = { rx, rx+w, rx+w, rx };
    int rectangle_ys[4] = { ry, ry  , ry+h, ry+h };
    return point_in_polygon(px, py, 4, rectangle_xs, rectangle_ys);
}

/* left and bottom lines are "inside" while right and top lines are "outside" */
int point_in_polygon(int px, int py, int n, int *xs, int *ys) {
    bool inside = false;
    int i;
    int j = n - 1;
    for (i = 0; i < n; ++i) {
        if ((ys[i] > py) != (ys[j] > py) &&
            (px < (xs[j] - xs[i]) * (py - ys[i]) / (ys[j] - ys[i]) + xs[i])) {
            inside = !inside;
        }
        j = i;
    }
    return inside;
}


bool lines_intersect(int line1[2][2], int line2[2][2]) {
    int line1Vector_x = line1[1][0] - line1[0][0];
    int line1Vector_y = line1[1][1] - line1[0][1];
    int line2Vector_x = line2[1][0] - line2[0][0];
    int line2Vector_y = line2[1][1] - line2[0][1];
    int diff_x = line2[0][0] - line1[0][0];
    int diff_y = line2[0][1] - line1[0][1];
    long cross_product1 = cross_product(line1Vector_x, line1Vector_y, line2Vector_x, line2Vector_y);
    long cross_product2 = cross_product(diff_x, diff_y, line1Vector_x, line1Vector_y);
    double frac, lambda1, lambda2;
    if (cross_product1 == 0) {
        /* lines are parallel check whether they coincide */
        if (cross_product2 == 0) {
            /* parallel and co-linear */
            frac = dot_product(line1Vector_x, line1Vector_y, line1Vector_x, line1Vector_y);
            lambda1 = dot_product(diff_x, diff_y, line1Vector_x, line1Vector_y) / frac;
            lambda2 = lambda1 + dot_product(line1Vector_x, line1Vector_y, line2Vector_x, line2Vector_y) / frac;
            if ((0 <= lambda1 && lambda1 <= 1) ||
                (0 <= lambda2 && lambda2 <= 1) ||
                (lambda1 < 0 && 1 < lambda2) ||
                (lambda2 < 0 && 1 < lambda1)) {
                return true;
            }
            return false;
        } else {
            /* parallel and non intersecting */
            return false;
        }
    } else {
        frac = 1. / cross_product1;
    }
    lambda1 = cross_product(diff_x, diff_y, line2Vector_x, line2Vector_y) * frac;
    lambda2 = cross_product(diff_x, diff_y, line1Vector_x, line1Vector_y) * frac;
    return ((0 <= lambda1 && lambda1 < 1) && (0 <= lambda2 && lambda2 < 1));
}
