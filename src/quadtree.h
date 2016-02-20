/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Lorenz Quack
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef DE_LORENZQUACK_CODE_QUADTREE_H
#define DE_LORENZQUACK_CODE_QUADTREE_H

/**
 * @mainpage Quadtree - a region quadtree for polygons
 *
 * @author Lorenz Quack (code@lorenzquack.de)
 * @date 8.11.2015
 * @version 1.0
 *
 * @section Introduction
 *
 * This library is used to efficiently finding polygons that contain a
 * given point in two dimensions. This is achieved by first placing
 * the polygons into a quadtree object and then querying the quadtree.
 *
 * The fundamental object is the \link quadtree.h#quadtree_t
 * quadtree_t \endlink which is an opaque object that should only be
 * used in conjunction with the functions provided by the library.
 *
 * @section Usage
 *
 * The library comes with a single header file: quadtree.h.
 *
 * The first step is to create a quadtree object by calling
 * quadtree_create() passing the bounding box of the area you want to
 * cover with this quadtree.
 *
 * Next, you should populate the quadtree with polygons by calling
 * quadtree_add().
 *
 * Once the quadtree is filled you can query it by calling
 * quadtree_query().  It expects a pointer to a quadtree_query_result_t
 * as one of its arguments which will be populated with the result
 * of the query.  To obtain a quadtree_query_result_t you should call
 * quadtree_query_result_allocate().  After processing the results you
 * can reuse a quadtree_query_result_t object for further queries.
 * Once you are done with a quadtree_query_result_t object you should
 * dispose of it by calling quadtree_query_result_free().
 *
 * Should a polygon change it has to be removed by calling
 * quadtree_remove() and readded by calling quadtree_add().
 *
 * When the quadtree is no longer needed it can be disposed of by
 * calling quadtree_destroy(). This will clean up all internal data
 * structures. It is \em not necessary to remove the polygons before
 * destroying a quadtree.
 *
 * @section Example
 *
 * \code{.c}
 * #include <stdio.h>
 * #include <quadtree.h>
 *
 * struct {
 *     int number_of_corners;
 *     int *x_coords;
 *     int *y_coords;
 * } polygon;
 *
 * int main() {
 *     polygon polygons[10];
 *     // ... fill polygons with data
 *
 *     // create and populate the quadtree
 *     quadtree_t quadtree = quadtree_create(0, 0, 800, 600);
 *     assert(quadtree != NULL);
 *     for (int i=0; i < 10; ++i) {
 *         quadtree_add(quadtree, (long) i,
 *                      polygons[i]->number_of_corners,
 *                      polygons[i]->x_coords, polygons[i]->y_coords);
 *     }
 *
 *     // query the quadtree
 *     quadtree_query_result_t *result = quadtree_query_result_allocate();
 *     int point_to_query_x = ...;
 *     int point_to_query_y = ...;
 *     int error_code;
 *     error_code = quadtree_query(quadtree,
 *                                 point_to_query_x, point_to_query_y,
 *                                 result);
 *     if (error_code != QUADTREE_SUCCESS) {
 *         // ... handle error
 *     }
 *     for (int i = 0; i < result->number_of_ids; ++i) {
 *         printf("polygon %ld contains the query point (%d, %d)\n",
 *                result->ids[i], point_to_query_x, point_to_query_y);
 *     }
 *
 *     // clean up
 *     quadtree_query_result_free(result);
 *     quadtree_destroy(quadtree);
 *     return 0;
 * }
 * \endcode
 */

/**
 * @file quadtree.h
 */

int QUADTREE_SUCCESS =                   0;
int QUADTREE_ERROR =                     1;
int QUADTREE_ERROR_OUT_OF_MEMORY =       2;
int QUADTREE_ERROR_OUT_OF_BOUNDS =       3;

/**
 * @brief Opaque object representing a quadtree.
 * @anchor quadtree_t
 */
typedef struct lq_quadtree_t *quadtree_t;

/**
 * @brief Structure to hold the result of a quadtree query
 *
 * This structure should be passed to quadtree_query() and will
 * then contain the result of that query.  It can be reused for
 * multiple queries befor being disposed of by calling
 * quadtree_query_result_free().
 *
 * @see quadtree_query_result_allocate()
 * @see quadtree_query_result_free()
 * @see quadtree_query()
 */
typedef struct {
    /** the number of ids this query result contains or -1 if the
     * query failed
     */
    int number_of_ids;
    /** an array of ids that resulted from the query */
    long *ids;
} quadtree_query_result_t;


/**
 * @brief Creates a new quadtree object
 *
 * Creates an opaque quadtree object for the area specified by the
 * arguments.  The arguments \a width and \a height must be positive.
 * The area cannot be changed, moved, or resized during the lifetime
 * of a quadtree.  Callers should check the return value before using
 * it.  When done using the quadtree it should be destroied by calling
 * quadtree_destroy().
 *
 * @param left the x coordinate of the left side of the area covered
 *             by the quadtree
 * @param bottom the y coordinate of the bottom side of the area
 *               covered by the quadtree
 * @param width the width of the area covered by the quadtree
 * @param height the height of the area covered by the quadtree
 * @returns the new quadtree object or NULL on failure
 * @see quadtree_destroy
 */
quadtree_t quadtree_create(int left, int bottom, int width, int height);

/**
 * @brief Deletes a quadtree object
 *
 * Cleans up all resources of the quadtree.
 *
 * @param quadtree the quadtree to be deleted
 * @see quadtree_create
 */
void quadtree_destroy(quadtree_t quadtree);

/**
 * @brief Place a polygon into the quadtree
 *
 * The polygon is specified by two array containing the x and y
 * coordinates of the polygon corners.  Furthermor an \a id is
 * associated with the polygon to later identify the polygon in calls
 * to quadtree_query() and quadtree_remove(). The caller is responsible
 * for making sure the \a id is unique.
 *
 * @param quadtree the quadtree to operate on
 * @param id a unique id to identify the polygon
 * @param number_of_polygon_points size of the following arrays
 * @param xs[] array of x coordinates
 * @param ys[] array of y coordinates
 * @returns QUADTREE_SUCCESS if successful.
 * @returns QUADTREE_OUT_OF_BOUNDS if part of the polygon lies outside
 *                                 the area covered by the quadtree.
 * @returns QUADTREE_ERROR_OUT_OF_MEMORY if the function could not
 *                                       allocate memory
 * @see quadtree_query
 * @see quadtree_remove
 */
int quadtree_add(quadtree_t quadtree, long id, int number_of_polygon_points, int xs[], int ys[]);

/**
 * @brief Get a list of polygon ids that contain the given point.
 *
 * Finds all polygons that contain the point (\a x, \a y) and returns
 * them in a newly allocated array.  The Caller is responsible for
 * freeing the allocated memory in the ids parameter.
 *
 * @param[in] quadtree the quadtree to operate on
 * @param[in] x the x coordinate of the point
 * @param[in] y the y coordinate of the point
 * @param[out] number_of_ids the number of ids that were put into the
 *                           ids array
 * @param[out] ids the function will put the list of ids at the
 *                 pointer this argument points to
 * @returns QUADTREE_SUCCESS if successful.
 * @returns QUADTREE_ERROR_OUT_OF_BOUNDS if (\a x, \a y) does not lie
 *                                       within the quadtree's
 *                                       bounding box
 * @returns QUADTREE_ERROR_OUT_OF_MEMORY if the function could not
 *                                       allocate memory
 * @see quadtree_add
 * @see quadtree_remove
 */
int quadtree_query(quadtree_t quadtree, int x, int y, quadtree_query_result_t *query_result);

/**
 * @brief Removes a polygon from a quadtree
 *
 * Should there be multiple polygons with the given \a id all of them
 * will be removed from the quadtree.
 *
 * @param quadtree the quadtree to operate on
 * @param id the id of the polygon to be removed from the quadtree
 * @returns QUADTREE_SUCCESS always. This function cannot fail.
 * @see quadtree_add
 * @see quadtree_query
 */
int quadtree_remove(quadtree_t quadtree, long id);

/**
 * @brief Allocate a new quadtree_query_result_t
 *
 * This should be used to create quadtree_query_result_t objects for use
 * with quadtree_query().  When done using the result object it should
 * be freed by calling quadtree_query_result_free().
 *
 * @return a pointer to a new quadtree_query_result_t object to be used
 *         with quadtree_query()
 * @see quadtree_query_result_free()
 * @see quadtree_query()
 */
quadtree_query_result_t *quadtree_query_result_allocate();

/**
 * @brief Frees the memory pointed to by \a query_result
 * @param query_result the quadtree_query_result_t object to dispose of
 * @see quadtree_query_result_allocate()
 * @see quadtree_query()
 */
void quadtree_query_result_free(quadtree_query_result_t *query_result);


#endif /* DE_LORENZQUACK_CODE_QUADTREE_H */
