#include "string.h"
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

size_t pow2(size_t val) {
	size_t res = 1;
	while (res < val) {
		res <<= 1;
	}
	return res;
}

void tryClosing(FILE *f, char* filename) {
	if (fclose(f) == EOF) {
		fprintf(stderr, "Error closing file %s\n", filename);
	}
}

enum inputMode {
	COMMAND,
	TEXT	
};

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Error! No filename provided!\n");
		return 0;	
	}
	for (int i = 1; i < argc; ++i) {
		FILE *f = fopen(argv[i], "a+");
		if (f == NULL) {
			fprintf(stderr, "Unable to open file %s\n", argv[i]);
			continue;
		}
		fseek(f, 0, SEEK_END);
		size_t fSize = ftell(f);
		size_t bufSize = pow2(fSize);
		rewind(f);
		string* buffer = constructor(bufSize);
		if (buffer == NULL) {
			fprintf(stderr, "Unable to allocate memory for opening file %s!\n", argv[i]);
			tryClosing(f, argv[i]);
			continue;
		}	
		size_t dataRead = fread(buffer->data, sizeof(char), fSize, f);
		if (dataRead != fSize) {
			fprintf(stderr, "Error reading file %s!\n", argv[i]);
			destructor(buffer);
			tryClosing(f, argv[i]);
			continue;
		}
		setSize(buffer);
		initscr();
		enum inputMode im = COMMAND;
		bool active = true;
		int row, col, curR = 0, curC = 0;
		size_t curSym = 0;
		while (active) {
			getmaxyx(stdscr, row, col);
			move(0, 0);
			printw(buffer->data);
			refresh();
			switch (im) {
				case COMMAND: {
					echo();	
					move(row-1, 0);
					int ch = getch();	
					switch (ch) {
						case 'q':
							active = false;
							break;
						case 'w':	
							if (rename(argv[i], "oldtext.tmp") != 0) {
								move(row-1, 0);
								printw("Error writing data!");
								break;
							}
							FILE *tmp = fopen(argv[i], "w");
							size_t wr = fwrite(buffer->data, sizeof(char), buffer->size, tmp);
							if (wr != buffer->size) {
								move(row-1, 0);
								printw("Error writing data!");
								remove(argv[i]);
								rename("oldtext.tmp", argv[i]);
								break;
							}
							remove("oldtext.tmp");
							break;
						case 'i':
							im = TEXT;
							break;
					}
					break;
				}
				case TEXT: {
					move(curR, curC);
					keypad(stdscr, TRUE);
					noecho();
					int ch = getch();
					if (ch == KEY_LEFT && curSym > 0) {
						curSym--;
						curC--;
						if (curC < 0) {
							curC += col;
							if (buffer->data[curSym] == '\n') {
								size_t strlen = curSym-prevNL(buffer, curSym-1);
								curC = strlen%col;
							}
							curR--;
						}
						continue;
					}
					if (ch == KEY_RIGHT && curSym < buffer->size && buffer->data[curSym] != 0) {
						curC++;
						if (buffer->data[curSym] == '\n') {
							curC = 0;
							curR++;
						}
						if (curC >= col) {
							curC -= col;
							curR++;
						}
						curSym++;
						continue;
					}
					if (ch == KEY_UP && curSym > 0) {
						size_t nl = prevNL(buffer, curSym-1);
						size_t strlen = curSym-nl;
						curSym = nl;
						if (curR > 0) {
							curR -= (strlen+col-1)/col;
						}
						if (curSym == 0) {
							curC = 0;
							continue;
						}
						nl = prevNL(buffer, curSym-1);
						strlen = curSym-nl-(buffer->data[nl] == '\n' ? 1 : 0);
						curC = strlen%col;
						continue;
					}
					if (ch == KEY_DOWN && curSym < buffer->size && buffer->data[curSym] != 0) {
						size_t nl = nextNL(buffer, curSym+1);
						size_t strlen = nl-curSym;
						curSym = nl;
						curC = 0;
						if (buffer->data[curSym] == 0) {
							size_t pnl = prevNL(buffer, curSym-1);
							curC = (curSym-pnl-1)%col;
						}
						curR += (strlen+col-1)/col;
						continue;
					}
					if (ch == 27) {
						im = COMMAND;
						continue;
					}
					if (ch == 263 && curSym > 0) {
						backspace(buffer, curSym);
						curC--;
						curSym--;
						if (curC < 0) {
							curC += col;
							if (buffer->data[curSym] == '\n') {
								size_t strlen = curSym-prevNL(buffer, curSym-1);
								curC = strlen%col;
							}
							curR--;
						}
						continue;
					}
					if (ch >= 0 && ch < 256) {
						append(buffer, (char)ch, curSym);
						++curSym;
						if (ch == 10) {
							curC = 0;
							++curR;
							continue;
						}
						++curC;
						if (ch == 9) {
							curC += (8-curC%8);
						}
						if (curC >= col) {
							curC -= col;
							curR++;
						}
						continue;
					}	
				}
			}
		}
		endwin();
		destructor(buffer);
		tryClosing(f, argv[i]);
	}
}
