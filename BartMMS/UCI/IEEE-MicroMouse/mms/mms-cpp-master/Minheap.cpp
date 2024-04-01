#include <iostream>
#include "API.h"
#include "Maze.h"
//#include "CW_Maze.cpp"

//arr[0] is root element, minimum value of entire tree after Heapify is run

/*
    sources: https://www.geeksforgeeks.org/c-program-to-implement-min-Heap/, 
    https://www.geeksforgeeks.org/introduction-to-Heap-data-structure-and-algorithm-tutorials/,
    and https://www.digitalocean.com/community/tutorials/min-Heap-binary-tree
*/

//init the heap
Heap* makeHeap(int max_capacity) { 
    Heap* h = (Heap*)malloc(sizeof(Heap));
    if (h == NULL) {
        std::cerr << "makeheap error" << std::endl; 
        return NULL;
    }

    h->size = 0;
    h->capacity = max_capacity;
    h->arr = (Node*)malloc(max_capacity * sizeof(int));

    return h;
}

//free Heap memory
void free_heap(Heap* h) { 
    free(h->arr);
    free(h);
}

//parent position
int parent(int i) { 
    return (i-1)/2;
}

//left child position
int l_child(int i) { 
    return 2*i + 1;
}

 //right child position
int r_child(int i) {
    return 2*i + 2;
}

//Sorts the data
void heapify (Heap* h, int i) {
    if (h->size != 0) {
        int left = l_child(i);
        int right = r_child(i);
        int min = i;

        if (left >= h->size || left < 0) {
            left = -1;
        }

        if (right >= h->size || right < 0) {
            right = -1;
        }

        if (left < h->size && h->arr[left].f_score < h->arr[i].f_score) {
            min = left;
        }

        if (right < h->size && h->arr[right].f_score < h->arr[i].f_score) {
            min = right;
        }

        if (min != i) {
            Node temp = h->arr[min];
            h->arr[min] = h->arr[i];
            h->arr[i] = temp;
            heapify(h, min);
        }
    }
    else {
        std::cerr << "done" << std::endl;
    }
}

//search for elem in heap
int heap_search(Heap* h, Node elem) {
    for (int i =0; i < h->size; i++) {
        if (elem.loc.x == h->arr[i].loc.x && elem.loc.y == h->arr[i].loc.y) {
		    return i;
        }
	}
	return -1;
}

//extracts min value and re-sorts the Heap
Node heap_extract(Heap* h) {
    Node min; 
    if (h->size == 0) {
        std::cerr << "extract failed" << std::endl; 
    }
    
    min = h->arr[0];
    h->arr[0] = h->arr[h->size-1];
    h->size--;
    
    heapify(h,0);
    return min;
}

//inserts new value and re-sorts the Heap
void heap_insert(Heap* h, Node elem) {
    if (h->size < h->capacity) {
        h->size++;

        int i = h->size - 1;
        h->arr[i] = elem;
        while (i > 0 && h->arr[parent(i)].f_score > h->arr[i].f_score) {
            Node temp = h->arr[parent(i)];
            h->arr[parent(i)] = h->arr[i];
            h->arr[i] = temp;
            i = parent(i);          
        }
    }
    //heapify(h,0);
}
