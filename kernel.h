#ifndef KERNEL_H
#define KERNEL_H

// Function prototypes
void clear_screen();
void print_char(char c, int color);
void print(const char* str, int color);
void print_logo();
void handle_input(char c);
void process_command(const char* cmd);
void itoa(int value, char* str, int base);

// Math operations
int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);

#endif // KERNEL_H