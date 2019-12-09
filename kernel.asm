bits 32
section .text
        				;переменные для работы запуска через multiboot
        align 4				;выравнивание для переменных multiboot
        dd 0x1BADB002              	;магические числа для multiboot
        dd 0x00                    	;флаги для multiboot
        dd - (0x1BADB002 + 0x00)   	;проеверочная сумма, м + ф + пс должно равняться нулю

global start				;объявление функций для доступа вне файла
global keyboard_handler
global read_port
global write_port
global load_idt
global shutdown

extern kmain 				;объявление внешних функций из других файлов
extern kernel_keyboard_handler

read_port:
	mov edx, [esp + 4]		;получение входных данных в функцию
					;al это нижние 8 бит eax
	in al, dx			;dx это нижние 16 бит edx
	ret				;al это возвращаемое значение

write_port:
	mov   edx, [esp + 4]    
	mov   al, [esp + 4 + 4]  
	out   dx, al  
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]			;загрузка таблицы IDT
	sti 				;включение прерываний
	ret

keyboard_handler:                 
	call    kernel_keyboard_handler
	iretd				;выход из прерывания

shutdown:
	hlt				;остановка процессора
	ret

start:
	cli 				;блокировка прерываний
	mov esp, stack_space		;инициализация указателя на стек
	call kmain
	hlt 				;остановка процессора

section .bss
resb 8192; 8KB for stack
stack_space:
