#include "kernel.h"

// Глобальні змінні для VGA терміналу
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

// Буфер для вводу команд
char input_buffer[256];
size_t input_index = 0;

// Зовнішні функції з асемблера
extern void enable_interrupts(void);
extern void disable_interrupts(void);
extern void keyboard_interrupt_handler(void);
extern void load_idt(void* idt_descriptor);
extern void reboot_system(void);
extern void shutdown_system(void);
extern void* idt_descriptor;

// Простий генератор випадкових чисел
static uint32_t random_seed = 12345;

void kernel_main(void) {
    // Ініціалізація терміналу
    terminal_initialize();
    
    // Показуємо логотип
    show_logo();
    
    // Ініціалізація PIC та переривань
    pic_init();
    
    // Ініціалізація shell
    shell_initialize();
    
    // Запуск shell
    shell_run();
}

// === VGA ФУНКЦІЇ ===

uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) 0xB8000;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_scroll(void) {
    // Пересуваємо всі рядки на один вгору
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            size_t index = y * VGA_WIDTH + x;
            size_t prev_index = (y - 1) * VGA_WIDTH + x;
            terminal_buffer[prev_index] = terminal_buffer[index];
        }
    }
    
    // Очищуємо останній рядок
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(c, terminal_color);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
            }
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

// === PIC ТА ПЕРЕРИВАННЯ ===

void pic_init(void) {
    // Ремапінг PIC
    outb(PIC1_COMMAND, 0x11); // ICW1
    outb(PIC2_COMMAND, 0x11);
    
    outb(PIC1_DATA, 0x20); // ICW2 - vector offset для master PIC
    outb(PIC2_DATA, 0x28); // ICW2 - vector offset для slave PIC
    
    outb(PIC1_DATA, 0x04); // ICW3 - tell master that slave is at IRQ2
    outb(PIC2_DATA, 0x02); // ICW3 - tell slave its cascade identity
    
    outb(PIC1_DATA, 0x01); // ICW4 - 8086 mode
    outb(PIC2_DATA, 0x01);
    
    // Маскуємо всі переривання окрім клавіатури (IRQ1)
    outb(PIC1_DATA, 0xFD); // 11111101 - дозволяємо тільки IRQ1
    outb(PIC2_DATA, 0xFF); // блокуємо всі переривання slave PIC
    
    // Ініціалізація IDT для клавіатури (IRQ1 = interrupt 33)
    uint32_t* idt = (uint32_t*)0x1000; // Адреса IDT
    uint32_t handler_addr = (uint32_t)keyboard_interrupt_handler;
    
    // Запис у IDT для interrupt 33 (IRQ1)
    idt[33 * 2] = (handler_addr & 0xFFFF) | (0x08 << 16); // Segment selector
    idt[33 * 2 + 1] = (handler_addr & 0xFFFF0000) | 0x8E00; // Type and attributes
    
    // Завантажуємо IDT
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) idtr = {
        .limit = 256 * 8 - 1,
        .base = 0x1000
    };
    
    asm volatile("lidt %0" :: "m"(idtr));
    
    enable_interrupts();
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    // Простий скан-код до ASCII конвертер (тільки для основних клавіш)
    static char scancode_to_ascii[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };
    
    if (scancode < sizeof(scancode_to_ascii) && scancode_to_ascii[scancode] != 0) {
        char c = scancode_to_ascii[scancode];
        
        if (c == '\n') {
            terminal_putchar('\n');
            input_buffer[input_index] = '\0';
            process_command(input_buffer);
            input_index = 0;
            terminal_writestring("nexus> ");
        } else if (c == '\b') {
            if (input_index > 0) {
                input_index--;
                terminal_putchar('\b');
            }
        } else if (input_index < sizeof(input_buffer) - 1) {
            input_buffer[input_index++] = c;
            terminal_putchar(c);
        }
    }
}

// === SHELL ФУНКЦІЇ ===

void shell_initialize(void) {
    input_index = 0;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("Nexus OS v0.1 - Готова до роботи!\n");
    terminal_writestring("Введіть 'help' для списку команд.\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("nexus> ");
}

void shell_run(void) {
    // Головний цикл shell - просто чекаємо переривання
    while (1) {
        asm volatile("hlt"); // Очікуємо переривання
    }
}

void process_command(const char* command) {
    // Пропускаємо пробіли на початку
    while (*command == ' ') command++;
    
    if (strlen(command) == 0) {
        return;
    }
    
    if (strcmp(command, "help") == 0) {
        show_help();
    } else if (strcmp(command, "clear") == 0) {
        terminal_clear();
        show_logo();
        terminal_writestring("nexus> ");
        return; // Не показуємо промпт знову
    } else if (strcmp(command, "shutdown") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Вимкнення системи...\n");
        shutdown();
    } else if (strcmp(command, "reboot") == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
        terminal_writestring("Перезавантаження системи...\n");
        reboot();
    } else if (strcmp(command, "rand") == 0) {
        int num = random_number();
        char buffer[16];
        itoa(num, buffer, 10);
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
        terminal_writestring("Випадкове число: ");
        terminal_writestring(buffer);
        terminal_writestring("\n");
    } else if (strncmp(command, "echo ", 5) == 0) {
        const char* text = command + 5;
        
        // Перевіряємо чи це математичний вираз
        if (strlen(text) > 2 && text[0] == '"' && text[strlen(text)-1] == '"') {
            // Видаляємо лапки
            char expr[256];
            strncpy(expr, text + 1, strlen(text) - 2);
            expr[strlen(text) - 2] = '\0';
            
            // Перевіряємо на математичний вираз
            int result = parse_math_expression(expr);
            if (result != -999999) { // -999999 = помилка парсингу
                char buffer[32];
                itoa(result, buffer, 10);
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
                terminal_writestring("Результат: ");
                terminal_writestring(buffer);
                terminal_writestring("\n");
            } else {
                // Просто виводимо текст
                terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_YELLOW, VGA_COLOR_BLACK));
                terminal_writestring(expr);
                terminal_writestring("\n");
            }
        } else {
            terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_YELLOW, VGA_COLOR_BLACK));
            terminal_writestring(text);
            terminal_writestring("\n");
        }
    } else {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Невідома команда: ");
        terminal_writestring(command);
        terminal_writestring("\n");
        terminal_writestring("Введіть 'help' для списку команд.\n");
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

void show_help(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("=== NEXUS OS v0.1 - ДОВІДКА ===\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Доступні команди:\n");
    terminal_writestring("  help        - показати цю довідку\n");
    terminal_writestring("  clear       - очистити екран\n");
    terminal_writestring("  shutdown    - вимкнути систему\n");
    terminal_writestring("  reboot      - перезавантажити систему\n");
    terminal_writestring("  rand        - згенерувати випадкове число (0-99)\n");
    terminal_writestring("  echo \"текст\" - вивести текст\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("\nМатематичні операції в echo:\n");
    terminal_writestring("  echo \"5+3\"   - додавання\n");
    terminal_writestring("  echo \"10-4\"  - віднімання\n");
    terminal_writestring("  echo \"6*7\"   - множення\n");
    terminal_writestring("  echo \"20/4\"  - ділення\n\n");
}

void show_logo(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    terminal_writestring(" * *________ ___ *_*___ \n");
    terminal_writestring("| \\ | | ____\\ \\ / / | \\/ |/ ____| \n");
    terminal_writestring("| \\| | |__ \\ V /| | \\ / | (___ \n");
    terminal_writestring("| . ` | **| > < | | |\\/| |\\**_ \\ \n");
    terminal_writestring("| |\\ | |____ / . \\| | | |____) |\n");
    terminal_writestring("|_| \\_|______/_/ \\_\\_| |_||_____/\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("                    OS v0.1\n\n");
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
}

// === МАТЕМАТИЧНІ ФУНКЦІЇ ===

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    if (b == 0) {
        terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
        terminal_writestring("Помилка: Ділення на нуль!\n");
        return 0;
    }
    return a / b;
}

int random_number(void) {
    random_seed = random_seed * 1103515245 + 12345;
    return (random_seed / 65536) % 100;
}

int parse_math_expression(const char* expr) {
    int num1 = 0, num2 = 0;
    char op = 0;
    int i = 0;
    
    // Парсимо перше число
    while (expr[i] && expr[i] >= '0' && expr[i] <= '9') {
        num1 = num1 * 10 + (expr[i] - '0');
        i++;
    }
    
    // Парсимо оператор
    if (expr[i] && (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/')) {
        op = expr[i];
        i++;
    } else {
        return -999999; // Помилка - не математичний вираз
    }
    
    // Парсимо друге число
    while (expr[i] && expr[i] >= '0' && expr[i] <= '9') {
        num2 = num2 * 10 + (expr[i] - '0');
        i++;
    }
    
    // Перевіряємо чи дійшли до кінця
    if (expr[i] != '\0') {
        return -999999; // Помилка - зайві символи
    }
    
    // Виконуємо операцію
    switch (op) {
        case '+': return add(num1, num2);
        case '-': return subtract(num1, num2);
        case '*': return multiply(num1, num2);
        case '/': return divide(num1, num2);
        default: return -999999;
    }
}

// === СИСТЕМНІ ФУНКЦІЇ ===

void shutdown(void) {
    disable_interrupts();
    shutdown_system();
}

void reboot(void) {
    disable_interrupts();
    reboot_system();
}

// === УТИЛІТАРНІ ФУНКЦІЇ ===

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strcpy(char* dest, const char* src) {
    char* orig_dest = dest;
    while ((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    while (n-- && (*dest++ = *src++));
    while (n-- > 0) *dest++ = '\0';
    return orig_dest;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    if (*str == '-') {
        sign = -1;
        str++;
    }
    
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return sign * result;
}

void itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    if (value < 0 && base == 10) {
        value = -value;
        *ptr++ = '-';
    }
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * base)];
    } while (value);
    
    *ptr-- = '\0';
    
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}