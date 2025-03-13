#include "string.h"
#include "stringlist.h"
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

int saveFile(char* name, stringlist* list) {
	if (access(name, F_OK) == 0 && rename(name, "oldtext.tmp") != 0) {
		return -1;
	}
	FILE *tmp = fopen(name, "w");
	size_t wr = writeList(list, tmp);
	if (wr != 0) {
		remove(name);
		if (access("oldtext.tmp", F_OK) == 0) {
			rename("oldtext.tmp", name);
		}
		return -1;
	}
	remove("oldtext.tmp");
	return 0;
}

enum inputMode {
	BASIC,
	COMMAND,
	TEXT	
};

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Error! No filename provided!\n");
		return 0;	
	}
	for (int i = 1; i < argc; ++i) {
		FILE *f = fopen(argv[i], "r+");
		stringlist* list;
		if (f != NULL) {	
			list = listConstructor(f);
			tryClosing(f, argv[i]);
		}
		else {
			list = listSingleConstructor(1);
			if (list == NULL) {
				fprintf(stderr, "Error! Couldn't create file");
			}
		}
		initscr();
		enum inputMode im = BASIC;
		bool active = true;
		int row, col, curR = 0, curC = 0;
		stringlist* curLine = list;
		stringlist* topLine = list;
		size_t curSym = 0, curW = 0;
		int curNum = 0;
		string* command = constructor(6);
		size_t comSymb = 0;
		bool changed = false;
		while (active) {
			getmaxyx(stdscr, row, col);
			col += 0;
			move(0, 0);
			erase(); 
			stringlist* iter = topLine;
			int rowsPrinted = 0;
			int num = 0;
			while (iter != NULL && rowsPrinted < row-1) {
				char* prbuf = NULL;
				print_format(iter->str, &prbuf);
				printw(prbuf);
				if (prbuf != NULL) {
					free(prbuf);
				}
				rowsPrinted += (iter->str->size+col-1)/col;
				iter = iter->next;
			}
			iter = list;
			while (iter != curLine) {
				iter = iter->next;
				num++;
			}
			{
				char* emptybuf = malloc(col+1*sizeof(char));
				for (int i = 0; i < col; ++i) {
					emptybuf[i] = ' ';
				}
				move(row-1, 0);
				printw(emptybuf);
			}
			move(row-1, 0);
			printw(command->data);
			move(row-1, col/2);
			char numStr[20];
			sprintf(numStr, "%d", num);
			printw(numStr);
			move(curR, curC);
			refresh();	
			switch (im) {
				case BASIC: {
					noecho();
					int ch = getch();
					if ((char)ch == 'i') {
						im = TEXT;
						continue;
					}
					if ((char)ch == ':') {
						im = COMMAND;
						continue;
					}
					break;
				}
				case COMMAND: {
					noecho();
					keypad(stdscr, TRUE);
					int ch = getch();
					if (ch == 27) {
						im = BASIC;
						continue;
					}
					if (ch == 263 && comSymb > 0) {
						backspace(command, comSymb);
						comSymb--;	
						continue;
					}
					if (ch == 10) {
						for (size_t j = 0; j < comSymb; ++j) {
							if (command->data[j] == 'w') {
								saveFile(argv[i], list);
								changed = false;
							}
							else if (command->data[j] == 'q' && j < comSymb-1 && command->data[j+1] == '!') {
								active = false;
							}
							else if (command->data[j] == 'q' && !changed) {
								active = false;
							}
							command->data[j] = 0;
						}
						comSymb = 0;
						continue;
					}
					if (ch > 0 && ch < 256) {
						append(command, (char)ch, comSymb);
						comSymb++;
						continue;
					}
					break;
				}
				case TEXT: {
					curR = 0;
					iter = topLine;
					while (iter != curLine && iter->next != NULL) {
						curR += (iter->str->width+col-1)/col;
						iter = iter->next;
					}
					curR += curW/col;
					curC = curW%col;
					move(curR, curC);
					keypad(stdscr, TRUE);
					noecho();
					int ch = getch();
					if (ch == KEY_LEFT && curSym > 0) {
						curSym--;
						if (curLine->str->data[curSym] == '\t') {
							curW = getCurWidth(curLine->str, curSym);
						}
						else {
							curW--;
						}
						continue;
					}
					if (ch == KEY_RIGHT && curSym < curLine->str->size && curLine->str->data[curSym] != '\n') {
						if (curLine->str->data[curSym] == '\t') {
							curW += 8-(curW&7);
						}
						else {
							++curW;
						}
						curSym++;
						continue;
					}
					if (ch == KEY_UP && curLine->prev != NULL) {
						curLine = curLine->prev;
						curNum--;
						if (curW > curLine->str->width-1) {
							curW = curLine->str->width-1;
						}
						curSym = getCurSym(curLine->str, curW);	
						curW = getCurWidth(curLine->str, curSym);
						if (curR == 0) {
							topLine = curLine;
						}
						continue;
					}
					if (ch == KEY_DOWN && curLine->next != NULL) {
						curLine = curLine->next;
						curNum++;
						if (curW > curLine->str->width-1) {
							curW = curLine->str->width-1;
						}
						curSym = getCurSym(curLine->str, curW);	
						curW = getCurWidth(curLine->str, curSym);
						if (curR > row-5) {
							topLine = topLine->next;
						}
						continue;
					}
					if (ch == KEY_DC && curSym < curLine->str->size-1) {
						changed = true;
						delete(curLine->str, curSym);	
						curW = getCurWidth(curLine->str, curSym);
						continue;
					}
					if (ch == KEY_DC && curSym >= curLine->str->size-1 && curLine->next != NULL) {
						changed = true;
						curW = getCurWidth(curLine->str, curSym);
						curLine = removeAndMergeLines(curLine->next);
					}
					if (ch == 263 && (curSym > 0 || curLine->prev != NULL)) {
						changed = true;
						if (curSym > 0) {
							backspace(curLine->str, curSym);
							curSym--;
							curW = getCurWidth(curLine->str, curSym);
							continue;
						}	
						curSym = curLine->prev->str->size-1;
						curW = getCurWidth(curLine->str, curSym);	
						curLine = removeAndMergeLines(curLine);
						if (curR <= 0) {
							topLine = curLine;
						}
						continue;
					}
					if (ch == 27) {
						im = BASIC;
						continue;
					}
					if (ch == 10) {
						curLine = createAfter(curLine, curSym);
						if (curR > row-5) {
							topLine = topLine->next;
						}
						curSym = 0;
						curW = 0;
						changed = true;
						continue;
					}
					if (ch >= 0 && ch < 256) {
						append(curLine->str, (char)ch, curSym);
						++curSym;
						changed = true;
						curW = getCurWidth(curLine->str, curSym);	
						continue;
					}
				}
			}
		}	
		endwin();
		slDestructor(list);
	}
}
