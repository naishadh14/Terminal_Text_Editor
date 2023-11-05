#ifndef PRINT_WINDOW_H
#define PRINT_WINDOW_H

// Debugging function to print a test message
void printTest();

// Function to print the file content on the screen
int printScreen(int fd, int fcp, int count, char **lines);

// Debugging function to print the cursor position
void printCursor(int fcp);

#endif