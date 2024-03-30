#include <iostream>
#include "API.h"
//#include "CW_Maze.cpp"

//arr[0] is root element, minimum value of entire tree after Heapify is run

/*
    sources: https://www.geeksforgeeks.org/c-program-to-implement-min-Heap/, 
    https://www.geeksforgeeks.org/introduction-to-Heap-data-structure-and-algorithm-tutorials/,
    and https://www.digitalocean.com/community/tutorials/min-Heap-binary-tree
*/


Heap* makeHeap(int max_elems, Maze *maze) {
    Heap* h = (Heap*)malloc(sizeof(Heap));
    if (h == NULL) {
        printf("error");
        return NULL;
    }

    h->arr = (Node*)malloc(max_elems * sizeof(int));
    h->size = 0;
    h->capacity = max_elems;
    if (h->arr == NULL) {
        printf("error");
        return NULL;
    }

    return h;
}

void free_heap(Heap* h) { //free Heap memory
    free(h->arr);
    free(h);
}

int parent(int i) { //parent position
    return (i-1)/2;
}

int l_child(int i) { //left child position
    return 2*i + 1;
}

int r_child(int i) { //right child position
    return 2*i + 2;
}

//inserts new value and re-sorts the Heap
void heap_insert(Heap* h, Node elem) {
    if (h->size < h->capacity) {
        h->arr[h->size - 1] = elem;
        int i = h->size;
        while (i > 0 && h->arr[parent(i)].f_score > h->arr[i].f_score) {
            Node temp = h->arr[parent(i)];
            h->arr[parent(i)] = h->arr[i];
            h->arr[i] = temp;
            i = parent(i);          
        }
        h->size++;
    }
}

//Heaps the data
void heapify (Heap* h, int i) {
    if (h->size == 0) {
        printf("Empty Heap");
    }

    int left = l_child(i);
    int right = r_child(i);
    int min = i;

    if (left >= h->size || left < 0) {
        left--;
    }

    if (right >= h->size || right < 0) {
        right--;
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

bool is_empty(Heap* h) {
    return h->size == 0;
}

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
    if (is_empty(h)) {
        printf("Empty Heap"); //needs error catch
    }
    
    min = h->arr[0];
    h->arr[0] = h->arr[h->size-1];
    h->size--;

    heapify(h,0);
    return min;
}
