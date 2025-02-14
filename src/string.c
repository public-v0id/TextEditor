#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setWidth(string* s) {
	s->width = 0;
	for (size_t i = 0; i < s->size; ++i) {
		if (s->data[i] == '\t') {
			s->width += 8-(s->width&7);
		}
		s->width++;
	}
}

string* constructor(size_t sz) {
	string* s = malloc(sizeof(string));
	s->data = calloc(sz, sizeof(char));
	if (s->data == NULL) {
		return NULL;
	}
	s->capacity = sz;
	s->size = 0;
	s->width = 0;
	return s;
}

void destructor(string* s) {
	if (s) {
		free(s->data);
		free(s);
	}
}

void setSize(string* s) {
	if (s->data[s->capacity-1] == 0) {
		s->size = strsize(s->data, s->capacity);
//		s->size = strlen(s->data);
		setWidth(s);
		return;
	}
	s->size = s->capacity;
	setWidth(s);	
}

int setSizeImm(string* s, size_t sz) {
	if (sz > s->capacity) {
		return -1;
	}
	s->size = sz;
	setWidth(s);	
	return 0;
}

int append(string* s, char c, size_t symb) {
	if (s->size >= s->capacity - 1) {	
		if (s->capacity == MAX_LEN) {
			return MAXCAPACITY;
		}
		size_t newCap;
		if (s->capacity >= MAX_LEN/2) {
			newCap = MAX_LEN;
		}
		else {
			newCap = s->capacity << 1;
		}
		char* newData = calloc(newCap, sizeof(char));
		if (newData == NULL) {
			return ALLOCERROR;
		}
		snprintf(newData, s->capacity, "%s", s->data);
		free(s->data);
		s->data = newData;
		s->capacity = newCap;
	}	
	for (size_t i = s->size; i >= symb; i--) {
		s->data[i+1] = s->data[i];
		if (i == 0) {
			break;
		}
	}
	s->data[symb] = c;
	++(s->size);
	if (c == '\t') {
		s->width += 8-(s->width&7);
		return 0;
	}
	s->width++;
	return 0;
}

void backspace(string* s, size_t symb) {
	for (size_t i = symb; i < s->size; ++i) {
		s->data[i-1] = s->data[i];
	}
	s->size--;	
	s->data[s->size] = 0;
}

size_t prevNL(string* c, size_t cur) {
	while (c->data[cur] != '\n' && cur > 0) {
		--cur;
	}
	return cur;
}

size_t nextNL(string* c, size_t cur) {
	while (cur < c->size && c->data[cur] != '\n' && c->data[cur] != 0) {
		++cur;
	}
	if (cur < c->size) {
		++cur;
	}
	return cur;
}

size_t strsize(char* str, size_t buflen) {
	size_t l = 0, r = buflen-1, cur = 0;
	while (l < r-1) {
		cur = (l+r)>>1;
		if (str[cur] > 0) {
			l = cur;
		}
		else {
			r = cur;
		}
	}
	return str[cur] == 0 ? cur : cur+1;
}	

size_t getCurWidth(string* s, size_t symb) {
	size_t w = 0;
	for (size_t i = 0; i < symb; ++i) {
		if (s->data[i] == '\t') {
			w += 8-(w&7);
		}
		w++;
	}
	return w;
}
