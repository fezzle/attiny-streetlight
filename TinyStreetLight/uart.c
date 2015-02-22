/* -----------------------------------------------------------------------
 * uart.c
 * adapted to ATtiny2313
 */

#include <inttypes.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h> 
#include "uart.h"
#include "main.h"


volatile char state = idle;

volatile char buffer[BUFFER_SIZE];
volatile char buffer_consumepos = 0;
volatile char buffer_pos = 0;
volatile char rx_errors = 0;

/*
 * init_uart
 */
void init_uart(void) {
  // set baud rate
  UBRRH = (char)(MYUBBR >> 8); 
  UBRRL = (char)(MYUBBR);
  // enable transmit
  UCSRB = (1 << TXEN) | (1 << RXEN) | (1<< RXCIE);
  // set frame format
  UCSRC = (1 << UCSZ1) | (1 << UCSZ0);	// asynchrone 8n1

	DDRD = (DDRD | (1 << TXD)) & ~(1 << RXD);
}


/*
 * uart_putc
 * Begins transmitting unless command is ready.
 * valid transitions:  (idle) -> (transmitting)
 */
void uart_putc(char c) {
	
	if (state != transmitting && state != idle) {
		return;
	} else if (state != transmitting) {
		buffer_pos = 0;
		buffer_consumepos = 0;
		state = transmitting;
	}
	
	if (buffer_pos == sizeof(buffer)) {
		return;
	} else {
		buffer[buffer_pos++] = c;
	}
	UCSRB |= (1<<UDRIE);
}


/*
 * uart_puts
 * Sends a string.
 */
void uart_puts(char *s) {
	while (*s) {
		uart_putc(*s);
		s++;
	}
}

void uart_pgm_puts(const char *s) {
	register char c;
	while (c = pgm_read_byte(s++)) {
		uart_putc(c);
	}
}

/*
 * RX receive interrupt sets command_ready when a NL is received.
 */
ISR(USART_RX_vect) {
	if (state != idle && state != receiving) {
		rx_errors++;
		return;
	} else if (state != receiving) {
		buffer_pos = 0;
		buffer_consumepos = 0;
		state = receiving;
	}
	
	char data = UDR;
	
	if (UCSRA & _BV(FE)) {
		rx_errors++;
	}

	if (UCSRA & _BV(DOR)) {
		rx_errors++;
	}

	if (data == '\n') {
		buffer[buffer_pos] = 0;
		state = command_ready;
		return;
	}
	
	if (buffer_pos + 1 == sizeof(buffer)) {
		// must drop
		rx_errors++;
		return;	
	}
	buffer[buffer_pos++] = data;
}

/*
 * ISR User Data Register Empty
 * Send a char out of buffer via UART. If sending is complete, the 
 * interrupt gets disabled.
 */
ISR(USART_UDRE_vect) {
	if (state != transmitting) {
		UCSRB &= ~(1 << UDRIE);
		return;
	}
	if (buffer_consumepos == buffer_pos) {
		buffer_consumepos = 0;
		buffer_pos = 0;
		state = idle;
		UCSRB &= ~(1 << UDRIE);
	} else {
		UDR = buffer[buffer_consumepos++];
	}
}
