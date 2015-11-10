from ctypes import *
import unittest

class Quadtree:
    class _QuadtreeStruct(Structure):
        pass
    class _QueryResultStruct(Structure):
        _fields_ = [("number_of_ids", c_int),
                    ("ids", POINTER(c_long))]
    _QuadtreePtr = POINTER(_QuadtreeStruct)
    _QueryResultPtr = POINTER(_QueryResultStruct)

    _lib = CDLL("../libquadtree.so")
    _lib.quadtree_create.argtypes = [c_int, c_int, c_int, c_int]
    _lib.quadtree_create.restype = _QuadtreePtr
    _lib.quadtree_destroy.argtypes = [_QuadtreePtr]
    _lib.quadtree_destroy.restype = None
    _lib.quadtree_add.argtypes = [_QuadtreePtr, c_long, c_int,
                                 POINTER(c_int), POINTER(c_int)]
    _lib.quadtree_add.restype = c_int
    _lib.quadtree_query.argtypes = [_QuadtreePtr, c_int, c_int, _QueryResultPtr]
    _lib.quadtree_query.restype = c_int
    _lib.quadtree_remove.argtypes = [_QuadtreePtr, c_long]
    _lib.quadtree_remove.restype = c_int
    _lib.quadtree_query_result_allocate.argtypes = []
    _lib.quadtree_query_result_allocate.restype = _QueryResultPtr

    _QUADTREE_SUCCESS = c_int.in_dll(_lib, "QUADTREE_SUCCESS").value
    _QUADTREE_ERROR = c_int.in_dll(_lib, "QUADTREE_ERROR").value
    _QUADTREE_ERROR_OUT_OF_MEMORY = c_int.in_dll(_lib, "QUADTREE_ERROR_OUT_OF_MEMORY").value
    _QUADTREE_ERROR_OUT_OF_BOUNDS = c_int.in_dll(_lib, "QUADTREE_ERROR_OUT_OF_BOUNDS").value

    def __init__(self, bounding_box):
        self.__query_result = 0
        self.__quadtree = Quadtree._lib.quadtree_create(bounding_box[0], bounding_box[1],
                                                       bounding_box[2], bounding_box[3])
        if not self.__quadtree:
            raise RuntimeError("Could not create quadtree with bounding box " + str(bounding_box))
        self.__query_result = Quadtree._lib.quadtree_query_result_allocate()
        if not self.__query_result:
            raise MemoryError("Could not create quadtree query result")
        self.__bounding_box = bounding_box

    def __del__(self):
        Quadtree._lib.quadtree_destroy(self.__quadtree)
        Quadtree._lib.quadtree_query_result_free(self.__query_result)

    def add(self, polygon_id, polygon_points):
        INT_ARRAY = c_int * len(polygon_points)
        xs, ys = INT_ARRAY(), INT_ARRAY()
        for i, point in enumerate(polygon_points):
            xs[i] = point[0]
            ys[i] = point[1]
        self._handle_errors(Quadtree._lib.quadtree_add(self.__quadtree, polygon_id,
                                                      len(polygon_points), xs, ys))

    def remove(self, polygon_id):
        self._handle_errors(Quadtree._lib.quadtree_remove(self.__quadtree,
                                                         polygon_id))

    def query(self, point):
        self._handle_errors(Quadtree._lib.quadtree_query(self.__quadtree,
                                                        point[0], point[1],
                                                        self.__query_result))
        result = self.__query_result.contents
        return list(result.ids[i] for i in range(result.number_of_ids))

    def _handle_errors(self, return_code):
        if return_code == Quadtree._QUADTREE_SUCCESS:
            pass
        elif return_code == Quadtree._QUADTREE_ERROR_OUT_OF_MEMORY:
            raise MemoryError("quadtree lib ran out of memory")
        elif return_code == Quadtree._QUADTREE_ERROR_OUT_OF_BOUNDS:
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
        quadtree.add(id1, [(0, 0), (800, 0), (0, 600)])
        self.assertEquals([id1], quadtree.query((0, 0)))
        self.assertEquals([], quadtree.query((700, 500)))

    def testTwoTriangles(self):
        quadtree = Quadtree([0, 0, 800, 600])
        id1 = 37
        id2 = 42
        quadtree.add(id1, [(0, 0), (800, 0), (0, 600)])
        quadtree.add(id2, [(3, 3), (800, 0), (0, 600)])
        self.assertEquals([id1], quadtree.query((0, 0)))
        self.assertEquals([id1, id2], sorted(quadtree.query((5, 5))))
        self.assertEquals([], quadtree.query((500, 500)))

    def testRemoval(self):
        quadtree = Quadtree([0, 0, 800, 600])
        id1 = 37
        quadtree.add(id1, [(0, 0), (800, 0), (0, 600)])
        self.assertEquals([id1], quadtree.query((0, 0)))
        quadtree.remove(id1)
        self.assertEquals([], quadtree.query((0, 0)))


if __name__ == '__main__':
    unittest.main()
