#pragma once

#include <iostream>
#include <string>


struct Coord {
	int x;
	int y;
};

enum Direction {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3
};

enum DirectionBitmask
{
	NORTH_MASK = 0b1000,
	EAST_MASK = 0b0100,
	SOUTH_MASK = 0b0010,
	WEST_MASK = 0b0001
};

struct Cell { 
	Coord pos;
	Direction dir;
	bool blocked;
};

struct CellList {
	int size;
	Cell *cells;
};

struct Maze {
	Coord mouse_pos;
	Direction mouse_dir;
	int distances[16][16];
	bool exploredCells[16][16];
	int cellWalls[16][16];
	Coord *goalPos;
};

struct Node {
    Coord loc;
    double g_score;
    double f_score;
    Node* parent;
};

struct Heap {
    Node *arr;   //dynamic array
    int size;   //number of elems in Heap
    int capacity; //max elems possible
};