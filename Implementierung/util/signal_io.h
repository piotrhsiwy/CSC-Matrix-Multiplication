#ifndef SIGNAL_IO_H
#define SIGNAL_IO_H

/**
 * Outputs a string to stdout in a signal safe manner
 * @param msg The string to print
 */
void print_sig(const char* msg);
/**
 * Gets exactly one character from the current line in stdin.
 * @return The character or '\0' if more than one characters where present in the line.
 */
char getchar_sig(void);

#endif // SIGNAL_IO_H
