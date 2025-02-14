C=gcc
CFLAGS=-Wall -Werror
INCLUDE=-lncursesw

all: editor

editor: src/main.c src/string.c
	$(C) $(CFLAGS) $(INCLUDE) src/*.c -o editor

clean:
	rm -rf *.o editor
