#include <stdio.h>
#include <stdlib.h>
#include "List.h"

List::List(){
	init();
}

List::~List(){
	clear();
}

void List::init(){
	_size = 0;
	_capacity = VECTOR_INITIAL_CAPACITY;
	_data = (int*)malloc(sizeof(int) * _capacity);
}

void List::clear(){
	free(_data);
}

void List::append(int value){
    resize();
    _data[_size++] = value;
}

void List::resize(){
    if(_size >= _capacity){
        _capacity *= 2;
        _data = (int*)realloc(_data, sizeof(int) * _capacity);
    }
}

int List::getSize(){
	return _size;
}


