#include <stdio.h>
#include <stdlib.h>
#include "testutils.h"
#include "quadtree.h"
#include "utils.c"

void test_point_in_polygon() {
    int rect_AA_xs[4] = { 2, 8, 8, 2 };
    int rect_AA_ys[4] = { 2, 2, 8, 8 };
    assertFalse("point in rect", point_in_polygon(0, 0, 4, rect_AA_xs, rect_AA_ys));
    assertTrue("point not in rect", point_in_polygon(3, 3, 4, rect_AA_xs, rect_AA_ys));
    /* test border conditions */
    assertTrue("point not in rect", point_in_polygon(2, 2, 4, rect_AA_xs, rect_AA_ys));
    assertTrue("point not in rect", point_in_polygon(4, 2, 4, rect_AA_xs, rect_AA_ys));
    assertTrue("point not in rect", point_in_polygon(2, 4, 4, rect_AA_xs, rect_AA_ys));
    assertFalse("point in rect", point_in_polygon(2, 8, 4, rect_AA_xs, rect_AA_ys));
    assertFalse("point in rect", point_in_polygon(8, 2, 4, rect_AA_xs, rect_AA_ys));
    assertFalse("point in rect", point_in_polygon(4, 8, 4, rect_AA_xs, rect_AA_ys));
    assertFalse("point in rect", point_in_polygon(8, 4, 4, rect_AA_xs, rect_AA_ys));
    assertFalse("point in rect", point_in_polygon(8, 8, 4, rect_AA_xs, rect_AA_ys));

    int tri_xs[4] = { 0, 8, 0 };
    int tri_ys[4] = { 0, 0, 8 };
    assertFalse("point in tri", point_in_polygon(-1, -1, 3, tri_xs, tri_ys));
    assertTrue("point not in tri", point_in_polygon(3, 3, 3, tri_xs, tri_ys));
    /* test border conditions */
    assertTrue("point not in tri", point_in_polygon(0, 0, 3, tri_xs, tri_ys));
    assertTrue("point not in tri", point_in_polygon(0, 4, 3, tri_xs, tri_ys));
    assertTrue("point not in tri", point_in_polygon(4, 0, 3, tri_xs, tri_ys));
    assertTrue("point not in tri", point_in_polygon(3, 3, 3, tri_xs, tri_ys));
    assertTrue("point not in tri", point_in_polygon(3, 4, 3, tri_xs, tri_ys));
    assertTrue("point not in tri", point_in_polygon(4, 3, 3, tri_xs, tri_ys));
    assertFalse("point in tri", point_in_polygon(0, 8, 3, tri_xs, tri_ys));
    assertFalse("point in tri", point_in_polygon(8, 0, 3, tri_xs, tri_ys));
    assertFalse("point in tri", point_in_polygon(4, 4, 3, tri_xs, tri_ys));
}

void test_lines_intersect() {
    int line1[2][2] = { {2, 2}, {10, 2} };
    int line1b[2][2] = { {11, 2}, {2, 2} };
    int line2[2][2] = { {5, 0}, {7, 5} };
    int line1parallel[2][2] = { {2, 4}, {10, 4} };
    int line1parallel2[2][2] = { {12, 2}, {14, 2} };
    int line1parallel3[2][2] = { {14, 2}, {12, 2} };
    int line1touching[2][2] = { {10, 2}, {20, 10} };
    int line1touching_left[2][2] = { {2, 0}, {2, 4} };
    int line1touching_right[2][2] = { {10, 0}, {10, 4} };
    int foo[2][2] = { {1, 0}, {3, 0} };
    int bar[2][2] = { {0, 0}, {5, 0} };
    assertTrue("line should intersect", 
               lines_intersect(foo, bar));
    assertTrue("line should intersect", 
               lines_intersect(line1, line2));
    assertTrue("identical lines should intersect",
               lines_intersect(line1, line1));
    assertTrue("identical lines should intersect",
               lines_intersect(line1, line1b));
    assertFalse("parallel lines should not intersect",
                lines_intersect(line1, line1parallel));
    assertFalse("parallel lines should not intersect",
                lines_intersect(line1, line1parallel2));
    assertFalse("parallel lines should not intersect",
                lines_intersect(line1, line1parallel3));
    assertFalse("parallel lines should not intersect",
                lines_intersect(line1parallel2, line1));
    assertFalse("parallel lines should not intersect",
                lines_intersect(line1parallel3, line1));
    assertFalse("touching lines should not intersect",
                lines_intersect(line1, line1touching));
    assertTrue("touching lines should intersect on left side",
                lines_intersect(line1, line1touching_left));
    assertFalse("touching lines should not intersect on right side",
                lines_intersect(line1, line1touching_right));
    /* TODO: interesting case: waht should happend here? */
    /*
    int line1revert[2][2] = { {10, 2}, {2, 2} };
    assertFalse("touching lines should not(?) intersect on right side",
                lines_intersect(line1revert, line1touching_right));
    */
}


void test_collide_polygon_rectangle() {
/*
    int tri_xs[4] = { 0, 8, 0 };
    int tri_ys[4] = { 0, 0, 8 };
    assertFalse("should not collide", 
                collide_polygon_rectangle(3, tri_xs, tri_ys, 0, 8, 8, 8));
*/
}

void test_rectangle_inside_polygon() {
    int pxs[] = { 0, 8, 0 };
    int pys[] = { 0, 0, 8 };

    assertTrue("should be in", rectangle_inside_polygon(1, 1, 2, 2, 3, pxs, pys));
    assertTrue("should be in", rectangle_inside_polygon(0, 5, 2, 2, 3, pxs, pys));
}

void test_next_power_of_2() {
    assertEqualsULong("", 1l, next_power_of_2(0));
    assertEqualsULong("", 1l, next_power_of_2(1));
    assertEqualsULong("", 2l, next_power_of_2(2));
    assertEqualsULong("", 4l, next_power_of_2(3));
    assertEqualsULong("", 4l, next_power_of_2(4));
    assertEqualsULong("", 8l, next_power_of_2(5));
    assertEqualsULong("", 8l, next_power_of_2(6));
    assertEqualsULong("", 1l<<30, next_power_of_2((1l<<30)-1));
    assertEqualsULong("", 1l<<30, next_power_of_2((1l<<30)));
    assertEqualsULong("", 1l<<31, next_power_of_2((1l<<31)-1));
    assertEqualsULong("", 1l<<31, next_power_of_2((1l<<31)));
}

int main() {
    test_point_in_polygon();
    test_lines_intersect();
    test_collide_polygon_rectangle();
    test_rectangle_inside_polygon();
    test_next_power_of_2();

    int i;
    long *ids;
    int number_of_ids;
    quadtree_t qt = quadtree_create(0, 0, 80, 60);

/*
    int xs[] = { 0, 80000, 0 };
    int ys[] = { 0, 0, 80000 };
    int xs2[] = { 1, 8, 0 };
    int ys2[] = { 1, 0, 8 };
    int xs3[] = { 13, 70, 60 };
    int ys3[] = { 43, 9, 9 };
    int xs4[] = { 62, 30, 0 };
    int ys4[] = { 15, 52, 23 };
    int xs5[] = { 66, 10, 63 };
    int ys5[] = { 16, 58, 43 };
    int xs6[] = { 12, 39, 60 };
    int ys6[] = { 34, 22, 23 };
*/
    int xs7[] = { 70, 32, 10 };
    int ys7[] = { 49, 14, 34 };
    int xs8[] = { 12, 39, 60 };
    int ys8[] = { 34, 22, 23 };
    int p[] = { 39, 39 };

    printf("put : %d\n", quadtree_put(qt, 0, 3, xs7, ys7));
    printf("put : %d\n", quadtree_put(qt, 1, 3, xs8, ys8));
    printf("get : %d\n", quadtree_get(qt, p[0], p[1], &number_of_ids, &ids));
    for (i = 0; i < number_of_ids; ++i) {
        printf("id(%d): %ld\n", i, ids[i]);
    }
    free(ids);

/*
    printf("put : %d\n", quadtree_put_polygon(qt, 42, 3, xs, ys));
    printf("get : %d\n", quadtree_get(qt, 3, 3, &number_of_ids, &ids));
    for (i = 0; i < number_of_ids; ++i) {
        printf("id(%d): %ld\n", i, ids[i]);
    }
    free(ids);

    printf("A\n");

    printf("put : %d\n", quadtree_put_polygon(qt, 43, 3, xs2, ys2));
    printf("get : %d\n", quadtree_get(qt, 3, 3, &number_of_ids, &ids));
    for (i = 0; i < number_of_ids; ++i) {
        printf("id(%d): %ld\n", i, ids[i]);
    }
    free(ids);

    printf("B\n");

    printf("put : %d\n", quadtree_put_polygon(qt, 42, 3, xs, ys));
    printf("get : %d\n", quadtree_get(qt, 3, 3, &number_of_ids, &ids));
    for (i = 0; i < number_of_ids; ++i) {
        printf("id(%d): %ld\n", i, ids[i]);
    }
    free(ids);

    printf("C\n");

    printf("get : %d\n", quadtree_get(qt, 0, 0, &number_of_ids, &ids));
    for (i = 0; i < number_of_ids; ++i) {
        printf("id(%d): %ld\n", i, ids[i]);
    }
    free(ids);

    printf("D\n");

    printf("remove : %d\n", quadtree_remove(qt, 42));
    printf("get : %d\n", quadtree_get(qt, 3, 3, &number_of_ids, &ids));
    for (i = 0; i < number_of_ids; ++i) {
        printf("id(%d): %ld\n", i, ids[i]);
    }
    free(ids);
*/
    quadtree_destroy(qt);
    return 0;
}
