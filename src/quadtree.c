#include "quadtree.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "utils.h"
#include "testutils.h"

#define FIRST_QUADRANT  (0)
#define SECOND_QUADRANT (1)
#define THIRD_QUADRANT  (2)
#define FOURTH_QUADRANT (3)
#define NUMBER_OF_QUADRANTS (4)

#define MAX_DEPTH (15)
#define MIN_SIZE (4)



#ifdef DEBUG
static void LOG_DEBUG(char *format, ...) {
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    va_end (args);
}
#else
#define LOG_DEBUG(...)
#endif


/************
 * Typedefs *
 ************/

typedef enum { false, true } bool;

typedef struct {
    int bottom;
    int left;
    int height;
    int width;
} lq_rect_t;

typedef struct {
    long id;
    int number_of_points;
    int *xs;
    int *ys;
} lq_polygon_t;

typedef struct lq_polygon_node_type {
    lq_polygon_t *p;
    int *ref_count;
    struct lq_polygon_node_type *next;
} lq_polygon_node_t;

typedef struct lq_quadtree_node_type {
    struct lq_quadtree_node_type *children[4];
    int depth;
    lq_rect_t *bounding_box;
    lq_polygon_node_t *polygons;
    int number_of_polygons;
} lq_quadtree_node_t;

typedef struct {
    lq_quadtree_node_t *root;
} lq_quadtree_t;

/************************
 * Forward declarations *
 ************************/

static lq_quadtree_node_t* lq_quadtree_node_allocate();
static void lq_quadtree_node_free(lq_quadtree_node_t *node);
static void lq_quadtree_node_clear_polygons(lq_quadtree_node_t *node);
static void lq_quadtree_node_initialize(lq_quadtree_node_t *node, int left, int bottom, int width, int height, int depth);
static int lq_quadtree_node_query(lq_quadtree_node_t *node, int x, int y, quadtree_query_result_t *query_result);
static lq_quadtree_node_t* lq_quadtree_node_find_leaf(lq_quadtree_node_t *node, int x, int y);
static int lq_quadtree_node_put_polygon(lq_quadtree_node_t *node, lq_polygon_node_t *polygon);
static void lq_quadtree_node_remove(lq_quadtree_node_t *node, long id);
static int lq_quadtree_node_populate_children(lq_quadtree_node_t *node);
static void lq_quadtree_node_add_polygon(lq_quadtree_node_t *node, lq_polygon_node_t *polygon);
static int lq_quadtree_node_add_polygons(lq_quadtree_node_t *node, lq_polygon_node_t *polygons);

static lq_polygon_node_t* lq_polygon_node_allocate();
static lq_polygon_node_t* lq_polygon_node_clone(lq_polygon_node_t *polygon);
static void lq_polygon_node_free(lq_polygon_node_t *polygon);
static int lq_polygon_node_initialize(lq_polygon_node_t *polygon, long id, int number_of_points, int *xs, int *ys);

static lq_rect_t* lq_rect_allocate();
static void lq_rect_free(lq_rect_t *rect);
static void lq_rect_initialize(lq_rect_t *rect, int rx, int ry, int rw, int rh);
static int lq_rect_get_quadrant(lq_rect_t *rect, int x, int y);
static bool lq_rect_point_is_in_bounds(lq_rect_t *rect, int x, int y);

static void lq_quadtree_query_result_reset(quadtree_query_result_t *query_results);


/*******************
 * Implementations *
 *******************/

quadtree_t quadtree_create(int left, int bottom, int width, int height) {
    if (height <= 0 || width <= 0) {
        return NULL;
    }
    lq_quadtree_t *quadtree = (lq_quadtree_t*) calloc(1, sizeof(lq_quadtree_t));
    if (quadtree == NULL) {
        return NULL;
    }
    lq_quadtree_node_t *root = lq_quadtree_node_allocate();
    if (root == NULL) {
        free(quadtree);
        return NULL;
    }
    lq_quadtree_node_initialize(root, left, bottom, next_power_of_2(width), next_power_of_2(height), 0);
    quadtree->root = root;
    return (quadtree_t)quadtree;
}

void quadtree_destroy(quadtree_t qt) {
    lq_quadtree_t *quadtree = (lq_quadtree_t*) qt;
    if (quadtree != NULL) {
        if (quadtree->root != NULL) {
            lq_quadtree_node_free(quadtree->root);
        }
        free(quadtree);
    }
}

int quadtree_add(quadtree_t qt, long id, int number_of_polygon_points, int *xs, int *ys) {
    int i;
    int error_code = QUADTREE_SUCCESS;
    lq_quadtree_t *quadtree = (lq_quadtree_t*) qt;
    lq_quadtree_node_t *root = quadtree->root;
    LOG_DEBUG("adding polygon id: %ld\n", id);
    for (i = 0; i < number_of_polygon_points; ++i) {
        if (!lq_rect_point_is_in_bounds(root->bounding_box, xs[i], ys[i])) {
            return QUADTREE_ERROR_OUT_OF_BOUNDS;
        }
    }
    lq_polygon_node_t *polygon = lq_polygon_node_allocate();
    if (polygon == NULL) {
        return QUADTREE_ERROR_OUT_OF_MEMORY;
    }
    error_code = lq_polygon_node_initialize(polygon, id, number_of_polygon_points, xs, ys);
    if (error_code == QUADTREE_SUCCESS) {
        error_code = lq_quadtree_node_put_polygon(root, polygon);
    }
    lq_polygon_node_free(polygon);
    return error_code;
}

int quadtree_remove(quadtree_t qt, long id) {
    lq_quadtree_t *quadtree = (lq_quadtree_t*) qt;
    lq_quadtree_node_t *root = quadtree->root;
    lq_quadtree_node_remove(root, id);
    return QUADTREE_SUCCESS;
}

int quadtree_query(quadtree_t qt, int x, int y, quadtree_query_result_t *query_result) {
    lq_quadtree_t *quadtree = (lq_quadtree_t*) qt;
    lq_quadtree_node_t *root = quadtree->root;
    if (!lq_rect_point_is_in_bounds(root->bounding_box, x, y)) {
        return QUADTREE_ERROR_OUT_OF_BOUNDS;
    }
    return lq_quadtree_node_query(root, x, y, query_result);
}

quadtree_query_result_t* quadtree_query_result_allocate() {
    quadtree_query_result_t *query_result = (quadtree_query_result_t*) calloc(1, sizeof(quadtree_query_result_t));
    return query_result;
}

void quadtree_query_result_free(quadtree_query_result_t *query_result) {
    lq_quadtree_query_result_reset(query_result);
    free(query_result);
}


/*********************
 * private functions *
 *********************/

static lq_quadtree_node_t* lq_quadtree_node_allocate() {
    lq_quadtree_node_t *node = (lq_quadtree_node_t*) calloc(1, sizeof(lq_quadtree_node_t));
    if (node != NULL) {
        lq_rect_t *rect = lq_rect_allocate();
        if (rect != NULL) {
            node->bounding_box = rect;
        } else {
            lq_quadtree_node_free(node);
            node = NULL;
        }
    }
    return node;
}

static void lq_quadtree_node_free(lq_quadtree_node_t *node) {
    int quadrant;
    if (node == NULL) {
        return;
    }
    lq_rect_free(node->bounding_box);
    node->bounding_box = NULL;
    lq_quadtree_node_clear_polygons(node);
    for (quadrant = FIRST_QUADRANT; quadrant < NUMBER_OF_QUADRANTS; ++quadrant) {
        if (node->children[quadrant] != NULL) {
            lq_quadtree_node_free(node->children[quadrant]);
        }
        node->children[quadrant] = NULL;
    }
    free(node);
}

static void lq_quadtree_node_clear_polygons(lq_quadtree_node_t *node) {
    int i = 0;
    lq_polygon_node_t *polygon = node->polygons;
    while (polygon != NULL) {
        lq_polygon_node_t *next = polygon->next;
        lq_polygon_node_free(polygon);
        polygon = next;
        ++i;
    }
    assertEqualsInt("unexpected number of polygons", node->number_of_polygons, i);
    node->number_of_polygons = 0;
    node->polygons = NULL;
}

static void lq_quadtree_node_initialize(lq_quadtree_node_t *node, int rx, int ry, int rw, int rh, int depth) {
    lq_rect_initialize(node->bounding_box, rx, ry, rw, rh);
    node->depth = depth;
}

static int lq_quadtree_node_query(lq_quadtree_node_t *node, int x, int y, quadtree_query_result_t *query_result) {
    int i;
    int number_of_ids = 0;
    lq_quadtree_query_result_reset(query_result);
    /* find all polygons... */
    lq_quadtree_node_t *leaf = lq_quadtree_node_find_leaf(node, x, y);
    lq_polygon_node_t *polygon = leaf->polygons;
    /* ...in the beginning we don't know how many polygons we are going to end up with.
     *    We only know it will be no more than leaf->number_of_polygons */
    long *tmp_ids = (long*) calloc(leaf->number_of_polygons, sizeof(long));
    if (tmp_ids == NULL) {
        return QUADTREE_ERROR_OUT_OF_MEMORY;
    }
    while (polygon != NULL) {
        if (point_in_polygon(x, y, polygon->p->number_of_points, polygon->p->xs, polygon->p->ys)) {
            tmp_ids[number_of_ids] = polygon->p->id;
            ++number_of_ids;
        }
        polygon = polygon->next;
    }

    /* now that we know all polygons copy them into the result */
    long *ids = (long*) malloc(number_of_ids * sizeof(long));
    if (ids == NULL) {
        free(tmp_ids);
        return QUADTREE_ERROR_OUT_OF_MEMORY;
    }
    for (i = 0; i < number_of_ids; ++i) {
        ids[i] = tmp_ids[i];
    }
    free(tmp_ids);

    query_result->number_of_ids = number_of_ids;
    query_result->ids = ids;
    return QUADTREE_SUCCESS;
}

static lq_quadtree_node_t* lq_quadtree_node_find_leaf(lq_quadtree_node_t *node, int x, int y) {
    assert (node != NULL);
    int quadrant = lq_rect_get_quadrant(node->bounding_box, x, y);
    if (node->children[quadrant] != NULL) {
        return lq_quadtree_node_find_leaf(node->children[quadrant], x, y);
    } else {
        return node;
    }
}

static int lq_quadtree_node_put_polygon(lq_quadtree_node_t *node, lq_polygon_node_t *polygon) {
    int rx = node->bounding_box->left;
    int ry = node->bounding_box->bottom;
    int rw = node->bounding_box->width;
    int rh = node->bounding_box->height;
    int quadrant;
    int error_code = QUADTREE_SUCCESS;
    if (!collide_polygon_rectangle(polygon->p->number_of_points, polygon->p->xs, polygon->p->ys,
                                  rx, ry, rw, rh)) {
        LOG_DEBUG("bail %d %d %d %d %d\n", rx, ry, rw, rh, node->depth);
    } else if (node->depth == MAX_DEPTH || rw <= MIN_SIZE || rh <= MIN_SIZE ||
               (node->children[FIRST_QUADRANT] == NULL &&
                rectangle_inside_polygon(rx, ry, rw, rh,
                                         polygon->p->number_of_points,
                                         polygon->p->xs, polygon->p->ys))) {
        LOG_DEBUG("put %d %d %d %d %d %d\n", rx, ry, rw, rh, node->depth, node->number_of_polygons);
        lq_polygon_node_t *clone = lq_polygon_node_clone(polygon);
        if (clone != NULL) {
            lq_quadtree_node_add_polygon(node, clone);
        } else {
            error_code = QUADTREE_ERROR_OUT_OF_MEMORY;
        }
    } else {
        LOG_DEBUG("desend %d %d %d %d %d\n", rx, ry, rw, rh, node->depth);
        error_code = lq_quadtree_node_populate_children(node);
        if (error_code == QUADTREE_SUCCESS) {
            for (quadrant = FIRST_QUADRANT; quadrant < NUMBER_OF_QUADRANTS; ++quadrant) {
                error_code = lq_quadtree_node_put_polygon(node->children[quadrant], polygon);
                if (error_code != QUADTREE_SUCCESS) {
                    break;
                }
            }
        }
    }
    return error_code;
}

static void lq_quadtree_node_remove(lq_quadtree_node_t *node, long id) {
    int quadrant;
    lq_polygon_node_t *previous, *tmp, *polygon;
    for (quadrant = FIRST_QUADRANT; quadrant < NUMBER_OF_QUADRANTS; ++quadrant) {
        if (node->children[quadrant] != NULL) {
            lq_quadtree_node_remove(node->children[quadrant], id);
        }
    }
    previous = NULL;
    polygon = node->polygons;
    while (polygon != NULL) {
        if (polygon->p->id == id) {
            LOG_DEBUG("removing polygon %ld from (%d %d %d %d)\n", id,
                      node->bounding_box->left, node->bounding_box->bottom,
                      node->bounding_box->width, node->bounding_box->height);
            if (previous != NULL) {
                previous->next = polygon->next;
            } else {
                node->polygons = polygon->next;
            }
            tmp = polygon;
            polygon = polygon->next;
            lq_polygon_node_free(tmp);
            node->number_of_polygons--;
        } else {
            previous = polygon;
            polygon = polygon->next;
        }
    }
}

static int lq_quadtree_node_populate_children(lq_quadtree_node_t *node) {
    /* TODO: make sure we divide the space up correctly in case the size is not a power of 2. */
    int quadrant;
    int rx = node->bounding_box->left;
    int ry = node->bounding_box->bottom;
    int half_width = node->bounding_box->width / 2;
    int half_height = node->bounding_box->height / 2;
    int new_rx, new_ry;
    int new_width = half_width;
    int new_height = half_height;
    int error_code;
    if (node->children[FIRST_QUADRANT] != NULL) {
        return QUADTREE_SUCCESS;
    }
    for (quadrant = FIRST_QUADRANT; quadrant < NUMBER_OF_QUADRANTS; ++quadrant) {
        lq_quadtree_node_t *new_node = lq_quadtree_node_allocate();
        if (new_node == NULL) {
            error_code = QUADTREE_ERROR_OUT_OF_MEMORY;
            goto error;
        }
        switch (quadrant) {
        case FIRST_QUADRANT:
            new_rx = rx + half_width;
            new_ry = ry + half_height;
            break;
        case SECOND_QUADRANT:
            new_rx = rx;
            new_ry = ry + half_height;
            break;
        case THIRD_QUADRANT:
            new_rx = rx;
            new_ry = ry;
            break;
        case FOURTH_QUADRANT:
            new_rx = rx + half_width;
            new_ry = ry;
            break;
        default:
            error_code = QUADTREE_ERROR;
            goto error;
        }

        lq_quadtree_node_initialize(new_node, new_rx, new_ry, new_width, new_height, node->depth + 1);
        error_code = lq_quadtree_node_add_polygons(new_node, node->polygons);
        node->children[quadrant] = new_node;
        if (error_code != QUADTREE_SUCCESS) {
            goto error;
        }
    }
    lq_quadtree_node_clear_polygons(node);
    return QUADTREE_SUCCESS;

error:
    for (quadrant = FIRST_QUADRANT; quadrant < NUMBER_OF_QUADRANTS; ++quadrant) {
        lq_quadtree_node_free(node->children[quadrant]);
        node->children[quadrant] = NULL;
    }
    return error_code;
}

static int lq_quadtree_node_add_polygons(lq_quadtree_node_t *node, lq_polygon_node_t *polygons) {
    lq_polygon_node_t *current = polygons;
    while (current != NULL) {
        lq_polygon_node_t *clone = lq_polygon_node_clone(current);
        if (clone == NULL) {
            goto out_of_memory;
        }
        lq_quadtree_node_add_polygon(node, clone);
        current = current->next;
    }
    return QUADTREE_SUCCESS;

out_of_memory:
    lq_quadtree_node_clear_polygons(node);
    return QUADTREE_ERROR_OUT_OF_MEMORY;
}

static void lq_quadtree_node_add_polygon(lq_quadtree_node_t *node, lq_polygon_node_t *polygon) {
    assertTrue("poly has unexpected next\n", polygon->next == NULL);
    polygon->next = node->polygons;
    node->polygons = polygon;
    node->number_of_polygons++;
    LOG_DEBUG("added polygon to node (%d %d %d %d). now has %d polygons\n", node->bounding_box->left, node->bounding_box->bottom, node->bounding_box->width, node->bounding_box->height, node->number_of_polygons);
}



static lq_polygon_node_t* lq_polygon_node_allocate() {
    lq_polygon_node_t *polygon = (lq_polygon_node_t*) calloc(1, sizeof(lq_polygon_node_t));
    return polygon;
}

static lq_polygon_node_t* lq_polygon_node_clone(lq_polygon_node_t *polygon) {
    lq_polygon_node_t *clone = lq_polygon_node_allocate();
    if (clone != NULL) {
        clone->p = polygon->p;
        clone->ref_count = polygon->ref_count;
        *(polygon->ref_count) += 1;
        clone->next = NULL;
    }
    return clone;
}

static void lq_polygon_node_free(lq_polygon_node_t *polygon) {
    if (polygon == NULL) {
        return;
    }
    (*(polygon->ref_count))--;
    if (*(polygon->ref_count) == 0) {
        if (polygon->p != NULL) {
            free(polygon->p->xs);
            free(polygon->p->ys);
            free(polygon->p);
        }
        free(polygon->ref_count);
    }
    free(polygon);
}

static int lq_polygon_node_initialize(lq_polygon_node_t *polygon, long id, int number_of_points, int *xs, int *ys) {
    int i;
    lq_polygon_t *p = (lq_polygon_t*) calloc(1, sizeof(lq_polygon_t));
    if (p == NULL) {
        goto out_of_memory;
    }
    p->id = id;
    p->number_of_points = number_of_points;
    p->xs = (int*) malloc(number_of_points * sizeof(int));
    p->ys = (int*) malloc(number_of_points * sizeof(int));
    if (p->xs == NULL || p->ys == NULL) {
        goto out_of_memory;
    }
    for (i = 0; i < number_of_points; ++i) {
        p->xs[i] = xs[i];
        p->ys[i] = ys[i];
    }
    polygon->p = p;
    polygon->ref_count = (int*) malloc(sizeof(int));
    if (polygon->ref_count == NULL) {
        goto out_of_memory;
    }
    *(polygon->ref_count) = 1;
    polygon->next = NULL;
    return QUADTREE_SUCCESS;

out_of_memory:
    if (p != NULL) {
        free(p->ys);
        p->xs = NULL;
        free(p->xs);
        p->ys = NULL;
        free(polygon->ref_count);
        polygon->ref_count = NULL;
        free(p);
        polygon->p = NULL;
    }
    return QUADTREE_ERROR_OUT_OF_MEMORY;
}

static lq_rect_t* lq_rect_allocate() {
    lq_rect_t *rect = (lq_rect_t*) calloc(1, sizeof(lq_rect_t));
    return rect;
}

static void lq_rect_free(lq_rect_t *rect) {
    free(rect);
}

static void lq_rect_initialize(lq_rect_t *rect, int rx, int ry, int rw, int rh) {
    rect->left = rx;
    rect->bottom = ry;
    rect->width = rw;
    rect->height = rh;
}

static int lq_rect_get_quadrant(lq_rect_t *rect, int x, int y) {
    assert(lq_rect_point_is_in_bounds(rect, x, y));
    int width_half = rect->width / 2;
    int height_half = rect->height / 2;
    int quadrant = 0;
    if (x < rect->left + width_half) {
        if (y < rect->bottom + height_half) {
            quadrant = THIRD_QUADRANT;
        } else {
            quadrant = SECOND_QUADRANT;
        }
    } else {
        if (y < rect->bottom + height_half) {
            quadrant = FOURTH_QUADRANT;
        } else {
            quadrant = FIRST_QUADRANT;
        }
    }
    return quadrant;
}

static bool lq_rect_point_is_in_bounds(lq_rect_t *rect, int x, int y) {
    if ((x < rect->left || rect->left + rect->width <= x) ||
        (y < rect->bottom || rect->bottom + rect->height <= y)) {
        return false;
    }
    return true;
}

static void lq_quadtree_query_result_reset(quadtree_query_result_t *query_result) {
    if (query_result != NULL) {
        free(query_result->ids);
        query_result->ids = NULL;
        query_result->number_of_ids = -1;
    }
}
