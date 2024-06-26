#include <iostream>
#include <string>
#include <cmath>
#include "API.h"
#include "CW_a_star.cpp"
#include "Maze.h"
//#include "Minheap.cpp"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

//Coordinate Structure inludes x and y for points
char dir_chars[4] = {'n', 'e', 's', 'w'};
int dir_mask[4] = {0b1000, 0b0100, 0b0010, 0b0001};

// implement Queue (queue is used for floodfill)

typedef struct _queueItem   //Define structure _queueItem to be an item in queue
{
    Coord pos;              //item in queue has position coords (x,y)
    _queueItem* next;       //neighboring items in queue can be accessed (next and prev)
    _queueItem* prev;
} queueItem;                //queueItem defined for other items in queue used by other parts of code such as other structures

typedef struct
{
    queueItem* head;
    queueItem* tail;
} Queue;                    //Queue structure looks at head and tail item (first and last items in queue)

void QueueInit(Queue& queue) //Establishes queue head and tail as null (empty queue or queue that has popped a queue item
{
    queue.head = NULL;
    queue.tail = NULL;
}

void QueuePush(Queue& queue, Coord pos) 
    //Creates new item in queue. The next item to be added is then set as NULL
    //The previous item added is set as the tail of the queue
    //The new item's position is set to the main position to be analyzed
{
    queueItem* newItem = (queueItem*)malloc(sizeof(queueItem));
    newItem->next = NULL;
    newItem->prev = queue.tail;
    newItem->pos = pos;
    
    if (queue.tail != NULL)         //if the item being pushed is not the first item pushed then set the value next to the queue tail to new item
        queue.tail->next = newItem;
    queue.tail = newItem;
    if (queue.head == NULL)         //if item pushed is the first item in queue set head to new item
        queue.head = newItem;
}

Coord QueuePop(Queue& queue)            //Removes head item and shifts queue forward
{
    Coord pos = queue.head->pos;        //Position saves current head position (will usually change to next head position)
    queueItem* lastItem = queue.head;   //last item created which takes in head item in queue
    queue.head = queue.head->next;      //moves item next to head into the head position
    if (queue.head == NULL)             //for first item (empty queue)
        queue.tail = NULL;
    free(lastItem);                     //frees head item held in last item position

    return pos;                         //returns new head item in queue
}

bool QueueEmpty(Queue& queue)           //Checks if queue is empty (floodfill)
{
    return ((queue.head == NULL) && (queue.tail == NULL));
}

//implement Stack (stack is used for remembering split road cells for rechecking)

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


void UpdateSimulator(Maze maze) // redraws the maze in simulator after each loop in main. Actually sets walls and text for distances
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

Coord FindNeighborCoord(Coord pos, Direction direction) //Returns neighbor cell based on direction mouse is facing and position the mouse is in
{
    switch (direction)
    {
        case NORTH: return (Coord){pos.x  , pos.y+1};
        case EAST:  return (Coord){pos.x+1, pos.y  };
        case SOUTH: return (Coord){pos.x  , pos.y-1};
        case WEST:  return (Coord){pos.x-1, pos.y  };
    }
}

void AddWall(Maze& maze, Coord pos, Direction direction) //Adds cell wall in pos coord + direction. A cell wall is also added to the neighbor facing in the same direction as the mouse but in the opposite direction
{
    maze.cellWalls[pos.y][pos.x] |= dir_mask[direction];
    Coord neighbor = FindNeighborCoord(pos, direction);
    if (neighbor.x>=0 && neighbor.x<16 && neighbor.y>=0 && neighbor.y<16)
        maze.cellWalls[neighbor.y][neighbor.x] |= dir_mask[(direction + 2) % 4];
}

bool ScanWalls(Maze& maze) //Scans wall around mouse after every iteration and adds walls based on if a wall is scanned in front, to the right, or to the left of the mouse. The function also returns if any walls were found (true/false)
{
    bool found = false;
    
    if (API::wallFront()) {
        AddWall(maze, maze.mouse_pos, maze.mouse_dir);
        found = true;
    }
    if (API::wallRight()) {
        AddWall(maze, maze.mouse_pos, (Direction)((maze.mouse_dir + 1) % 4));
        found = true;
    }
    if (API::wallLeft()) {
        AddWall(maze, maze.mouse_pos, (Direction)((maze.mouse_dir + 3) % 4));
        found = true;
    }
    
    return found;
}

void Floodfill(Maze& maze)
{
    //use a queue to process cells in Breadth First Search (BFS) order
    Queue queue;
    QueueInit(queue);
    
    //initialize distances
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 16; y++){
            maze.distances[y][x] = 999;
        }
    }
    
    //add the 4 goal squares
    for(int x = 7; x <= 8; x++){
        for(int y = 7; y <= 8; y++){
            maze.distances[y][x] = 0;
            QueuePush(queue, (Coord){x,y});
        }
    }
    
    while (!QueueEmpty(queue))
    {
        Coord pos = QueuePop(queue);    //Pops very first coord from the queue for analysis

        for(int j = 0; j < 4; j++)
        {
            //for all 4 directions
            Direction direction = (Direction)j;
            //check if there is no wall in that direction
            if (!(maze.cellWalls[pos.y][pos.x] & dir_mask[direction]))
            {
                //find the neighbor in that direction
                Coord neighbor = FindNeighborCoord(pos, direction);
                //check if neighbor has been visited yet
                if (maze.distances[neighbor.y][neighbor.x] == 999)
                {
                    maze.distances[neighbor.y][neighbor.x] = maze.distances[pos.y][pos.x] + 1; //Makes neighbor value, previous value (like 0) buts adds 1 to it
                    QueuePush(queue, neighbor); //Pushes neighbor cells no interrupted by walls into the queue
                }
            }
        }
    }
}

void SwitchMouseDirection(Maze& maze, Direction direction) //Mouse turns a certain way based on the direction it faces based on the true directions (N,W,S,E) (Relative vs Global Directions)
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

void MoveMouse(Maze& maze, Coord pos) //Mouse either turns or moves forward based on if the position from stack (exploring) is different from the mouse's position
{
    if (pos.x > maze.mouse_pos.x)
        SwitchMouseDirection(maze, EAST);
    else if (pos.x < maze.mouse_pos.x)
        SwitchMouseDirection(maze, WEST);
    else if (pos.y > maze.mouse_pos.y)
        SwitchMouseDirection(maze, NORTH);
    else if (pos.y < maze.mouse_pos.y)
        SwitchMouseDirection(maze, SOUTH);
    
    API::moveForward();
    maze.mouse_pos = pos;
}

int main(int argc, char* argv[]) {
    Maze  maze;
    
    //initialize mouse
    maze.mouse_pos = Coord{0, 0};
    maze.mouse_dir = NORTH;

    //initialize maze
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 16; y++) {
            maze.visited[y][x] = false;
            maze.cellWalls[y][x] = 0;            
        }
    }
    
    //use a stack to do a Depth First Search (DFS)
    Stack stack;
    
    //initializae stack with origin and first step
    StackInit(stack);
    StackPush(stack, (Coord){0, 0});
    StackPush(stack, (Coord){0, 1});
    
    while (!StackEmpty(stack)) {
        //Pops first coord from stack to analyze
        Coord pos = StackPop(stack);
        
        //move mouse to new position
        MoveMouse(maze, pos);
        
        //If there are walls do floodfill and update maze based on new walls found
        if (ScanWalls(maze)) {
            //update floodfill if new walls found
            Floodfill(maze);
            UpdateSimulator(maze);
        }
        
        //check if we have explored the 3 directions (forward, left, right)
        for (int n=0; n<3; n++) {
            Direction direction;
            switch (n) {
                case 0: direction = maze.mouse_dir; break; //forward
                case 1: direction = (Direction)((maze.mouse_dir+1)%4); break; //left
                case 2: direction = (Direction)((maze.mouse_dir+3)%4); break; //right
            }
            //find the neighbor in this direction
            Coord neighbor = FindNeighborCoord(maze.mouse_pos, direction);

            //check if there is no wall && neighbor hasn't been visited yet && neighbor could have shorter distance
            if ((!(maze.cellWalls[pos.y][pos.x] & dir_mask[direction])) && !maze.visited[neighbor.y][neighbor.x]) {
                StackPush(stack, pos); //Puts middle of the road coords (when both coords in a maze display same distance) into stack for analysis
                StackPush(stack, neighbor);
                maze.visited[neighbor.y][neighbor.x] = true;
            }
        }
    }

    maze.mouse_dir = NORTH;
	std::cerr << "start a*" << std::endl; 
	Node* path = a_star_algo(&maze, Coord{7,7});
	std::cerr << "algo done" << std::endl; 
}
