//Check behavior of lseek(), that's what's causing the problem
//Also check if line is greater than xmax (without terminating with \n
//Keep line count if contains only '\n' that might solve the scrolling problem
//that skips lines with only '\n'
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>

int countLine(int fd) {
	int i, xmax, ymax, flag = 0, j = 0;
	char c;
	i = lseek(fd, 0, SEEK_CUR);
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
	int ymax, xmax, flag = 0, j = 0;
	char c;
	getmaxyx(stdscr, ymax, xmax);
	while(j < xmax - 1 && read(fd, &c, 1)) {
		if(c == '\n') {
			lines[i][j] = c;
			j++;
			flag = 1;
			break;
		}
		lines[i][j] = c;
		j++;	
	}
	lines[i][j++] = '\0';
	if(flag == 0 && j != xmax)
		return INT_MAX;
	return j;
}

int printScreen(int fd, int count, char **lines) {
	int xmax, ymax, check, eof, sum, i;
	sum = eof = 0;
	getmaxyx(stdscr, ymax, xmax);
	lseek(fd, 0, SEEK_SET);
	for(i = 0; i < count; i++) {
		sum+= countLine(fd);
	}
	move(ymax - 1, 0);
	lseek(fd, sum, SEEK_SET);
	move(0, 0);
	for(i = 0; i < ymax; i++) {
		check = readLine(fd, lines, i);
		printw("%s", lines[i]);
		if(check == INT_MAX) {
			eof = 1;
			break;
		}
	}
	return eof;
}

void resize(int sig) {
	
}

int main(int argc, char *argv[]) {
	int x, y, xmax, ymax, ssize, newxmax, newymax;
	int ch, fd;
	char **lines, c;
	int flag = 0, count = 0, i = 0, check, eof = 0, sum = 0;
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
		return 1;
	for(i = 0; i < ymax; i++) {
		lines[i] = (char *)malloc(xmax + 1);
		if(lines[i] == NULL)
			return 1;
	}
	
	printScreen(fd, count, lines);
	while(1) {
		ch = getch();
		//signal(SIGWINCH, resize);
		/*
		getmaxyx(stdscr, newymax, newxmax);
		if(newxmax != xmax || newymax != ymax) {
			ymax = newymax;
			xmax = newxmax;
			ssize = xmax * ymax;
			for(i = 0; i < ymax; i++) {
				free(lines[i]);
				lines[i] = (char *)malloc(xmax + 1);
				if(lines[i] == NULL)
					return 1;
			}
			for(i = 0; i < ymax; i++) {
				check = readLine(fd, lines, i);
				printw("%s", lines[i]);
				if(check == INT_MAX) {
					eof = 1;
					break;
				}
			}
		}
		*/
		getyx(stdscr, y, x);
		switch(ch) {
			case KEY_LEFT:
				move(y, x - 1);
				break;
			case KEY_RIGHT:
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
				getmaxyx(stdscr, ymax, xmax);
				for(i = 0; i < ymax; i++) {
					lines[i] = (char *)realloc(lines[i], xmax + 1);
					if(lines[i] == NULL)
						return 1;
				}
				printScreen(fd, count, lines);
				break;
			default:
				printw("%c", ch);
		}
		if(flag == 1)
			break;
	}
	
	close(fd);
	endwin();
	return 0;
}
