/*
* 1. Keep line count if contains only '\n' that might solve the scrolling problem
* that skips lines with only '\n'
* 2. Program snaps back to '\0' for each line
* 3. Yet to handle enter
* 4. Look into getmaxx to remove unused ymax ke compiler warnings
* 5. Check program hanging when KEY_UP from a high up file position
*/
#include <ncurses.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
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

void copy(int fp, int fd) { //copies all data from fp into fd
    char c;
    while(read(fp, &c, 1))
        write(fd, &c, 1);
}

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
		move(y - 1, prevlinecount); //do something
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
		move(y - 1, prevlinecount); //do something
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
    //insertCharIntoFile(y, x, ch, fd);
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

void moveFileCursorScreen(int fcp, int count) {
    /* This function is to be used when printing the screen with fd.
        It will move fcp such that it points to the same position in file as fd.
    */
    int xmax, ymax, check, eof, i;
    getmaxyx(stdscr, ymax, xmax);
    char **test;
    test = (char **)malloc(sizeof(char *));
    *test = (char *)malloc(sizeof(xmax + 1));
	long long sum;
	sum = 0;
    eof = -1;
	lseek(fcp, 0, SEEK_SET);
	for(i = 0; i < count; i++) {
		sum = countLineFile(fcp);
		lseek(fcp, sum, SEEK_CUR);
	}
	for(i = 0; i < ymax; i++) {
		check = readLine(fcp, test, 0);
		if(check == INT_MAX) {
			eof = i;
			break;
		}
	}
}

void printTest() { //debugging function
    int x, y, xmax, ymax;
    getyx(stdscr, y, x);
    getmaxyx(stdscr, ymax, xmax);
    move(ymax - 1, xmax - 3);
    printw("Hi");
    move(y, x);
}

int printScreen(int fd, int fcp, int count, char **lines) {
	int xmax, ymax, check, eof, i, fcpset;
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
    //fcpset = lseek(fd, 0, SEEK_CUR);
    //lseek(fcp, fcpset, SEEK_SET);
	return eof; /* if eof is -1, it has not reached end of file
                * if eof is i, it means eof was reached on ith line
                */
}

void fileCursorLeft(int fcp) {
    lseek(fcp, -1, SEEK_CUR);
}

void fileCursorRight(int fcp) {
    lseek(fcp, 1, SEEK_CUR);
}

void fileCursorUp(int fcp) {
    int distance, fpp, xmax, ymax, count, count2, i, fppset;
    char c;
    getmaxyx(stdscr, ymax, xmax);
    fpp = open("temp.txt", O_RDONLY);
    distance = locatePreviousN(fcp);
    fppset = lseek(fcp, -distance, SEEK_CUR); //get file pointer of '\n'
    lseek(fpp, fppset, SEEK_SET);
    lseek(fcp, distance, SEEK_CUR); //set fcp to it's original position
    if(distance >= xmax)
        lseek(fcp, -xmax, SEEK_CUR);
    //see if you need a separate condition for distance == xmax
    else {
        count = locatePreviousN(fcp);
        lseek(fcp, -(count + 1), SEEK_CUR);
        count2 = locatePreviousN(fcp);
        lseek(fcp, -count2, SEEK_CUR);
        for(i = 0; i < count; i++) {
            read(fcp, &c, 1);
            if(c == '\n') {
                lseek(fcp, -1, SEEK_CUR);
                break;
            }
        }
    }
    close(fpp);
}

void fileCursorDown(int fcp) {
    int distance, count, count2, i, fpp, xmax, ymax, fppset;
    char c;
    fpp = open("temp.txt", O_RDONLY);
    getmaxyx(stdscr, ymax, xmax);
    distance = locatePreviousN(fcp);
    fppset = lseek(fcp, -distance, SEEK_CUR);
    lseek(fcp, distance, SEEK_CUR);
    lseek(fpp, fppset, SEEK_SET);
    count = countLineFile(fpp);
    count = count - 1;
    if(count <= xmax) {
        lseek(fcp, count - distance + 1, SEEK_CUR);
        for(i = 0; i < distance; i++) {
            read(fcp, &c, 1);
            if(c == '\n') {
                lseek(fcp, -1, SEEK_CUR);
                break;
            }
        }
    }
    else {
        count2 = countLineFile(fcp);
        count2 = count2 - 1;
        if(count2 < xmax)
            lseek(fcp, count2, SEEK_CUR);
        else
            lseek(fcp, xmax, SEEK_CUR);
    }
    /*
    move(ymax - 3, 0);
    printw("xmax is %d", xmax);
    move(ymax - 2, 0);
    printw("count is %d", count);
    move(ymax - 1, 0);
    printw("distance is %d", distance);
    */
    close(fpp);
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
	int x, y, xmax, ymax, ssize, newxmax, newymax;
	int ch, fd, fp; //work with fd, fp is for original file
    int fcp; //File Cursor Position keeps track of where in the file the cursor on screen is
	char **lines, c;
	int flag = 0, count = 0, i = 0, check, eof = -1, sum = 0, movecheck;
	int linecount, prevlinecount, nextlinecount;
    //work with the copy of the file always
    //save it back to the original file when user clicks 'save'
	/* lines is a array of strings.
	* Each string is the characters to be displayed on a line.
	*/
	if(argc != 2) {
		printf("Please enter the file name as the 2nd argument\n");
		return EINVAL;
	}
	fp = open(argv[1], O_RDWR | O_CREAT);
	if(fp == 0) {
		perror("Could not open file: ");
		return errno;
	}
	fd = open("temp.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(fd == 0) {
        perror("Could not open file: ");
        return errno;
	}
    fcp = open("temp.txt", O_RDWR);
	copy(fp, fd); //copy all data in fp to fd
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
	eof = printScreen(fd, fcp, count, lines);
    //lseek(fcp, -1, SEEK_CUR);
    //moveFileCursorScreen(fcp, count);
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
                printCursor(fcp);
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
                    printCursor(fcp);
					break;
				}
				movecheck = move(y, x + 1);
                if(movecheck == ERR)
                    break;
                fileCursorRight(fcp);
                printCursor(fcp);
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
                printCursor(fcp);
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
                printCursor(fcp);
				break;
			case KEY_BACKSPACE:
				backspace(y, x, lines);
				//printScreen(fd, count, lines);
				break;
			case KEY_DC:
				delete(y, x, lines);
				move(y, x);
				break;
            case KEY_ENTER: //check what KEY vlaue is actually returned for this
                move(0, 0);
                printw("Enter was pressed\n");
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
				printScreen(fd, fcp, count, lines); //test with eof = printScreen
				break;
			default:
				insert(y, x, ch, lines);
                break;
		}
		if(flag == 1)
			break;
	}

    close(fcp);
    close(fp);
	close(fd);
	endwin();
	return 0;
}
