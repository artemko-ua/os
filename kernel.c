#include "kernel.h"

// VGA text buffer properties
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_WHITE 15

// Buffer for input
#define INPUT_BUFFER_SIZE 256
char input_buffer[INPUT_BUFFER_SIZE];
int input_index = 0;

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

// Function to handle keyboard input
void handle_input(char c) {
    if (input_index < INPUT_BUFFER_SIZE - 1) {
        if (c == '\n') {
            // Process input when Enter is pressed
            input_buffer[input_index] = '\0';
            print("\nYou entered: ", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            print(input_buffer, VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            print_char('\n', VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
            
            // Reset input buffer
            input_index = 0;
        } else if (c == '\b') {
            // Handle backspace
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

// Kernel main function
void kmain() {
    // Clear the screen
    clear_screen();
    
    // Print Nexus logo
    print_logo();
    
    // Print a welcome message
    print("\nWelcome to Nexus OS\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    print("Enter your command:\n", VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));
    
    // Simulated input handling (placeholder)
    // In a real OS, this would be replaced by actual keyboard interrupt handling
    const char* demo_input = "Hello, Nexus!";
    for (int i = 0; demo_input[i]; i++) {
        handle_input(demo_input[i]);
    }
    handle_input('\n');
}