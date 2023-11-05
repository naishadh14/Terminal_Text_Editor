#include "insert_logic.h"
#include "text_operations.h"
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void insertInFile(int fcp, char ch) {
	int fd, fcpset, i;
	char c;
	fd = open("temp1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(fd == -1) {
		perror("Error opening file: ");
		exit(1);
	}
	fcpset = lseek(fcp, 0, SEEK_CUR);
	lseek(fcp, 0, SEEK_SET);
	for(i = 0; i < fcpset; i++) {
		read(fcp, &c, 1);
		write(fd, &c, 1);
	}
	write(fd, &ch, 1);
	copy(fcp, fd);
	lseek(fcp, 0, SEEK_SET);
	lseek(fd, 0, SEEK_SET);
	copy(fd, fcp);
	close(fd);
	remove("temp1.txt"); //remove temp file
	lseek(fcp, fcpset + 1, SEEK_SET);
}

void insert(int y, int x, char ch, char **lines) {
	int count, i, xmax, ymax, j;
	char c;
	count = strlen(lines[y]);
	getmaxyx(stdscr, ymax, xmax);
	if(count < xmax) {
		if(x > count) {
			i = count;
			while(i != x) //if user starts from middle
				lines[y][i++] = ' '; //put spaces until x-1 then put the char entered
			if(ch == '\t') {
				lines[y][x++] = ' ';
				lines[y][x++] = ' ';
				lines[y][x++] = ' ';
				lines[y][x++] = ' ';
				lines[y][x] = '\0';
				move(y, 0);
				printw("%s", lines[y]);
				move(y, x + 4);
			}
			else {
				lines[y][x] = ch;
				lines[y][x+1] = '\0';
				move(y, 0);
				printw("%s", lines[y]);
				move(y, x + 1);
			}
		}
		else if(x <= count) {
			if(ch == '\t') {
				if(count + 4 > xmax)
					return;
				j = x;
				shiftRight(y, x, lines);
				lines[y][x++] = ' ';
				shiftRight(y, x, lines);
				lines[y][x++] = ' ';
				shiftRight(y, x, lines);
				lines[y][x++] = ' ';
				shiftRight(y, x, lines);
				lines[y][x++] = ' ';
				move(y, 0);
				printw("%s", lines[y]);
				x = j;
				move(y, x + 4);
			}
			else {
				shiftRight(y, x, lines);
				lines[y][x] = ch;
				move(y, 0);
				printw("%s", lines[y]);
				move(y, x + 1);
			}
		}
	}
	else { //if count is xmax, we need to put chars in the next line
		c = shiftRight(y, x, lines);
		lines[y][x] = ch;
		move(y, 0);
		printw("%s", lines[y]);
		insert(y + 1, 0, c, lines);
		move(y, x + 1);
	}
}