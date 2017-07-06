/*
Hardware notes
==============

DDRx   : 1 = output, 0 = input
PORTx : output buffer
PINx  : digital input buffer ( writes set pullups )
Port B is the chip IO port ( other ports are control registers )

    AtTiny24 pinout:
                __ __
    3     VCC -|  U  |- GND    1
      BCK PB0 -|     |- PA0 G1
      SHU PB1 -|     |- PA1 B1
    4 RST PB3 -|     |- PA2 R1
      PLY PB2 -|     |- PA3 G2 
      FWD PA7 -|     |- PA4 B2 5
    2 IND PA6 -|_____|- PA5 R2 6
            
    Programmer pinout, 6 pin:
            
    6 MISO +-+  VCC 3
    5 SCK  + + MOSI 2 
    4 RST  +-+  GND 1
    
    Programmer pinout, 10 pin:
            
    3 vcc  +-+   MOSI 2
           + +    
           + +]  RST  4 
           + +   SCK  5 
    1 gnd  +-+   MISO 6

    Circuit:
                __ __
           VCC-|  U  |-GND
        GND-/ -|     |-----\
        GND-/ -|     |------(RGB1)-GND
       VCC-www-|     |-www-/
        GNd-/ -|     |-----\
        GNd-/ -|     |------(RGB2)-GND
    GND-(LED1)-|_____|-www-/

    VCC-(LED0)-GND

Here is a diagram of the circuit board :
 
              R-GBR-GB
     _________||||||||___
    |o+++++++ |  ||  |  o|
    | +++++++ |__||__|(i)|
    |= [x] [<] [=] [>](p)|
    |o__________________o|

The buttons are as follows, from left to right :

[x] Shuffle      : if playing, toggle shuffled playback, else skip to random
[<] Skip Back    : skips to the previous light patch
[=] Play/Pause   : toggles automatic advancing through patches
[>] Skip Forward : advances to the next light patch
 
The LEDs indicate :
 (i) -- indicator LED. on if playing, flashing if playing+shuffle
 (p) -- power LED, on if powered on. 
 
When powered on, the goggles are paused at patch zero. Press play/pause to 
begin the default light sequence. Press forward and back to move through the
light patches when paused or playing. Press shuffle to skip to a random 
patch when paused, or press shuffle when playing to toggle whether or not
patches are played in a random order. When the goggles are paused, the
indicator light will be off. When the goggles are playing, the indicator
light will be on. When the goggles are playing back in a random order, the
indicator light will be flashing. The light program loops around to the 
beginning once it has reached the end. There are 123 patches, though not all
are unique.

*/


// STANDARD DEFINITIONS
#define HIGH   1
#define LOW    0
#define INPUT  0
#define OUTPUT 1
#define HI   HIGH
#define LO   LO
#define IN   INPUT
#define OUT  OUTPUT
#define NOP (__no_operation())
#define DIGITAL_OUT_PORT PORTA
#define DIGITAL_IN_PORT  PINA
#define DIGITAL_DIRECTION_PORT DDRA
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
#define ever (;;)
#define pinMode PIN_MODE
#define digitalWrite DIGITAL_WRITE

// define the processor speed if it's not been defined at the compiler's 
// command line. So, it doesn't really matter what these values are
// all that matters is that the combination causes 0xff to 
// correspond to the correct frequency of strobe
// where "correct" is whatever you feel like the slowest frequency should be
// presently its set so that 512 minitics equals about 1 second,
// so, I guess thats about 2 milliseconds
#define DELAYMS 27
#include <avr/pgmspace.h>
#include <avr/io.h>

//remember, 1KB is the limit
//that 1024 bytes ! -- 2048 is the limit now!
//conservatively, this means no more than 512 bytes light program
//which probably means approximately 50-60 patches
#define N1 (1<<5)
#define N2 (2<<5)
#define N3 (3<<5)
#define N4 (4<<5)
#define N5 (5<<5)
#define N6 (6<<5)
#define N7 (7<<5)
#define N8 (8<<5)

#define MAXLIGHTS  7
#define LIGHTBYTES 4

//0x00 is actually 0x100 here, because 0x00 doesn't mean anything
//if DELAYMS is set such that 1 tic = 1/510th of a second
//equivalently : so that on/off = 0xff = 1Hz
//then setting this to 0x80 gives a megatic duration of about quarter
//second, which gives a max patch duration of about 32 seconds.
#define TICS_PER_MEGATIC 0xff

//PWM bit mask patterns
#define LEVEL0 0x00
#define LEVEL1 0x01
#define LEVEL2 0x11
#define LEVEL3 0x29
#define LEVEL4 0x55
#define LEVEL5 0x75
#define LEVEL6 0x77
#define LEVEL7 0xEF
#define LEVEL8 0xFF 
uint8_t level[8] = {LEVEL0,LEVEL1,LEVEL2,LEVEL3,LEVEL4,LEVEL5,LEVEL6,LEVEL7};
uint8_t blue_level[4] = {LEVEL0,LEVEL2,LEVEL4,LEVEL7};

//8-bit color masks and alignment offsets
#define R8MASK  ((uint8_t)0xE0)
#define G8MASK  ((uint8_t)0x1C)
#define B8MASK  ((uint8_t)0x03)
#define R16MASK ((uint16_t)0x7C00)
#define G16MASK ((uint16_t)0x03E0)
#define B16MASK ((uint16_t)0x001F)

// COLOR DEFINITIONS
#define RED           (7<<5)
#define GREEN         (7<<2)
#define BLUE          (3)
#define MEDIUM_RED    (5<<5)
#define MEDIUM_GREEN  (5<<2)
#define MEDIUM_BLUE   (2)
#define DARK_RED      (3<<5)
#define DARK_GREEN    (3<<2)
#define DARK_BLUE     (1)
#define FAINT_RED     (1<<5)
#define FAINT_GREEN   (2<<2)
#define FAINT_BLUE    MEDIUM_BLUE
#define WHITE         (RED|GREEN|BLUE)
#define GREY          ((2<<5)|(2<<2)|(1))
#define YELLOW        (RED|GREEN)
#define MAGENTA       (RED|BLUE)
#define CYAN          (BLUE|GREEN)
#define YELLOW_GREEN  (DARK_RED|GREEN)
#define DEEP_YELLOW   (DARK_GREEN|DARK_RED)
#define DEEP_CYAN     (DARK_GREEN|DARK_BLUE)
#define DEEP_MAGENTA  (DARK_RED|DARK_BLUE)
#define BROWN         DEEP_YELLOW
#define ORANGE        (RED|DARK_GREEN)
#define PURPLE        (BLUE|DARK_RED)
#define VERDE         (GREEN|DARK_BLUE)
#define PINK          (RED|DARK_BLUE)
#define AQUA          (BLUE|DARK_GREEN)
#define RED_ORANGE    (RED|FAINT_GREEN)
#define YELLOW_ORANGE (RED|MEDIUM_GREEN)
#define DARK_ORANGE   (MEDIUM_RED|FAINT_GREEN)
#define MEDIUM_ORANGE (MEDIUM_RED|DARK_GREEN)
#define SKY           (BLUE|FAINT_RED|FAINT_GREEN)

//port states for application
#define PORTA_IO_STATE (0x7f) // 0 1 11 1111 ( forward, indicator, RGBRGB )
#define PORTA_PULL_UP  (~PORTA_IO_STATE)
#define PORTB_IO_STATE (0x07) // (voidx4) (reset) play shuffle back
#define PORTB_PULL_UP  7

//button constants ( port pin numbers )
#define FORWARD  0x08
#define PLAY     0x04
#define SHUFFLE  0x02
#define BACKWARD 0x01

//mask defining the pin of the indicator light
#define INDICATOR_MASK 0x40;

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
 * minimum delay. Currently it is set so that 256 ticks is appx 1 second
 */
 //this must be correct to avoid array-out-of-bounds errors

/*
#define N_PATCHES 3
uint8_t lights[] PROGMEM = {
N7,32, 85, 17, 34, 32, 71, 31, 27, 32, 61, 41, 22, 32, 51, 51, 17, 32, 41, 61, 12, 32, 31, 71, 7, 32, 17, 85, 0,
N7,36, 85, 17, 34, 36, 71, 31, 27, 36, 61, 41, 22, 36, 51, 51, 17, 36, 41, 61, 12, 36, 31, 71, 7, 36, 17, 85, 0,
N7,4, 85, 17, 34, 4, 71, 31, 27, 4, 61, 41, 22, 4, 51, 51, 17, 4, 41, 61, 12, 4, 31, 71, 7, 4, 17, 85, 0,
};
*/

#define N_PATCHES 102
const uint8_t lights[] PROGMEM = {
    N1| 1,((1)|(7<<5)|(4<<2)),2,0,0,
    N1| 1,((2)|(7<<5)|(4<<2)),2,0,0,
    N1| 1,((3)|(7<<5)|(7<<2)),2,0,0,
    N1| 1,((3)|(6<<5)|(6<<2)),2,0,0,
    N1| 1,((3)|(5<<5)|(5<<2)),2,0,0,
    N1| 1,((3)|(4<<5)|(4<<2)),2,0,0,
    N1| 1,((3)|(3<<5)|(3<<2)),2,0,0,
    N1| 1,((3)|(2<<5)|(2<<2)),2,0,0,
    N1| 1,((1)|(2<<5)|(2<<2)),2,0,0,
    N1| 1,((1)|(0<<5)|(0<<2)),2,0,0,
    N1| 1,((1)|(2<<5)|(0<<2)),2,0,0,
    N1| 1,((0)|(2<<5)|(0<<2)),2,0,0,
    N1| 1,((0)|(2<<5)|(1<<2)),2,0,0,
    N1| 1,((0)|(0<<5)|(2<<2)),2,0,0,
    N1| 1,((1)|(0<<5)|(2<<2)),2,0,0,
    N1| 1,((2)|(3<<5)|(3<<2)),2,0,0,
    N1| 1,((2)|(0<<5)|(0<<2)),2,0,0,
    N1| 1,((2)|(3<<5)|(0<<2)),2,0,0,
    N1| 1,((0)|(4<<5)|(0<<2)),2,0,0,
    N1| 1,((0)|(4<<5)|(2<<2)),2,0,0,
    N1| 1,((0)|(0<<5)|(3<<2)),2,0,0,
    N1| 1,((2)|(0<<5)|(3<<2)),2,0,0,
    N1| 1,((3)|(7<<5)|(7<<2)),2,0,0,
    N1| 1,((3)|(0<<5)|(0<<2)),2,0,0,
    N1| 1,((3)|(7<<5)|(0<<2)),2,0,0,
    N1| 1,((0)|(7<<5)|(0<<2)),2,0,0,
    N1| 1,((0)|(7<<5)|(3<<2)),2,0,0,
    N1| 1,((0)|(0<<5)|(7<<2)),2,0,0,
    N1| 1,((3)|(0<<5)|(7<<2)),2,0,0,
	N1| 1,(7<<5)|(0<<2)|(0),2,0,0,
	N1| 1,(7<<5)|(1<<2)|(0),2,0,0,
	N1| 1,(7<<5)|(2<<2)|(0),2,0,0,
	N1| 1,(7<<5)|(3<<2)|(0),2,0,0,
	N1| 1,(7<<5)|(4<<2)|(0),2,0,0,
	N1| 1,(7<<5)|(5<<2)|(0),2,0,0,
	N1| 1,(6<<5)|(7<<2)|(0),2,0,0,
	N1| 1,(4<<5)|(7<<2)|(0),2,0,0,
	N1| 1,(3<<5)|(7<<2)|(0),2,0,0,
	N1| 1,(2<<5)|(7<<2)|(0),2,0,0,
	N1| 1,(1<<5)|(7<<2)|(0),2,0,0,
	N1| 1,(0<<5)|(7<<2)|(0),2,0,0,
	N1| 1,(0<<5)|(7<<2)|(1),2,0,0,
	N1| 1,(0<<5)|(7<<2)|(2),2,0,0,
	N1| 1,(0<<5)|(7<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(6<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(5<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(4<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(3<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(2<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(1<<2)|(3),2,0,0,
	N1| 1,(0<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(1<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(2<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(3<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(4<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(5<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(6<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(7<<5)|(0<<2)|(3),2,0,0,
	N1| 1,(7<<5)|(0<<2)|(2),2,0,0,
	N1| 1,(7<<5)|(0<<2)|(1),2,0,0,
	N1| 1,(7<<5)|(0<<2)|(0),2,0,0,
    N1| 1,((3)|(7<<5)|(7<<2)),2,0,0,
	N2|30,WHITE,5,84,0,WHITE,5,83,0, //ok-- -- ---
	N2|30,GREEN,5,17,0,RED,6,17,3, //-- really good
	N2|10,BLUE,5,17,0,RED,6,17,3, //-- really good
	N2|10,GREEN,5,17,0,RED,6,17,3, //-- really good
	N2|10,RED,5,17,0,RED,6,17,3, //-- really good
	N2|10,RED,5,25,0,BLUE,5,24,23, //ok-- -- ---
	N2|20,RED,5,35,0,BLUE,5,34,33, //ok-- --
	N2|30,RED,5,45,0,BLUE,5,44,43, //ok-- --
	N2|5,RED,18,18,0,BLUE,18,18,18, //ok
	N2|5,RED,5,15,0,BLUE,5,13,13, //ok-- --
	N2|5,RED,16,16,0,BLUE,16,16,15, //ok
	N2|10,BLUE,3,16,0,RED,3,16,3,
	N2|30,RED,5,24,0,GREEN,5,22,23, //ok-- -- ---
	N2|10,RED,12,12,0,GREEN,11,11,11, //ok-- --
	N2|20,RED,14,14,0,GREEN,14,14,14, //ok--
	N2|10,RED,15,15,0,GREEN,14,14,14, //ok-- --
	N2|10,RED,18,18,0,GREEN,17,17,17, //ok-- --
	N2|5,RED,18,18,0,GREEN,18,18,18, //ok--
	N2|10,RED,4,18,0,GREEN,5,17,3,
	N2|10,GREEN,4,18,0,MAGENTA,4,18,3,
	N2|10,BLUE,4,18,0,YELLOW,4,18,3,
	N3|5,GREEN,3,16,0,BLUE,3,16,0,RED,30,30,0,
	N3|30,RED,5,34,0,GREEN,5,33,0,BLUE,5,32,0, //ok-- -- ---
	N3|30,RED,5,24,0,GREEN,5,23,0,BLUE,5,22,0, //ok-- -- ---
	N3|30,RED,5,14,0,GREEN,5,13,0,BLUE,5,12,0, //ok-- -- ---
	N3|5,RED,100,100,0,GREEN,50,50,0,BLUE,200,200,0,
	N3|5,RED,5,24,0,GREEN,5,23,23,1,255,255,0, //ok-- -- ---
	N3|10,RED,12,12,0,GREEN,11,11,11,1,255,255,0, //ok-- --
	N3|10,RED,24,24,0,GREEN,23,24,24,1,255,255,0, //ok--
	N3|10,RED,15,15,0,GREEN,14,14,14,2,255,255,0, //ok-- --
	N3|10,RED,18,18,0,GREEN,17,17,17,2,255,255,0, //ok-- --
	N3|5,RED,5,25,0,BLUE,5,23,23,4,255,255,0, //ok-- -- ---
	N3|5,RED,5,35,0,BLUE,5,33,33,4,255,255,0, //ok-- --
	N3|10,RED,5,45,0,BLUE,5,43,43,4,255,255,0, //ok-- --
	N6|31,RED,5,184,0,GREEN,5,183,0,BLUE,5,182,0,RED,5,185,0,GREEN,5,186,0,BLUE,5,187,0, //ok-- -- --- ugh painful
	N6|31,RED,5,84,0,GREEN,5,83,0,BLUE,5,82,0,RED,5,85,0,GREEN,5,86,0,BLUE,5,87,0, //ok-- -- ---
	N6|31,RED,5,44,0,GREEN,5,43,0,BLUE,5,42,0,RED,5,45,0,GREEN,5,46,0,BLUE,5,87,0, //ok-- -- ---
    N6|30,1,7,11,0,1<<5,5,13,0,1<<2,3,17,0,(1<<5),2,19,0,(1<<5)|(1<<2),1,23,0,(1<<5),11,7,0,
	N6|30,RED,5,84,0,BLUE,5,83,0,BLUE,5,82,0,RED,5,85,0,RED,5,86,0,BLUE,5,87,0, //ok-- -- ---
	N6|30,RED,5,84,0,GREEN,5,83,0,RED,5,82,0,RED,5,85,0,GREEN,5,86,0,GREEN,5,87,0, //ok-- -- ---
};


//program state ( global )

//we have a "high quality" 8-bit random number generator
uint8_t random_state = 79;
uint8_t random_multiple = 149;
uint8_t entropy_pool=153;

//indicator light state
uint8_t automatic_advance = 0;//0x80; //must be 0 or 0x80, (bool via 8th bit)
uint8_t is_suffle_on = 0;
uint8_t patchnum = 0;
uint8_t *lightp = &lights[0];
uint8_t button_debounce = 0;
uint8_t button_hold = 0;
uint8_t skip = 0;
uint8_t nlights, megatic, minitic, i;
uint8_t color[MAXLIGHTS], 
           on[MAXLIGHTS], 
          off[MAXLIGHTS], 
         tics[MAXLIGHTS];
uint8_t state = 0xff;

/** advance to a new light patch
 *  This reads and sets the number of lights, and the pointer into 
 *  the memory that stores the light program information.
 */
inline void advance_light()
{
    nlights = pgm_read_byte_near(lightp);
	++lightp;
	megatic = nlights&0x1F;
	nlights >>= 5;
	if (!megatic)
	    megatic = 32; //zeros are a special case
	if (!nlights)
	    nlights = 8;  //zeros are a special case
}

/** we have a very weak random number generator that derives most of 
 *  its randomness from the fact that button presses will be randomly timed
 */
inline void update_random()
{
    random_state=random_state+random_multiple+entropy_pool; //totally makes sense
}

/** This loads the current light state and does software PWM to achive a
 *  small measure of variable brightness.
 */
inline void display_lightshow()
{
	uint8_t R=0x00, G=0x00, B=0x00 ;
	for(i=0;i<nlights;++i) { 
		uint8_t mask=1<<i;

		if (state&mask) {
			uint8_t C=color[i];
			R+=(C>>5)&7;
			G+=(C>>2)&7;
			B+=(C&3);
		}
		tics[i]-=1;
		while (!tics[i])
		    tics[i] = (state^=mask)&mask ? on[i] : off[i];
	}
	R=level[R];
	G=level[G];
	B=blue_level[B];
	for(i=0;i<DELAYMS;i++) {
		uint8_t j=i&7;
		uint8_t c=((R>>j)&1<<2)|((G>>j)&1)|((B>>j)&1<<1);
		PORTA=PORTA_PULL_UP|c|(c<<3)|automatic_advance&(~is_suffle_on|minitic&0x40);
	}
}

/** Poll the current state of the buttons. This is much simpler than dealing 
 *  with interrupts in AVR, and allows exact control of debouncing provided 
 *  that the kernel executes fast enough.
 */
inline void poll_buttons() 
{ 
	uint8_t buttons  = (((~PINA)&PORTA_PULL_UP)>>4)|((~PINB)&PORTB_PULL_UP);
	if (button_debounce>0)
	    button_debounce--;
	buttons=buttons&0xf;
	if (buttons) {
	    entropy_pool+=random_state+7;
	    if (!button_debounce && !button_hold) {
	        if (buttons&(FORWARD|BACKWARD|SHUFFLE))
                megatic = 0;
	        if (buttons&BACKWARD) 
                skip = N_PATCHES-2;
	        if (buttons&SHUFFLE) {
	            if (automatic_advance)
	                is_suffle_on^=~0;
	            else
                    skip = random_state;
            }
	        if (buttons&PLAY)
                automatic_advance^=INDICATOR_MASK;
		    button_debounce = 0x20;
		    button_hold = 1;
		}
	} else button_hold = 0;
}

/** This kernel handles software PWM, as well as rapid polling of button 
 *  state.
 */
inline void PWM_kernel()
{
    minitic = 0;
	while (megatic) {
        update_random();
        display_lightshow();
        poll_buttons();			
		//automatic advance, if enabled
		if (automatic_advance) {
		    ++minitic;
		    if (minitic==TICS_PER_MEGATIC) {
			    --megatic;
			    minitic=0;
		    }
		}						
	}
}

/** This loads and runs a single light pattern, while paying attention to
 *  button state and responding to events
 */
inline void execute_light()
{
	if (is_suffle_on)
	    megatic=10;
	for(i=0;i<nlights;++i) 
	{
		uint8_t mask  = 1<<i;
		color[i]       = pgm_read_byte_near(&lightp[0]);
		on   [i]       = pgm_read_byte_near(&lightp[1]);
		off  [i]       = pgm_read_byte_near(&lightp[2]);
		uint8_t phase = pgm_read_byte_near(&lightp[3]);
		lightp += LIGHTBYTES;
		tics[i] = on[i];
		while (phase) { //count off the phase tics
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
	PWM_kernel();
	if (is_suffle_on && automatic_advance)
	    skip+=random_state;
}

/** This is the main kernel
 *  in a loop, in perpetuity, it
 *  -- displays the current light patch
 *  -- polls the button state
 *  -- advances the light state
 */
int main()
{
  //set IO state of application
  PORTA = ((PORTA_PULL_UP)|0x00);
  DDRA  = PORTA_IO_STATE;
  PORTB = ((PORTB_PULL_UP)|0x00);
  DDRB  = PORTB_IO_STATE;
  //outer kernel
  for ever {
	advance_light();
    if (skip) {
		lightp+=nlights*LIGHTBYTES;
		skip--;
	} 
	else execute_light();
	patchnum++;
	if (patchnum>=N_PATCHES) {
		patchnum=0;
		lightp=&lights[0];
	}
  }
  //control flow should not reach here
} 



