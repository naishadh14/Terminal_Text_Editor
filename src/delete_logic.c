#include "delete_logic.h"
#include "text_operations.h"
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void backspace(int y, int x, char **lines) {
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
	else {
		prevlinecount = strlen(lines[y - 1]);
		move(y - 1, prevlinecount);
	}
}

void delete(int y, int x, char **lines) {
	int count, i, xmax, ymax, prevlinecount;
	if(y == 0 && x == 0)
		return;
	count = strlen(lines[y]);
	getmaxyx(stdscr, ymax, xmax);
	if(count != 0) { //not an empty line
		shiftLeft(y, x, lines);
		move(y, 0);
		for(i = 0; i < xmax; i++) {
			move(y, i);
			printw(" ");
		}
		move(y, 0);
		printw("%s", lines[y]);
		move(y, x);
	}
	else { //if line is empty
		prevlinecount = strlen(lines[y - 1]);
		move(y - 1, prevlinecount);
	}
}

void deleteFromFile(int fcp) {
	int fd, i, fcpset, length;
	char c;
	fd = open("temp1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
	fcpset = lseek(fcp, 0, SEEK_CUR);
	lseek(fcp, 0, SEEK_SET);
	for(i = 0; i < fcpset; i++) {
		read(fcp, &c, 1);
		write(fd, &c, 1);
	}
	lseek(fcp, 1, SEEK_CUR);
	copy(fcp, fd);
	length = lseek(fcp, 0, SEEK_CUR);
	lseek(fcp, 0, SEEK_SET);
	lseek(fd, 0, SEEK_SET);
	copy(fd, fcp);
	ftruncate(fcp, length - 1);
	lseek(fcp, fcpset, SEEK_SET);
	close(fd);
	remove("temp1.txt");
}

void backspaceFromFile(int fcp) {
	int fd, i, fcpset, length;
	char c;
	fd = open("temp1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
	fcpset = lseek(fcp, 0, SEEK_CUR);
	lseek(fcp, 0, SEEK_SET);
	for(i = 0; i < fcpset - 1; i++) {
		read(fcp, &c, 1);
		write(fd, &c, 1);
	}
	lseek(fcp, 1, SEEK_CUR);
	copy(fcp, fd);
	length = lseek(fcp, 0, SEEK_CUR);
	lseek(fcp, 0, SEEK_SET);
	lseek(fd, 0, SEEK_SET);
	copy(fd, fcp);
	ftruncate(fcp, length - 1);
	lseek(fcp, fcpset - 1, SEEK_SET);
	close(fd);
	remove("temp1.txt");
}