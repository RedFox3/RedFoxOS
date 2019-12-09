#ifndef KERNEL_H
#define KERNEL_H

void kprint_to(int, int, char[], unsigned char);
void paint_to(int, int, int, int, unsigned char);
void set_theme(unsigned char);
void set_keyboard_handler(void (*)(char));

#endif
