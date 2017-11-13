/*
 * FM70GX48par.h
 *
 * Created: 2016-11-29 20:41:42
 *  Author: saper
 */ 


#ifndef FM70GX48PAR_H_
#define FM70GX48PAR_H_

#define FM_ADDR07_PORT PORTC
#define FM_ADDR07_DDR DDRC
#define FM_ADDR89_PORT PORTD
#define FM_ADDR89_DDR DDRD
#define FM_ADDR89_PN 6 // pin for A8, A9 will be next one pin

#define FM_DATA_PORT PORTA
#define FM_DATA_DDR DDRA

#define FM_WRITE_PORT PORTD
#define FM_WRITE_DDR DDRD
#define FM_WRITE_PN  2

#define FM_READY_PORT PORTD
#define FM_READY_DDR DDRD
#define FM_READY_PIN PIND
#define FM_READY_PN 3

#define FM_CLEAR_PORT PORTD
#define FM_CLEAR_DDR DDRD
#define FM_CLEAR_PN 4

#define FM_DSPE_PORT PORTB
#define FM_DSPE_DDR DDRB
#define FM_DSPE_PN 1

#define FM_SEG1_PORT PORTB
#define FM_SEG1_DDR DDRB
#define FM_SEG1_PN 2

#define FM_SEG10_PORT PORTB
#define FM_SEG10_DDR DDRB
#define FM_SEG10_PN 3

#define FM_ADDR89_MASK (0x03<<FM_ADDR89_PN)

#define fm_write_lo FM_WRITE_PORT &= ~(1<<FM_WRITE_PN)
#define fm_write_hi FM_WRITE_PORT |=  (1<<FM_WRITE_PN)

#define fm_clear_lo FM_CLEAR_PORT &= ~(1<<FM_CLEAR_PN)
#define fm_clear_hi FM_CLEAR_PORT |=  (1<<FM_CLEAR_PN)

#define fm_disp_on FM_DSPE_PORT |=  (1<<FM_DSPE_PN)
#define fm_disp_off FM_DSPE_PORT &= ~(1<<FM_DSPE_PN)

#define fm_get_ready ((FM_READY_PIN & (1<<FM_READY_PN)) == 0)


#define FM_ADDR_ICON_BARS 0x46
#define FM_ADDR_ICON_BATT_BARS 0xC6
#define FM_ADDR_ICON_BATT_FRAME 0x2C6

#define FM_PX_OFF 0
#define FM_PX_ON 1
#define FM_PX_XOR 2

#define FM_WIDTH 70
#define FM_HEIGHT 48

#define FM_ICON_PLUG 0x01
#define FM_ICON_DROP 0x02
#define FM_ICON_BATT 0x04
#define FM_ICON_BATT_BAR0 0x10 // top bar at battery icon
#define FM_ICON_BATT_BAR1 0x20 
#define FM_ICON_BATT_BAR2 0x40
#define FM_ICON_BATT_BAR3 0x80 // bottom bar at battery icon
#define FM_ICON_ON 1
#define FM_ICON_OFF 0

// init ports & display
void fm_init(void);
// control display ON/OFF (DSPE=1=on / DSPE=0=off)
void fm_disp_onoff(uint8_t on);
// set icon state (0=off, 1=on) on display: plug, drop, battery frame, battery bars
void fm_icon_set(uint8_t icon, uint8_t state);
// update directly icon buffer variable and update display icon states according to buffer variable
void fm_icon_set_direct(uint8_t icon_states);
// set bars each bit represents one bar: LSB=top bar ... MSB=bottom bar
void fm_icon_bar_set(uint8_t bars);
// set a one specific pixel
void fm_setPix(uint8_t x, uint8_t y, uint8_t mode);
// clear display content (by CLEAR signal, buffer is not wiped)
void fm_clear(void);
// clear buffer
void fm_clear_buff(uint8_t pattern);
// update display from graphic buffer
void fm_update(void);


void fm_address(uint16_t addr);
void fm_data_write(uint8_t data);


#endif /* FM70GX48PAR_H_ */