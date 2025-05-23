# Nexus OS v0.1 Makefile

# Компілятори та інструменти
ASM = nasm
CC = gcc
LD = ld
QEMU = qemu-system-x86_64

# Прапори компіляції
ASMFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -nostdlib -nostdinc -fno-builtin -fno-stack-protector -fno-pic
LDFLAGS = -m elf_i386 -T linker.ld --nmagic

# Файли
ASM_SOURCES = kernel.asm
C_SOURCES = kernel.c
OBJECTS = kernel_asm.o kernel.o
TARGET = nexus.bin
ISO = nexus.iso

# Головна ціль
all: $(TARGET)

# Збірка ядра
$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Компіляція асемблерного файлу
kernel_asm.o: kernel.asm
	$(ASM) $(ASMFLAGS) -o $@ $<

# Компіляція C файлу
kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Створення ISO образу
iso: $(TARGET)
	mkdir -p iso/boot/grub
	cp $(TARGET) iso/boot/
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "Nexus OS v0.1" {' >> iso/boot/grub/grub.cfg
	echo '    multiboot2 /boot/$(TARGET)' >> iso/boot/grub/grub.cfg
	echo '    boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) iso

# Запуск в QEMU
run: $(TARGET)
	$(QEMU) -kernel $(TARGET) -serial stdio -m 512M

# Запуск з ISO
run-iso: iso
	$(QEMU) -cdrom $(ISO) -serial stdio -m 512M

# Налагодження
debug: $(TARGET)
	$(QEMU) -kernel $(TARGET) -s -S -serial stdio -m 512M

# Налагодження ISO
debug-iso: iso
	$(QEMU) -cdrom $(ISO) -s -S -serial stdio -m 512M

# Очищення
clean:
	rm -f $(OBJECTS) $(TARGET) $(ISO)
	rm -rf iso

# Перевірка залежностей
check-deps:
	@echo "Перевірка залежностей..."
	@which $(ASM) >/dev/null || (echo "Помилка: NASM не знайдено" && false)
	@which $(CC) >/dev/null || (echo "Помилка: GCC не знайдено" && false)
	@which $(LD) >/dev/null || (echo "Помилка: LD не знайдено" && false)
	@which $(QEMU) >/dev/null || (echo "Попередження: QEMU не знайдено - запуск неможливий")
	@echo "Всі залежності знайдено!"

# Встановлення залежностей (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install build-essential nasm qemu-system-x86 grub-pc-bin xorriso

# Показати розмір ядра
size: $(TARGET)
	@echo "Розмір скомпільованого ядра:"
	@ls -lh $(TARGET)
	@file $(TARGET)

# Дамп секцій
objdump: $(TARGET)
	objdump -h $(TARGET)

# Показати інформацію про проект
info:
	@echo "Nexus OS v0.1 - Проста операційна система"
	@echo "=========================================="
	@echo "Команди:"
	@echo "  make           - збірка ядра"
	@echo "  make run       - запуск в QEMU (без GRUB)"
	@echo "  make iso       - створення ISO образу"
	@echo "  make run-iso   - запуск ISO в QEMU"
	@echo "  make debug     - запуск з налагодженням"
	@echo "  make debug-iso - налагодження ISO"
	@echo "  make clean     - очищення файлів збірки"
	@echo "  make size      - показати розмір ядра"
	@echo "  make objdump   - показати секції ядра"
	@echo "  make check-deps - перевірка залежностей"

# Додаткові цілі
.PHONY: all run run-iso debug debug-iso clean check-deps install-deps info iso size objdump