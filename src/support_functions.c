#include "support_functions.h"
#include <unistd.h>
#include <stdio.h>
#include <curses.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

int locatePreviousN(int fd) {
    int count = 0, fcp, fcpset;
    char c;
    fcp = open("temp.txt", O_RDONLY);
    fcpset = lseek(fd, 0, SEEK_CUR);
    lseek(fcp, fcpset - 1, SEEK_SET);
    while(1) {
        read(fcp, &c, 1);
        if(c == '\n')
            break;
        count++;
        lseek(fcp, -2, SEEK_CUR);
    }
    close(fcp);
    return count;
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
	if(flag == 0) //it didn't reach '\n'. This means line is greater than xmax
		return INT_MAX;
	return j;
}

// Function to add a newline character if not present at the end of the file
void addN(int fd) {
	int fdset;
	char c;
	fdset = lseek(fd, -1, SEEK_END);
	read(fd, &c, 1);
	if(c == '\n')
		return;
	c = '\n';
	write(fd, &c, 1);
}

// Function to read a line from the file and store it in the lines array
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
	if(lines[i][0] == '\0')
		return INT_MIN;
	return j;
}

// Function to save changes back to the original file
void save(int fp, int fd, int fcp, char *file) {
	close(fp);
	remove(file);
	rename("temp.txt", file);
}

void clearScreen(int fcp, int fp, int fd, char *message) {
	close(fcp);
	close(fp);
	close(fd);
	endwin();
	initscr();
	clear();
	noecho();
	printw(message);
	getch();
	endwin();
}