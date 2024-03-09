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
    Node pos;
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

void StackPush(Stack& stack, Node pos) //New item created. Assigned current position. Previous new item assigned to the old top. Top assigned to new item (pushes old item down 1)
{
    stackItem* newItem = (stackItem*)malloc(sizeof(stackItem));
    newItem->pos = pos;
    newItem->prev = stack.top;
    stack.top = newItem;
}

Node StackPop(Stack&stack)             //Sets position to top item position (value). Old item assigned old top item. Top item assigned to new top item (previous item). Pops / frees top item and pushes previous item to top item
{
    Node pos = stack.top->pos;
    stackItem* oldItem = stack.top;
    stack.top = stack.top->prev;
    free(oldItem);
    return pos;
}

bool StackEmpty(Stack& stack)      //Check if stack empty
{
    return stack.top == NULL;
}

bool stackSearch(Stack& stack, Node elem) {
	for (int i = 0; i < sizeof(stack.top); i++) {
		if (elem.loc.x == stack.top[i].pos.loc.x && elem.loc.y == stack.top[i].pos.loc.y) {
			return true;
		}
	}
	return false;
}

Coord* pathing(Coord n) {
	Coord path[5];
	return path;
}

int heuristic(Coord a, Coord b){
	return (abs(a.x - b.x)) + (abs(a.y - b.y)); //manhatten heuristic to avoid weird turningstuff
}

Node* neighborNodes(Maze* maze, Node current) {
	CellList *neighborCells = getNeighborCells(maze, current.loc.x, current.loc.y);
	int size;
	for (int j = 0; j < neighborCells->size; j++) {
		Cell c = neighborCells->cells[j];
		if (!c.blocked) {
			size++;
		}
	}
	Node *neighbors = (Node *)malloc(size * sizeof(Node));
	int i = 0;
	for (int j = 0; j < neighborCells->size; j++) {
		Cell c = neighborCells->cells[j];
		if (!c.blocked && c.dir == maze->mouse_dir) {
			neighbors[i] = Node{c.pos, current.g_score + 0.8, MAX_COST}; //if mouse is facing same direction as the next cell (dont need to turn)
			i++;
		}
		else if (!c.blocked) {
			neighbors[i] = Node{c.pos, current.g_score + 1, MAX_COST};
			i++;
		}
	}
}

Coord* a_star_algo(Maze* maze, Coord start, Coord goal) {
	Node current;
	Stack closeList;
    StackInit(closeList);
	Node nodes[256];
	Node* neighbors;

	nodes[0].g_score = 0;
	nodes[0].f_score = nodes[0].g_score + heuristic(start, goal);
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			nodes[y*16+x] = Node{Coord{x,y}, MAX_COST, MAX_COST};
		}
	}
	Heap* openList = makeHeap(256, maze);
	while (!is_empty(openList)) {
		current = heap_extract(openList);
		if (current.loc.x == goal.x && current.loc.y == goal.y) {
			return pathing(goal);
		}
		StackPush(closeList, current);
		neighbors = neighborNodes(maze, current);
		for (int i = 0; i < sizeof(neighbors); i++) {
			if (!stackSearch(closeList, neighbors[i])) {
				neighbors[i].f_score = neighbors[i].g_score + heuristic(neighbors[i].loc, goal);
				if (!heapSearch(openList, neighbors[i])) {
					heap_insert(openList, neighbors[i]);
				}
				else
			}
		}
	}
	return NULL;
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