#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "first.h"

// Global Variables
int hitsA = 0, missesA = 0, readsA = 0, writesA = 0; //data for Cache A
int hitsB = 0, missesB = 0, readsB = 0, writesB = 0; //data for Cache B

int main(int argc, char **argv) {

	if(argc != 5){ //few or too many number of arguments
		printf("\tInvalid input or number of arguments.\n \tCorrect Format: <cache size> <associativity> <block size> <trace file>\n");
		return 0;
	}
	
	// Arguments
	int cache_size = atoi(argv[1]);
	char* associativity = argv[2];
	int block_size = atoi(argv[3]);
	FILE* tracefile = fopen(argv[4], "r");
	
	// Argument 1 - Check Cache Size
	if(pow2(cache_size, 0) == -1){
		printf("\tError: Cache size is invalid (empty or not a power of 2)\n");
		return 0;
	}
	
	// Argument 3 - Check Block Size
	if(pow2(block_size, 0) == -1 || block_size > cache_size){
		printf("\tError: Block size is invalid (empty, not a power of 2, or larger than cache)\n");
		return 0;
	}
	
	// Argument 2 - Check Associativity
	int sets;
	int assoc;
	
	if(strcmp(associativity, "direct") == 0){
		sets = (cache_size / block_size);
		assoc = 1;
	} else if(strcmp(associativity, "assoc") == 0){
		sets = 1;
		assoc = (cache_size / block_size);
	} else if(strncmp(associativity, "assoc:", 6) == 0){
		char* temp = strchr(associativity, ':');
		assoc = atoi(temp + 1);
		
		if(pow2(assoc, 0) == -1){
			printf("\tError: Associativity is invalid (empty or not a power of 2)\n");
			return 0;
		}
		
		sets = (cache_size / (block_size * assoc));
		
	} else {
		printf("\tAssociativity is invalid (not direct, assoc, or assoc:n)\n");
		return 0;
	}
	
	//printf("set: %d -- assoc: %d\n", sets, assoc);
	
	int blockOffset = log(block_size) / log(2);
	int indexBits = log(sets) / log(2);
	int y = indexBits + 1;
	int tagBits = 48 - indexBits - blockOffset;
	int x = tagBits + 1;
	
	Queue* arrA = createArray(assoc, sets, 0);
	Queue* arrB = createArray(assoc, sets, 0);
	
	//printf("block offset: %i and index bits: %i\n", blockOffset, indexBits);
	//printf("set size: %d\n", sets);
	//printf("tag bits: %i\n", tagBits);
		
	// Argument 4 - Check Trace Files
	if(tracefile != NULL){
	
		char addr[30];
		char operation[2];
		char check[30];
		char* buffer;
		
		while(fscanf(tracefile, "%s %s %s", check, operation, addr) != EOF && strcmp(check, "#eof") != 0){
		
			char* binary = hexToBinary(addr); //hexadecimal address in binary
			int i; //index
			
			// Tag A -- String -> Decimal
			char* tag_A = calloc(x, sizeof(char));
			for(i = 0; i < tagBits; i++){
				char a[2];
				a[0] = binary[i];
				strcat(tag_A, a);
			}
			int tagA = (int)strtol(tag_A, &buffer, 2);
			Item* storageA = malloc(sizeof(Item));
			storageA->tag = tagA; //store tag A
			free(tag_A); //free string tag A
			
			// Tag B -- String -> Decimal
			char* tag_B = calloc(x, sizeof(char));
			tag_B = getValue(binary, tag_B, indexBits, tagBits);
			int tagB = (int)strtol(tag_B, &buffer, 2);
			Item* storageB = malloc(sizeof(Item));
			storageB->tag = tagB; //store tagB
			free(tag_B); //free string tag B
			
			// Index A -- String -> Decimal
			char* index_A = calloc(y, sizeof(char));
			index_A = getValue(binary, index_A, tagBits, indexBits);
			int indexA = (int)strtol(index_A, &buffer, 2);
			Queue* QueueA = &arrA[indexA];
			free(index_A); //free string index A
			
			// Index B -- String -> Decimal
			char* index_B = calloc(y, sizeof(char));
			for(i = 0; i < indexBits; i++){
				char b[2];
				b[0] = binary[i];
				strcat(index_B, b);
			}
			int indexB = (int)strtol(index_B, &buffer, 2);
			Queue* QueueB = &arrB[indexB];
			free(index_B); //free string index B
			free(binary); //frees binary string
			
			// Cache A -- R/W + Hits/Misses
			cacheA_RW(operation[0], QueueA, storageA, tagA);
			free(storageA);
				
			// Cache B -- R/W + Hits/Misses
			cacheB_RW(operation[0], QueueB, storageB, tagB);
			free(storageB);
			
		}
		
		printData('A', readsA, writesA, hitsA, missesA);
		printData('B', readsB, writesB, hitsB, missesB);
	
	} else {
		printf("\tError: Trace file is invalid (empty or incorrect)\n");
		return 0;
	}
	
	fclose(tracefile);
	return 0;
}

int pow2(int num, int power) {

	if(num < 1){
		return -1;
	} else if(num == 1){
		return power;
	} else {
		while(num != 1){
			if((num % 2) != 0){
				return -1; //not even
			} else {
				num /= 2;
				power++;
			}
		}
	}
	
	return power; //num = 2^(power)
}

char* hexToBinary(char* addr) {

	char delete = addr[2];
	char* hex = strchr(addr, delete); //hex value without '0x'
	int i;
	
	char* binary = calloc(49, sizeof(char));
	
	for(i = 0; i < (48 - (strlen(hex) * 4)); i++){
		strcat(binary, "0"); //adding 0's before binary conversion
	}
	
	binary = hexToBinaryHelper(binary, hex, 0, strlen(hex)); //converts hex to binary
	
	return binary;
}

char* hexToBinaryHelper(char* binary, char* addr, int index, int length) {
	
	while(index < length){
	
		switch(addr[index]){
		
			case '0': strcat(binary, "0000"); break;
			case '1': strcat(binary, "0001"); break;
			case '2': strcat(binary, "0010"); break;
			case '3': strcat(binary, "0011"); break;
			case '4': strcat(binary, "0100"); break;
			case '5': strcat(binary, "0101"); break;
			case '6': strcat(binary, "0110"); break;
			case '7': strcat(binary, "0111"); break;
			case '8': strcat(binary, "1000"); break;
			case '9': strcat(binary, "1001"); break;
			case 'a': strcat(binary, "1010"); break;
			case 'b': strcat(binary, "1011"); break;
			case 'c': strcat(binary, "1100"); break;
			case 'd': strcat(binary, "1101"); break;
			case 'e': strcat(binary, "1110"); break;
			case 'f': strcat(binary, "1111"); break;
				
		}
	
		index++;
		
		if(index == length){
			strcat(binary, "\0"); //adds string terminator at the end
		}
		
	}

	return binary;
}

Queue* createArray(int assoc, int sets, int index) {

	Queue* arr = calloc(sets, sizeof(Queue));
	
	while(index < sets){
		Queue* queue = malloc(sizeof(Queue));
		queue = assignValues(queue, assoc);
		arr[index] = *queue;
		index++;
	}
	
	return arr;
}

Queue* assignValues(Queue* queue, int assoc) {

	queue->front = 0;
	queue->rear = -1;
	queue->size = 0;
	queue->capacity = assoc;
	queue->items = malloc(sizeof(Item) * assoc);
	
	return queue;
}

char* getValue(char* binary, char* src, int x, int y) {

	strncpy(src, (binary + x), y);
	
	return src;
}

void cacheA_RW(char operation, Queue* Queue, Item* Storage, int tag) {

	if(search(Queue, 0, tag) == false){ //miss
		missesA++;
		readsA++;
		if(operation == 'W'){ writesA++; } //write
		enqueue(Queue, Storage);
	} else { //hit
		hitsA++;
		if(operation == 'W'){ writesA++; } //write
	}
}

void cacheB_RW(char operation, Queue* Queue, Item* Storage, int tag) {

	if(search(Queue, 0, tag) == false){ //miss
		missesB++;
		readsB++;
		if(operation == 'W'){ writesB++; } //write
		enqueue(Queue, Storage);
	} else { //hit
		hitsB++;
		if(operation == 'W'){ writesB++; } //write
	}
}

bool search(Queue* queue, int index, int tag) {

	if(queue->size == 0){ //empty -> not found
		return false;
	} else {
		while(index < queue->size){
			Item* ptr = queue->items;
			int data = ptr[index].tag;
			if(data == tag){ //found
				return true;
			}
			index++;
		}
	}
	
	return false; //not found
}

void enqueue(Queue* queue, Item* item) {

	if(queue->size == queue->capacity){
		dequeue(queue); //dequeue because Queue is currently full
	}

	queue->rear++;
	queue->rear = reset(queue);
	queue->items[queue->rear] = *item;
	queue->size++;
}

void dequeue(Queue* queue) {

	if(queue->size == 0){
		printf("Error: Cannot dequeue an empty Queue");
		return;
	}
	
	queue->front++;
	queue->size--;
	
	if(queue->size == 0){
		queue->front = 0;
		queue->rear = -1;
	}
}

int reset(Queue* queue) {
	
	if((queue->capacity - queue->rear) == 0){ return 0; }
	
	return queue->rear;
}

void printData(char cacheName, int reads, int writes, int hits, int misses) {

	printf("cache %c\n", cacheName); //cache A or B
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hits);
	printf("Cache misses: %d\n", misses);
}
