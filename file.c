/*
This version can succesfully open a file and interact with it's content
*/
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int readfile(int fd, char *text, int size) {
	char c;
	int i = 0;
	while(read(fd, &c, 1)) {
		text[i] = c;
		i++;
	}
	text[i] = '\0';
	return i;
}

int main(int argc, char *argv[]) {
	int x, y, xmax, ymax, ssize;
	int ch;
	int fd;
	char *text;
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
	
	text = (char *)malloc(ssize);
	readfile(fd, text, ssize);
	printw("%s", text);
	
	while(1) {
		ch = getch();
		getyx(stdscr, y, x);
		switch(ch) {
			case KEY_LEFT:
				move(y, x - 1);
				break;
			case KEY_RIGHT:
				move(y, x + 1);
				break;
			case KEY_UP:
				move(y - 1, x);
				break;
			case KEY_DOWN:
				move(y + 1, x);
				break;
			case KEY_BACKSPACE:
				mvprintw(y, x - 1, " ");
				move(y, x - 1);
				break;
			case KEY_DC:
				printw(" ");
				move(y, x);
				break;
			default:
				printw("%c", ch);
		}
	}
	
	close(fd);
	endwin();
	return 0;
}
