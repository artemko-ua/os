#ifndef KERNEL_H
#define KERNEL_H

// Власні типи замість стандартних бібліотек
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned long size_t;

#define NULL ((void*)0)
#define true 1
#define false 0

// Кольори для VGA тексту
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_LIGHT_YELLOW  14
#define VGA_COLOR_WHITE         15

// Розміри VGA екрану
#define VGA_WIDTH  80
#define VGA_HEIGHT 25

// Виправлені константи
#define VGA_BUFFER_ADDRESS  0xB8000
#define IDT_ADDRESS         0x1000
#define IDT_ENTRIES         256
#define IDT_ENTRY_SIZE      8

// Обмеження буферів
#define MAX_INPUT_SIZE      256
#define MAX_COMMAND_LEN     64
#define MATH_BUFFER_SIZE    32

// PIC константи
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1
#define PIC_EOI         0x20

// Клавіатура
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64
#define KEYBOARD_SCANCODE_MAX   57

// Коди помилок
#define SUCCESS                 0
#define ERROR_INVALID_INPUT     -1
#define ERROR_BUFFER_OVERFLOW   -2
#define ERROR_DIVISION_BY_ZERO  -3
#define ERROR_INVALID_EXPRESSION -4

// Математичні операції - результати
typedef enum {
    MATH_SUCCESS = 0,
    MATH_ERROR_INVALID_EXPR,
    MATH_ERROR_DIV_BY_ZERO,
    MATH_ERROR_OVERFLOW,
    MATH_ERROR_PARSE_FAIL
} math_result_t;

// Структура для результатів математичних операцій
typedef struct {
    int value;
    math_result_t error;
} math_calculation_t;

// Функції VGA
void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_clear(void);
void terminal_scroll(void);
uint8_t vga_entry_color(uint8_t fg, uint8_t bg);
uint16_t vga_entry(unsigned char uc, uint8_t color);

// Функції портів
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// Функції IDT
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// Функції переривань
void pic_init(void);
void keyboard_handler(void);
void enable_interrupts(void);
void disable_interrupts(void);

// Функції shell
void shell_initialize(void);
void shell_run(void);
void process_command(const char* command);
void show_help(void);
void show_logo(void);

// Математичні функції
int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int divide(int a, int b);
int random_number(void);
math_calculation_t parse_math_expression_safe(const char* expr);

// Системні функції
void shutdown(void);
void reboot(void);

// Утилітарні функції
size_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int atoi(const char* str);
void itoa(int value, char* str, int base);
int parse_math_expression(const char* expr);

// Функції валідації
int is_valid_input(const char* input);
int is_safe_buffer_operation(size_t current_size, size_t max_size, size_t add_size);

// Глобальні змінні
extern char input_buffer[MAX_INPUT_SIZE];
extern size_t input_index;
extern size_t terminal_row;
extern size_t terminal_column;
extern uint8_t terminal_color;
extern uint16_t* terminal_buffer;

#endif 