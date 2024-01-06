#include <iostream>
#include <string>

#include "API.h"

void log(const std::string& text) 
{
    std::cerr << text << std::endl;
}

enum Direction {
    NORTH = 0,
    EAST = 1,
    SOUTH = 2,
    WEST = 3
};

enum DirectionBitmask {
    NORTH_MASK = 0b1000,
    EAST_MASK  = 0b0100,
    SOUTH_MASK = 0b0010,
    WEST_MASK  = 0b0001
};

struct Coord {
    int x;
    int y;
};

struct Cell {
    Coord pos;
    Direction dir;
    bool blocked;
};

struct CellList {
    int size;
    Cell* cells;
};

struct Maze {
    Coord mouse_pos;
    Direction mouse_dir;

    int distances[16][16];
    int cellWalls[16][16];
    bool visited[16][16];

    //Coord* goalPos;
};

char dir_chars[4] = {'n', 'e', 's', 'w'};
int dir_mask[4] = {0b1000, 0b0100, 0b0010, 0b0001};

// 2. FILL THIS IN
void updateSimulator(Maze maze) // redraws the maze in simulator after each loop in main
{
    for(int y = 0; y < 16; y++){
        for(int x = 0; x < 16; x++){
            API::setText(x,y, std::to_string(maze.distances[y][x]));
            if(maze.cellWalls[y][x] & NORTH_MASK){
                API::setWall(x, y, 'n');
            }
            if(maze.cellWalls[y][x] & EAST_MASK){
                API::setWall(x, y, 'e');
            }
            if(maze.cellWalls[y][x] & SOUTH_MASK){
                API::setWall(x, y, 's');
            }
            if(maze.cellWalls[y][x] & WEST_MASK){
                API::setWall(x, y, 'w');
            }
        }
    }
}

// 5. FILL THIS IN
void scanWalls(Maze* maze)
{
    //TODO: update the walls in the cells next door as well!
        if (API::wallFront())
            maze->cellWalls[maze->mouse_pos.y][maze->mouse_pos.x] |= dir_mask[maze->mouse_dir];
        if (API::wallRight())
            maze->cellWalls[maze->mouse_pos.y][maze->mouse_pos.x] |= dir_mask[(maze->mouse_dir + 1) % 4];
        if (API::wallLeft())
            maze->cellWalls[maze->mouse_pos.y][maze->mouse_pos.x] |= dir_mask[(maze->mouse_dir + 3) % 4];
}

Coord findNeighbor(Maze& maze, Direction direction)
{
    switch (direction)
    {
        case NORTH: return (Coord){maze.mouse_pos.x  , maze.mouse_pos.y+1};
        case EAST:  return (Coord){maze.mouse_pos.x+1, maze.mouse_pos.y  };
        case SOUTH: return (Coord){maze.mouse_pos.x  , maze.mouse_pos.y-1};
        case WEST:  return (Coord){maze.mouse_pos.x-1, maze.mouse_pos.y  };
    }
}

void addWall(Maze& maze, Direction direction)
{
    maze.cellWalls[maze.mouse_pos.y][maze.mouse_pos.x] |= dir_mask[direction];
    Coord neighbor = findNeighbor(maze, direction);
    if (neighbor.x>=0 && neighbor.x<16 && neighbor.y>=0 && neighbor.y<16)
        maze.cellWalls[neighbor.y][neighbor.x] |= dir_mask[(direction + 2) % 4];
}

void scanWalls2(Maze& maze)
{
    if (API::wallFront())
        addWall(maze, maze.mouse_dir);
    if (API::wallRight())
        addWall(maze, (Direction)((maze.mouse_dir + 1) % 4));
    if (API::wallLeft())
        addWall(maze, (Direction)((maze.mouse_dir + 3) % 4));
}

// 3. FILL THIS IN
void updateMousePos(Coord* pos, Direction dir)
{
        if (dir == NORTH)
            pos->y++;
        if (dir == SOUTH)
            pos->y--;
        if (dir == WEST)
            pos->x--;
        if (dir == EAST)
            pos->x++;
    // std::cerr << "inside function: (" << pos.x << ", " << pos.y << ")" << std::endl;
}

void setGoalCell(Coord* goalPos, int x, int y){
    //Top right Quadrant
    if((x >= 8) && (y >= 8)){
        goalPos->x = 8;
        goalPos->y = 8;
    }
    
    //Bottom right Quadrant
    if((x >= 8) && (y <= 7)){
        goalPos->x = 8;
        goalPos->y = 7;
    }
    
    //Bottom left Quadrant
    if((x <= 7) && (y <= 7)){
        goalPos->x = 7;
        goalPos->y = 7;
    }
    
    //Top left Quadrant
    if((x <= 7) && (y >= 8)){
        goalPos->x = 7;
        goalPos->y = 8;
    }
}

// 6. FILL THIS IN
CellList* getNeighborCells(Maze* maze, Coord cellPos) /* ADD CellPos, so that getNeighborCells applies to BOTH mouse and the goal cells
    aka do something like
    CellList* getNeighborCells(Maze* maze, Coord cellPos)
    where cellPos is either goalPos <---- floodfill or mouse_pos <--- main
                                        */
{
    int i = 0;
    CellList* cellList = (CellList*)malloc(sizeof(CellList));

    //logic to set the cell list size
    cellList->size = 4;
    cellList->cells = (Cell*)malloc(cellList->size * sizeof(Cell));

    //check if north cell blocked
    //Ignore corner or edge cells by checking if mouse position is at y=15
    if(cellPos.y != 15){
        if(maze->cellWalls[cellPos.y][cellPos.x] & NORTH_MASK){
            //std::cerr << "north blocked" << std::endl;
            cellList->cells[i] = (Cell){(Coord){cellPos.x,(cellPos.y)+1}, NORTH, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.y)+1
        }
        else{
         //PRINT NEIGHBOR CELL COORDINATE THEN CHANGE TO DISTANCE TO GOAL
            cellList->cells[i] = (Cell){(Coord){cellPos.x,(cellPos.y)+1}, NORTH, false};
         }
    }
    else{
        //std::cerr << "north blocked" << std::endl;
        cellList->cells[i] = (Cell){(Coord){cellPos.x,(cellPos.y)+1}, NORTH, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.y)+1
    }
    i = i + 1;
        
    //check if south cell
    if(cellPos.y != 0){
        if(maze->cellWalls[cellPos.y][cellPos.x] & SOUTH_MASK){
            //std::cerr << "south blocked" << std::endl;
            cellList->cells[i] = (Cell){(Coord){cellPos.x,(cellPos.y)-1}, SOUTH, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.y)-1
        }
        else{
         //PRINT NEIGHBOR CELL COORDINATE THEN CHANGE TO DISTANCE TO GOAL
            cellList->cells[i] = (Cell){(Coord){cellPos.x,(cellPos.y)-1}, SOUTH, false};
         }
    }
    else{
        //std::cerr << "south blocked" << std::endl;
        cellList->cells[i] = (Cell){(Coord){cellPos.x,(cellPos.y)-1}, SOUTH, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.y)-1
    }
    i = i + 1;
    
    //check if east cell
    if(cellPos.x != 15){
        if(maze->cellWalls[cellPos.y][cellPos.x] & EAST_MASK){
            //std::cerr << "east blocked" << std::endl;
            cellList->cells[i] = (Cell){(Coord){(cellPos.x)+1,cellPos.y}, EAST, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.x)+1
        }
        else{
         //PRINT NEIGHBOR CELL COORDINATE THEN CHANGE TO DISTANCE TO GOAL
            cellList->cells[i] = (Cell){(Coord){(cellPos.x)+1,cellPos.y}, EAST, false};
         }
    }
    else{
        //std::cerr << "east blocked" << std::endl;
        cellList->cells[i] = (Cell){(Coord){(cellPos.x)+1,cellPos.y}, EAST, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.x)+1
    }
    i = i + 1;
    
    //check if west cell
    if(cellPos.x != 0){
        if(maze->cellWalls[cellPos.y][cellPos.x] & WEST_MASK){
            //std::cerr << "west blocked" << std::endl;
            cellList->cells[i] = (Cell){(Coord){(cellPos.x)-1,cellPos.y}, WEST, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.x)-1
        }
        else{
         //PRINT NEIGHBOR CELL COORDINATE THEN CHANGE TO DISTANCE TO GOAL
            cellList->cells[i] = (Cell){(Coord){(cellPos.x)-1,cellPos.y}, WEST, false};
         }
    }
    else{
        //std::cerr << "west blocked" << std::endl;
        cellList->cells[i] = (Cell){(Coord){(cellPos.x)-1,cellPos.y}, WEST, true}; //COPY FREE POINT STD::CERR STATEMENT (maze->mouse_pos.x)-1
    }
    i = i + 1;
    
    //std::cerr << i << std::endl;
    return cellList;
};

#if 1
typedef struct _queueItem
{
    Coord pos;
    _queueItem* next;
    _queueItem* prev;
} queueItem;

typedef struct
{
    queueItem* head;
    queueItem* tail;
    int size;
} Queue;

void QueueInit(Queue& queue)
{
    queue.head = NULL;
    queue.tail = NULL;
    queue.size = 0;
}

void QueuePush(Queue& queue, Coord pos)
{
    queueItem* newItem = (queueItem*)malloc(sizeof(queueItem));
    newItem->next = NULL;
    newItem->prev = queue.tail;
    newItem->pos = pos;
    
    if (queue.tail != NULL)
        queue.tail->next = newItem;
    queue.tail = newItem;
    if (queue.head == NULL)
        queue.head = newItem;
    queue.size += 1;
}

Coord QueuePop(Queue& queue)
{
    Coord pos = queue.head->pos;
    queueItem* lastItem = queue.head;
    queue.head = queue.head->next;
    if (queue.head == NULL)
        queue.tail = NULL;
    free(lastItem);
    queue.size -= 1;

    return pos;
}

bool QueueEmpty(Queue& queue)
{
    return ((queue.head == NULL) && (queue.tail == NULL));
}

//stack implementation

typedef struct _stackItem
{
    Coord pos;
    _stackItem* prev;
} stackItem;

typedef struct
{
    stackItem* top;
    int size;
} Stack;

void stackInit(Stack& stack)
{
    stack.top = NULL;
    stack.size = 0;
}

void stackPush(Stack& stack, Coord pos)
{
    stackItem* newItem = (stackItem*)malloc(sizeof(stackItem));
    newItem->pos = pos;
    newItem->prev = stack.top;
    stack.top = newItem;
    stack.size += 1;
}

Coord stackPop(Stack&stack)
{
    Coord pos = stack.top->pos;
    stackItem* oldItem = stack.top;
    stack.top = stack.top->prev;
    stack.size -= 1;
    free(oldItem);
    return pos;
}

bool stackEmpty(Stack& stack)
{
    return stack.top == NULL;
}
#endif

void floodfill(Maze& maze, Coord goalPos){
    
    Coord queue[256];
    int head = 0;
    int tail = 0;
    int currentDis;
    CellList* neighborCells;
    
    /*  Set up  */
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 16; y++){
            maze.distances[y][x] = 255;
        }
    }
    
    for(int x = 7; x <= 8; x++){
        for(int y = 7; y <= 8; y++){
            setGoalCell(&goalPos, x, y);
            maze.distances[y][x] = 0;
    /*   Set up   */
            
            queue[tail] = goalPos;
            tail++;
        }
    }
    
    while(tail - head > 0){
//        fprintf(stderr, "queue[%d] = (%d,%d)\n", head, queue[head].x, queue[head].y);
        goalPos = queue[head];
        head++;
        currentDis = maze.distances[goalPos.y][goalPos.x];

        /* CHANGE getNeighborCells!!! Look at function below */
        neighborCells = getNeighborCells(&maze, goalPos);
        for(int j = 0; j <= 3; j++){
            if(neighborCells->cells[j].blocked == false)
            {
                if(maze.distances[neighborCells->cells[j].pos.y][neighborCells->cells[j].pos.x] == 255) //> currentDis + 1)
                {
                    if (goalPos.x < 5 && goalPos.y < 5)
                        fprintf(stderr, "maze.distance[%d, %d] = %d (%d, %d = %d)\n", neighborCells->cells[j].pos.x, neighborCells->cells[j].pos.y, currentDis + 1,
                            goalPos.x, goalPos.y, currentDis);
                    maze.distances[neighborCells->cells[j].pos.y][neighborCells->cells[j].pos.x] = currentDis + 1;
                    queue[tail] = neighborCells->cells[j].pos;
                    tail++;
                }
            }
            else
                if (goalPos.x < 5 && goalPos.y < 5)
                    fprintf(stderr, "current(%d, %d) - neightboxCells[%d] = BLOCKED\n", goalPos.x, goalPos.y, j);
        }
    }
}

Cell getBestCell(Maze maze, Coord goalPos, CellList* adjacentCells){
    Cell trueCandidate;
    CellList bestCellCandidates;
    int currentDistance;
    int trueDistance;
    int rivalDistance;
    int counter = 0;
    
    
    //CHECK IF WALL BLOCKED: Do this by checking if Cell.blocked == true for each cell in cell list
    for(int j = 0; j <= 3; j++){
        if(adjacentCells->cells[j].blocked == false){
            bestCellCandidates.cells[counter] = adjacentCells->cells[j];
            counter++;
        }
    }
   
    trueCandidate = bestCellCandidates.cells[0];
    //After finding out which cells are not blocked, check and compare the manhattan distances of each cell
    
    for(int i = 0; i < counter; i++){
        std::cerr << "best cell candidates: " << bestCellCandidates.cells[i].pos.x << ", " << bestCellCandidates.cells[i].pos.y << std::endl;
    }
    
    if(counter > 1){
        setGoalCell(&goalPos, maze.mouse_pos.x, maze.mouse_pos.y);
        currentDistance = abs(goalPos.x-maze.mouse_pos.x)+abs(goalPos.y-maze.mouse_pos.y);
        
        for(int i = 1; i < counter; i++){
            setGoalCell(&goalPos, trueCandidate.pos.x, trueCandidate.pos.y);
            trueDistance = abs(goalPos.x-trueCandidate.pos.x)+abs(goalPos.y-trueCandidate.pos.y);
            
            setGoalCell(&goalPos,bestCellCandidates.cells[i].pos.x,bestCellCandidates.cells[i].pos.y);
            rivalDistance = abs(goalPos.x-bestCellCandidates.cells[i].pos.x)+abs(goalPos.y-bestCellCandidates.cells[i].pos.y);
            
            if(rivalDistance < trueDistance){
                trueCandidate = bestCellCandidates.cells[i];
            }
        }
        
        if(currentDistance < abs(goalPos.x-trueCandidate.pos.x)+abs(goalPos.y-trueCandidate.pos.y)){

            floodfill(maze, goalPos);
            
        }
    }
    
    //best cell is determined by lowest manhattan distance to goal
    std::cerr << "TRUE BEST CELL: " << trueCandidate.pos.x << ", " << trueCandidate.pos.y << std::endl;
    
    return trueCandidate;
};

void SwitchDirection(Maze& maze, Direction direction)
{
    if (maze.mouse_dir == (Direction)((direction + 1) % 4))
        API::turnLeft();
    else if (maze.mouse_dir == (Direction)((direction + 3) % 4))
        API::turnRight();
    else if (maze.mouse_dir != direction)
    {
        API::turnRight();
        API::turnRight();
    }
    maze.mouse_dir = direction;
}

void MoveMouse(Maze& maze, Coord pos)
{
    if (pos.x > maze.mouse_pos.x)
        SwitchDirection(maze, EAST);
    else if (pos.x < maze.mouse_pos.x)
        SwitchDirection(maze, WEST);
    else if (pos.y > maze.mouse_pos.y)
        SwitchDirection(maze, NORTH);
    else if (pos.y < maze.mouse_pos.y)
        SwitchDirection(maze, SOUTH);
    
    API::moveForward();
    maze.mouse_pos.x = pos.x;
    maze.mouse_pos.y = pos.y;
}

int main(int argc, char* argv[])
{
    Maze maze;
    Coord goalPos;

    //This section declares and initializes everything
    //----------------------------------------------------//
    int i;
    
    maze.mouse_pos.x = 0;
    maze.mouse_pos.y = 0;
    
    maze.mouse_dir = NORTH;
    
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 16; y++){
            setGoalCell(&goalPos, x, y);
            maze.distances[y][x] = abs(goalPos.x-x) + abs(goalPos.y-y);
            maze.cellWalls[y][x] = 0;
            maze.visited[y][x] = false;
        }
    }
    
    //initialize the outer maze walls
    for (int x=0; x<16; x++) {
        maze.cellWalls[ 0][x] |= SOUTH_MASK;
        maze.cellWalls[15][x] |= NORTH_MASK;
    }
    for (int y=0; y<16; y++) {
        maze.cellWalls[y][ 0] |= WEST_MASK;
        maze.cellWalls[y][15] |= EAST_MASK;
    }
    
    //----------------------------------------------------//

#if 1
    Stack stack;
    stackInit(stack);
    
    stackPush(stack, (Coord){0, 1});
    
    while (!stackEmpty(stack))
    {
        Coord pos = stackPop(stack);
        
        //move mouse to new position
        MoveMouse(maze, pos);
        
        scanWalls2(maze);
        
        maze.visited[pos.y][pos.x] = true;
        
        //update floodfill if new walls found
        Coord goalPos;
        setGoalCell(&goalPos, maze.mouse_pos.x, maze.mouse_pos.y);
        floodfill(maze, goalPos);
        updateSimulator(maze);
        
        for (int n=0; n<4; n++)
        {
            //NOTE: Could skip the direction we came from as it is already visited
            Coord neighbor;
            int mask = 0;
            switch (n)
            {
                case 0: neighbor.x = pos.x-1; neighbor.y = pos.y;   mask = WEST_MASK;  break;
                case 1: neighbor.x = pos.x+1; neighbor.y = pos.y;   mask = EAST_MASK;  break;
                case 2: neighbor.x = pos.x;   neighbor.y = pos.y-1; mask = SOUTH_MASK; break;
                case 3: neighbor.x = pos.x;   neighbor.y = pos.y+1; mask = NORTH_MASK; break;
            }
            if ((!(maze.cellWalls[pos.y][pos.x] & mask)) &&
                !maze.visited[neighbor.y][neighbor.x] &&
                (maze.distances[neighbor.y][neighbor.x] < maze.distances[pos.y][pos.x]))
            {
                stackPush(stack, pos);
                stackPush(stack, neighbor);
                maze.visited[neighbor.y][neighbor.x] = true;
            }
        }
    }
    fprintf(stderr, "STACK EMPTY - DONE\n");
    exit(1);
#endif
    
    while (true) {
        scanWalls(&maze);
        CellList* adjacentCells = getNeighborCells(&maze, maze.mouse_pos);
        
        //std::cerr << i << std::endl;
        for(int j = 0; j <= 3; j++){
            std::cerr << adjacentCells->cells[j].pos.x << ", " << adjacentCells->cells[j].pos.y << ", " << adjacentCells->cells[j].dir << ", " << adjacentCells->cells[j].blocked << std::endl;
        }
        
        setGoalCell(&goalPos, maze.mouse_pos.x, maze.mouse_pos.y);
        floodfill(maze, goalPos);
        
        Cell trueCandidate = getBestCell(maze, goalPos, adjacentCells);
        updateSimulator(maze);
        
        std::cerr << "CURRENT CELL: " << "(" << maze.mouse_pos.x << ", " << maze.mouse_pos.y << ")" << std::endl;
        
        free(adjacentCells->cells);
        free(adjacentCells);
        
        // Left Wall Follow Code
        //CHANGE MOVEMENT CODE SO THAT IT MOVES TO THE BEST CELL IT CAN MOVE TO
        
        //while(maze.mouse_dir != trueCandidate.dir
        //  API::turnRight();
        //  maze.mouse_dir = (Direction)((maze.mouse_dir + 1) % 4);
        //API::moveForward();
        
        //Make it so that if mouse is not facing the trueCandidate then make it turn right until it does (optimally, it should turn which every way is shorter). Afterwards, it moves forward and repeats this process.
        
        /*if (!API::wallLeft())
        {
            API::turnLeft();
            maze.mouse_dir = (Direction)((maze.mouse_dir + 3) % 4);
        }
        while (API::wallFront()) 
        {
            API::turnRight();
            maze.mouse_dir = (Direction)((maze.mouse_dir + 1) % 4);
        }

        API::moveForward();*/
        
        while(maze.mouse_dir != trueCandidate.dir){
            API::turnRight();
            maze.mouse_dir = (Direction)((maze.mouse_dir + 1) % 4);
        }
        
        API::moveForward();
    
        // 3. UPDATE THIS WITH POINTERS updateMousePos
        updateMousePos(&maze.mouse_pos, maze.mouse_dir);

        // 5. MOVE TO updateSimulator() + scanWalls() FUNCTION
    
    }
}
