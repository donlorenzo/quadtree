#ifndef __UTILS_H__
#define __UTILS_H__

#define NO_COLLISION (0)
#define COLLISION    (1)

int collide_polygon_rectangle(int n, int *xs, int *ys, int rx, int ry, int w, int h);
int rectangle_inside_polygon(int rx, int ry, int w, int h, int n, int *xs, int *ys);
int point_in_polygon(int px, int py, int n, int *xs, int *ys);
unsigned long next_power_of_2(unsigned long n);

#endif /* __UTILS_H__ */
