#ifndef KERNEL_H
#define KERNEL_H

// Function prototypes
void clear_screen();
void print_char(char c, int color);
void print(const char* str, int color);
void print_logo();
void handle_input(char c);

#endif // KERNEL_H