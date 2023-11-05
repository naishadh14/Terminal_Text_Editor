#include "cursor_movement.h"
#include "insert_logic.h"
#include "support_functions.h"
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

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
    if (ch == c)
        return 0;
    else
        return -1;
}