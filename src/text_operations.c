#include "text_operations.h"
#include <string.h>
#include <ncurses.h>
#include <unistd.h>

//copies all data from position of fp into fd
void copy(int fp, int fd) { 
    char c;
    while(read(fp, &c, 1)) {
    	if(c == '\t') {
    		c = ' ';
    		write(fd, &c, 1);
    		write(fd, &c, 1);
    		write(fd, &c, 1);
    		write(fd, &c, 1);
    	}
    	else
        	write(fd, &c, 1);
	}
}

void shiftLeft(int y, int x, char **lines) {
    int i, count;
    count = strlen(lines[y]);
    i = x;
    while (i < count) {
        lines[y][i] = lines[y][i + 1];
        i++;
    }
}

char shiftRight(int y, int x, char **lines) {
    int count, i, xmax, ymax;
    char c;
    getmaxyx(stdscr, ymax, xmax);
    count = strlen(lines[y]);
    i = count;
    while (i != x) {
        lines[y][i] = lines[y][i - 1];
        i--;
    }
    if (count == xmax) {
        c = lines[y][count];
        lines[y][count] = '\0';
    }
    return c;
}