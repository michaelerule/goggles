#ifndef __lightprogram_h__
#define __lightprogram_h__
#include <avr/pgmspace.h> //permits storing constant arrays in flash, giving us more RAM

/*
 *--- 3 : Light Program Language ------------------------------------------------------
 * The goggles generate color based on a program stored in the flash array 
 * "program". This program consists of a series of patches. Patches are 
 * played in series, sequentially, until the end of the program. The program
 * starts over again once it is finished. Each patch begins with a byte
 * identifying the type of patch. The byte 0xff signals the end of the program.
 * All patches begin with a type identifier followed by a duration, measured
 * in eighths of a second. Data specific to different patch types follows.
 *
 * Currently Defined Types of Patch
 * 1 : default type : a collection of lights
 *     1 byte patch type ( which defines where to look for data )
 *     1 byte T the time duration in eigth seconds of the patch ( up to 32 seconds )  
 *     1 byte L number of active lights ( up to MAXLIGHTS )
 *     L light structs each with
 *       1 byte waveform type, which implies number and types of X bytes of parameters
 *       X bytes of parameters ( up to MAXPARAMS )
 *     next light up to L
 * 0 : patch link HARD TO USE, TODO : WRITE LIGHT PROGRAM GUI + "Compiler" 
 *     1 byte patch type ( which defines where to look for data )
 *     1 byte T the time duration in eighth seconds of the patch ( up to 32 seconds )  
 *     2 byte int referencing the header of a default patch
 * 255 : not a patch, but the end of the program ( loop back to beginning )
 *
 * The default patch type consist of a set of lights. There are many different
 * types of lights, with different parameters.  
 *
 * Currently Defined Types of Light
 * 0 : 3 channel Constant 
 *     3 bytes of state [R,G,B]
 * 1 : 3 channel Square wave
 *     5 bytes of state [R,G,B,f,p]
 *       6 bytes of RGB (3 left, 3 right) color
 *       1 bytes frequency in quarter Hz
 *       1 bytes is a phase shift in 1/256ths of a cycle
 *       1 bytes cycle 255 = full 0 = off 128 = half //DUTY CYCLE NOT IMPLEMENTED !! IGNORE (TODO : IMPLEMENT IT)
 * 2 : 3 channel Sine wave
 *     5 bytes of state [R,G,B,f,p]
 *       6 bytes of RGB (3 left, 3 right) color
 *       1 bytes frequency in quarter Hz
 *       1 bytes is a phase shift in 1/256ths of a cycle
 * 3 : 6 channel Constant 
 *     6 bytes of state [Rl,Gl,Bl,Rr,Gr,Br]
 * 4 : 6 channel Square wave
 *     8 bytes of state [Rl,Gl,Bl,Rr,Gr,Br,f,p]
 *       6 bytes of RGB (3 left, 3 right) color
 *       1 bytes frequency in quarter Hz
 *       1 bytes is a phase shift in 1/256ths of a cycle
 *       1 bytes cycle 255 = full 0 = off 128 = half //DUTY CYCLE NOT IMPLEMENTED !! IGNORE (TODO : IMPLEMENT IT)
 * 5 : 6 channel Sine wave
 *     8 bytes of state [Rl,Gl,Bl,Rr,Gr,Br,f,p]
 *       6 bytes of RGB (3 left, 3 right) color
 *       1 bytes frequency in quarter Hz
 *       1 bytes is a phase shift in 1/256ths of a cycle
 */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Light program readability #defines  /////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PATCH             (0x01)
#define END               (0xff)
#define CONSTANT          (0x00)
#define LINK              (0x00)
#define SQUARE            (0x01)
#define SINE              (0x02)
#define CONSTANT_6CHANNEL (0x03)
#define SQUARE_6CHANNEL   (0x04)
#define SINE_6CHANNEL     (0x05)

/// maximum size of ligting data
/// use this to define a temp buffer to store program
#define MAXLIGHTS 7
#define MAXPARAMS 9

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Light program array  ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
const unsigned char program[] PROGMEM = 
{
  //example : long grey ( for whitepoint adjustment )
  //PATCH, 0x10, 1, CONSTANT, 255, 255, 255,
  
  //example : 15 Hz white square wave strobe for 4 seconds
  PATCH, 0x20, 1,
  SQUARE, 0xaf, 0xaf, 0xaf, 60, 0,
  
  //example : 14.75 Hz orange sinewave strobe for 3 seconds
  PATCH, 0x18, 1,
  SINE, 0xff, 0x80, 0x00, 59, 0,

  //example : a random color
  //PATCH, 0x10, 1,
  //CONSTANT, 34, 78, 10,
  
  //example : Yellow/Blue crossfade strobe, out of phase
  PATCH, 0x20, 2,
  SINE_6CHANNEL, 255, 255, 0, 0, 0, 255, 20*4, 0,
  SINE_6CHANNEL, 0, 0, 255, 255, 255, 0, 20*4, 128,
  
  //more
  PATCH, 0x10, 0x01, //patch type, duration, and number of lights
  SINE, 0xff, 0x00, 0x00, 56, 0,//light 1 [type,r,g,b,f,p]
  
  PATCH, 0x22, 0x04, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 44, 0x0,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 44, 0x80,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 46, 0x0,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 46, 0x80,//light 1 [type,r,g,b]
  
  PATCH, 0x12, 0x02, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0xff, 0xff, 0,0,0, 52, 0,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0,0,0, 0xff, 0xff, 0xff, 58, 0,//light 1 [type,r,g,b]

  PATCH, 0x12, 0x02, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0xff, 0xff, 0,0,0, 34, 0,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0,0,0, 0xff, 0xff, 0xff, 36, 0,//light 1 [type,r,g,b]
  
  PATCH, 0x32, 0x01, //patch type, duration, and number of lights
  SINE, 0xff, 0x00, 0x00, 56, 0,//light 1 [type,r,g,b]

  PATCH, 32, 0x01, //patch type, duration, and number of lights
  SINE, 0xff, 0x00, 0x00, 112, 0,//light 1 [type,r,g,b]

  PATCH,  8,  0x02,                 //patch type, duration, and number of lights
  SINE,  0x00,  0xff, 0x00, 40, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  SINE,  0x00,  0x00, 0xff, 40, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  6,  0x01,                 //patch type, duration, and number of lights 
  SINE,  0xff,  0x00, 0x00, 45, 0,    //light 1 [type,r,g,b,frequency,phase,length]


  PATCH,  6,  0x01,                 //patch type, duration, and number of lights 
  SINE,  0xff,  0x40, 0x00, 50, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  6,  0x01,                 //patch type, duration, and number of lights 
  0x02,  0xff,  0x80, 0x00, 53, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x00, 0x00, 54, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x40, 0x00, 55, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x80, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0xc0, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0xff, 0x00, 57, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0xff, 0x00, 58, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0xff, 0x00, 61, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x00, 64, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x40, 67, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x80, 70, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xc0, 72, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xff, 74, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xc0, 0xff, 77, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x80, 0xff, 79, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x40, 0xff, 81, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 83, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0x00, 0xff, 85, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0x00, 0xff, 110, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0x00, 0xff, 140, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xff, 170, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xc0, 200, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x80, 230, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x40, 255, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0, 0, 0, 0, 0, 46, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0xff, 0, 0, 46, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]

  PATCH,  32,  0x02,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x00, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x03,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0xff, 12, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xd0,  0xff, 0xd0, 20, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x90,  0xff, 0xb0, 28, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x70,  0xff, 0xa0, 32, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x50,  0xff, 0x90, 40, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0xff, 0x70, 48, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x55,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x30,  0xff, 0x50, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x25,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x20,  0xff, 0x30, 64, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x15,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x10,  0xff, 0x10, 76, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                     //patch type, duration, and number of lights
  0x02,  0x10,  0xff,  0x10,   76,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0x10,  0xff,  0x10,   75,  128,  //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                     //patch type, duration, and number of lights
  0x02,  0x00,  0xff,  0x00,   64,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0x00,  0x00,  0xff,   62,  128,  //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                     //patch type, duration, and number of lights
  0x02,  0x00,  0xff,  0x00,   56,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0xff,  0x00,  0x00,   54,  128,  //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x03,                     //patch type, duration, and number of lights
  0x02,  0x00,  0xff,  0x00,   56,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0xff,  0x00,  0x00,   55,  128,  //light 2 [type,r,g,b,frequency,phase,length]
  0x02,  0x00,  0x00,  0xff,    6,    0,  //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  32,  0x03,                  //patch type, duration, and number of lights 
  0x01,  0xff,  0x00, 0x00, 56, 0,   //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff, 0x00, 56, 127, //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0x10,112, 0,   //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                       //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   36,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,   36,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                       //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   40,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,   40,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x02,                          //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   48,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x90,  0x60,   48,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x02,                          //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   56,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x60,  0x90,   56,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                       //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   64,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00,  0xff,   64,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0xff, 0, 0, 0, 0, 46, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0xff, 0xff, 0, 46, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]

  PATCH,  0x10,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x00, 60,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0xff, 60, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x00, 56,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0xff, 56, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x10,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x00, 54,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0xff, 54, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xcf,  0x20, 0x00, 50,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0xff, 50, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  8,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xaf,  0x40, 0x00, 40,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0xff, 40, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x15,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 56,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0xff,  0x00, 56,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0x00, 0xff, 62, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0x00, 0xff, 63, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xff, 64, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xc0, 65, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x80, 66, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x40, 67, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 51,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0xff,  0x00, 52,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 56,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0xff,  0x00, 56,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 58,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0xff,  0x00, 58,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 60,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0xff,  0x00, 60,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0, 0xff, 0, 0, 0, 66, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0xff, 0, 0xff, 66, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0xc0, 0x00, 47, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0x00, 48, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0xff, 0x00, 49, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xff, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xc0, 0xff, 57, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x80, 0xff, 58, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x40, 0xff, 59, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 60, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0x00, 0xff, 61, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  32,  0x03,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 56, 0,  //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0x00, 0x00, 56, 128,  //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff, 0x00,112, 0,  //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 128,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00, 124, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 68,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00, 64, 128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff,  58,   0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,  54,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0x00, 36, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00, 0xff, 30, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xff, 46, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0x00, 0x00, 46, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0, 0xff, 0, 0, 0, 0, 46, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0, 0xff, 0, 46, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  88,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,  84,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  108,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,  104,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  128,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,  124,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  88,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff,  0x00,  84,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  108,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x80,  0xff,  104,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   58,    0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0x00,  0xff,   54,  128,    //light 2 [type,r,g,b,frequency,phase,length]

  PATCH,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 66, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0xff,  0x00, 0x00, 66, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  0x00,  0xff, 0x00, 57, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0xff, 0x00, 50, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0xff, 0x00, 51, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x00, 52, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x40, 53, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x80, 54, 0,    //light 1 [type,r,g,b,frequency,phase,length]

  PATCH,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xc0, 55, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  END
};

// a sine wave lookup table where [-1,1]->[0,255] range and [0,2pi]->[0,255] domain
const unsigned char lookupsine[] PROGMEM = {
  128, 131, 134, 137, 140, 143, 146, 149, 152, 156, 159, 162, 165, 168, 171, 174, 
  176, 179, 182, 185, 188, 191, 193, 196, 199, 201, 204, 206, 209, 211, 213, 216, 
  218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 237, 239, 240, 242, 243, 245, 
  246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255, 
  255, 255, 255, 255, 255, 255, 254, 254, 253, 252, 252, 251, 250, 249, 248, 247, 
  246, 245, 243, 242, 240, 239, 237, 236, 234, 232, 230, 228, 226, 224, 222, 220, 
  218, 216, 213, 211, 209, 206, 204, 201, 199, 196, 193, 191, 188, 185, 182, 179, 
  176, 174, 171, 168, 165, 162, 159, 156, 152, 149, 146, 143, 140, 137, 134, 131, 
  128, 124, 121, 118, 115, 112, 109, 106, 103, 99, 96, 93, 90, 87, 84, 81, 
  79, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39, 
  37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10, 
  9, 8, 7, 6, 5, 4, 3, 3, 2, 1, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 3, 4, 5, 6, 7, 8, 
  9, 10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35, 
  37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76, 
  79, 81, 84, 87, 90, 93, 96, 99, 103, 106, 109, 112, 115, 118, 121, 124
};

// stores the size of the parameter set for all defined light types
byte lightSize[] = {
  4, //type 0 constant   , with 6 parameters + 1 type byte
  6, //type 1 square wave, with 8 parameters + 1 type byte
  6, //type 1 sine wave  , with 8 parameters + 1 type byte
  7, //type 0 constant   , with 6 parameters + 1 type byte
  9, //type 1 square wave, with 8 parameters + 1 type byte
  9, //type 1 sine wave  , with 8 parameters + 1 type byte
};

#endif //__lightprogram_h__
