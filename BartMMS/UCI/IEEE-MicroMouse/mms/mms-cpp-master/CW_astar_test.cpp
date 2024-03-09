#include <iostream>
#include <string.h>
#include "API.h"
#include <cmath>
#include "Minheap.cpp"
#include "CW_Maze.cpp"

//a* algo
//stack impl from bart's code but imma use cells instead
typedef struct _stackItem   //Stack item structure remembers its position and previous stackitem
{
    Coord pos;
    _stackItem* prev;
} stackItem;

typedef struct              //Stack structure knows top stack item (most recent)
{
    stackItem* top;
} Stack;

void StackInit(Stack& stack)    //Sets top stack item to null (empty)
{
    stack.top = NULL;
}

void StackPush(Stack& stack, Coord pos) //New item created. Assigned current position. Previous new item assigned to the old top. Top assigned to new item (pushes old item down 1)
{
    stackItem* newItem = (stackItem*)malloc(sizeof(stackItem));
    newItem->pos = pos;
    newItem->prev = stack.top;
    stack.top = newItem;
}

Coord StackPop(Stack&stack)             //Sets position to top item position (value). Old item assigned old top item. Top item assigned to new top item (previous item). Pops / frees top item and pushes previous item to top item
{
    Coord pos = stack.top->pos;
    stackItem* oldItem = stack.top;
    stack.top = stack.top->prev;
    free(oldItem);
    return pos;
}

bool StackEmpty(Stack& stack)      //Check if stack empty
{
    return stack.top == NULL;
}

Coord neighborNodes(Maze* maze, Coord current) {
	getNeighborCells(maze, current.x, current.y);
}

int gScore(Coord start, Coord current) {
    if (current.x == start.x && current.y == start.y) {
        return 0; 
	}
	return 0; //actual squares covered thus far
}

int fScore(Coord start, Coord current, Coord goal) {
    return gScore(start, current) + (abs(goal.x-current.x)) + (abs(goal.y - current.y)); //manhatten heuristic to avoid weird turningstuff
}

void a_star(Maze *maze, Coord start, Coord goal) {
    Coord current;
	Stack closeList;
    StackInit(closeList);
	Heap* openList = makeHeap(256, maze);
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			heap_insert(openList, )
		}
	}
	while (!is_empty(openList)) {
		
	}
}

int dis = 0;
Maze maze;
int main(int argc, char *argv[]) {
	maze.mouse_pos = Coord{0, 0};
	maze.mouse_dir = NORTH;
	setGoalCell(&maze, 4);
	floodfill(&maze);
	int round = 0;
	while (round < 15) {
		scanWalls(&maze);
		updateSimulator(maze);
		floodfill(&maze);
		rotate(&maze, maze.mouse_pos.x, maze.mouse_pos.y);
		move(&maze);
		updateMousePos(&maze.mouse_pos, maze.mouse_dir);
		if (maze.distances[maze.mouse_pos.y][maze.mouse_pos.x] == 0) {
			setGoalCell(&maze, 1);
			round++;
		}
		if ((maze.mouse_pos.x == 0) && (maze.mouse_pos.y == 0)) {
			dis = maze.distances[maze.mouse_pos.y][maze.mouse_pos.x];
			setGoalCell(&maze, 4);
			round++;
		}
	}
}