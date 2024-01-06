#include <iostream>
#include <string>

#include "API.h"

void log(const std::string& text) {
    std::cerr << text << std::endl;
}

struct Coord{
    int x;
    int y;
};

enum Direction{
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};
/* enum Direction{
     NORTH = 8, //1000
     EAST = 4,  //0100
     SOUTH = 2, //0010
     WEST = 1   //0001
     //13->1101->N,E,W walls
 }; */

struct Cell{
    Coord pos;
    Direction dir;
    bool blocked;
};

struct Maze{
    public:
        Coord mousePos;
        Direction mouseDir;
        int distances[16][16];
        bool exploredCells[16][16];
        int cellWalls[16][16];
        Coord* goalPos;
};

char dirs[4] = {'n','e','s','w'};

int x = 0;
int y = 0;

Direction dir = NORTH;

void updateSimulator(){

    if(dir == NORTH){
        y++;
    }
    else if(dir == EAST){
        x++;
    }
    else if(dir == SOUTH){
        y--;
    }
    else if(dir == WEST){
        x--;
    }
    
    if(API::wallFront()){
        API::setWall(x,y,dirs[dir]);
    }
    if(API::wallLeft()){
        API::setWall(x,y,dirs[(dir+3)%4]);
    }
    if(API::wallRight()){
        API::setWall(x,y,dirs[(dir+1)%4]);
    }

    std::cerr << " (" << x << ", " << y << ") " << std::endl

}

Direction clockwiseStep(Direction mouseDir){
    return mouseDir;
}

Direction counterClockwiseStep(Direction mouseDir){
    return mouseDir;
}

/*
Cell* getNeighborCells(Coord* pos, int x, int y){
    struct Cell* mousePos = (struct Cell*)malloc(4 * sizeOf(struct Cell));
    pos->x++;
    pos->y++;
    return pos;
} */ //Ask chatgpt to write me an array that returns an array pointer of size 4

/*Cell getBestCell(struct Cell){
    return Cell;
}*/

void rotate(){
    if (!API::wallRight()) {
        dir = (Direction)((dir + 1)%4);
        API::turnRight();
    }
    while (API::wallFront()) {
        dir = (Direction)((dir + 3)%4);
        API::turnLeft();
    }
}

/*void move(Coord* pos){
    pos->x++;
}*/

void move(){
    API::moveForward();
}

void setGoalCell(){
    
}

int main(int argc, char* argv[]) {
    
    log("Running...");
    API::setColor(0, 0, 'G');
    API::setText(0, 0, "abc");
    
    Coord posMouse = (Coord){0,0};
    std::cerr << posMouse.x << std::endl;
    //move(&posMouse);
    std::cerr << posMouse.x << std::endl;
    
    
    while (true) {
        rotate();
        move();
        updateSimulator();
    }
}
