
#################################################
##########THIS SCRIPT IS AI GENERATED!!##########
#################################################

import random

def initialize_maze(size):
    """ Initialize the maze with walls """
    maze = [['#'] * size for _ in range(size)]
    return maze

def carve_passages_from(cx, cy, maze, size):
    """ Carve passages in the maze using an iterative DFS algorithm """
    stack = [(cx, cy)]
    maze[cy][cx] = '.'
    
    while stack:
        (x, y) = stack[-1]
        directions = [(0, 2), (2, 0), (0, -2), (-2, 0)]  # Right, Down, Left, Up
        random.shuffle(directions)
        
        carved = False
        for direction in directions:
            nx, ny = x + direction[0], y + direction[1]
            if 0 <= nx < size and 0 <= ny < size and maze[ny][nx] == '#':
                maze[ny][nx] = '.'
                maze[y + direction[1]//2][x + direction[0]//2] = '.'
                stack.append((nx, ny))
                carved = True
                break
        if not carved:
            stack.pop()

def place_start_and_goal(maze, size):
    """ Place the start and goal points """
    maze[1][0] = 's'
    maze[size - 2][size - 1] = 'e'

def generate_maze(size):
    """ Generate the maze with start and goal points """
    maze = initialize_maze(size)
    carve_passages_from(1, 1, maze, size)
    place_start_and_goal(maze, size)
    return maze

def save_maze_to_file(maze, filename):
    """ Save the maze to a text file """
    with open(filename, 'w') as f:
        for row in maze:
            f.write(''.join(row) + '\n')

size = 100
filename = 'maze.txt'

maze = generate_maze(size)
save_maze_to_file(maze, filename)
