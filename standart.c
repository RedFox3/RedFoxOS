#include "standart.h"
#include "kernel.h"

int atoi(char a[]) {
	int res = 0;
	
	int size = 0;
	while (a[size] != '\0')
		size++;
	
	int sqrt = 1;
	for (int i = 1; i < size; i++) {
		sqrt *= 10;
	}
	
	for (int i = 0; i < size; i++) {
		res += (a[i] - '0') * sqrt;		
		sqrt /= 10;
	}
	
	return res;
}

char res[80];
unsigned char res_loc;
char* itoa(int a) {
	for (int i = 0; i < 80; i++)
		res[i] = 0;
	res_loc = 0;
	
	if (a < 0) {
		res[res_loc++] = '-';
		a *= -1;
	}
	
	if (a < 10) {
		res[res_loc++] = a + '0';
		return res;
	}
	
	int desyatok = -1;
	int t = a;
	while (t > 9) {
		t /= 10;
		desyatok++;
	}
	
	int sqrt = 10;
	for (int i = 0; i < desyatok; i++)
		sqrt *= 10;
	
	while (sqrt > 1) {
		res[res_loc++] = ((a / sqrt) % 10) + '0';
		sqrt /= 10;
	}
	
	res[res_loc++] = (a % 10) + '0';
	
	return res;
}

unsigned char compare(char f[], char s[]) {
	int i = 0;
	while (f[i] != '\0' && s[i] != '\0') {
		if (f[i] != s[i])
			return 0;
		i++;
	}
	return 1;
}

int calc(char exp[]) {
	typedef unsigned char byte;
	
	char sym = exp[5];
	byte status = 1; /* first1 flag2 second4 */
	char first[80]; byte first_loc = 0;
	char second[80]; byte second_loc = 0;
	byte flag; /* +1 -2 *4 /8 */
	int i = 6;
	while(sym != '\0') {
		if (sym == ' ')
			status = (byte)(status << 1);
		else if (status & 0x01)
			first[first_loc++] = sym;
		else if (status & 0x02) {
			switch (sym) {
				case '+': flag = 0x01; break;
				case '-': flag = 0x02; break;
				case '*': flag = 0x04; break;
				case '/': flag = 0x08; break;
				default: return 0; // здесь должно быть оповещение об ошибке
			}
		}
		else if (status & 0x04)
			second[second_loc++] = sym;
		sym = exp[i++];
	}
	int f = 0, s = 0, a;
	char t;
	
	first[first_loc++] = '\0';
	f = atoi(first);
	
	second[second_loc++] = '\0';
	s = atoi(second);

	switch (flag) {
		case 0x01: a = f + s; break;
		case 0x02: a = f - s; break;
		case 0x04: a = f * s; break;
		case 0x08: a = f / s; break;
	}
	return a;	
}
