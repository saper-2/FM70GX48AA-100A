/*
 * FM70GX48VFD-PAR.c
 *
 * Created: 2016-11-29 20:31:23
 * Author : saper
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "lib/delay.h"
#include "lib/usart.h"
#include "lib/FM70GX48par.h"

// set to 1 to enable extra debug messages
#define DEBUG_MODE 1

#define led_toggle PORTB ^= 0x01
#define led_off PORTB &= ~0x01
#define led_on PORTB |= 0x01

#define RS_BUFF_SIZE 24
uint8_t rsBuffPtr = 0;
uint8_t rsBuff[RS_BUFF_SIZE];

void flushRSBuff(void) {
	for (uint8_t i=0;i<RS_BUFF_SIZE;i++) rsBuff[i]=0;
	rsBuffPtr=0;
	#if DEBUG_MODE == 1
	usart_send_strP(PSTR("<<<RS-FLUSH>>>"));
	#endif
}


uint8_t hex2dec(char hex) {
	if (hex > 0x2f && hex < 0x3a) return hex-0x30;
	if (hex > 0x40 && hex < 0x47) return hex-0x37;
	if (hex > 0x60 && hex < 0x67) return hex-0x57;
	return 0x0f;
}

uint8_t bcd2bin(uint8_t bcd) {
	uint8_t res=(bcd&0x0f);
	bcd >>= 4;
	res += (bcd&0x0f)*10;
	return res;
}

void usart_crlf(void) {
	usart_send_char(0x0d);
	usart_send_char(0x0a);
}

void processRSBuff(void) {
	uint8_t t8=0;
	uint16_t t16=0;
	
	if (rsBuff[0] == 'a') {
		t16 = hex2dec(rsBuff[1])<<12 | hex2dec(rsBuff[2])<<8 | hex2dec(rsBuff[3])<<4 | hex2dec(rsBuff[4]);
		usart_send_char('a');
		usart_send_hex_byte(t16>>8);
		usart_send_hex_byte(t16);
		fm_address(t16);
	} else if (rsBuff[0] == 'd') {
		t8 = hex2dec(rsBuff[1])<<4 | hex2dec(rsBuff[2]);
		usart_send_char('d');
		usart_send_hex_byte(t8);
		fm_data_write(t8);
	} else if (rsBuff[0] == 'c') {
		usart_send_char('c');
		fm_clear();
		fm_clear_buff(0x00);
	} else if (rsBuff[0] == 'i') {
		t8 = hex2dec(rsBuff[1])<<4 | hex2dec(rsBuff[2]);
		usart_send_char('i');
		usart_send_hex_byte(t8);
		fm_icon_set_direct(t8);
	} else if (rsBuff[0] == 'b') {
	t8 = hex2dec(rsBuff[1])<<4 | hex2dec(rsBuff[2]);
	usart_send_char('b');
	usart_send_hex_byte(t8);
	fm_icon_bar_set(t8);
	} else if (rsBuff[0] == 'd') {
	} else if (rsBuff[0] == 'd') {
	}

	usart_crlf();
	flushRSBuff();
}

int main(void) {
	
	uint8_t x, y;
	
	DDRB |= 1<<0;
    
    sei();
	// 38400 8N1
    usart_config(25,USART_RX_ENABLE|USART_TX_ENABLE|USART_RX_INT_COMPLET,USART_MODE_8N1);
    

	usart_send_strP(PSTR("\r\n\r\n********** FM70GX48AA **********\r\n"));
	usart_send_strP(PSTR("********** VFD 70x48px w/h red line + 17 icons **********\r\n"));
	
	usart_send_strP(PSTR("Pre-init delay (for fun :) )"));
	for (x=0;x<25;x++) {
		delay1ms(200);
		usart_send_char('.');
	}
    
	usart_send_strP(PSTR("\r\nDisplay init...\r\n"));
	fm_init();

	usart_send_strP(PSTR("Cleaning buffers...\r\n"));
	fm_clear_buff(0x00);
	fm_clear();

	usart_send_strP(PSTR("One-pixel fill test (every pixel whole buffer is sent to display (70*48/8=420bytes))...\r\n"));
	// one pixel fill test
	for (y=0;y<FM_HEIGHT;y++) {
		for (x=0;x<FM_WIDTH;x++) {
			fm_setPix(x,y,FM_PX_ON);
			fm_update();
			//usart_send_char('.');
			//delay1ms(20);
		}
	}

	usart_send_strP(PSTR("Icon test (icon++)...\r\n"));
	for (x=0;x<255;x++) {
		fm_icon_set_direct(x);
		fm_icon_bar_set(x);
		delay1ms(100);
	}
	// last step
	fm_icon_set_direct(0xff);
	fm_icon_bar_set(0xff);
	delay1ms(100);
	// */

    while(1) {
	    delay1ms(50);
	    led_toggle;
	    if (rsBuffPtr == 254) {
		    processRSBuff();
	    }
    }
}



SIGNAL(USART_RXC_vect) {
	char c = UDR;
	
	if (c == 0x0d) { // CR
		rsBuffPtr=254; // mark as buffer to be processed
		} else if (c == 0x1b) { // ESC
		flushRSBuff();
		} else {
		if (rsBuffPtr<RS_BUFF_SIZE) {
			rsBuff[rsBuffPtr]=c;
			rsBuffPtr++;
		}
	}
}


