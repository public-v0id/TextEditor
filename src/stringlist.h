#ifndef STRINGLIST_H
#define STRINGLIST_H
#include "string.h"
#include <stdio.h>

typedef struct stringlist{
	string* str;
	struct stringlist* next;
	struct stringlist* prev;
	int color;
} stringlist;

stringlist* listSingleConstructor(size_t sz);

stringlist* listConstructor(FILE* file);

void slDestructor(stringlist* sl);

stringlist* createAfter(stringlist* after, size_t symb);

int writeList(stringlist* list, FILE* f);

void removeSl(stringlist* str);

stringlist* removeAndMergeLines(stringlist* str);

void shuffle(stringlist* list, int n);
#endif
