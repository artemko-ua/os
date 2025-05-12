# Makefile for Nexus OS (x86_64)

# Compilers and tools
CC = gcc
NASM = nasm
LD = ld

# Compiler flags
CFLAGS = -m64 -ffreestanding -fno-stack-protector -nostdlib -nodefaultlibs -Wall -Wextra
NASMFLAGS = -f elf64
LDFLAGS = -nostdlib -nodefaultlibs -T linker.ld

# Source files
KERNEL_SRCS = kernel.c
ASM_SRCS = kernel.asm

# Object files
KERNEL_OBJS = kernel.o
ASM_OBJS = kernel_asm.o

# Target
TARGET = nexus.bin

# Default target
all: $(TARGET) nexus.iso

# Compile C kernel
kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Compile Assembly
kernel_asm.o: kernel.asm
	$(NASM) $(NASMFLAGS) kernel.asm -o kernel_asm.o

# Link kernel
$(TARGET): $(KERNEL_OBJS) $(ASM_OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(ASM_OBJS) $(KERNEL_OBJS)

# Create bootable ISO
nexus.iso: $(TARGET)
	mkdir -p iso/boot/grub
	cp $(TARGET) iso/boot/
	echo "menuentry 'Nexus OS' {" > iso/boot/grub/grub.cfg
	echo "    multiboot2 /boot/$(TARGET)" >> iso/boot/grub/grub.cfg
	echo "    boot" >> iso/boot/grub/grub.cfg
	echo "}" >> iso/boot/grub/grub.cfg
	grub-mkrescue -o nexus.iso iso

# Run in QEMU
run: nexus.iso
	qemu-system-x86_64 -cdrom nexus.iso

# Clean up
clean:
	rm -f *.o $(TARGET) nexus.iso
	rm -rf iso

.PHONY: all run clean