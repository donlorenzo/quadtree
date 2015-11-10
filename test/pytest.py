from quadtree import Quadtree
import pygame
import random

W, H = (800, 600)
N_triangles = 10

BLACK = (0,0,0)
RED = (255,0,0)
GREEN = (0,255,0)
BLUE = (0,0,255)

def get_random_point():
    return (random.randint(0, W), random.randint(0, H))

def get_random_triangle():
    return (get_random_point(), get_random_point(), get_random_point())

def test():
    pygame.init()
    screen = pygame.display.set_mode((W, H), 0)
    quadtree = Quadtree((0, 0, W, H))
    triangles = []
    p = (0, 0)
    while True:
        new_triangles = False
        new_point = False
        update = False
        events = pygame.event.get()
        for e in events:
            if e.type == pygame.QUIT:
                return
            elif e.type == pygame.KEYUP and e.key == pygame.K_ESCAPE:
                return
            elif e.type == pygame.KEYUP and e.key == pygame.K_SPACE:
                new_triangles = True
            elif e.type == pygame.KEYUP and e.key == pygame.K_RETURN:
                new_point = True
            elif e.type == pygame.MOUSEBUTTONUP:
                p = e.pos
                update = True
                print p
        if new_triangles:
            for i in range(N_triangles):
                quadtree.remove(i)
            print "foo"
            triangles = []
            for i in range(N_triangles):
                triangles.append(get_random_triangle())
                quadtree.add(i, triangles[-1])
                print triangles[-1]

        if new_point:
            p = get_random_point()

        if new_point or new_triangles or update:
            screen.fill(BLACK)
            ids = quadtree.query(p)
            print ids
            for i in range(N_triangles):
                color = GREEN if i in ids else BLUE
                pygame.draw.lines(screen, color, True, triangles[i])
            pygame.draw.line(screen, RED, (p[0]-2, p[1]-2), (p[0]+2, p[1]+2))
            pygame.draw.line(screen, RED, (p[0]-2, p[1]+2), (p[0]+2, p[1]-2))
            pygame.display.update()

if __name__ == '__main__':
    test()
