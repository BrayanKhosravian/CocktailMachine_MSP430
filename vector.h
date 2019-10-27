//#ifndef VECTOR_
//#define VECTOR_

#include <stdbool.h>
#define VECTOR_INITIAL_CAPACITY 16

typedef struct {
    int size;
    int capacity;
    int *data;
} Vector;

void init(Vector *vector);
void append(Vector *vector, int value);
void prepend(Vector *vector, int value);
void removeAt(Vector *vector, int index);
void delete_value(Vector *vector, int value);
void set(Vector *vector, int index, int value);
void resize(Vector *vector);
void clear(Vector *vector);

int get(Vector *vector, int value);
int pop(Vector *vector);
int find_value(Vector *vector, int value);
int size(Vector *vector);
int capacity(Vector *vector);

bool is_empty(Vector *vector);

//#endif /* VECTOR_ */
