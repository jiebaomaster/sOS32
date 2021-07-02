#!Makefile

C_SOURCES = $(shell find . -name "*.c")
C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
S_SOURCES = $(shell find . -name "*.s")
S_OBJECTS = $(patsubst %.s, %.o, $(S_SOURCES))

CC = gcc
LD = ld
ASM = nasm

# -m32 生成32位代码，这样的话开发环境也可以使用64位的Linux系统
# -ggdb 和-gstabs+ 添加相关的调试信息，调试对后期的排错很重要
# -nostdinc 不包含C语言的标准库里的头文件
# -fno-builtin 要求gcc不主动使用自己的内建函数，除非显式声明
# -fno-stack-protector 不使用栈保护等检测。
C_FLAGS = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-pic -fno-builtin -fno-stack-protector -I include
# -T scripts/kernel.ld 使用我们自己的链接器脚本。
# -m elf_i386 生成i386平台下的ELF格式的可执行文件
# -nostdlib 不链接C语言的标准库
LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -nostdlib
ASM_FLAGS = -f elf -g -F stabs

all: $(S_OBJECTS) $(C_OBJECTS) link update_image

.c.o:
	@echo 编译代码文件 $< ...
	$(CC) $(C_FLAGS) $< -o $@

.s.o:
	@echo 编译汇编文件 $< ...
	$(ASM) $(ASM_FLAGS) $<

link:
	$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) -o bbcore

.PHONY:clean
clean:
	$(RM) $(S_OBJECTS) $(C_OBJECTS) bbcore

.PHONY:update_image
update_image:
	mkdir _kernel
	sudo mount floppy.img ./_kernel
	sudo cp bbcore ./_kernel/hx_kernel
	sleep 1
	sudo umount ./_kernel

.PHONY:qemu
qemu:
	qemu-system-i386 -fda floppy.img -boot a

.PHONY:debug
debug:
	qemu-system-i386 -S -s -fda floppy.img -boot a &
	sleep 1
	cgdb -x scripts/gdbinit