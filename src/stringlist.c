#include "stringlist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MAX_INT 0x7FFFFFFF

stringlist* listSingleConstructor(size_t sz) {
	stringlist* sl = malloc(sizeof(stringlist));
	if (sl == NULL) {
		return NULL;
	}
	sl->str = constructor(sz);
	if (sl->str == NULL) {
		return NULL;
	}
	sl->prev = NULL;
	sl->next = NULL;
	return sl;
}

stringlist* listConstructor(FILE* file) {
	stringlist* first = NULL;
	stringlist* prev = NULL;
	char* tempbuf = calloc(MAX_INT, sizeof(char));
	while (fgets(tempbuf, MAX_INT, file) != NULL) {
		stringlist* cur = malloc(sizeof(stringlist));
		if (first == NULL) {
			first = cur;
		}
		size_t strsz = strsize(tempbuf, MAX_INT);
		cur->str = constructor(strsz+1);
		snprintf(cur->str->data, strsz+1, "%s", tempbuf);
		cur->prev = prev;
		if (prev != NULL) {
			prev->next = cur;
		}
		cur->next = NULL;
		setSizeImm(cur->str, strsz);
		prev = cur;
		for (int i = 0; i < strsz; ++i) {
			tempbuf[i] = 0;
		}
	}
	free(tempbuf);
	return first;
}

void slDestructor(stringlist* sl) {
	if (sl == NULL) {
		return;
	}
	while (sl != NULL) {
		stringlist* next = sl->next;
		destructor(sl->str);
		free(sl);
		sl = next;
	}
}

stringlist* createAfter(stringlist* after, size_t symb) {
	if (after == NULL) {
		fprintf(stderr, "Error!\n");
		return NULL;
	}
	stringlist* next = after->next;
	stringlist* cur = listSingleConstructor(after->str->size-symb+2);
	snprintf(cur->str->data, after->str->size-symb+2, "%s", after->str->data+symb);
	after->str->data[symb] = '\n';	
	for (size_t i = symb+1; i < after->str->size; ++i) {
		after->str->data[i] = 0;
	}
	setSize(after->str);
	setSize(cur->str);
	after->next = cur;
	cur->prev = after;
	if (next != NULL) {
		cur->next = next;
		next->prev = cur;
	}
	return cur;
}

void removeSl(stringlist* str) {
	if (str->next != NULL) {
		str->next->prev = str->prev;
	}
	if (str->prev != NULL) {
		str->prev->next = str->next;
	}
	destructor(str->str);
	free(str);
}

stringlist* removeAndMergeLines(stringlist* str) {
	if (str->prev == NULL) {
		return NULL;
	}
	stringlist* prev = str->prev;
	string* newStr = constructor(str->str->size+1+prev->str->size);
	snprintf(newStr->data, prev->str->size, "%s", prev->str->data);
	snprintf((newStr->data)+(prev->str->size-1), str->str->size+1, "%s", str->str->data);
	setSizeImm(newStr, prev->str->size+str->str->size-1);
	destructor(prev->str);
	destructor(str->str);
	prev->next = str->next;
	if (str->next != NULL) {
		str->next->prev = prev;
	}
	free(str);
	prev->str = newStr;
	return prev;
}

int writeList(stringlist* list, FILE* f) {
	while (list != NULL) {
		size_t wr = fwrite(list->str->data, sizeof(char), list->str->size, f);
		if (wr != list->str->size) {
			return -1;
		}
		list = list->next;
	}
	return 0;
}
