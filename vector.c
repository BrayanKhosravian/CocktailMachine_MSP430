#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

void init(Vector *vector){
    vector->size = 0;
    vector->capacity = VECTOR_INITIAL_CAPACITY;
    vector->data = (int*)malloc(sizeof(int) * vector->capacity);
}

int get(Vector *vector, int index){
    return vector->data[index];
}

void set(Vector *vector, int index, int value){
    while(index >= vector->size){
        append(vector, 0);
    }

    vector->data[index] = value;
}

void append(Vector *vector, int value){
    resize(vector);

    vector->data[vector->size++] = value;
}

void prepend(Vector *vector, int value){
    set(vector, 0, value);
    vector->size++;
}

int pop(Vector *vector){
    int data = vector->data[vector->size - 2];
    set(vector, vector->size - 1, 0);
    vector->size = vector->size - 1;
    return data;
}

void removeAt(Vector *vector, int index){
    int i;
	for(i = 0; i < index; i++){
        vector->data[index + i] = vector->data[index + i + 1];
    }
    vector->size = vector->size - 1;
}

void delete_value(Vector *vector, int value){
	int i;
    for(i = 0; i < vector->size; i++){
        if(vector->data[i] == value){
        	removeAt(vector, i);
        }
    }
}

int find_value(Vector *vector, int value){
	int i;
	for(i = 0; i < vector->size; i++){
        if(vector->data[i] == value){
            return i;
        }
    }

    return -1;
}

void resize(Vector *vector){
    if(vector->size >= vector->capacity){
        vector->capacity *= 2;
        vector->data = (int*)realloc(vector->data, sizeof(int) * vector->capacity);
    }
}

int size(Vector *vector){
    return vector->size;
}

int capacity(Vector *vector){
    return vector->capacity;
}

bool is_empty(Vector *vector){
    return vector->size == 0;
}

void clear(Vector *vector){
    free(vector->data);
}
