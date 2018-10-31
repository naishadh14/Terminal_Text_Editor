/*
* 1. Keep line count if contains only '\n' that might solve the scrolling problem
* that skips lines with only '\n'
* 2. Program snaps back to '\0' for each line
*/
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

void shiftRight(int x, int y, char **lines) {
	int count, i;
	count = strlen(lines[y]);
	i = count + 1;
	while(i != x) {
		lines[y][i] = lines[y][i - 1];
		i--;
	}
}

void insert(char ch, char **lines) {
	int count, y, x, i, xmax, ymax;
	getyx(stdscr, y, x);
	count = strlen(lines[y]);
	getmaxyx(stdscr, ymax, xmax);
	if(count < xmax) {
		if(x > count) {
			i = count;
			while(i != x)
				lines[y][i++] = ' ';
			lines[y][x] = ch;
			lines[y][x+1] = '\0';
			move(y, 0);
			printw("%s", lines[y]);
			move(y, x + 1);
		}
		else if(x <= count) {
			shiftRight(x, y, lines);
			lines[y][x] = ch;
			move(y, 0);
			printw("%s", lines[y]);
			move(y, x + 1);
		}
	}
	else {
		//do something
	}
}

int countLineFile(int fd) {
	int i, xmax, ymax, flag = 0, j = 0;
	char c;
	i = lseek(fd, 0, SEEK_CUR);
	if(i == -1)
		return errno;
	getmaxyx(stdscr, ymax, xmax);
	while(read(fd, &c, 1) && j < xmax) {
		if(c == '\n') {
			j++;
			flag = 1;
			break;
		}
		j++;	
	}
	lseek(fd, i, SEEK_SET);
	if(flag == 0)
		return INT_MAX;
	return j;
}

int readLine(int fd, char **lines, int i) {
	int ymax, xmax, j = 0, eof = 1;
	char c;
	getmaxyx(stdscr, ymax, xmax);
	while(j < xmax && (eof = read(fd, &c, 1))) {
		if(c == '\n')
			break;
		if(c == '\t') {
			c = ' ';
			lines[i][j++] = c;
			lines[i][j++] = c;
			lines[i][j++] = c;
			lines[i][j++] = c;
			continue;
		}
		lines[i][j] = c;
		j++;	
	}
	lines[i][j++] = '\0';
	if(eof == 0)
		return INT_MAX;
	return j;
}

int printScreen(int fd, int count, char **lines) {
	int xmax, ymax, check, eof, sum, i;
	sum = eof = 0;
	getmaxyx(stdscr, ymax, xmax);
	lseek(fd, 0, SEEK_SET);
	for(i = 0; i < count; i++) {
		sum+= countLineFile(fd);
	}
	move(ymax - 1, 0);
	lseek(fd, sum, SEEK_SET);
	move(0, 0);
	for(i = 0; i < ymax; i++) {
		check = readLine(fd, lines, i);
		if(check != 1) {
			printw("%s", lines[i]);
			if(check != xmax + 1)
				printw("\n");
		}
		if(check == INT_MAX) {
			eof = 1;
			break;
		}
	}
	return eof;
}

int main(int argc, char *argv[]) {
	int x, y, xmax, ymax, ssize, newxmax, newymax;
	int ch, fd;
	char **lines, c;
	int flag = 0, count = 0, i = 0, check, eof = 0, sum = 0;
	int linecount, prevlinecount, nextlinecount;
	/* lines is a array of strings. 
	* Each string is the characters to be displayed on a line.
	*/
	if(argc != 2) {
		printf("Please enter 2 arguments\n");
		return EINVAL;
	}
	fd = open(argv[1], O_RDWR);
	if(fd == 0) {
		perror("Could not open file: ");
		return errno;
	}
	
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	getmaxyx(stdscr, ymax, xmax);
	ssize = xmax * ymax;
	lines = (char **)malloc(ymax * sizeof(char *));
	if(lines == NULL) 
		return ENOMEM;
	for(i = 0; i < ymax; i++) {
		lines[i] = (char *)malloc(xmax + 1);
		if(lines[i] == NULL)
			return ENOMEM;
	}
	printScreen(fd, count, lines);
	while(1) {
		ch = getch();
		getyx(stdscr, y, x);
		switch(ch) {
			case KEY_LEFT:
				if(x == 0 && y != 0) {
					prevlinecount = strlen(lines[y - 1]);
					move(y - 1, prevlinecount);
				}
				move(y, x - 1);
				break;
			case KEY_RIGHT:
				linecount = strlen(lines[y]);
				if(x > linecount - 1) {
					move(y + 1, 0);	
					break;
				}
				move(y, x + 1);
				break;
			case KEY_UP:
				if(y == 0) {
					if(count == 0)
						break;
					eof = 0;
					clear();
					count--;
					eof = printScreen(fd, count, lines);
					move(0, x);
				}
				else {
					prevlinecount = strlen(lines[y - 1]);
					if(x > prevlinecount)
						move(y - 1, prevlinecount);
					else
						move(y - 1, x);
				}
				break;
			case KEY_DOWN:
				if(y == ymax - 1) {
					if(eof == 1) { //change eof to 0 in KEY_UP
						break;
					}
					clear();
					count++;
					eof = printScreen(fd, count, lines);
				}
				else {
					nextlinecount = strlen(lines[y + 1]);
					if(x > nextlinecount)
						move(y + 1, nextlinecount);
					else
						move(y + 1, x);
				}
				break;
			case KEY_BACKSPACE:
				mvprintw(y, x - 1, " ");
				move(y, x - 1);
				break;
			case KEY_DC:
				printw(" ");
				move(y, x);
				break;
			case KEY_RESIZE:
				for(i = 0; i < ymax; i++)
					free(lines[i]);
				getmaxyx(stdscr, ymax, xmax);
				lines = (char **)realloc(lines, ymax * sizeof(char *));
				if(lines == NULL) 
					return ENOMEM;
				for(i = 0; i < ymax; i++) {
					lines[i] = (char *)malloc(xmax + 1);
					if(lines[i] == NULL)
						return ENOMEM;
				}
				printScreen(fd, count, lines);
				break;
			default:
				insert(ch, lines);
		}
		if(flag == 1)
			break;
	}
	
	close(fd);
	endwin();
	return 0;
}
