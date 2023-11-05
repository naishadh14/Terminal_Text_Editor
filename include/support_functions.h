#ifndef SUPPORT_H
#define SUPPORT_H

int locatePreviousN(int fd);
int countLineFile(int fd);
void addN(int fd);
int readLine(int fd, char **lines, int i);
void save(int fp, int fd, int fcp, char *file);

#endif