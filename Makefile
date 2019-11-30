CFLAGS=-fno-stack-protector

all: kernel

kernel: kasm.o kc.o std.o pgm.o
	ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o std.o pgm.o

kasm.o: kernel.asm
	nasm -f elf32 kernel.asm -o kasm.o
	
kc.o: kernel.c
	gcc $(CFLAGS) -m32 -c kernel.c -o kc.o

std.o: standart.c
	gcc $(CFLAGS) -m32 -c standart.c -o std.o
	
pgm.o: pseudo_graphic_mode.c
	gcc $(CFLAGS) -m32 -c pseudo_graphic_mode.c -o pgm.o
	
clean:
	rm -rf *.o kernel