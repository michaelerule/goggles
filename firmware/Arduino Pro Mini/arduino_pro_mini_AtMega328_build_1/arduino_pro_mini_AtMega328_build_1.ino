
#include <avr/pgmspace.h> //permits storing constant arrays in flash, giving us more RAM

#include "lightprogram.h"
#include "sevensegment.h"
#include "led.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Program global variables & state  ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  count = 0;  //counts program loop iterations
int  CPatch;     //index into program array pointing to current patch
int  patchID;    //the ID of the current patch
int  NLight;     //the number of lights in the current patch
int  CLight;     //the current light ( not sure why this is global )
long startTime;  //when we started the current patch
long patchTime;
/// space to hold the lighting information for a given patch
/// copy from flash to RAM for better read speed when rendering
int lights[MAXLIGHTS*MAXPARAMS];


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Main program logic ( setup & light rendering code )  ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**init :
 * set up output pins for number display
 * initialize the current patch to the start of the program
 * initialize serial port println ( for debugging only )
 * seed a random number generater with maybe-random noise ( not used.. yet )
 * 
 */




void setup() {
  CPatch  = 0;
  patchID = 0;
  Serial.begin(9600);
  Serial.println("SERIAL RUNNING");
  randomSeed(analogRead(18));
  sevensegment_setup();
  Serial.println("INITIALIZATION COMPLETE");
  
  writeChar('A',0);
  writeChar('b',1);
  writeNumberHex(0x2,0);
  writeNumberHex(0x6,1);
  writeCode(0b1000000,0);
  writeCode(0b0100000,1);
  writeCode(0b0000001,0);
  writeCode(0b0000010,1);
  writeCode(0b0000100,0);
  writeCode(0b0001000,1);
  writeCode(0b0010000,0);
  writeCode(0b0100000,1);
  writeCode(0b1110111,1);
  writeCode(DIGITS[6],1);
  //writeNumberHex(0x6,1);
    //char *msg = "Hi there";
  //int len = strlen(msg);
  //while(!scrollString(msg,len,count++,180));
  
  loadNextPatch();
}

void loop() {
  sevensegment_scan();
  renderFrame();
  if ( millis()-startTime >= patchTime )
    loadNextPatch();
}

void loadNextPatch() {
  byte patchType = readProgram(CPatch) ;
  patchTime = (long)(readProgram(CPatch + 1))*(long)1000>>3; //convert patch duration into milliseconds
  startTime = millis();
  switch (patchType) {
  case 0 :
    doPatch( *((int *)(readProgram(CPatch + 2))) );
    CPatch = CPatch + 4;
    patchID++; 
    break; 
  case 1 :
    CPatch = doPatch( CPatch ) ;
    patchID ++ ;
    break;
  default : //end of program
    CPatch  = 0 ;
    patchID = 0 ;
    break;
  }
  writeDigitHex((patchID>>0)&0xf,1);
  writeDigitHex((patchID>>4)&0xf,0);
}

int doPatch( int CPatch ) {
  NLight = readProgram(CPatch + 2) ;
  CLight = CPatch + 3 ;
  for ( int light = 0 ; light < NLight ; light ++ ) {
    int storeIndex = light*MAXPARAMS ;
    for ( int i = 0 ; i < MAXPARAMS ; i ++ )
      lights[storeIndex + i] = readProgram(CLight + i);
    CLight += lightSize[lights[storeIndex]];
  }
  return CLight;
}

void renderFrame() {
    long time = millis()<<6; //im sure the 6 is there fore a reason...
    int RGB[6] = {0,0,0,0,0,0};
    for ( int light=0; light<NLight; light++ )
    {
      int i = light*MAXPARAMS+1;
      switch ( lights[i-1] ) {
      case CONSTANT : // constant
        for (int c=0; c<3; c++) {
          int color = lights[i+c];
          RGB[c  ] += color;
          RGB[c+3] += color;
        }
        break ;
      case SQUARE : { // Square wave
          byte F = (lights[i+3]); //frequency
          byte P = (lights[i+4]); //phase
          byte V = 0xff*(((FIXED8MUL(F,time)>>2)+P>>7)&0x1); //compute amplitude
          for (int c=0; c<3; c++) {
            int color = FIXED8MUL(lights[i+c],V);
            RGB[c  ]+=color;
            RGB[c+3]+=color;
          }
        }
        break;
      case SINE : {// Sine wave
          byte F = (lights[i+3]); //frequency
          byte P = (lights[i+4]); //phase
          int V = sine((F*millis()>>4)+P); //compute amplitude
          for (int c=0; c<3; c++) {
            int color = FIXED8MUL(lights[i+c],V);
            RGB[c  ]+=color;
            RGB[c+3]+=color;
          }
        }
        break ; 
      case CONSTANT_6CHANNEL : // constant ( 6 channels )
        for (int c=0; c<6; c++)
          RGB[c]+=lights[i+c];
        break ;
      case SQUARE_6CHANNEL : { // Square wave ( 6 channels )
          int F = (lights[i+6]); //frequency
          int P = (lights[i+7]); //phase
          int V = 0xff*((((FIXED8MUL(F,time)>>2)+P)>>7)&0x1); //compute amplitude
          for (int c=0; c<6; c++)
            RGB[c]+=FIXED8MUL(lights[i+c],V);
        }
        break;
      case SINE_6CHANNEL : { // Sine wave ( 6 channels )
          int F = (lights[i+6]); //frequency
          int P = (lights[i+7]); //phase
          int V = sine((F*millis()>>4)+P); //compute amplitude
          for (int c=0; c<6; c++)
            RGB[c]+=FIXED8MUL(lights[i+c],V);
        }
        break ; 
      }
    }
    writeColorArray(RGB);
  }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Other utility functions  ////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

int readProgram(int index) {
  return pgm_read_byte(&program[index]);
}

// something that resembles a sine funtion
int sine(int n) {
  return pgm_read_byte(&lookupsine[n&0xff]);
}

void errorstate() {
  writeColor( 0xff, 0xff, 0x00, 0x00, 0xff, 0xff );
  delay(500);
  writeColor( 0x00, 0xff, 0xff, 0xff, 0xff, 0x00 );
  delay(500);
}


