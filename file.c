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

void shiftLeft(int y, int x, char **lines) {
	int i, count;
	count = strlen(lines[y]);
	i = x;
	while(i < count) {
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
	while(i != x) {
		lines[y][i] = lines[y][i - 1];
		i--;
	}
	if(count == xmax) {
		c = lines[y][count];
		lines[y][count] = '\0';
	}
	return c;
	//return last element of the array to deal with character overflow while inserting
}

void delete(int y, int x, char **lines) {
	int count, i, xmax, ymax, prevlinecount;
	if(y == 0 && x == 0)
		return;
	count = strlen(lines[y]);
	getmaxyx(stdscr, ymax, xmax);
	if(count != 0) { //not an empty line
		shiftLeft(y, x - 1, lines);
		move(y, 0);
		for(i = 0; i < xmax; i++) {
			move(y, i);
			printw(" ");
		}
		move(y, 0);
		printw("%s", lines[y]);
		move(y, x - 1);
	}
	else { //if line is empty
		prevlinecount = strlen(lines[y - 1]);
		move(y - 1, prevlinecount);//do something
	}
}

void insert(int y, int x, char ch, char **lines) { //handle tabs and enter
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
	int ymax, xmax, j = 0, eof = 1, flag = 0;
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
	if(lines[i][0] == '\0')
		return INT_MIN;
	return j;
}

int printScreen(int fd, int count, char **lines) {
	int xmax, ymax, check, eof, i;
	long long sum;
	sum = eof = 0;
	getmaxyx(stdscr, ymax, xmax);
	lseek(fd, 0, SEEK_SET);
	for(i = 0; i < count; i++) {
		sum = countLineFile(fd);
		lseek(fd, sum, SEEK_CUR);
	}
	move(0, 0);
	for(i = 0; i < ymax; i++) {
		check = readLine(fd, lines, i);
		if(check == INT_MAX) {
			eof = 1;
			break;
		}
		if(check == INT_MIN) {
			printw("\n");
		}
		else if(check != 1) {
			printw("%s", lines[i]);
			if(check != xmax + 1)
				printw("\n");
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
		printf("Please enter the file name as the 2nd argument\n");
		return EINVAL;
	}
	fd = open(argv[1], O_RDWR | O_CREAT);
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
	eof = printScreen(fd, count, lines);
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
				delete(y, x, lines);
				//printScreen(fd, count, lines);
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
				printScreen(fd, count, lines); //eof = printScreen
				break;
			default:
				insert(y, x, ch, lines);
		}
		if(flag == 1)
			break;
	}
	
	close(fd);
	endwin();
	return 0;
}
