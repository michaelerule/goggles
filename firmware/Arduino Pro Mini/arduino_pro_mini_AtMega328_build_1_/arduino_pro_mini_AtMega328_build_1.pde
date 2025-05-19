#include <avr/pgmspace.h> //permits storing constant arrays in flash, giving us more RAM

/** Documentation
 *  Contents of this comment : 
 *  1 : Changes since last version
 *  2 : A note about representing things that arent integers on a 16 bit microcontroller
 *  3 : Light program language
 *  4 : layout of the remainder of this document
 *  
 *--- 0 : 20250503 Changes ------------------------------------------------------------
 *  I'm revisiting this 15 years later, trying to get the old Arduino pro-mini goggles
 *  builds to run again. 
 * 
 *  
 * 
 * 
 *--- 1 : Changes since last version --------------------------------------------------
 *  I made a few changes to the program, but the underlying structure is the same
 *  Superficial changes include making a new driver for the 4 character display, and
 *  defining some constants for use in the light program, to make things more readable.
 *
 *  The new display driver can scroll strings across the display. This is entertaining
 *  but is not used for anything important at the moment.
 *
 *  I also implemented Sine waveform lights ( which appear to be slightly less 
 *  effective at inducing geometric hallucinations ).
 *
 *  I added versions of the constant, squarewave, and sinewave lights that take
 *  six channels, RGB for the left and then RGB for the right eye. Patches that flash
 *  different colors, frequencies, or phases to different eyes produce amusing 
 *  effects ( well, no, not really, just different. disagreememnt between the inputs
 *  to the left and right eyes seem to weaken the ability to form patterns, at least
 *  to me)
 *
 *  I moved the light program into flash, so the program can now be much longer
 *  without filling up the RAM.
 *
 *  You no longer need to manually count the number of lights and change a constant
 *  to make the program work.
 *
 *  I removed a feature from the rendering code that normalized the light
 *  intensity per number of lights in a patch. This fixes a bug where in order
 *  to, say, alternate red and green lights, you would need to define two lights.
 *  the automatic normalization would then adjust both of these lights to half
 *  intensity. This creates the problem of making sure that the amplitude of the
 *  sum of lights in all channels never exceeds 255. For single light patches this 
 *  sould not be an issue. For multiple lights that may interfere, you will need to
 *  manually adjust light intensity so that the sum of the peaks of all waves is
 *  always less than 255 in all channels.
 *
 *  I have also restructured the program in a (slghtly) better way. Now intead of
 *  diving into a function which plays light patches, we crudely separate the work
 *  into a "light rendering thread" and a "display rendering thread", which are 
 *  interleaved in the main loop. This form of hard coded multithreading is common
 *  in embedded systems. New threads ( perhaps to poll input hardware or talk to 
 *  a computer attached to USB ) can be added to the main loop. At the moment all
 *  functions called from the main loop must usually take less than 10 ms, otherwise
 *  the light rendering will noticably stall.
 *
 *
 *--- 2 : A note about representing things that aren't ints on a 16 bit microcontroller
 *  The ATmega168 which drives your current version of the goggles can only perform 
 *  computations on up to 16 bit integers, no floating point is supported. 
 *  Because of this, in several points in the program, we use some number of low order bits
 *  as fractional bits to emulate fixed point decimal representations. This is done in a rather
 *  ad-hoc fasion, depending on the range of values we wish ro represent.
 * 
 *  Color values for R,G,B are in the range [0,1). This range is mapped to [0,255].
 *  multiplication of color values is provided by the FLOAT8MUL macro
 * 
 *  Frequency values are in the range [0,63] Hz and are mapped onto [0,255].
 *  This means that representations of the frequency code will appear to be 4x their
 *  actual value
 *
 *  Time durations of patches range from [0,31] and are mapped to [0,255].
 *  This means that representations of the duration of a patch will appear in units
 *  of one eights of a second. In hexidecimal notation, the second least significant
 *  digit will count half seconds, so that 0x40 represent 8 seconds.
 *
 *  Phase, in the range of [0,2*pi) is mapped to [0,255]. A phase shift of
 *  128 is equal to 180 degrees, 64 is equal to 90 degrees, and so on and so forth
 * 
 *  The output of the sine function [-1,1] is mapped to [0,255]
 *  is is convenient for generating sine wave output immediately suitable for display
 * 
 *  
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
 * 
 * 7 segment LED notation
 * 0     _
 * 123  |_|
 * 456  |_|
 *
 *
 *--- 4 : Layout of the remainder of this document ------------------------------------
 * 
 * Light program readability #defines
 * Light program array
 * Lookup tables ( Sinewave, light type lengths & display lookup )
 * Program #defines
 * Program global variables & state
 * Main program logic ( setup & light rendering code )
 * Other utility functions
 * Display logic ( drivers and string functions )
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Light program array  ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
PROGMEM prog_uchar program[] = 
{
  
  //example : long grey ( for whitepoint adjustment )
  PATCH, 0x10, 1, CONSTANT, 255, 255, 255,
  
  //example : 15 Hz white square wave strobe for 4 seconds
  PATCH, 0x20, 1,
  SQUARE, 0xaf, 0xaf, 0xaf, 60, 0,
  
  //example : 14.75 Hz orange sinewave strobe for 3 seconds
  PATCH, 0x18, 1,
  SINE, 0xff, 0x80, 0x00, 59, 0,

  //example : a random color
  PATCH, 0x10, 1,
  CONSTANT, 34, 78, 10,
  
  //example : Yellow/Blue crossfade strobe, out of phase
  PATCH, 0x20, 2,
  SINE_6CHANNEL, 255, 255, 0, 0, 0, 255, 2, 0,
  SINE_6CHANNEL, 0, 0, 255, 255, 255, 0, 2, 128,
  
  //now its just random trippyness.
  PATCH, 0x10, 0x01, //patch type, duration, and number of lights
  SINE, 0xff, 0x00, 0x00, 56, 0,//light 1 [type,r,g,b,f,p]
  
  PATCH, 0x22, 0x04, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0x00, 0, 0,0,0, 44, 0x0,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0, 0, 0xff, 0,0,0, 44, 0x80,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0,0,0, 0xff, 0x00, 0, 46, 0x0,//light 1 [type,r,g,b]
  SINE_6CHANNEL, 0,0,0, 0, 0, 0xff, 46, 0x80,//light 1 [type,r,g,b]
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
  0x01,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x00, 0x00, 54, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x40, 0x00, 55, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x80, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0xc0, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0xff, 0x00, 57, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0xff, 0x00, 58, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0xff, 0x00, 61, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x00, 64, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x40, 67, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x80, 70, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xc0, 72, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xff, 74, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xc0, 0xff, 77, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x80, 0xff, 79, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x40, 0xff, 81, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 83, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0x00, 0xff, 85, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0x00, 0xff, 110, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0x00, 0xff, 140, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xff, 170, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xc0, 200, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x80, 230, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x40, 255, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0, 0, 0, 0, 0, 46, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0xff, 0, 0, 46, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]
  0x01,  32,  0x02,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0x00, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff, 0x00, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x03,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0xff, 12, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xd0,  0xff, 0xd0, 20, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x90,  0xff, 0xb0, 28, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x70,  0xff, 0xa0, 32, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x50,  0xff, 0x90, 40, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x02,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0xff, 0x70, 48, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x55,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x30,  0xff, 0x50, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x25,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x20,  0xff, 0x30, 64, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x15,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x10,  0xff, 0x10, 76, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                     //patch type, duration, and number of lights
  0x02,  0x10,  0xff,  0x10,   76,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0x10,  0xff,  0x10,   75,  128,  //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                     //patch type, duration, and number of lights
  0x02,  0x00,  0xff,  0x00,   64,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0x00,  0x00,  0xff,   62,  128,  //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                     //patch type, duration, and number of lights
  0x02,  0x00,  0xff,  0x00,   56,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0xff,  0x00,  0x00,   54,  128,  //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x03,                     //patch type, duration, and number of lights
  0x02,  0x00,  0xff,  0x00,   56,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x02,  0xff,  0x00,  0x00,   55,  128,  //light 2 [type,r,g,b,frequency,phase,length]
  0x02,  0x00,  0x00,  0xff,    6,    0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  32,  0x03,                  //patch type, duration, and number of lights 
  0x01,  0xff,  0x00, 0x00, 56, 0,   //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff, 0x00, 56, 127, //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0x10,112, 0,   //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                       //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   36,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,   36,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                       //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   40,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,   40,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x02,                          //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   48,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x90,  0x60,   48,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x02,                          //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   56,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x60,  0x90,   56,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                       //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   64,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00,  0xff,   64,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0xff, 0, 0, 0, 0, 46, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0xff, 0xff, 0, 46, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]
  0x01,  0x10,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x00, 60,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0xff, 60, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x00, 56,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0xff, 56, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x10,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x00, 54,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0xff, 54, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xcf,  0x20, 0x00, 50,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0xff, 50, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  8,  0x02,                   //patch type, duration, and number of lights
  0x01,  0xaf,  0x40, 0x00, 40,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0xff, 40, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x15,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 56,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0xff,  0x00, 56,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0x00, 0xff, 62, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0x00, 0xff, 63, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xff, 64, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0xc0, 65, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x80, 66, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0x00, 0x40, 67, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 51,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0xff,  0x00, 52,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 56,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0xff,  0x00, 56,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 58,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0xff,  0x00, 58,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x05,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 60,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0xff,  0x00, 60,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0xff, 0, 0xff, 0, 0, 0, 66, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0xff, 0, 0xff, 66, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights 
  0x01,  0xff,  0xc0, 0x00, 47, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0x00, 48, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0xc0,  0xff, 0x00, 49, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xff, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xc0, 0xff, 57, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x80, 0xff, 58, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x40, 0xff, 59, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 60, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0x00, 0xff, 61, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  32,  0x03,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 56, 0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0x00, 0x00, 56, 128,  //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff, 0x00,112, 0,  //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 128,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00, 124, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff, 68,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00, 64, 128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0x00,  0x00,  0xff,  58,   0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,  54,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0xff,  0xff, 0x00, 36, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00, 0xff, 30, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xff, 46, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0x00, 0x00, 46, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  PATCH, 0x18, 2, //patch type, duration, and number of lights
  SINE_6CHANNEL, 0, 0xff, 0, 0, 0, 0, 46, 0,   //light 1 [type,rl,gl,bl,rr,gr,br,f,p]
  SINE_6CHANNEL, 0, 0, 0, 0, 0xff, 0, 46, 128, //light 2 [type,rl,gl,bl,rr,gr,br,f,p]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  88,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,  84,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  108,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,  104,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  128,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,  124,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  88,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff,  0x00,  84,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,  108,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x80,  0xff,  104,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  0x40,  0x02,                      //patch type, duration, and number of lights
  0x01,  0xff,  0x00,  0x00,   58,    0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0x00,  0xff,   54,  128,    //light 2 [type,r,g,b,frequency,phase,length]
  0x01,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 66, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0xff,  0x00, 0x00, 66, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  32,  0x02,                 //patch type, duration, and number of lights
  0x01,  0x00,  0x00, 0xff, 56, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  0x00,  0xff, 0x00, 57, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x80,  0xff, 0x00, 50, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x40,  0xff, 0x00, 51, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x00, 52, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x40, 53, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0x80, 54, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  0x01,  4,  0x01,                 //patch type, duration, and number of lights
  0x01,  0x00,  0xff, 0xc0, 55, 0,    //light 1 [type,r,g,b,frequency,phase,length]
  END
};
  

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Lookup tables ( Sinewave, light type lengths & display lookup )  ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// a sine wave lookup table where [-1,1]->[0,255] range and [0,2pi]->[0,255] domain
PROGMEM prog_uchar lookupsine[] = {
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

int CELL[7] = {16,19,12,18,7,8,4};
int LINE[4] = {13,14,17,2};

int DIGITS[16] = {
  0x7b, //0111 1011, 0
  0x48, //0100 1000, 1
  0x3d, //0011 1101, 2
  0x6d, //0110 1101, 3
  0x4e, //0100 1110, 4
  0x67, //0110 0111, 5
  0x77, //0111 0111, 6
  0x49, //0100 1001, 7
  0x7f, //0111 1111, 8
  0x6f, //0110 1111, 9
  0x5f, //0101 1111, A ( 0111 1101 ) a
  0x76, //0111 0110, b ( 0111 1111 ) B = 8
  0x33, //0011 0011, C ( 0011 0100 ) c
  0x7c, //0111 1100, d
  0x37, //0011 0111, E ( 0011 1111 ) e
  0x17,  //0001 0111, f
};

int ASCII[95] = {
  0x00,//0000 0000, ' '
  0x00,// '!' ( no translation )
  0x0a,//000 1010, '"'
  0x00,// '#' ( no translation )
  0x00,// '$' ( no translation )
  0x00,// '%' ( no translation )
  0x00,// '&' ( no translation )
  0x08,//0000 1000 '''
  0x13,//0001 0011 '('
  0x68,//0110 1000 ')'
  0x00,// '*' ( no translation )
  0x00,// '+' ( no translation )
  0x60,//0110 0000 ','
  0x04,//0000 0100 '-'
  0x00,// '.' ( no translation )
  0x00,// '/' ( no translation )
  0x7b,//0111 1011, 0
  0x48,//0100 1000, 1
  0x3d,//0011 1101, 2
  0x6d,//0110 1101, 3
  0x4e,//0100 1110, 4
  0x67,//0110 0111, 5
  0x77,//0111 0111, 6
  0x49,//0100 1001, 7
  0x7f,//0111 1111, 8
  0x6f,//0110 1111, 9
  0x00,// ':' ( no translation )
  0x00,// ';' ( no translation )
  0x00,// '<' ( no translation )
  0x24,//0010 0100 '='
  0x00,// '>' ( no translation )
  0x1d,//0001 1101 "?"
  0x00,// '@' ( no translation )
  0x5f,//0101 1111, A ( 0111 1101 ) a
  0x76,//0111 0110, b ( 0111 1111 ) B = 8
  0x33,//0011 0011, C ( 0011 0100 ) c
  0x7c,//0111 1100, d
  0x37,//0011 0111, E ( 0011 1111 ) e
  0x17,//0001 0111, f
  0x73,//0111 0011, G ( 0110 1111 ) g=9
  0x5e,//0101 1110, H
  0x12,//0001 0010, i ( 0001 0010 ) I
  0x78,//0111 1000, j ( 0110 1000 ) j
  0x57,//0101 0111, k ( fail ) 
  0x32,//0011 0010, L ( 0001 0010 ) l 
  0x35,//0011 0101, m ( fail ) 
  0x54,//0101 0100, n ( 0101 1011 ) N
  0x74,//0111 0100, o ( 0111 1011 ) O=0
  0x1f,//0001 1111, p
  0x4f,//0100 1111, q ( 0010 1111 ) Q
  0x14,//0001 0100, r ( 0001 0011 ) R
  0x67,//0110 0111, S=5 ( 0100 0110 ) s
  0x36,//0011 0110, t
  0x7a,//0111 0000, u ( 0111 1010 ) U
  0x30,//0011 0000, v 
  0x2d,//0010 1101, w ( fail ) 
  0x25,//0010 0101, x
  0x6e,//0110 1110, y
  0x3d,//0011 1101, Z=2 ( 0001 1100 ) z
  0x13,//0001 0011 '['
  0x00,// '\' ( no translation )
  0x68,//0110 1000 ']'
  0x09,//0000 1001 '^'
  0x20,//0010 0000 '_'
  0x08,//0010 0000 '`'
  0x7d,//0101 1111, A ( 0111 1101 ) a
  0x76,//0111 0110, b ( 0111 1111 ) B = 8
  0x34,//0011 0011, C ( 0011 0100 ) c
  0x7c,//0111 1100, d
  0x3f,//0011 0111, E ( 0011 1111 ) e
  0x17,//0001 0111, f
  0x6f,//0111 0011, G ( 0110 1111 ) g=9
  0x56,//0101 0110, h ( 0101 1110 ) H
  0x10,//0001 0000, i ( 0001 0010 ) I
  0x78,//0111 1000, j ( 0110 1000 ) j
  0x57,//0101 0111, k ( fail ) 
  0x32,//0011 0010, L ( 0001 0010 ) l 
  0x35,//0011 0101, m ( fail ) 
  0x54,//0101 0100, n ( 0101 1011 ) N
  0x74,//0111 0100, o ( 0111 1011 ) O=0
  0x1f,//0001 1111, p
  0x4f,//0100 1111, q ( 0010 1111 ) Q
  0x14,//0001 0100, r ( 0001 0011 ) R
  0x46,//0100 0110, S=5 ( 0100 0110 ) s
  0x36,//0011 0110, t
  0x70,//0111 0000, u ( 0111 1010 ) U
  0x30,//0011 0000, v 
  0x2d,//0010 1101, w ( fail ) 
  0x25,//0010 0101, x
  0x6e,//0110 1110, y
  0x1c,//0001 1100, Z=2 ( 0001 1100 ) z
  0x13,//0001 0011 '{'
  0x00,// '' ( no translation )
  0x68,//0110 1000 '}'
  0x00 // '~' ( no translation )
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Program #defines  ///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// we use 8 bit fixed point to represent number in [0,1]
#define FLOAT8MUL(x,y) ((x)*(y)>>8)

// RGB LED PINS
#define R1pin 3
#define B1pin 5
#define G1pin 6
#define R2pin 9
#define B2pin 10
#define G2pin 11
int COLORPIN[6] = {R1pin, G1pin, B1pin, R2pin, G2pin, B2pin};

///  global lighting adjustment parameters
///  RGB rescale values
///  we have a different white point for each eye
///  which also differs between individual boards
///  these are for barod A-2]
#define RRscale 0xff
#define GRscale 0xe0
#define BRscale 0xe0
#define RLscale 0xff
#define GLscale 0x90
#define BLscale 0x90

int SCALE[6] = {RLscale, GLscale, BLscale, RRscale, GRscale, BRscale};

/// maximum size of ligting data
/// use this to define a temp buffer to store program
#define MAXLIGHTS 7
#define MAXPARAMS 9

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Program global variables & state  ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int  count = 0;  //counts program loop iterations
int  CPatch;     //index into program array pointing to current patch
int  patchID;    //the ID of the current patch
int  NLight;     //the number of lights in the current patch
int  CLight;     //the current light ( not sure why this is global )
long stopTime;   //the end time of the current patch
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
  CPatch = 0;
  patchID = 0;
  Serial.begin(9600);
  Serial.println("SERIAL RUNNING");
  randomSeed(analogRead(18));
  setup_display();
  Serial.println("INITIALIZATION COMPLETE");
  
  //a little fun
  char *msg = "GREETINGS PROFESSOR - - - ENJOY  ";
  int len = strlen(msg);
  while(!scrollString(msg,len,count++,180));
  
  loadNextPatch();
}

void loop() {
  writeNumberDec(patchID, count++);
  renderFrame();
  if ( millis() >= stopTime )
    loadNextPatch();
}

void loadNextPatch() {
  byte patchType = readProgram(CPatch) ;
  long patchTime = (long)(readProgram(CPatch + 1))*(long)1000>>3; //convert patch duration into milliseconds
  long startTime = millis();
  stopTime  = startTime + patchTime ;
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
          byte V = 0xff*(((FLOAT8MUL(F,time)>>2)+P>>7)&0x1); //compute amplitude
          for (int c=0; c<3; c++) {
            int color = FLOAT8MUL(lights[i+c],V);
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
            int color = FLOAT8MUL(lights[i+c],V);
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
          int V = 0xff*((((FLOAT8MUL(F,time)>>2)+P)>>7)&0x1); //compute amplitude
          for (int c=0; c<6; c++)
            RGB[c]+=FLOAT8MUL(lights[i+c],V);
        }
        break;
      case SINE_6CHANNEL : { // Sine wave ( 6 channels )
          int F = (lights[i+6]); //frequency
          int P = (lights[i+7]); //phase
          int V = sine((F*millis()>>4)+P); //compute amplitude
          for (int c=0; c<6; c++)
            RGB[c]+=FLOAT8MUL(lights[i+c],V);
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

/** writeColor performs whitepoint adjustment then write a color to 
 *  the eyes. Left and Right eyes are specified independently
 */
void writeColor(int Rl, int Gl, int Bl, int Rr, int Gr, int Br) {
  analogWrite(R1pin, (byte)FLOAT8MUL(Rl,RLscale) ); 
  analogWrite(G1pin, (byte)FLOAT8MUL(Gl,GLscale) ); 
  analogWrite(B1pin, (byte)FLOAT8MUL(Bl,BLscale) ); 
  analogWrite(R2pin, (byte)FLOAT8MUL(Rr,RRscale) ); 
  analogWrite(G2pin, (byte)FLOAT8MUL(Gr,GRscale) ); 
  analogWrite(B2pin, (byte)FLOAT8MUL(Br,BRscale) ); 
}


/** writeColorArray performs whitepoint adjustment then write a color to 
 *  the eyes. Left and Right eyes are specified independently 
 *  @param RGB an array of six integers, RGB for left and then right eye
 */
void writeColorArray(int RGB[6]) {
  for (int c=0; c<6; c++)
    analogWrite(COLORPIN[c],(byte)FLOAT8MUL(RGB[c],SCALE[c]));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////  Display logic ( drivers and string functions )  /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** writeNumber will write a number to the numerical display.
 *  digits are multiplexed. Which digit gets writtin
 *  depends on the parity of cycle 
 */
void writeNumberHex( int number, int cycle ) {
  number>>=(4*(cycle&3));
  number=DIGITS[number&15];
  for (int i=0;i<4;i++) //clear cathodes
    digitalWrite(LINE[i],HIGH);
  for(int i=0; i<7; i++) {
    digitalWrite( CELL[i], (number&1)? HIGH : LOW );
    number>>=1;
  }
  digitalWrite(LINE[cycle&3], LOW );
}

/** writeNumber will write a number to the numerical display.
 *  digits are multiplexed. Which digit gets writtin
 *  depends on the parity of cycle 
 */
void writeNumberDec( int number, int cycle ) {
  int digit = cycle&3;
  for(int i=0; i<digit; i++)
    number/=10;
  number=DIGITS[number%10];
  for (int i=0;i<4;i++) //clear cathodes
    digitalWrite(LINE[i],HIGH);
  for(int i=0; i<7; i++) {
    digitalWrite( CELL[i], (number&1)? HIGH : LOW );
    number>>=1;
  }
  digitalWrite(LINE[cycle&3], LOW );
}

/** scrollString will scroll a string across the display from left to right
 *  letters are only approximately represented, with M,W,K, and X being 
 *  very poorly rendered. 'v' also doesn't look great
 *
 *  @param string string to scroll
 *  @param length lenght of string to scroll
 *  @param cycle program iteration ( for scanning multiplexed array )
 *  @param msrate time delay in milliseconds before scrolling
 *  @return 1 if string has been fully displayed, else 0
 */
int scrollString(char *string, int length, int cycle, int msrate ) {
  int i = cycle&3 ;
  int frame = millis()/msrate;
  int auglen = length+5;
  int code = frame%auglen-i;
  code = code<length && code>=0 ? string[code] : ' ';
  code = ASCII[ code<32 || code>126? 0 : code-32 ];
  for (int i=0; i<4; i++) //clear cathodes
    digitalWrite( LINE[i], HIGH);
  for (int i=0; i<7; i++) {
    digitalWrite( CELL[i], code&1?HIGH:LOW );
    code>>=1;
  }
  digitalWrite( LINE[i], LOW );
  return frame>=auglen;
}

/** strlen computes the length of a well formed null terminated string
 *  @param str string to find the length of
 */
int strlen(char *str) {
  int len = 0;
  int i = 0;
  while (str[i++]!=0) len++ ;
  return len;
} 

char *string ="- - HEllo   CItIzEnS Of SPACEsHIp EArth - tHIs IS your CAptAIn - your CAptAIn IS DEAD - rELIGI0n IS A viruS of thE S0UL And SHALL bE ErAdICAtEd bEf0rE it CAn P0LLUtE Us All - It is LEFT to us - AlonE on EArth - to rebel Against the tyranny of the selfish replicators";

/** setup_diplay initialized the LED display driver
 */
void setup_display() {
  for (int i=0;i<20;i++)
    pinMode(i,OUTPUT);
}

