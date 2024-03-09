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

heap* makeHeap(int max_elems) { //originally had int* nums, but we'll init it all to 0
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
    for (i = 0; i < h->capacity; i++) {
        h->arr[i] = 0; //originally to nums[i] but we'll init it all to 0
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

int parent(int i) {
    return (i-1)/2;
}

int l_child(int i) {
    return 2*i + 1;
}

int r_child(int i) {
    return 2*i + 2;
}

//inserts new value and re-sorts the heap
void insert(heap* h, int elem) {
    if (h->size < h->capacity) {
        h->arr[h->size - 1] =  elem;
        int i = h->size;
        while (i > 0 && h->arr[parent(i)] > h->arr[i]) {
            int temp = h->arr[parent(i)];
            h->arr[parent(i)] = h->arr[i];
            h->arr[i] = temp;
            i = parent(i);          
        }
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

heap* heapify(heap* h, int i) {
    if (h->size == 0) {
        printf("Empty heap");
        return h;
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

    if (left < h->size && h->arr[left] < h->arr[i]) {
        min = left;
    }

    if (right < h->size && h->arr[right] < h->arr[i]) {
        min = right;
    }

    if (min != i) {
        int temp = h->arr[min];
        h->arr[min] = h->arr[i];
        h->arr[i] = temp;
        heapify(h, min);
    }
}
