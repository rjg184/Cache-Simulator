#ifndef _first_h
#define _first_h

// Item structure
typedef struct Item {

	int tag; //tag to be stored

} Item;

// Queue structure
typedef struct Queue {

	int front; //front -> where to be deleted (default value is 0)
	int rear; //end -> where to be inserted (default value is -1)
	int size; //current size (default value is 0)
	int capacity; //max capacity (default value is 0)
	Item* items; //structures that store the tags

} Queue;

int pow2(int num, int power); //checks if an integer is a power of 2 and also calculates that value

Queue* createArray(int assoc, int sets, int index); //creates an array of Queue structures

Queue* assignValues(Queue* queue, int assoc); //assigns values to the allocated Queue structures

char* getValue(char* binary, char* src, int x, int y); //gets string value for Tag B and Index A

void cacheA_RW(char operation, Queue* Queue, Item* Storage, int tag); //performs the reads and writes for Cache A and increments appropriate counters

void cacheB_RW(char operation, Queue* Queue, Item* Storage, int tag); //performs the reads and writes for Cache B and increments appropriate counters

bool search(Queue* queue, int index, int tag); //searches for tag in the Queue

void enqueue(Queue* queue, Item* item); //adds item to the rear of the Queue

void dequeue(Queue* queue); //removes item from the front of the Queue

int reset(Queue* queue); //resets rear to zero if needed

char* hexToBinary(char* addr); //adds appropriate zero-entries and then sends the hexadecimal address to the helper

char* hexToBinaryHelper(char* binary, char* addr, int index, int length); //converts hex address into binary & then appends it to the created zero-entries

void printData(char cacheName, int reads, int writes, int hits, int misses); //prints the output for the appropriate cache

#endif
