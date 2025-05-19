#ifndef __led_h__
#define __led_h__

/// we use 8 bit fixed point to represent number in [0,1]
#define FIXED8MUL(x,y) ((x)*(y)>>8)

///  global lighting adjustment parameters
///  RGB rescale values
///  we have a different white point for each eye
///  which also differs between individual boards
///  these are for barod A-2]
#define RRscale 0xff
#define GRscale 0xA0
#define BRscale 0x38
#define RLscale 0xff
#define GLscale 0xA0
#define BLscale 0x38
int SCALE[6] = {RLscale, GLscale, BLscale, RRscale, GRscale, BRscale};

// RGB LED PINS
#define R1pin 9
#define G1pin 5
#define B1pin 6
#define R2pin 3
#define G2pin 11
#define B2pin 10
int COLORPIN[6] = {R1pin, G1pin, B1pin, R2pin, G2pin, B2pin};

/** writeColor performs whitepoint adjustment then write a color to 
 *  the eyes. Left and Right eyes are specified independently
 */
void writeColor(int Rl, int Gl, int Bl, int Rr, int Gr, int Br) {
  analogWrite(R1pin, (byte)FIXED8MUL(Rl,RLscale) ); 
  analogWrite(G1pin, (byte)FIXED8MUL(Gl,GLscale) ); 
  analogWrite(B1pin, (byte)FIXED8MUL(Bl,BLscale) ); 
  analogWrite(R2pin, (byte)FIXED8MUL(Rr,RRscale) ); 
  analogWrite(G2pin, (byte)FIXED8MUL(Gr,GRscale) ); 
  analogWrite(B2pin, (byte)FIXED8MUL(Br,BRscale) ); 
}

/** writeColorArray performs whitepoint adjustment then write a color to 
 *  the eyes. Left and Right eyes are specified independently 
 *  @param RGB an array of six integers, RGB for left and then right eye
 */
void writeColorArray(int RGB[6]) {
  for (int c=0; c<6; c++)
    analogWrite(COLORPIN[c],(byte)FIXED8MUL(RGB[c],SCALE[c]));
}

void testLEDs() {
  const int my_ms_delay = 0;
  writeColor(0,0,0,10,0,0);
  delay(my_ms_delay);
  writeColor(0,0,0,0,10,0);
  delay(my_ms_delay);
  writeColor(0,0,0,0,0,10);
  delay(my_ms_delay);
  writeColor(10,0,0,0,0,0);
  delay(my_ms_delay);
  writeColor(0,10,0,0,0,0);
  delay(my_ms_delay);
  writeColor(0,0,10,0,0,0);
  delay(my_ms_delay);
  writeColor(255,255,255,255,255,255);
}

#endif // __led_h__