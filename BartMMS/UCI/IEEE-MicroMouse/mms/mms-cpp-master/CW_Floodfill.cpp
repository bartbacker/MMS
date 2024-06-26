#include <iostream>
#include <string.h>
#include "API.h"
#include <cmath>
#include "Maze.h"
#include "CW_a_star.cpp"

const int MAX_COST = 255;
 
void log(const std::string &text) { 
	std::cerr << text << std::endl;
}

char dir_chars[4] = {'n', 'e', 's', 'w'};
int dir_mask[4] = {0b1000, 0b0100, 0b0010, 0b0001};

int goalSize;
void setGoalCell(Maze *maze, int size) {  //sets goal to middle of 16x16 square or to the begnning cell
	goalSize = size;
	maze->goalPos = (Coord *)malloc(size * sizeof(Coord));
	int i = 0;
	if (size == 4){
		for (int x = 7; x < 9; x++) {
			for (int y = 7; y < 9; y++) {
				maze->goalPos[i] = Coord{x, y};
				i++;
			}
		}
	}
	if (size == 1) {
		maze->goalPos[i] = Coord{0, 0};
	}
}

CellList *getNeighborCells(Maze *maze, int x, int y) //returns the neighboring cells and if they are blocked or not
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

Cell getBestCell(Maze *maze, int x, int y) { //returns the optimal (least cost and unblocked) cell a mouse can turn to
	CellList *adjacentCells = getNeighborCells(maze, x, y);
	Cell bestCell;
	int dis[adjacentCells->size];
	for (int i = 0; i < adjacentCells->size; i++) {
		int a = adjacentCells->cells[i].pos.x;
		int b = adjacentCells->cells[i].pos.y;
		if (adjacentCells->cells[i].blocked) dis[i] = MAX_COST;
		else dis[i] = maze->distances[b][a];
	}
	int cell = 0;
	int best = dis[0];
	for (int j = 0; j < adjacentCells->size; j++) 		{
		if (dis[j] <= best) {
			best = dis[j];
			cell = j;
		}
	}
	bestCell = adjacentCells->cells[cell];
	free(adjacentCells->cells);
	free(adjacentCells);
	return bestCell;
}

void rotate(Maze *maze, int targetDir) { //rotates the mouse to face the target cell
	if ((maze->mouse_dir + 3) % 4 == targetDir) {
		API::turnLeft();
		maze->mouse_dir = (Direction)((maze->mouse_dir + 3) % 4);
	}
	while (maze->mouse_dir != targetDir) {
		API::turnRight();
		maze->mouse_dir = (Direction)((maze->mouse_dir + 1) % 4);
	}
}

void updateSimulator(Maze maze) { //updates the simulator with new cell walls
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			if (maze.cellWalls[y][x] & NORTH_MASK)
				API::setWall(x, y, 'n');
			if (maze.cellWalls[y][x] & EAST_MASK)
				API::setWall(x, y, 'e');
			if (maze.cellWalls[y][x] & SOUTH_MASK)
				API::setWall(x, y, 's');
			if (maze.cellWalls[y][x] & WEST_MASK)
				API::setWall(x, y, 'w');
			API::setText(x, y, std::to_string(maze.distances[y][x]));
		}
	}
}

void scanWalls(Maze *maze) { //scan the walls of current cell and updates it in memory
	if (API::wallFront())
		maze->cellWalls[maze->mouse_pos.y][maze->mouse_pos.x] |= dir_mask[maze->mouse_dir];
	if (API::wallRight())
		maze->cellWalls[maze->mouse_pos.y][maze->mouse_pos.x] |= dir_mask[(maze->mouse_dir + 1) % 4];
	if (API::wallLeft())
		maze->cellWalls[maze->mouse_pos.y][maze->mouse_pos.x] |= dir_mask[(maze->mouse_dir + 3) % 4];
}

void updateMousePos(Coord *pos, Direction dir) { //updates known mouse positionin memory
	if (dir == NORTH)
		pos->y++;
	if (dir == SOUTH)
		pos->y--;
	if (dir == WEST)
		pos->x--;
	if (dir == EAST)
		pos->x++;
}

Coord queue[255];
int head = 0;
int tail = 0;
void floodfill(Maze *maze) { //implementation of floodfill
	head = 0; 
	tail = 0;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			maze->distances[y][x] = MAX_COST;
		}
	}
	for (int i = 0; i < goalSize; i++) {
		int x = maze->goalPos[i].x;
		int y = maze->goalPos[i].y;
		maze->distances[y][x] = 0;
		queue[i] = Coord{x, y};
		tail++;
	}
	int newcost;
	while (tail - head > 0) {
		Coord curr = queue[head];
		head++;
		newcost = maze->distances[curr.y][curr.x] + 1;
		CellList *neighbors = getNeighborCells(maze, curr.x, curr.y);
		for (int j = 0; j < neighbors->size; j++) {
			Cell c = neighbors->cells[j];
			if (!c.blocked) {
				if (maze->distances[c.pos.y][c.pos.x] > newcost) {
					maze->distances[c.pos.y][c.pos.x] = newcost;
					queue[tail] = c.pos;
					tail++;
				}	
			}
		}
		free(neighbors->cells);
		free(neighbors);
	}
}

//runs the thing in the simulator
Maze maze;
int dis = 0;
int main(int argc, char *argv[]) {
	maze.mouse_pos = Coord{0, 0};
	maze.mouse_dir = NORTH;
	setGoalCell(&maze, 4);
	floodfill(&maze);
	int round = 0;
	while (round < 6) { //prev_dis != final_dis) {
		scanWalls(&maze);
		updateSimulator(maze);
		floodfill(&maze);
		rotate(&maze, getBestCell(&maze, maze.mouse_pos.x, maze.mouse_pos.y).dir);
		API::moveForward();
		dis++;
		updateMousePos(&maze.mouse_pos, maze.mouse_dir);
		if (maze.distances[maze.mouse_pos.y][maze.mouse_pos.x] == 0) {
			setGoalCell(&maze, 1);
			round++;
		}
		if ((maze.mouse_pos.x == 0) && (maze.mouse_pos.y == 0)) {
			setGoalCell(&maze, 4);
			round++;
		}
	}
	//maze.mouse_dir = NORTH;
	//std::cerr << "start a*" << std::endl; 
	//Node* path = a_star_algo(&maze, Coord{7,7});
	//std::cerr << "algo done" << std::endl; 
}
