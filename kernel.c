#include "kernel.h"
#include "keyboard_map.h"
#include "shift_keyboard_map.h"
#include "standart.h"
#include "pseudo_graphic_mode.h"

/* there are 25 lines each of 80 columns; each element takes 2 bytes */
#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C
#define BACKSPACE_KEY_CODE 0x0E
#define LEFT_SHIFT 0x2A
#define CAPS_LOCK 0x3A
#define UP 0x48
#define LEFT 0x4B
#define RIGHT 0x4D
#define DOWN 0x50

extern unsigned char keyboard_map[128];
extern unsigned char shift_keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern void shutdown(void);

/* current cursor location */
unsigned int current_loc = 0;
/* current term location */
unsigned int term_loc = 0;
/* font theme */
unsigned int theme = 0x07;
/* command variable */
char command[80];
/* command location */
unsigned int command_loc = 0;
/* video memory begins at address 0xb8000 */
char *vidptr = (char*)0xb8000;

const char *uname = "Welcome to RedFoxOS!";
/* terminal style */
const char *term = ">";

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];


void idt_init(void)
{
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	/* populate IDT entry of keyboard's interrupt */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Ports
	*	 PIC1	PIC2
	*Command 0x20	0xA0
	*Data	 0x21	0xA1
	*/

	/* ICW1 - begin initialization */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - remap offset address of IDT */
	/*
	* In x86 protected mode, we have to remap the PICs beyond 0x20 because
	* Intel have designated the first 32 interrupts as "reserved" for cpu exceptions
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - setup cascading */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - environment info */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Initialization finished */

	/* mask interrupts */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* fill the IDT descriptor */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void kb_init(void)
{
	/* 0xFD is 11111101 - enables only IRQ1 (keyboard)*/
	write_port(0x21 , 0xFD);
}

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = theme;
	}
}

void kprint_symbol(char sym)
{
	vidptr[current_loc++] = sym;
	vidptr[current_loc++] = theme;
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void kprint_to(int x, int y, char input[], unsigned char style) {
	int coordinate = 80 * 2 * y + x * 2;
	int i = 0;
	while (input[i] != '\0') {
		vidptr[coordinate++] = input[i];
		vidptr[coordinate++] = style;
		i++;
	}
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = theme;
	}
			
	command[0] = ' ';
	command_loc = 0;
	
	current_loc = 0; 
	kprint(uname);
	kprint_newline();
	
	kprint_newline();
	kprint(term);
	term_loc = current_loc;
}

void paint_to(int x1, int y1, int x2, int y2, unsigned char color) {
	int coordinate1 = 80 * 2 * y1 + x1 * 2 + 1;
	int coordinate2 = 80 * 2 * y2 + x2 * 2 + 1;
	while (coordinate1 <= coordinate2) {
		vidptr[coordinate1] = color;
		coordinate1 += 2;
	}
}

void set_theme(unsigned char color) {
	theme = color;
}

char commands[8][80] = { "help", "say", "shutdown", "exit", "clear", "calc", "pgm", "ru" };

unsigned char caps = 0;
unsigned char shift = 0;
void keyboard_handler_main(void)
{
	unsigned char status;
	char keycode;

	/* write EOI (End Of Interruption) */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Lowest bit of status will be set if buffer is not empty */
	
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if(keycode < 0) {
			
			if (keycode == (-128 + LEFT_SHIFT))
				shift = 0;
			
			return;
		}

		if (pgm_get_status() == 1) {
			pgm_keyboard_handler(keycode);
			return;
		} else if(keycode == ENTER_KEY_CODE) {
			command[command_loc++] = '\0';
			
			kprint_newline();
			
			int com = -1;
			
			for (int i = 0; i < 8; i++) {
				if (compare(command, commands[i])) {
					com = i;
					break;
				}
			}
			
			switch (com) {
				case 0: help(); break;
				case 1: say(command); break;
				case 2: case 3: kprint("Shutting down"); shutdown(); break;
				case 4: 
					clear_screen(); 
					return;
					break;
				case 5: calc(command); break;
				case 6: pgm(); return; break;
				case 7: kprint("Проверка русского языка"); break;
				default: kprint("Wrong command!");
			}
			
			kprint_newline();
			kprint(term);
			term_loc = current_loc;
			
			command[0] = ' ';
			command_loc = 0;
			return;
		} else if (keycode == BACKSPACE_KEY_CODE) {
			if (current_loc > term_loc) {
				vidptr[--current_loc] = theme;
				vidptr[--current_loc] = ' ';
			
				command[--command_loc] = ' ';
			}
			return;
		} else if (keycode == LEFT_SHIFT) {
			shift = 1;
			return;
		} else if (keycode == CAPS_LOCK) {
			if (caps & 1)
				caps = 0;
			else
				caps = 1;
			return;
		}
		
		if (caps ^ shift) {
			command[command_loc++] = shift_keyboard_map[(unsigned char) keycode];
			kprint_symbol(shift_keyboard_map[(unsigned char) keycode]);
		} else {
			command[command_loc++] = keyboard_map[(unsigned char) keycode];
			kprint_symbol(keyboard_map[(unsigned char) keycode]);
		}
	}
}

void kmain(void)
{
	clear_screen();
	/*kprint(uname);
	kprint_newline();
	kprint_newline();
	kprint(term);
	term_loc = current_loc;*/

	idt_init();
	kb_init();

	while(1);
}
