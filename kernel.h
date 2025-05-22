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
void keyboard_handler();
void setup_keyboard();
char get_key();

// Math operations
int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);

// Custom string functions
int strcmp(const char* str1, const char* str2);
char* strchr(const char* str, int c);
int atoi(const char* str);

#endif // KERNEL_H
