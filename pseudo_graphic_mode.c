#include "kernel.h"
#include "standart.h"

#define ENTER_KEY_CODE 0x1C
#define UP 0x48
#define LEFT 0x4B
#define RIGHT 0x4D
#define DOWN 0x50
#define F1 0x3B
#define F12 0x58

extern void shutdown(void);

char *vid = (char*)0xb8000;

unsigned char status = 0;
unsigned char button = 0; /* 0 - OK, 1 - Cancel */
unsigned char border_color = 0x70;
unsigned char theme_color = 0x17;

unsigned char pgm_get_status() {
	return status;
}

void pgm_clear_screen(void)
{
	for (int i = 2; i < 24; i++) {
		int j = 2;
		while (j < 80 * 2 - 2) {
			vid[80 * 2 * i + j] = ' ';
			j++;
			vid[80 * 2 * i + j] = theme_color;
			j++;
		}
	}
}

unsigned char prompt_status = 0;
void prompt(char input[]) { // const char str
	paint_to(20, 11, 60, 11, 0x27);
	paint_to(20, 12, 60, 12, 0x27);
	paint_to(20, 13, 60, 13, 0x27);
	paint_to(20, 14, 60, 14, 0x27);
	paint_to(20, 15, 60, 15, 0x27);
	
	kprint_to(25, 12, input, 0x27);
	
	kprint_to(30, 14, "[ OK ]", 0x70);
	kprint_to(40, 14, "[ Cancel ]", 0x30);
	
	prompt_status = 1;
}

void pgm_start(void) {
	kprint_to(1, 24, "F1 [ MENU ]", border_color);
	kprint_to(57, 24, "F12 [ RETURN TO TERM ]", border_color);
}

unsigned char menu_status = 0;
char programs[4][80] = { "Clear screen", "Calc", "Settings", "Shutdown" };
unsigned char menu_choice = 0;

void menu_update() {
	if (menu_status) {
		paint_to(1, 19, 20, 19, 0x27);
		paint_to(1, 20, 20, 20, 0x27);
		paint_to(1, 21, 20, 21, 0x27);
		paint_to(1, 22, 20, 22, 0x27);
		paint_to(1, 23, 20, 23, 0x27);
	
		switch (menu_choice) {
			case 0: paint_to(1, 19, 20, 19, border_color); break;
			case 1: paint_to(1, 20, 20, 20, border_color); break;
			case 2: paint_to(1, 21, 20, 21, border_color); break;
			case 3: paint_to(1, 22, 20, 22, border_color); break;
		}
	}
}

void menu_open(void) {
	if (!menu_status) {
		menu_status = 1;
	
		paint_to(1, 24, 11, 24, 0x27);
	
		paint_to(1, 19, 20, 19, 0x27);
		paint_to(1, 20, 20, 20, 0x27);
		paint_to(1, 21, 20, 21, 0x27);
		paint_to(1, 22, 20, 22, 0x27);
		paint_to(1, 23, 20, 23, 0x27);
	
		kprint_to(1, 19, programs[0], 0x27);
		kprint_to(1, 20, programs[1], 0x27);
		kprint_to(1, 21, programs[2], 0x27);
		kprint_to(1, 22, programs[3], 0x27);
	
		menu_update();
	}
}

void menu_close(void) {
	if (menu_status) {
		menu_status = 0;
		menu_choice = 0;
	
		paint_to(1, 24, 11, 24, border_color);
	
		paint_to(1, 19, 20, 19, theme_color);
		paint_to(1, 20, 20, 20, theme_color);
		paint_to(1, 21, 20, 21, theme_color);
		paint_to(1, 22, 20, 22, theme_color);
		paint_to(1, 23, 20, 23, theme_color);
	
		kprint_to(1, 19, "                   ", theme_color);
		kprint_to(1, 20, "                   ", theme_color);
		kprint_to(1, 21, "                   ", theme_color);
		kprint_to(1, 22, "                   ", theme_color);
	}
}

void pgm_exit() {
	prompt_status = 0;
	status = 0;
	button = 0;
	menu_close();
	clear_screen();
}

void pgm_paint_window(int x1, int y1, int x2, int y2, char name[]) {
	int i = 0;
	while (i < (x2 - x1) * 2) {
		vid[y1 * 80 * 2 + x1 * 2 + i] = ' ';
		vid[y2 * 80 * 2 + x1 * 2 + i] = ' ';
		i++;
		vid[y1 * 80 * 2 + x1 * 2 + i] = border_color;
		vid[y2 * 80 * 2 + x1 * 2 + i] = border_color;
		i++;
	}
	
	for (i = y1 * 80 * 2 + 80 * 2 + x1 * 2; i < y2 * 80 * 2 + x1 * 2; i += 80 * 2) {
		vid[i] = ' ';
		vid[i+1] = border_color;
		vid[i+(x2-x1)*2 - 2] = ' ';
		vid[i+(x2-x1)*2 - 1] = border_color;
	}
	
	
	for (i = y1 * 80 * 2 + 80 * 2 + x1 * 2 + 2; i < y2 * 80 * 2 - 80 * 2 + x2 * 2 - 2; i += 80 * 2) {
		for (int j = 0; j < (x2 - x1 - 2)*2;) {
			vid[i + (j++)] = ' ';
			vid[i + (j++)] = 0x37;
		}
	}
	
	kprint_to(x1+1, y1, name, border_color);
}

// Добавить сохранение экрана если меню перекрывает его
unsigned char calc_status = 0;
unsigned char calc_loc = 17;
int f = 0;
int s = 0;
char action = -1;
void pgm_calc() {
	calc_status = 1;
	button = 0;
	calc_loc = 17;
	f = 0;
	s = 0;
	action = -1;
	
	pgm_clear_screen();
	pgm_paint_window(15, 10, 67, 17, "Calc v0.1");
	
	paint_to(17, 12, 64, 12, border_color);
	
	kprint_to(17, 14, "[ 0 ]", 0x70);
	kprint_to(23, 14, "[ 1 ]", 0x30);
	kprint_to(29, 14, "[ 2 ]", 0x30);
	kprint_to(35, 14, "[ 3 ]", 0x30);
	kprint_to(41, 14, "[ 4 ]", 0x30);
	kprint_to(47, 14, "[ 5 ]", 0x30);
	kprint_to(53, 14, "[ 6 ]", 0x30);
	kprint_to(59, 14, "[CLR]", 0x30);
	
	kprint_to(17, 16, "[ 7 ]", 0x30);
	kprint_to(23, 16, "[ 8 ]", 0x30);
	kprint_to(29, 16, "[ 9 ]", 0x30);
	kprint_to(35, 16, "[ + ]", 0x30);
	kprint_to(41, 16, "[ - ]", 0x30);
	kprint_to(47, 16, "[ * ]", 0x30);
	kprint_to(53, 16, "[ / ]", 0x30);
	kprint_to(59, 16, "[ = ]", 0x30);
}

void pgm_calc_update(unsigned char btn) {
	switch (button) {
		case 0: paint_to(17, 14, 21, 14, 0x30); break;
		case 1: paint_to(23, 14, 27, 14, 0x30); break;
		case 2: paint_to(29, 14, 33, 14, 0x30); break;
		case 3: paint_to(35, 14, 39, 14, 0x30); break;
		case 4: paint_to(41, 14, 45, 14, 0x30); break;
		case 5: paint_to(47, 14, 51, 14, 0x30); break;
		case 6: paint_to(53, 14, 57, 14, 0x30); break;
		case 7: paint_to(59, 14, 63, 14, 0x30); break;
		case 8: paint_to(17, 16, 21, 16, 0x30); break;
		case 9: paint_to(23, 16, 27, 16, 0x30); break;
		case 10: paint_to(29, 16, 33, 16, 0x30); break;
		case 11: paint_to(35, 16, 39, 16, 0x30); break;
		case 12: paint_to(41, 16, 45, 16, 0x30); break;
		case 13: paint_to(47, 16, 51, 16, 0x30); break;
		case 14: paint_to(53, 16, 57, 16, 0x30); break;
		case 15: paint_to(59, 16, 63, 16, 0x30); break;
	}
	
	button = btn;
	switch (button) {
		case 0: paint_to(17, 14, 21, 14, 0x70); break;
		case 1: paint_to(23, 14, 27, 14, 0x70); break;
		case 2: paint_to(29, 14, 33, 14, 0x70); break;
		case 3: paint_to(35, 14, 39, 14, 0x70); break;
		case 4: paint_to(41, 14, 45, 14, 0x70); break;
		case 5: paint_to(47, 14, 51, 14, 0x70); break;
		case 6: paint_to(53, 14, 57, 14, 0x70); break;
		case 7: paint_to(59, 14, 63, 14, 0x70); break;
		case 8: paint_to(17, 16, 21, 16, 0x70); break;
		case 9: paint_to(23, 16, 27, 16, 0x70); break;
		case 10: paint_to(29, 16, 33, 16, 0x70); break;
		case 11: paint_to(35, 16, 39, 16, 0x70); break;
		case 12: paint_to(41, 16, 45, 16, 0x70); break;
		case 13: paint_to(47, 16, 51, 16, 0x70); break;
		case 14: paint_to(53, 16, 57, 16, 0x70); break;
		case 15: paint_to(59, 16, 63, 16, 0x70); break;
	}
}

void pgm_settings() {
	pgm_paint_window(15, 10, 67, 17, "Settings");
	kprint_to(17, 13, "There are nothing", 0x70);
}

void pgm_keyboard_handler(unsigned char key) {
	
	if (prompt_status) {
		if (key == ENTER_KEY_CODE && button == 0) {
			prompt_status = 0;
			pgm_clear_screen();
			pgm_start();
			return;
		} else if (key == ENTER_KEY_CODE && button == 1) {
			pgm_exit();
			return;
		} else if (key == RIGHT && !button) {
			button = 1;
			paint_to(30, 14, 35, 14, 0x30);
			paint_to(40, 14, 49, 14, 0x70);
		} else if (key == LEFT && button) {
			button = 0;
			paint_to(30, 14, 35, 14, 0x70);
			paint_to(40, 14, 49, 14, 0x30);
		}
		return;
	}
	
	if (key == F1) {
		if (!menu_status)
			menu_open();
		else
			menu_close();
		return;
	} if (key == F12) {
		pgm_exit();
		return;
	}
	
	// if change app, = 0 to all vars
	if (menu_status) {
		if (key == ENTER_KEY_CODE) {
			switch (menu_choice) {
				case 0: pgm_clear_screen(); break;
				case 1: pgm_calc(); break;
				case 2: pgm_settings(); break;
				case 3: pgm_exit(); shutdown(); break;
			}
			menu_close();
		} else if (key == UP && menu_choice > 0) {
			menu_choice--;
			menu_update();
		} else if (key == DOWN && menu_choice < 3 /*max*/) {
			menu_choice++;
			menu_update();
		}
		return;
	}
	
	if (calc_status) {
		if (key == ENTER_KEY_CODE) {
			if (calc_loc >= 64)
				button = 7;
			switch (button) {
				case 0: kprint_to(calc_loc++, 12, "0", border_color); 
						if (action == -1)
							f *= 10;
						else
							s *= 10;
						break;
				case 1: kprint_to(calc_loc++, 12, "1", border_color); 
						if (action == -1) {
							f *= 10; f += 1;
						} else {
							s *= 10; s += 1;
						}							
						break;
				case 2: kprint_to(calc_loc++, 12, "2", border_color);  
						if (action == -1) {
							f *= 10; f += 2;
						} else {
							s *= 10; s += 2;
						}
						break;
				case 3: kprint_to(calc_loc++, 12, "3", border_color);  
						if (action == -1) {
							f *= 10; f += 3;
						} else {
							s *= 10; s += 3;
						}
						break;
				case 4: kprint_to(calc_loc++, 12, "4", border_color);  
						if (action == -1) {
							f *= 10; f += 4;
						} else {
							s *= 10; s += 4;
						}
						break;
				case 5: kprint_to(calc_loc++, 12, "5", border_color);  
						if (action == -1) {
							f *= 10; f += 5;
						} else {
							s *= 10; s += 5;
						}
						break;
				case 6: kprint_to(calc_loc++, 12, "6", border_color);  
						if (action == -1) {
							f *= 10; f += 6;
						} else {
							s *= 10; s += 6;
						}
						break;
				case 7: calc_loc = 17;
						f = 0;
						s = 0;
						action = -1;
						kprint_to(calc_loc, 12, "                                               ", border_color);
						break;
				case 8: kprint_to(calc_loc++, 12, "7", border_color);  
						if (action == -1) {
							f *= 10; f += 7;
						} else {
							s *= 10; s += 7;
						}
						break;
				case 9: kprint_to(calc_loc++, 12, "8", border_color);  
						if (action == -1) {
							f *= 10; f += 8;
						} else {
							s *= 10; s += 8;
						}
						break;
				case 10: kprint_to(calc_loc++, 12, "9", border_color);  
						if (action == -1) {
							f *= 10; f += 9;
						} else {
							s *= 10; s += 9;
						}
						break;
				case 11: kprint_to(calc_loc, 12, " + ", border_color);
						calc_loc += 3;
						action = 0;
						break;
				case 12: kprint_to(calc_loc, 12, " - ", border_color);
						calc_loc += 3;
						action = 1;
						break;
				case 13: kprint_to(calc_loc, 12, " * ", border_color);
						calc_loc += 3;
						action = 2;
						break;
				case 14: kprint_to(calc_loc, 12, " / ", border_color);  
						calc_loc += 3;
						action = 4;
						break;
				case 15: kprint_to(calc_loc, 12, " = ", border_color);
						calc_loc += 3;	
						switch (action) {
							case 0: kprint_to(calc_loc++, 12, itoa(f + s), border_color); break;
							case 1: kprint_to(calc_loc++, 12, itoa(f - s), border_color); break;
							case 2: kprint_to(calc_loc++, 12, itoa(f * s), border_color); break;
							case 4: kprint_to(calc_loc++, 12, itoa(f / s), border_color); break;
							//default:
						}
						f = 0;
						s = 0; 
						action = -1;
			}
		} else if (key == UP && button > 7)
			pgm_calc_update(button - 8);
		else if ((key == DOWN && button < 8))
			pgm_calc_update(button + 8);
		else if (key == LEFT && button > 0)
			pgm_calc_update(button - 1);
		else if (key == RIGHT && button < 15)
			pgm_calc_update(button + 1);
		return;
	}
}

void pgm(void) { // pseudo-graphical mode
	paint_to(0, 0, 80, 25, 0x17);
	
	int i = 0;
	while (i < 80 * 2) {
		vid[i++] = ' ';
		vid[i++] = border_color;
	}
	
	for (i = 80 * 2; i < 80 * 25 * 2 - 80 * 2; i += 80 * 2) {
		vid[i] = ' ';
		vid[i+1] = border_color;
		vid[i+158] = ' ';
		vid[i+159] = border_color;
	}
	
	i = 80 * 25 * 2 - 80 * 2;
	while (i < 80 * 25 * 2) {
		vid[i++] = ' ';
		vid[i++] = border_color;
	}
	
	pgm_clear_screen();
	prompt("Do you want to start pgm?");
	status = 1;
}