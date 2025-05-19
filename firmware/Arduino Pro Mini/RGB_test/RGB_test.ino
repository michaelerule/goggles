
/// we use 8 bit fixed point to represent number in [0,1]
#define FLOAT8MUL(x,y) ((x)*(y)>>8)

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

// RGB LED PINS
#define R1pin 9
#define G1pin 5
#define B1pin 6

#define R2pin 3
#define G2pin 11
#define B2pin 10

void writeColor(int Rl, int Gl, int Bl, int Rr, int Gr, int Br) {
  analogWrite(R1pin, (byte)FLOAT8MUL(Rl,RLscale) ); 
  analogWrite(G1pin, (byte)FLOAT8MUL(Gl,GLscale) ); 
  analogWrite(B1pin, (byte)FLOAT8MUL(Bl,BLscale) ); 
  analogWrite(R2pin, (byte)FLOAT8MUL(Rr,RRscale) ); 
  analogWrite(G2pin, (byte)FLOAT8MUL(Gr,GRscale) ); 
  analogWrite(B2pin, (byte)FLOAT8MUL(Br,BRscale) ); 
}

void test_LEDs() {

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

void loop() {
}



