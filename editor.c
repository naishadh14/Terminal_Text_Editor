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

void copy(int fp, int fd) { //copies all data from position of fp into fd
    char c;
    while(read(fp, &c, 1)) {
    	if(c == '\t') {
    		c = ' ';
    		write(fd, &c, 1);
    		write(fd, &c, 1);
    		write(fd, &c, 1);
    		write(fd, &c, 1);
    	}
    	else
        	write(fd, &c, 1);
	}
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

void fileCursorLeft(int fcp) {
    lseek(fcp, -1, SEEK_CUR);
}

void fileCursorRight(int fcp) {
    lseek(fcp, 1, SEEK_CUR);
}

void fileCursorDown(int fcp) {
    int distance, count, count2, i, fpp, xmax, ymax;
    int fppset, fcppos, fpppos, length;
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
    else if(distance <= xmax) {
        count2 = countLineFile(fcp);
        count2 = count2 - 1;
        if(count2 < xmax)
            lseek(fcp, count2, SEEK_CUR);
        else
            lseek(fcp, xmax, SEEK_CUR);
    }
    else {
    	fcppos = lseek(fcp, 0, SEEK_CUR);
    	fpppos = lseek(fpp, 0, SEEK_CUR);
    	length = fcppos - fpppos;
    	length = length - xmax;
    	while(read(fcp, &c, 1))
    		if(c == '\n')
    			break;
    	lseek(fcp, length, SEEK_CUR);
    }
    close(fpp);
}

void fileCursorUp(int fcp) {
    int distance, fpp, xmax, ymax, count, count2, i, fppset;
    char c;
    getmaxyx(stdscr, ymax, xmax);
    fpp = open("temp.txt", O_RDONLY);
    if(fpp == 0) {
    	perror("Error opening file: ");
    	exit(1);
    }
    distance = locatePreviousN(fcp);
    fppset = lseek(fcp, -distance, SEEK_CUR); //get file pointer of '\n'
    lseek(fpp, fppset, SEEK_SET);
    lseek(fcp, distance, SEEK_CUR); //set fcp to it's original position
    if(distance >= xmax)
        lseek(fcp, -xmax, SEEK_CUR);
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
        if(count2 > xmax)
        	fileCursorDown(fcp);
    }
    close(fpp);
}

int filePositionCheck(int fcp) {
	int chget, ch;
	char c;
	read(fcp, &c, 1);
	lseek(fcp, -1, SEEK_CUR);
	chget = inch();
	ch = chget & A_CHARTEXT;
	if(ch == c)
		return 0;
	else
		return -1;
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
	int ch, fd, fp, c; //work with fd, fp is for original file
    int fcp; //File Cursor Position keeps track of where in the file the cursor on screen is
	char **lines;
	//char *template;
	//template = (char *)malloc(10);
	//strcpy(template, "tmpXXXXXX");
	int flag = 0, count = 0, i = 0, eof = -1, movecheck, savecheck = 0;
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
	fp = open(argv[1], O_RDWR | O_CREAT, 0777);
	if(fp == -1) {
		perror("Could not open file: ");
		return errno;
	}
	//fd = mkstemp(template);
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
