/*
 * TinyStreetLight.c
 *
 * Created: 2/6/2015 12:33:25 AM
 *  Author: eric
 */ 
#include <string.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h> 
#include "uart.h"
#include "main.h"
#include "stdlib.h"

// Light pins on PortB
#define RED_PIN 7
#define YELLOW_PIN 6
#define GREEN_PIN 5
#define BLINKY_PIN1 4
#define BLINKY_PIN0 3

#define HELP_LINES_COUNT 6
const char HELP_LINES_0[] PROGMEM = "Commands:\n";
const char HELP_LINES_1[] PROGMEM = " ping\n";
const char HELP_LINES_2[] PROGMEM = " light <green|y";
const char HELP_LINES_3[] PROGMEM = "ellow|red> <on|o";
const char HELP_LINES_4[] PROGMEM = "ff>\n";
const char HELP_LINES_5[] PROGMEM = " stats\n\n";
char help_lines_i = 0;

void putnibble(uint8_t val) {
	if (val >= 10) {
		uart_putc('a' - 10 + val);
		} else {
		uart_putc('0' + val);
	}
}

void puthex(uint8_t val) {
	putnibble(val >> 4);
	putnibble(val & 0xf);
}

void putdec(uint8_t val) {
	char num = 100;
	
	char acc = '0';
	while (val > num) {
		acc++;
		val -= num;
	}
	uart_putc(acc);
	
	acc = '0';
	num = 10;
	while (val > num) {
		acc++;
		val -= num;
	}
	uart_putc(acc);
	
	num = 1;
	acc = '0';
	while (val > num) {
		acc++;
		val -= num;
	}
	uart_putc(acc);
}

void handle_command(char *commandstr) {
	char *tok = strtok(commandstr, " ");
	if (strcmp_P(tok, PSTR("ping")) == 0) {
		state = idle;
		uart_pgm_puts(PSTR("> ping? pong!\n"));
		
	} else if (strcmp_P(tok, PSTR("light")) == 0) {
		tok = strtok(NULL, " ");
		char *tok2 = strtok(NULL, " ");	
		
		if (!tok2) {
			help_lines_i = 0;
		}
		
		uint8_t pin = -1;
		if (strcmp_P(tok, PSTR("red")) == 0) {
			pin = RED_PIN;
		} else if (strcmp_P(tok, PSTR("yellow")) == 0) {
			pin = YELLOW_PIN;
		} else if (strcmp_P(tok, PSTR("green")) == 0) {
			pin = GREEN_PIN;
		} 
		uint8_t set_on = (strcmp_P(tok2, PSTR("on")) == 0);
		
		state = idle;
		if (pin == -1) {
			uart_pgm_puts(PSTR("> invalid light\n"));
			return;
		}
		
		uart_pgm_puts(PSTR("> light "));
				
		if (set_on) {
			PORTB = PORTB | (1<<pin);
			uart_pgm_puts(PSTR("on\n"));
		} else {
			PORTB = PORTB & (~(1<<pin));
			uart_pgm_puts(PSTR("off\n"));
		}
		
	} else if (strcmp_P(tok, PSTR("stats")) == 0) {
		state = idle;
		uart_pgm_puts(PSTR("> ex_err:"));
		putdec(rx_errors);
		uart_putc('\n');
				
	} else {
		state = idle;
		help_lines_i = 0;
	}
}
	
int main(void) {
	int loopcounter=0;
	char counter=0;

	DDRB = 0xFF;
	PORTB = 0x00;
	PORTB = PORTB | (1<<BLINKY_PIN1);
	PORTB = PORTB & ~(1<<BLINKY_PIN0);
	
	init_uart();
	sei();

	for (int i=1; i!=0; i++) {
		state = idle;
	}
	
    while(1) {
		if (state == command_ready) {
			handle_command((char*)buffer);
		}
		if (state == idle && help_lines_i < HELP_LINES_COUNT) {
			switch (help_lines_i++) {
				case 0: 
					uart_pgm_puts(HELP_LINES_0);
					break;
				case 1:
					uart_pgm_puts(HELP_LINES_1);
					break;
				case 2:
					uart_pgm_puts(HELP_LINES_2);
					break;
				case 3:
					uart_pgm_puts(HELP_LINES_3);
					break;
				case 4:
					uart_pgm_puts(HELP_LINES_4);
					break;
				case 5:
					uart_pgm_puts(HELP_LINES_5);
					break;
			}
		}
		
		
		if (++loopcounter == 0) {
			PORTB = PORTB ^ ((1<<BLINKY_PIN1) | (1<<BLINKY_PIN0));
			if (counter++ == 20) {
				counter=0;
				uart_pgm_puts(PSTR("> ping!\n"));
			}
		}
    }
}