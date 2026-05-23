#ifndef LIBSYSY_SYSY_H_
#define LIBSYSY_SYSY_H_

// Reads an integer from the standard input.
int getint();

// Reads a character from the standard input.
int getch();

// Reads an array of integers from the standard input.
int getarray(int a[]);

// Reads a float from the standard input.
float getfloat();

// Writes an integer to the standard output.
void putint(int num);

// Writes a character to the standard output.
void putch(int ch);

// Writes a float to the standard output.
void putfloat(float num);

// Writes an array of integers to the standard output.
void putarray(int n, int a[]);

// Starts and stops the timer.
// Nested calls to these functions are not allowed.
void starttime();
void stoptime();

#endif  // LIBSYSY_SYSY_H_
