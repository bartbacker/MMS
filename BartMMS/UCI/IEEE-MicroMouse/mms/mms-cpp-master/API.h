#pragma once

#include <string>

class API {

public:

    static int mazeWidth();
    static int mazeHeight();

    static bool wallFront();
    static bool wallRight();
    static bool wallLeft();

    static void moveForward(int distance = 1);
    static void turnRight();
    static void turnLeft();

    static void setWall(int x, int y, char direction);
    static void clearWall(int x, int y, char direction);

    static void setColor(int x, int y, char color);
    static void clearColor(int x, int y);
    static void clearAllColor();

    static void setText(int x, int y, const std::string& text);
    static void clearText(int x, int y);
    static void clearAllText();

    static bool wasReset();
    static void ackReset();

};

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