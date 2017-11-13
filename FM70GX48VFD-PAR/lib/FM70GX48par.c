/*
* FM70GX48par.c
*
* Created: 2016-11-29 22:05:41
*  Author: saper
*/ 

#include <avr/io.h>
#include <inttypes.h>
#include <avr/pgmspace.h>

#include "FM70GX48par.h"
#include "delay.h"
#include "usart.h"

#define FM_BUFF_SIZE ((FM_WIDTH*FM_HEIGHT)/8)

uint8_t fmBuff[FM_BUFF_SIZE];
// fmBuffIcon - each bit represent one icon: 
// 0-Plug , 1-Drop, 2-Battery frame, 3- n/a, 4-Battery bar (counting from top) 0, 5-Batt bar 1, 6-Batt bar 2, 7-Batt bar 3
uint8_t fmBuffIcon;
// fmBuffIconBars - each bit represent one bar, bit0 is top bar, bit7 is bottom
uint8_t fmBuffIconBars;

void fm_address(uint16_t addr) {
	// set address
	FM_ADDR07_PORT = (addr&0x00ff);
	FM_ADDR89_PORT &= ~FM_ADDR89_MASK ; // clear MSB bits
	FM_ADDR89_PORT |= (FM_ADDR89_MASK & ((addr>>8)<<FM_ADDR89_PN)); // set MSB bits
}

void fm_data_write(uint8_t data) {
	
	while (!fm_get_ready) { nop; }
	nop; nop; 
	FM_DATA_PORT = data;
	nop; nop; nop; nop;
	fm_write_lo;
	nop; nop; nop; nop;
	fm_write_hi;
}

void fm_init(void) {
	// reset variables
	for (uint16_t i=0;i<FM_BUFF_SIZE;i++) fmBuff[i]=0;
	fmBuffIcon=fmBuffIconBars=0;
	// init ports
	FM_ADDR07_PORT = 0xFF;
	FM_ADDR07_DDR = 0xFF; // output
	FM_ADDR89_PORT |= FM_ADDR89_MASK;
	FM_ADDR89_DDR |= FM_ADDR89_MASK;
	FM_DATA_PORT = 0xFF;
	FM_DATA_DDR = 0xFF;
	// write
	fm_write_hi;
	FM_WRITE_DDR |= 1<<FM_WRITE_PN;
	// ready
	FM_READY_DDR &= ~(1<<FM_READY_PN); // input
	FM_READY_PORT |= 1<<FM_READY_PN; // pullup
	// clear
	fm_clear_hi;
	FM_CLEAR_DDR |= 1<<FM_CLEAR_PN;
	// DSPE - display enable, switch off display
	fm_disp_off;
	FM_DSPE_DDR |= 1<<FM_DSPE_PN;
	// seg1 & seg10 - off
	FM_SEG1_PORT &= ~(1<<FM_SEG1_PN);
	FM_SEG1_DDR |= 1<<FM_SEG1_PN;
	FM_SEG10_PORT &= ~(1<<FM_SEG10_PN);
	FM_SEG10_DDR |= 1<<FM_SEG10_PN;
	//
	// port setup done, clear
	delay1ms(100); // small delay
	// clear
	fm_clear_lo;
	delay1ms(10);
	fm_clear_hi;
	while (!fm_get_ready) { nop; }
	// done :D

	fm_disp_onoff(1);
}

void fm_disp_onoff(uint8_t on) {
	if (on > 0) {
		fm_disp_on;
	} else {
		fm_disp_off;
	}

}

void fm_icon_set(uint8_t icon, uint8_t state) {
	if (state == FM_ICON_OFF) {
		/*switch (icon) {
			case FM_ICON_PLUG: fmBuffIcon &= ~(0x01); break;
			case FM_ICON_DROP : fmBuffIcon &= ~(0x02); break;
			case FM_ICON_BATT : fmBuffIcon &= ~(0x04); break;
			case FM_ICON_BATT_BAR0 : fmBuffIcon &= ~(0x10); break;
			case FM_ICON_BATT_BAR1 : fmBuffIcon &= ~(0x20); break;
			case FM_ICON_BATT_BAR2 : fmBuffIcon &= ~(0x40); break;
			case FM_ICON_BATT_BAR3 : fmBuffIcon &= ~(0x80); break;
		}*/
		fmBuffIcon &= ~(icon);
	} else {
		/*switch (icon) {
			case FM_ICON_PLUG: fmBuffIcon |= (0x01); break;
			case FM_ICON_DROP : fmBuffIcon |= (0x02); break;
			case FM_ICON_BATT : fmBuffIcon |= (0x04); break;
			case FM_ICON_BATT_BAR0 : fmBuffIcon |= (0x10); break;
			case FM_ICON_BATT_BAR1 : fmBuffIcon |= (0x20); break;
			case FM_ICON_BATT_BAR2 : fmBuffIcon |= (0x40); break;
			case FM_ICON_BATT_BAR3 : fmBuffIcon |= (0x80); break;
		}*/
		fmBuffIcon |= icon;
	}

	fm_icon_set_direct(fmBuffIcon);
}

void fm_icon_set_direct(uint8_t icon_states) {
	fmBuffIcon  = (0xf7 & icon_states);
	// update icon
	if (fmBuffIcon & 0x01) {
		FM_SEG1_PORT |= 1<<FM_SEG1_PN;
		} else {
		FM_SEG1_PORT &= ~(1<<FM_SEG1_PN);
	}
	if (fmBuffIcon & 0x02) {
		FM_SEG10_PORT |= 1<<FM_SEG10_PN;
		} else {
		FM_SEG10_PORT &= ~(1<<FM_SEG10_PN);
	}
	if (fmBuffIcon & 0x04) {
		fm_address(FM_ADDR_ICON_BATT_FRAME);
		fm_data_write(0x01);
		} else {
		fm_address(FM_ADDR_ICON_BATT_FRAME);
		fm_data_write(0x00);
	}
	// icons
	fm_address(FM_ADDR_ICON_BATT_BARS);
	fm_data_write(fmBuffIcon>>4);
}

void fm_icon_bar_set(uint8_t bars) {
	fm_address(FM_ADDR_ICON_BARS);
	fm_data_write(bars);
}

void fm_setPix(uint8_t x, uint8_t y, uint8_t mode) {
	uint8_t b, bit;
	uint16_t adr;

	if (x>=FM_WIDTH) x=FM_WIDTH-1;
	if (y>=FM_HEIGHT) y=FM_HEIGHT-1;
	adr = x+(y/8)*FM_WIDTH;
	bit = y%8;
	b = fmBuff[adr];

	if (mode == FM_PX_OFF) {
		b &= ~(1<<bit);
	} else if (mode == FM_PX_ON) {
		b |= (1<<bit);
	} else if (mode == FM_PX_XOR) {
		if (b&(1<<bit)) {
			// px is on
			b &= ~(1<<bit);
		} else {
			// px is off
			b &= ~(1<<bit);
		}
	}

	// update buffer
	fmBuff[adr] = b;
}


void fm_clear(void) {
	// clear low
	fm_clear_lo;
	nop; nop; nop; nop;
	// return to high
	fm_clear_hi;
	nop; nop; nop; nop;
	// wait until disp is ready
	while (!fm_get_ready) { nop; }
}

void fm_clear_buff(uint8_t pattern) {
	for (uint16_t i=0;i<FM_BUFF_SIZE;i++) fmBuff[i]=pattern;

	if (pattern == 0x00) {
		fm_clear_lo;
		nop; nop; nop; nop;
		fm_clear_hi;
		nop; nop; nop; nop;
		while (!fm_get_ready) { nop; }
	}

}


void fm_update(void) {
	// update pixel buffer
	uint16_t ptr=0, addr;
	for (uint8_t y=0;y<6;y++) {
		for (uint8_t x=0;x<70;x++) {
			addr = (y*128) + x;
			fm_address(addr);
			fm_data_write(fmBuff[ptr]);
			ptr++;
		}
	}
}