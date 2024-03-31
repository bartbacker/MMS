#include <iostream>
#include <string.h>
#include "API.h"
#include <cmath>
#include "Maze.h"
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

Node* pathing(Node node) {
	int size = 0;
	while (node.parent != NULL) {
		size++;
		node = *(node.parent);
	}
	Node *path = (Node *)malloc(size * sizeof(Node));
	int i = 0;
	while (node.parent != NULL) {
		node = *(node.parent);
		path[i] = node;
		std::cerr << "x coord" << path[i].loc.x << "y coord" << path[i].loc.x << std::endl;
	}
	return path;
}

double heuristic(Coord a, Coord b){
	return (abs(a.x - b.x)) + (abs(a.y - b.y)); //manhatten heuristic to avoid weird turningstuff
}

Node* neighborNodes(Maze* maze, Node current) {		
	CellList *neighborCells = getNeighborCells(maze, current.loc.x, current.loc.y);
	int size;
	std::cerr << "number of neighbors "<< neighborCells->size << std::endl;
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
			std::cerr << "same_direction" << std::endl; //will need to implement "turning" within routing algo
			neighbors[i] = Node{c.pos, current.g_score + 0.75, INT_MAX, &current}; //if mouse is facing same direction as the next cell (dont need to turn)
			i++;
		}
		else if (!c.blocked) {
			std::cerr << "needs_turn" << std::endl; 
			neighbors[i] = Node{c.pos, current.g_score + 1, INT_MAX, &current};
			i++;
		}
	}
	return neighbors;
}

Node* a_star_algo(Maze* maze, Coord goal) {
	Node current;
	Stack closeList;
    StackInit(closeList);
	Node* neighbor;
	int heapIndex;
	Node openNeighbor;


	Coord start = Coord{0,0};
	Node start_node = Node{start, 0, heuristic(start, goal)}; //g = 0, f = heuristic

	Heap* openList = makeHeap(256);
	heap_insert(openList, start_node);
	
	while (openList->size != 0) {
		current = heap_extract(openList);
		std::cerr << "current " << current.loc.x << ", "<< current.loc.y << std::endl;
		StackPush(closeList, current);	
		if (current.loc.x == goal.x && current.loc.y == goal.y) {
			std::cerr << "reached goal" << std::endl;
			return pathing(current);
		}

		neighbor = neighborNodes(maze, current);
		for (int i = 0; i < sizeof(neighbor); i++) {
			if (!stackSearch(closeList, neighbor[i])) {
				neighbor[i].f_score = neighbor[i].g_score + heuristic(neighbor[i].loc, goal);
				heapIndex = heap_search(openList, neighbor[i]);
				if (heapIndex < 0) {
					heap_insert(openList, neighbor[i]);
				}
				else {
					openNeighbor = openList->arr[heapIndex];
					if (neighbor[i].g_score < openNeighbor.g_score) {
						openNeighbor.g_score = neighbor->g_score;
						openNeighbor.parent = neighbor->parent;
					}
				}
			}
		}
	}
	return NULL;
}
