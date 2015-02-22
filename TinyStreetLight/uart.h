/*
 * IncFile1.h
 *
 * Created: 9/1/2014 5:04:59 PM
 *  Author: eric
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdio.h>

#define TXD PD1
#define RXD PD0

enum {
	idle, receiving, transmitting, command_ready
};

void init_uart(void);
void uart_putc(char c);
void uart_puts(char *s);
void uart_pgm_puts(const char *s);

extern volatile char buffer[];
extern volatile char buffer_pos;

extern volatile char state;
extern volatile char rx_errors;
#endif /*UART_H_*/


