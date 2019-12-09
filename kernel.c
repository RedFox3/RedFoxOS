#include "kernel.h"
#include "standart.h"
#include "pseudo_graphic_mode.h"
#include "terminal.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);

/* видео память начинается по адресу 0xb8000 */
char *vidptr = (char*)0xb8000;

void (*current_keyboard_handler)(char) = 0;

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

	/* заполняем таблицу IDT для клавиатурных прерываний */
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	/*     Порты
	*	  PIC1	 PIC2
	*Комманда 0x20	 0xA0
	*Значение 0x21	 0xA1
	*/

	/* ICW1 - начало инициализации */
	write_port(0x20 , 0x11);
	write_port(0xA0 , 0x11);

	/* ICW2 - переназначение адреса смешения IDT */
	/*
	* В x86 защищённом режиме мы должны переназначать PIC дальше 0x20, так как
	* Intel назначила первые 32 прерывания как "зарезервированые" для процессорных исключений
	*/
	write_port(0x21 , 0x20);
	write_port(0xA1 , 0x28);

	/* ICW3 - установка каскадирования */
	write_port(0x21 , 0x00);
	write_port(0xA1 , 0x00);

	/* ICW4 - информация об окружающей среде */
	write_port(0x21 , 0x01);
	write_port(0xA1 , 0x01);
	/* Инициализация завершена */

	/* маска прерываний */
	write_port(0x21 , 0xff);
	write_port(0xA1 , 0xff);

	/* заполнение IDT дескриптора */
	idt_address = (unsigned long)IDT ;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16 ;

	load_idt(idt_ptr);
}

void keyboard_init(void)
{
	/* 0xFD это 11111101 - активирует только IRQ1 (клавиатура) */
	write_port(0x21 , 0xFD);
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

void paint_to(int x1, int y1, int x2, int y2, unsigned char color) {
	int coordinate1 = 80 * 2 * y1 + x1 * 2 + 1;
	int coordinate2 = 80 * 2 * y2 + x2 * 2 + 1;
	while (coordinate1 <= coordinate2) {
		vidptr[coordinate1] = color;
		coordinate1 += 2;
	}
}

void set_keyboard_handler(void (*func)(char)) {
	current_keyboard_handler = func;
}

void kernel_keyboard_handler(void)
{
	unsigned char status;
	char keycode;

	/* запись EOI (конец прерывания) */
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	/* Нижний бит статуса будет установлен, если буфер не пуст */
	
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		
		current_keyboard_handler(keycode);
	}
}

void kmain(void)
{
	idt_init();
	keyboard_init();
	
	terminal();

	while(1);
}
