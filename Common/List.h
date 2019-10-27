
#ifndef LIST_H_
#define LIST_H_

#define VECTOR_INITIAL_CAPACITY 16

class List {
public:
	List();
	virtual ~List();

	void init();
	void clear();

	void append(int value);

	int getSize();

private:
	int _size;
	int _capacity;
	int *_data;

	void resize();

};



#endif
