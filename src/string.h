#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#define MAX_LEN 4294967295
typedef struct {
	char* data;
	size_t size;
	size_t capacity;
	size_t width;
} string;

enum strError {
	MAXCAPACITY = -1,
	ALLOCERROR = -2	
};

string* constructor(size_t sz);

void destructor(string* s);

void setSize(string* s);

int setSizeImm(string* s, size_t sz);

int append(string* s, char c, size_t symb);

size_t prevNL(string* c, size_t cur);

size_t nextNL(string* c, size_t cur);

void backspace(string* s, size_t symb);

size_t strsize(char* str, size_t buflen);

size_t getCurWidth(string* s, size_t symb);

size_t print_format(string* s, char** buf);
#endif
