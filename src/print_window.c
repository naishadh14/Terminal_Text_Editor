#include "print_window.h"
#include "support_functions.h"
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Debugging function to print a test message
void printTest() {
    int x, y, xmax, ymax;
    getyx(stdscr, y, x);
    getmaxyx(stdscr, ymax, xmax);
    move(ymax - 1, xmax - 3);
    printw("Hi");
    move(y, x);
}

// Function to print the file content on the screen
int printScreen(int fd, int fcp, int count, char **lines) {
	int xmax, ymax, check, eof, i;
	long long sum;
	sum = 0;
    eof = -1;
	getmaxyx(stdscr, ymax, xmax);
	lseek(fd, 0, SEEK_SET);
    lseek(fcp, 0, SEEK_SET);
	for(i = 0; i < count; i++) {
		sum = countLineFile(fd);
		lseek(fd, sum, SEEK_CUR);
        lseek(fcp, sum, SEEK_CUR);
	}
	move(0, 0);
	for(i = 0; i < ymax; i++) {
		check = readLine(fd, lines, i);
		if(check == INT_MAX) {
			eof = i;
			break;
		}
		if(check == INT_MIN) {
			printw("\n");
		}
		else if(check != 1) {
			printw("%s", lines[i]);
            lseek(fcp, check, SEEK_CUR);
			if(check != xmax + 1)
				printw("\n");
		}
	}
	return eof; /* if eof is -1, it has not reached end of file
                * if eof is i, it means eof was reached on ith line
                */
}

// Debugging function to print the cursor position
void printCursor(int fcp) {
    int x, y, xmax, ymax;
    char c;
    getyx(stdscr, y, x);
    getmaxyx(stdscr, ymax, xmax);
    move(0, xmax - 2);
    read(fcp, &c, 1);
    lseek(fcp, -1, SEEK_CUR);
    printw("%c", c);
    move(y, x);
}