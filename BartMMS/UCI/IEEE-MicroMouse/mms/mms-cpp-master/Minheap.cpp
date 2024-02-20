#include <iostream>

typedef struct Heap heap;
struct Heap {
    int *arr;   //dynamic array
    int size;   //number of elems in heap
    int capacity; //max elems possible
};

//arr[0] is root element, minimum value of entire tree after heapify is run

/*
    sources: https://www.geeksforgeeks.org/c-program-to-implement-min-heap/, 
    https://www.geeksforgeeks.org/introduction-to-heap-data-structure-and-algorithm-tutorials/,
    and https://www.digitalocean.com/community/tutorials/min-heap-binary-tree
*/


heap* makeHeap(int max_elems, int* nums) {
    
    heap* h = (heap*)malloc(sizeof(heap));
    if (h == NULL) {
        printf("error");
        return NULL;
    }

    h->arr = (int*)malloc(max_elems * sizeof(int));
    h->size = 0;
    h->capacity = max_elems;
    if (h->arr == NULL) {
        printf("error");
        return NULL;
    }

    int i;
    for (i = 0; i < capacity; i++) {
        arr[i] = nums[i];
    }
    h->size = i;
    i = (h->size -2)/2;

    while (i >= 0) {
        heapify(h, i);
        i--;
    }

    return h;
}

void free_heap(heap* h) {
    free(h->arr);
    free(h);
}

int parent(heap* h, int i) {
    if (i == 0) {
        printf("root elem");
        return h->arr[i];
    }
    return h->arr[i-1/2];
}

int l_child(heap* h, int i) {
    if(i > h->capacity-2) {
        printf("leaf");
        return h->arr[i];
    }
    return h->arr[2*i + 1];
}

int r_child(heap* h, int i) {
    if(i > h->capacity-3) {
        printf("leaf");
        return h->arr[i];
    }
    return h->arr[2*i + 2];
}

//inserts new value and re-sorts the heap
void insert(heap* h, int elem) {
    if (h->size >= capacity) {
        printf("Heap is full");
    }
    else {
        heap->arr[heap->size - 1] =  elem;
        h->size++;



    }

}

//extracts min value and re-sorts the heap
int extract(heap* h) {
    int min;
    if (h->size == 0) {
        printf("Empty heap");
        return NULL;
    }
    
    min = h->arr[0];
    h->arr[0] = h->arr[h->size-1];
    h->size--;

    heapify(h,0);
    return min;
}

void heapify(heap* h, int i) {

}