#include <iostream>
#include <string.h>
#include "API.h"
#include <cmath>
#include "Maze.h"
#include "Minheap.cpp"

CellList *findNeighborCells(Maze *maze, int x, int y) //returns neighbor cells to a specific cell from the maze
{
	CellList *cellList = (CellList *)malloc(sizeof(CellList));
	int walls = maze->cellWalls[y][x];

	int sides;
	if ((x == 0 || x == 15) && (y == 0 || y == 15)) {
		sides = 2;
	}
	else if ((x == 0 || x == 15) || (y == 0 || y == 15)) {
		sides = 3;
	}
	else {
		sides = 4;
	}

	cellList->size = sides;
	cellList->cells = (Cell *)malloc(cellList->size * sizeof(Cell));

	int i = 0;
	if (x != 0)	{
		if ((walls & WEST_MASK) || (maze->cellWalls[y][x-1] & EAST_MASK)) {
			cellList->cells[i] = (Cell){Coord{x - 1, y}, Direction::WEST, true};
			i++;
		}
		else {
			cellList->cells[i] = (Cell){Coord{x - 1, y}, Direction::WEST, false};
			i++;
		}
	}
	if (y != 15) {
		if ((walls & NORTH_MASK) || (maze->cellWalls[y+1][x] & SOUTH_MASK)) {
			cellList->cells[i] = (Cell){Coord{x, y + 1}, Direction::NORTH, true};
			i++;
		}
		else {
			cellList->cells[i] = (Cell){Coord{x, y + 1}, Direction::NORTH, false};
			i++;
		} 
	}
	if (x != 15) {
		if ((walls & EAST_MASK) || (maze->cellWalls[y][x+1] & WEST_MASK)) {
			cellList->cells[i] = (Cell){Coord{x + 1, y}, Direction::EAST, true};
			i++;
		}
		else {
			cellList->cells[i] = (Cell){Coord{x + 1, y}, Direction::EAST, false};
			i++;
		}
	}
	if (y != 0)	{
		if ((walls & SOUTH_MASK) || (maze->cellWalls[y-1][x] & NORTH_MASK)){
			cellList->cells[i] = (Cell){Coord{x, y - 1}, Direction::SOUTH, true};
			i++;
		}
		else {
			cellList->cells[i] = (Cell){Coord{x, y - 1}, Direction::SOUTH, false};
			i++;
		}
	}
	return cellList;
}

//a* algo
//stack impl from bart's code but using Nodes
typedef struct _nodeStackItem   //Stack item structure remembers its position and previous stackitem
{
    Node pos;
    _nodeStackItem* prev;
} nodeStackItem;

typedef struct              //Stack structure knows top stack item (most recent)
{
    nodeStackItem* top;
} nodeStack;

void nodeStackInit(nodeStack& stack)    //Sets top stack item to null (empty)
{
    stack.top = NULL;
}

void nodeStackPush(nodeStack& stack, Node pos) //New item created. Assigned current position. Previous new item assigned to the old top. Top assigned to new item (pushes old item down 1)
{
    nodeStackItem* newItem = (nodeStackItem*)malloc(sizeof(nodeStackItem));
    newItem->pos = pos;
    newItem->prev = stack.top;
    stack.top = newItem;
}

Node nodeStackPop(nodeStack& stack)             //Sets position to top item position (value). Old item assigned old top item. Top item assigned to new top item (previous item). Pops / frees top item and pushes previous item to top item
{
    Node pos = stack.top->pos;
    nodeStackItem* oldItem = stack.top;
    stack.top = stack.top->prev;
    free(oldItem);
    return pos;
}

bool nodeStackEmpty(nodeStack& stack)      //Check if stack empty
{
    return stack.top == NULL;
}

bool nodeStackSearch(nodeStack& stack, Node elem) { //searches through stack for specific elem
	nodeStackItem *curr_item = stack.top;
	while (curr_item != NULL) {
		if (curr_item->pos.loc.x == elem.loc.x && curr_item->pos.loc.y == elem.loc.y) {
			return true;
		}
		curr_item = curr_item->prev;
	}
	return false;
}

Node* pathing(Node node) {  //returns path from goal node to start node
	std::cerr << "pathing" << std::endl;
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
		//std::cerr << "x coord" << path[i].loc.x << "y coord" << path[i].loc.x << std::endl;
	}
	return path;
}

double heuristic(Coord a, Coord b){ //manhatten heuristic to avoid weird turning stuff
	return (abs(a.x - b.x)) + (abs(a.y - b.y)); 
}

Node* neighborNodes(Maze* maze, Node current) {		//returns the adjacent nodes to a specific node
	CellList *neighborCells = findNeighborCells(maze, current.loc.x, current.loc.y);
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
			 //will need to implement "turning" within routing algo in order to get here
			neighbors[i] = Node{c.pos, current.g_score, INT_MAX, &current}; 
			i++;
		}
		else if (!c.blocked) {
			neighbors[i] = Node{c.pos, current.g_score /*+ 0.25*/, INT_MAX, &current}; //if need to turn to cell, add a modifier to g_score (turning cost)
			i++;
		}
	}
	return neighbors;
}

Node* a_star_algo(Maze* maze, Coord goal) { //main algorithm, finds shortest path to set goal
	Node current;
	nodeStack closeList;
    nodeStackInit(closeList);
	Node* neighbor;
	int heapIndex;
	Node openNeighbor;


	Coord start = Coord{0,0};
	Node start_node = Node{start, 0, heuristic(start, goal)}; //g = 0, f = heuristic

	Heap* openList = makeHeap(256);
	heap_insert(openList, start_node);
	
	while (openList->size != 0) { 
		current = heap_extract(openList);
		std::cerr <<"current." << current.loc.x << ", "<< current.loc.y << std::endl;
		nodeStackPush(closeList, current);
		if (current.loc.x == goal.x && current.loc.y == goal.y) {
			std::cerr << "reached goal" << std::endl;
			return pathing(current);
		}
		neighbor = neighborNodes(maze, current);
		for (int i = 0; i < sizeof(neighbor); i++) {
			if (nodeStackSearch(closeList, neighbor[i])) {
				std::cerr << "skip" << neighbor[i].loc.x <<", " << neighbor[i].loc.y << std::endl;
				continue;
			}
			neighbor[i].f_score = neighbor[i].g_score + heuristic(neighbor[i].loc, goal);
			heapIndex = heap_search(openList, neighbor[i]);
			if (heapIndex < 0) {
				std::cerr << "push" << neighbor[i].loc.x <<", " << neighbor[i].loc.y << std::endl;
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
	std::cerr << "path not found"<< std::endl;
	return NULL;
}