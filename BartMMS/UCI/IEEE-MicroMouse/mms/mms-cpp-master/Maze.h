#pragma once

#include <iostream>
#include <string>

class Maze:
    Coord mousePos
    Direction mouseDir
    int distances[16][16]
    bool exploredCells[16][16]
    int cellWalls[16][16]
    Coord* goalPos

struct Coord:
    int x,
    int y

enum Direction:
    NORTH = 0,
    WEST,
    SOUTH,
    EAST

struct Cell:
    Coord pos,
    Direction dir,
    bool blocked
