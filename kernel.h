#ifndef KERNEL_H
#define KERNEL_H

void kprint(const char*);
void kprint_symbol(char);
void kprint_newline(void);
void kprint_to(int, int, char[], unsigned char);
void clear_screen(void);
void paint_to(int, int, int, int, unsigned char);
void set_theme(unsigned char);

#endif