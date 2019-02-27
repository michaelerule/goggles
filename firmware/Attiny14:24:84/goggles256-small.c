// DDRx   : 1 = output, 0 = input
// PORTx : output buffer
// PINx  : digital input buffer ( writes toggle portbits )
// Port B is the chip IO port ( other ports are control registers )

// random uselessness I wrote
#define HIGH   1
#define LOW    0
#define INPUT  0
#define OUTPUT 1
#define HI   HIGH
#define LO   LO
#define IN   INPUT
#define OUT  OUTPUT
#define NOP (__no_operation())
#define DIGITAL_OUT_PORT PORTB
#define DIGITAL_IN_PORT  PINB
#define DIGITAL_DIRECTION_PORT DDRB
#define inp(port) (port)
#define outp(val, port) (port)=(val)
#define inb(port) (port)
#define outb(port, val) (port)=(val)
#define sbi(port, bit) (port)|=(1<<(bit))
#define cbi(port, bit) (port)&=~(1<<(bit)) 
#define PINMODES(MODES) (DIGITAL_DIRECTION_PORT=(MODES))
#define PIN_MODE(PIN,MODE) (PINMODES(DIGITAL_DIRECTION_PORT&~(1<<(PIN))|((MODE)<<(PIN))))
#define DIGITAL_WRITE_HI(PINS) (DIGITAL_OUT_PORT|=(PINS))
#define DIGITAL_WRITEXOR(PINS) (DIGITAL_IN_PORT=(PINS))
#define DIGITAL_WRITEALL(PINS) (DIGITAL_OUT_PORT=(PINS))
#define DIGITAL_WRITELOW(PINS) (DIGITAL_OUT_PORT&=~(PINS))
#define DIGITAL_OUT_STAT()     (DIGITAL_OUT_PORT)
#define DIGITAL_WRITE(PIN,VAL) (DIGITAL_OUT_PORT=(DIGITAL_OUT_PORT&~(1<<(PIN)))|((VAL)<<(PIN)))
#define DIGITAL_READALL() (DIGITAL_IN_PORT)
#define DIGITAL_READ(PIN) ((DIGITAL_IN_PORT>>(PIN))&1)
#define FOREVER for(;;)
#define pinMode PIN_MODE
#define digitalWrite DIGITAL_WRITE

// define the processor speed if it's not been defined at the compiler's 
// command line. So, it doesn't really matter what these values are
// all that matters is that the combination causes 0xff to 
// correspond to the correct frequency of strobe
// where "correct" is whatever you feel like the slowest frequency should be
// presently its set so that 512 minitics equals about 1 second,
// so, I guess thats about 2milliseconds
//
// I have observed that cycles shorter than 8 tics (~16ms) can not be seens
//

#define DELAYMS (56*2-1)
#include <avr/pgmspace.h>
#include <avr/io.h>

//remember, 1KB is the limit
//that 1024 bytes !
//conservatively, this means no more than 512 bytes light program
//which probably means approximately 50-60 patches

#define ONE (1<<6)
#define TWO (2<<6)
#define THREE (3<<6)
#define FOUR (4<<6)

#define MAXLIGHTS 4
#define LIGHTBYTES 4

//0x00 is actually 0x100 here, cause 0x00 doesn't mean anything
//if DELAYMS is set such that 1 tic = 1/510th of a second
//equivalently : so that on/off = 0xff = 1Hz
//then setting this to 0x80 gives a megatic duration of about quarter
//second, which gives a max patch duration of about 32 seconds.
//#define TICS_PER_MEGATIC 0x80
#define TICS_PER_MEGATIC 0x20

//hypothetical PWM bit mask patterns
/*
#define LEVEL0 0x00
#define LEVEL1 0x01
#define LEVEL2 0x11
#define LEVEL3 0x29
#define LEVEL4 0x55
#define LEVEL5 0x75
#define LEVEL6 0x77
#define LEVEL7 0xEF
#define LEVEL8 0xFF
*/ 
//uint8_t level[8] = {0x00,0x01,0x11,0x29,0x55,0x77,0xEF,0xff};
uint8_t level[4] = {0x00,0x52,~0x52,0xff};
/*
00000000
0101 0010 52
1010 1101  
11111111
uint32_t level[8] = {
	0x000000,
	0x000001,
	0x001001,
	0x008081,
	0x041081,
	0x049089,
	0x049289,
	0x249289};
*/
/*
--0--0--0--0--0--0--0--0
--0--0--0--0--0--0--0--1
--0--0--0--1--0--0--0--1
--0--0--1--0--0--1--0--1
--0--1--0--1--0--1--0--1
--0--1--1--1--0--1--1--1
--0--1--1--1--1--1--1--1
--1--1--1--1--1--1--1--1
0000 0000 0000 0000 0000 0000
0000 0000 0000 0000 0000 0001
0000 0000 0001 0000 0000 0001
0000 0000 1000 0000 0100 0001
0000 0100 0001 0000 0100 0001
0000 0100 1001 0000 0100 1001
0000 0100 1001 0010 0100 1001
0010 0100 1001 0010 0100 1001
0 0 0 0 0 0
0 0 0 0 0 1
0 0 1 0 0 1
0 0 8 0 8 1
0 4 1 0 8 1
0 4 9 0 8 9
0 4 9 2 8 9
2 4 9 2 8 9
0x000000
0x000001
0x001001
0x008081
0x041081
0x049089
0x049289
0x249289
*/
//0000000000000000
//0000000000000001
//0000000100000001
//0000010000100001
//0001000100010001
//0001001001001001
//0100100101001001
//0101010100100101
//0101010101010101
//8-bit color masks
//and alignment offsets
#define R8MASK  ((uint8_t)0xE0)
#define G8MASK  ((uint8_t)0x1C)
#define B8MASK  ((uint8_t)0x03)
#define R16MASK ((uint16_t)0x7C00)
#define G16MASK ((uint16_t)0x03E0)
#define B16MASK ((uint16_t)0x001F)

#define RED         (3<<4)
#define DARK_RED    (2<<4)
#define FAINT_RED   (1<<4)
#define GREEN       (3<<2)
#define DARK_GREEN  (2<<2)
#define FAINT_GREEN (1<<2)
#define BLUE        (3)
#define DARK_BLUE   (2)
#define FAINT_BLUE  (1)
#define WHITE       (0x3F)
#define CYAN (BLUE|GREEN)
#define YELLOW (RED|GREEN)
#define MAGENTA (RED|BLUE)
#define BROWN (DARK_GREEN|DARK_RED)
#define DEEP_CYAN (DARK_GREEN|DARK_BLUE)
#define DEEP_MAGENTA (DARK_RED|DARK_BLUE)
#define ORANGE (RED|DARK_GREEN)
#define PURPLE (BLUE|DARK_RED)
#define VERDE (GREEN|DARK_BLUE)
#define PINK (RED|DARK_BLUE)
#define AQUA (BLUE|DARK_GREEN)
#define RED_ORANGE (RED|FAINT_GREEN)
#define DARK_ORANGE (DARK_RED|FAINT_GREEN)
#define SKY (BLUE|FAINT_RED|FAINT_GREEN)

#define ON(x)  ((x&0xF)<<4)
#define OFF(x) (x&0xF)
#define CYCLE(x,y) (ON(x)|OFF(y))

#define PORTB_IO_STATE 0x07
#define PORTB_PULL_UP  0x18
//#define PORTB_IO_STATE 0x07
//#define PORTB_PULL_UP  0x00

/* LIGHT PROGRAM LANGUAGE : 
 * patches consist of a list of up to MAXLIGHTS different lights
 *
 * the upper 2 bits of the first byte of each patch are the number of lights
 * the lower 6 bits is the duration of the patch in megatics
 * maybe I should use those 6 bits for something else
 * so this means don't ask for durations longer than 2^6-1 = 63 megatics
 * actually I'm going to let 0 = 64 megatics
 * and 0 = 4 lights
 * the number of megatics per tic is defined by TICS_PER_MEGATIC
 *
 * each light starts with a color byte, which is 8 bit 0bRRRGGGBB color
 * followed by a "number of tics on" value
 * a "number of tics off" value
 * and a "start at" number of tics ( whish lets you do phase shifts )
 *
 * the time of a tic is arbitrary and defined by the clock speed and
 * minimum delay
 */
//this must be correct to avoid array-out-of-bounds errors
#define N_PATCHES 1
uint8_t lights[] PROGMEM = {
	THREE|RED,CYCLE(2,4),0,GREEN,CYCLE(3,4),0,BLUE,CYCLE(30,30),0,
	/**
	THREE|RED,CYCLE(2,4),0,BLUE,3,4,0,GREEN,30,30,0,
	THREE|BLUE,CYCLE(2,4),0,GREEN,3,4,0,RED,30,30,0,
	THREE|GREEN,CYCLE(3,6),0,RED,3,6,0,BLUE,30,30,0,
	THREE|BLUE,CYCLE(3,6),0,RED,3,6,0,GREEN,30,30,0,
	THREE|GREEN,CYCLE(3,6),0,BLUE,3,6,0,RED,30,30,0,
	THREE|RED,3,6,0,GREEN,3,5,0,BLUE,20,20,0,
	THREE|RED,3,6,0,BLUE,3,5,0,GREEN,20,20,0,
	THREE|BLUE,3,6,0,GREEN,3,5,0,RED,20,20,0,
	TWO|BLUE,3,6,0,RED,3,6,3,
	TWO|BLUE,3,3,0,RED,3,3,3,
	TWO|GREEN,3,6,0,RED,3,6,3,
	TWO|GREEN,3,3,0,RED,3,3,3,
	TWO|BLUE,3,6,0,GREEN,3,6,3,
	TWO|BLUE,3,3,0,YELLOW,3,3,3,
	TWO|RED,3,6,0,GREEN,3,5,3,
	TWO|RED,3,3,0,GREEN,3,4,3,
	TWO|RED,3,6,0,BLUE,3,5,3,
	TWO|RED,3,3,0,BLUE,3,4,3,
	TWO|GREEN,3,6,0,BLUE,3,5,3,
	TWO|GREEN,3,3,0,BLUE,3,4,3,
	TWO|RED,3,6,0,BLUE,4,7,3,
	TWO|RED,3,3,0,BLUE,4,4,0,
	TWO|RED,3,6,0,GREEN,4,7,3,
	TWO|GREEN,3,6,0,BLUE,4,7,3,
	TWO|RED,3,3,0,GREEN,4,4,3,
	TWO|GREEN,3,3,0,BLUE,4,4,3,
	/*
	ONE|1,WHITE,2,2,0,
	ONE|1,WHITE,3,3,0,
	ONE|1,WHITE,4,4,0,
	ONE|1,WHITE,6,6,0,
	ONE|1,WHITE,8,8,0,
	ONE|1,WHITE,10,10,0,
	ONE|1,WHITE,12,12,0,
	ONE|1,RED,2,2,0,
	ONE|1,RED,3,3,0,
	ONE|1,RED,4,4,0,
	ONE|1,RED,6,6,0,
	ONE|1,RED,8,8,0,
	ONE|1,CYAN,3,3,0,
	ONE|1,CYAN,4,4,0,
	ONE|1,CYAN,6,6,0,
	ONE|1,CYAN,8,8,0,
	ONE|1,MAGENTA,2,2,0,
	ONE|1,MAGENTA,3,3,0,
	ONE|1,MAGENTA,4,4,0,
	ONE|1,MAGENTA,6,6,0,
	ONE|1,MAGENTA,8,8,0,
	ONE|1,MAGENTA,10,10,0,*/
};

#define ever (;;)

int main()
{
  PORTB = PORTB_PULL_UP|0x00;
  DDRB  = PORTB_IO_STATE;
  //DDRB=~0x18;
  //PORTB=0x18;
  
  /*
  FOREVER {
	uint8_t x = PINB;
	x&=0x18;
	x^=0xff;
	x>>=3;
	PORTB=0x18|x;
  }*/
  
  uint8_t patchnum = 0;
  uint8_t *lightp = &lights[0];
  uint8_t button_debounce = 0;
  uint8_t button_hold = 0;
  uint8_t skip = 0;
  uint8_t nlights, megatic, minitic, i;
  uint8_t color[MAXLIGHTS], on[MAXLIGHTS], off[MAXLIGHTS], tics[MAXLIGHTS];
  for ever {
	uint8_t state = 0xff;
	nlights = pgm_read_byte_near(lightp);
	++lightp;
	megatic = nlights&0x3F;
	nlights >>= 6;
	if (!megatic) megatic = 64;
	if (!nlights) nlights = 4;
	if (skip) {
		lightp+=nlights*LIGHTBYTES;
		skip--;
	} else {
		for(i=0;i<nlights;++i) {
			uint8_t mask = 1<<i;
			color[i]=pgm_read_byte_near(&lightp[0]);
			on[i]=pgm_read_byte_near(&lightp[1]);
			off[i]=pgm_read_byte_near(&lightp[2]);
			uint8_t phase=pgm_read_byte_near(&lightp[3]);
			lightp+=LIGHTBYTES;
			tics[i]=on[i];
			while (phase>0) { //count off the phase tics
				if (phase<tics[i]) {
					tics[i]-=phase;
					break;
				} else {
					phase-=tics[i];
					state^=mask;
					tics[i]=state&mask?on[i]:off[i];
				}
			}
		}
		minitic = 0;
		while (megatic) {
			uint8_t R=0x00, G=0x00, B=0x00 ;
			for(i=0;i<nlights;++i) { 
				uint8_t mask=1<<i;
				if (state&mask) {
					uint8_t C=color[i];
					R+=(C>>5)&7;
					G+=(C>>2)&7;
					B+=(C&3);
				}
				if(!(tics[i]-=1))tics[i]=(state^=mask)&mask?on[i]:off[i];
			}
			R=level[R];
			G=level[G];
			B=level[B<<1];
			for(i=0;i<DELAYMS;i++) {
				uint8_t j=i&7;
				PORTB=PORTB_PULL_UP|((R>>j)&1)|((G>>j)&1<<2)|((B>>j)&1<<1);
			}
			//automatic advance ?
			//++minitic;
			//if (minitic==TICS_PER_MEGATIC) { //counting on 8 bit overflow
			//	--megatic;
			//	minitic=0;
			//}
			
			//POLL BUTTONS
			
			int buttons = (~PINB)&0x18;
			if (button_debounce>0) button_debounce--;
			if (buttons && !button_debounce && !button_hold) {
				if (buttons&0x08) skip = N_PATCHES-2;
				megatic = 0;
				button_debounce = 0x20;
				button_hold = 1;
			} else button_hold = 0;
			
		}
    }
	//advance to next patch if applicable
	patchnum++;
	if (patchnum>=N_PATCHES) {
		patchnum=0;
		lightp=&lights[0];
	}
  }
  return 0;
} 

