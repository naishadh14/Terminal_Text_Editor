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