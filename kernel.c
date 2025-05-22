
#include "kernel.h"
#include <stdint.h>

// VGA text buffer properties
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_WHITE 15

// Buffer for input
#define INPUT_BUFFER_SIZE 256
char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;

// Command handling
#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 10

// Custom string functions (since we can't use standard library)
int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return 0;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    // Handle sign
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

// Basic math operations
int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
int multiply(int a, int b) { return a * b; }
int divide(int a, int b) { return b != 0 ? a / b : 0; }

// Simple random number generator (not cryptographically secure)
uint32_t rand_seed = 12345;
uint32_t rand() {
    rand_seed = rand_seed * 1103515245 + 12345;
    return rand_seed;
}

// Function to clear the screen
void clear_screen() {
    volatile char* video_memory = (volatile char*)0xB8000;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        video_memory[i] = ' ';
        video_memory[i + 1] = VGA_COLOR_BLACK | (VGA_COLOR_WHITE << 4);
    }
}

// Function to print a character to the screen
void print_char(char c, int color) {
    static int x = 0, y = 0;
    volatile char* video_memory = (volatile char*)0xB8000;
    
    if (c == '\n') {
        x = 0;
        y++;
        return;
    }
    
    if (x >= VGA_WIDTH) {
        x = 0;
        y++;
    }
    
    if (y >= VGA_HEIGHT) {
        // Simple scrolling by moving everything up
        for (int row = 1; row < VGA_HEIGHT; row++) {
            for (int col = 0; col < VGA_WIDTH; col++) {
                video_memory[(row - 1) * VGA_WIDTH * 2 + col * 2] = 
                    video_memory[row * VGA_WIDTH * 2 + col * 2];
                video_memory[(row - 1) * VGA_WIDTH * 2 + col * 2 + 1] = 
                    video_memory[row * VGA_WIDTH * 2 + col * 2 + 1];
            }
        }
        y = VGA_HEIGHT - 1;
        x = 0;
    }
    
    video_memory[(y * VGA_WIDTH + x) * 2] = c;
    video_memory[(y * VGA_WIDTH + x) * 2 + 1] = color;
    x++;
}

// Function to print a string
void print(const char* str, int color) {
    while (*str) {
        print_char(*str++, color);
    }
}

// Function to print Nexus logo
void print_logo() {
    const char* logo_lines[] = {
        " * *________ ___ *_*___ ",
        "| \\ | | ____\\ \\ / / | \\/ |/ ____| ",
        "| \\| | |__ \\ V /| | \\ / | (___ ",
        "| . ` | **| > < | | |\\/| |\\**_ \\ ",
        "| |\\ | |____ / . \\| | | |____) |",
        "|_| \\_|______/_/ \\_\\_| |_||_____/"
    };
    
    for (int i = 0; i < 6; i++) {
        print(logo_lines[i], VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print_char('\n', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    }
}

// Command processing
void process_command(const char* cmd) {
    char command[MAX_COMMAND_LENGTH];
    char args[MAX_ARGS][MAX_COMMAND_LENGTH];
    int arg_count = 0;
    
    // Simple command parsing
    int i = 0, j = 0, k = 0;
    while (cmd[i] && j < MAX_COMMAND_LENGTH - 1) {
        if (cmd[i] == ' ') {
            command[j] = '\0';
            j = 0;
            i++;
            while (cmd[i] && cmd[i] != ' ' && k < MAX_COMMAND_LENGTH - 1) {
                args[arg_count][k++] = cmd[i++];
            }
            if (k > 0) {
                args[arg_count][k] = '\0';
                arg_count++;
                k = 0;
            }
        } else {
            command[j++] = cmd[i++];
        }
    }
    command[j] = '\0';

    // Command handling
    if (strcmp(command, "help") == 0) {
        print("Available commands:\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("  shutdown - shutdown the system\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("  reboot - reboot the system\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("  echo \"text\" - print text\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("  echo \"num1+num2\" - math operations (+, -, *, /)\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("  rand - generate random number (0-99)\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("  clear - clear screen\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    }
    else if (strcmp(command, "clear") == 0) {
        clear_screen();
        print_logo();
        print("\nWelcome to Nexus OS v0.1\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    }
    else if (strcmp(command, "shutdown") == 0) {
        print("Shutting down...\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        // In a real OS, we would use ACPI to shutdown
        while(1) { __asm__("hlt"); }
    }
    else if (strcmp(command, "reboot") == 0) {
        print("Rebooting...\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        // In a real OS, we would use ACPI to reboot
        __asm__("jmp 0xFFFF0000");
    }
    else if (strcmp(command, "echo") == 0) {
        if (arg_count > 0) {
            // Check if it's a math operation
            if (strchr(args[0], '+') || strchr(args[0], '-') || 
                strchr(args[0], '*') || strchr(args[0], '/')) {
                int a = atoi(args[0]);
                char op = strchr(args[0], '+') ? '+' : 
                         strchr(args[0], '-') ? '-' :
                         strchr(args[0], '*') ? '*' : '/';
                int b = atoi(strchr(args[0], op) + 1);
                int result = 0;
                
                switch(op) {
                    case '+': result = add(a, b); break;
                    case '-': result = subtract(a, b); break;
                    case '*': result = multiply(a, b); break;
                    case '/': result = divide(a, b); break;
                }
                
                char num_str[32];
                itoa(result, num_str, 10);
                print(num_str, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
                print("\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            } else {
                print(args[0], VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
                print("\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            }
        }
    }
    else if (strcmp(command, "rand") == 0) {
        char num_str[32];
        itoa(rand() % 100, num_str, 10);
        print(num_str, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    }
    else {
        print("Unknown command: ", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print(command, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        print("Type 'help' for available commands.\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    }
}

// Helper function for integer to string conversion
void itoa(int value, char* str, int base) {
    int i = 0;
    int is_negative = 0;
    
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }
    
    do {
        str[i++] = value % base + '0';
        value = value / base;
    } while (value > 0);
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse the string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Update handle_input function
void handle_input(char c) {
    if (input_index < INPUT_BUFFER_SIZE - 1) {
        if (c == '\n') {
            input_buffer[input_index] = '\0';
            print("\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            process_command(input_buffer);
            print("nexus> ", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            input_index = 0;
        } else if (c == '\b') {
            if (input_index > 0) {
                input_index--;
                print_char('\b', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
                print_char(' ', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
                print_char('\b', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            }
        } else {
            input_buffer[input_index++] = c;
            print_char(c, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
        }
    }
}

// Update kmain function
void kmain() {
    clear_screen();
    print_logo();
    print("\nWelcome to Nexus OS v0.1\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    print("Type 'help' for available commands\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    print("nexus> ", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
}
