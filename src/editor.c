#include "insert_logic.h"
#include "text_operations.h"
#include "cursor_movement.h"
#include "support_functions.h"
#include "delete_logic.h"
#include "print_window.h"
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int main(int argc, char *argv[]) {
	int x, y, xmax, ymax;
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