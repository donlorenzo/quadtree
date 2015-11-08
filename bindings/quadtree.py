from ctypes import *
import unittest

class Quadtree:
    lib = CDLL("../libquadtree.so")
    libc = CDLL("libc.so.6")
    QUADTREE_SUCCESS = c_int.in_dll(lib, "QUADTREE_SUCCESS").value
    QUADTREE_ERROR = c_int.in_dll(lib, "QUADTREE_ERROR").value
    QUADTREE_ERROR_OUT_OF_MEMORY = c_int.in_dll(lib, "QUADTREE_ERROR_OUT_OF_MEMORY").value
    QUADTREE_ERROR_OUT_OF_BOUNDS = c_int.in_dll(lib, "QUADTREE_ERROR_OUT_OF_BOUNDS").value

    def __init__(self, bounding_box):
        self.__quadtree = Quadtree.lib.quadtree_create(bounding_box[0], bounding_box[1],
                                                       bounding_box[2], bounding_box[3])
        if self.__quadtree == 0:
            raise RuntimeError("Could not create quadtree with bounding box " + str(bounding_box))
        self.__bounding_box = bounding_box

    def __del__(self):
        Quadtree.lib.quadtree_destroy(self.__quadtree)

    def put(self, polygon_id, polygon_points):
        INT_ARRAY = c_int * len(polygon_points)
        xs, ys = INT_ARRAY(), INT_ARRAY()
        for i, point in enumerate(polygon_points):
            xs[i] = point[0]
            ys[i] = point[1]
        self._handle_errors(Quadtree.lib.quadtree_put(self.__quadtree, polygon_id,
                                                      len(polygon_points), xs, ys))

    def remove(self, polygon_id):
        self._handle_errors(Quadtree.lib.quadtree_remove(self.__quadtree, polygon_id))

    def get(self, point):
        number_of_ids = c_int()
        ids = POINTER(c_long)()
        self._handle_errors(Quadtree.lib.quadtree_get(self.__quadtree, point[0], point[1],
                                                      byref(number_of_ids), byref(ids)))
        id_set = set(ids[i] for i in range(number_of_ids.value))
        Quadtree.libc.free(ids)
        return id_set

    def _handle_errors(self, return_code):
        if return_code == Quadtree.QUADTREE_SUCCESS:
            pass
        elif return_code == Quadtree.QUADTREE_ERROR_OUT_OF_MEMORY:
            raise MemoryError("quadtree lib ran out of memory")
        elif return_code == Quadtree.QUADTREE_ERROR_OUT_OF_BOUNDS:
            raise RuntimeError("Out of bounds. Current bounding box: " + self.__bounding_box)
        else:
            raise RuntimeError("quadtree lib failed with error code: " + str(return_code))



class TestQuadtree(unittest.TestCase):
    def testIllegalCreation(self):
        with self.assertRaises(RuntimeError):
            Quadtree([0, 0, 0, 0])
        with self.assertRaises(RuntimeError):
            Quadtree([0, 0, 1, -1])
        with self.assertRaises(RuntimeError):
            Quadtree([0, 0, -1, 1])

    def testCreation(self):
        Quadtree([0, 0, 10, 10])
        Quadtree([-10, -10, 10, 10])
        Quadtree([0, 0, 160000, 160000])

    def testSingleTriangle(self):
        quadtree = Quadtree([0, 0, 800, 600])
        id1 = 37
        quadtree.put(id1, [(0, 0), (800, 0), (0, 600)])
        self.assertEquals({id1}, quadtree.get((0, 0)))
        self.assertEquals(set(), quadtree.get((700, 500)))

    def testTwoTriangles(self):
        quadtree = Quadtree([0, 0, 800, 600])
        id1 = 37
        id2 = 42
        quadtree.put(id1, [(0, 0), (800, 0), (0, 600)])
        quadtree.put(id2, [(3, 3), (800, 0), (0, 600)])
        self.assertEquals({id1}, quadtree.get((0, 0)))
        self.assertEquals({id1, id2}, quadtree.get((5, 5)))
        self.assertEquals(set(), quadtree.get((500, 500)))

    def testRemoval(self):
        quadtree = Quadtree([0, 0, 800, 600])
        id1 = 37
        quadtree.put(id1, [(0, 0), (800, 0), (0, 600)])
        self.assertEquals({id1}, quadtree.get((0, 0)))
        quadtree.remove(id1)
        self.assertEquals(set(), quadtree.get((0, 0)))


if __name__ == '__main__':
    unittest.main()
