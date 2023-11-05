#include "insert_logic.h"
#include "text_operations.h"
#include "cursor_movement.h"
#include "support_functions.h"
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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
	else { //if line is empty
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

void printTest() { //debugging function
    int x, y, xmax, ymax;
    getyx(stdscr, y, x);
    getmaxyx(stdscr, ymax, xmax);
    move(ymax - 1, xmax - 3);
    printw("Hi");
    move(y, x);
}

void save(int fp, int fd, int fcp, char *file) {
	int fcpset;
	close(fp);
	remove(file);
	rename("temp.txt", file);
}

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

void printCursor(int fcp) { //debugging function
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

int main(int argc, char *argv[]) {
	int x, y, xmax, ymax, set;
	// fd is the file pointer for the temporary working file
	// fp is the file pointer for the original user file
	int ch, fd, fp, c;
	// File Cursor Pointer (FCP) keeps track of cursor's screen position 
    int fcp;

	// Lines is an array of string
	// Each string contains the characters to be displayed on a line
	char **lines;

	int flag = 0, count = 0, i = 0, eof = -1, movecheck, savecheck = 0;
	int linecount, prevlinecount, nextlinecount;
    
	// Always work with the copy of the file
    // Save it back to the original file when user clicks 'save'

	// Command arguments validation
	if(argc != 2) {
		printf("Please enter the file name as the 2nd argument\n");
		return EINVAL;
	}

	// Sanity checks for file IO operations
	fp = open(argv[1], O_RDWR | O_CREAT, 0777);
	if(fp == -1) {
		perror("Could not open file: ");
		return errno;
	}

	fd = open("temp.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(fd == -1) {
        perror("Could not open temp work file: ");
        return errno;
	}

    fcp = open("temp.txt", O_RDWR);
    if(fcp == -1) {
    	perror("Could not open temp work file: ");
        return errno;
    }

	copy(fp, fd); //copy all data in fp to fd
	addN(fd);
	initscr();
	noecho();
	move(0, 0);
	printw("Instructions on using the editor.\n");
	printw("Press F2 to save.\n");
	printw("If the file could not be saved, an error message will be shown.\n");
	printw("Press F3 to exit the program\n");
	printw("Press Enter key to continue\n");
	c = getch();
	if(c != 10) {
		endwin();
		return 1;
	}
	clear();
	endwin(); 
    initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	getmaxyx(stdscr, ymax, xmax);
	//ssize = xmax * ymax;
	lines = (char **)malloc(ymax * sizeof(char *));
	if(lines == NULL)
		return ENOMEM;
	for(i = 0; i < ymax; i++) {
		lines[i] = (char *)malloc(xmax + 1);
		if(lines[i] == NULL)
			return ENOMEM;
	}
	eof = printScreen(fd, fcp, count, lines);
	getyx(stdscr, y, x);
	backspace(y, x, lines);
	for(i = 0; i < 5; i++) {
		getyx(stdscr, y, x);
		if(x == 0 && y == 0)
			break;
		if(x == 0 && y == 0) {
			prevlinecount = strlen(lines[y - 1]);
			if(prevlinecount == xmax)
				break;
			move(y - 1, prevlinecount);
		}
		move(y, x - 1);
		fileCursorLeft(fcp);
	}
	for(i = 0; i < 5; i++) {
		getyx(stdscr, y, x);
		if(y == eof)
			break;
		linecount = strlen(lines[y]);
		if(x > linecount - 1) {
			if(y == eof - 1)
				break;
			movecheck = move(y + 1, 0);
			if(movecheck == ERR)
				break;
			fileCursorRight(fcp);
			break;
		}
		movecheck = move(y, x + 1);
		if(movecheck == ERR)
			break;
		fileCursorRight(fcp);
	}
    while(1) {
    	printCursor(fcp);
		ch = getch();
		getyx(stdscr, y, x);
		switch(ch) {
			case KEY_LEFT:
                if(x == 0 && y == 0)
                    break;
				if(x == 0 && y != 0) {
					prevlinecount = strlen(lines[y - 1]);
                    if(prevlinecount == xmax)
                        break;
					move(y - 1, prevlinecount);
				}
				move(y, x - 1);
                fileCursorLeft(fcp);
				break;
			case KEY_RIGHT:
                if(y == eof)
                    break;
				linecount = strlen(lines[y]);
				if(x > linecount - 1) {
                    if(y == eof - 1)
                        break;
					movecheck = move(y + 1, 0); //move to 1st char of next line if end of line
                    if(movecheck == ERR)
                        break;
                    fileCursorRight(fcp);
					break;
				}
				movecheck = move(y, x + 1);
                if(movecheck == ERR)
                    break;
                fileCursorRight(fcp);
				break;
			case KEY_UP:
				if(y == 0) {
					if(count == 0)
						break;
					eof = -1;
					clear();
					count--;
					eof = printScreen(fd, fcp, count, lines);
					move(0, x);
				}
				else {
					prevlinecount = strlen(lines[y - 1]);
					if(x > prevlinecount)
						move(y - 1, prevlinecount);
					else
						move(y - 1, x);
				}
                fileCursorUp(fcp);
				break;
			case KEY_DOWN:
				if(y == ymax - 1) {
					if(eof != -1) { //change eof to -1 in KEY_UP
						break;
					}
					clear();
					count++;
					eof = printScreen(fd, fcp, count, lines);
				}
				else {
                    if(y == eof - 1)
                        break;
					nextlinecount = strlen(lines[y + 1]);
					if(x > nextlinecount)
						move(y + 1, nextlinecount);
					else
						move(y + 1, x);
				}
                fileCursorDown(fcp);
				break;
			case KEY_BACKSPACE:
				backspace(y, x, lines);
				backspaceFromFile(fcp);
				savecheck = -1;
				break;
			case KEY_DC:
				delete(y, x, lines);
				deleteFromFile(fcp);
				move(y, x);
				break;
            case 10: //value for KEY ENTER
                ch = '\n';
                insertInFile(fcp, ch);
                eof = printScreen(fd, fcp, count, lines);
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
				printScreen(fd, fcp, count, lines);
				break;
			case KEY_F(2): //save
				fileCursorLeft(fcp);
				move(y, x - 1);
				savecheck = filePositionCheck(fcp);
				if(savecheck == -1) {
					close(fcp);
					close(fp);
					close(fd);
					remove("temp.txt");
					endwin();
					initscr();
					clear();
					noecho();
					printw("Error encountered while saving.\n");
					printw("Press any key to exit.\n");
					getch();
					endwin();
					return 0;
				}
				save(fp, fd, fcp, argv[1]);
				close(fcp);
				close(fp);
				close(fd);
				endwin();
				initscr();
				clear();
				noecho();
				printw("File saved successfully.\n");
				printw("Press any key to exit.\n");
				getch();
				endwin();
				return 0;
			case KEY_F(3): //exit
				flag = 1;
				break;
			default:
				insert(y, x, ch, lines);
				insertInFile(fcp, ch);
                break;
		}
			savecheck = filePositionCheck(fcp); //returns -1 if positions are distorted
		if(flag == 1)
			break;
	}
	
    close(fcp);
	close(fp);
	close(fd);
	remove("temp.txt");
	endwin();
	return 0;
}
