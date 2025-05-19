#ifndef __sevensegment_h__
#define __sevensegment_h__

/*
 * 7 segment LED notation
 * 0     _
 * 123  |_|
 * 456  |_|
 *
 */

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
///////  Display logic ( drivers and string functions )  /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * 7 segment LED notation
 * 0     _
 * 123  |_|
 * 456  |_|
 *
 */

#define NSEGMENTS (7)
#define NCATHODES (2)
int CELL[NSEGMENTS] = {13,12,7,14,2,4,8};
//int CELL[NSEGMENTS] = {13,12,7,14,2,4,8};
//int CELL[NSEGMENTS] = {13,14,7,12,8,4,2};
int LINE[NCATHODES] = {16,15};


void sevensegment_off() {
  for (int i=0;i<NSEGMENTS; i++) pinMode(CELL[i],INPUT);
  for (int i=0;i<NCATHODES; i++) pinMode(LINE[i],INPUT);
  for (int i=0;i<NSEGMENTS; i++) digitalWrite(CELL[i],LOW);
  for (int i=0;i<NCATHODES; i++) digitalWrite(LINE[i],HIGH);
}
void sevensegment_on() {
  for (int i=0;i<NSEGMENTS; i++) pinMode(CELL[i],OUTPUT);
  //for (int i=0;i<NCATHODES; i++) pinMode(LINE[i],OUTPUT);
}
int display_state[NCATHODES];
int scan_phase;
unsigned long tlastscan;
void sevensegment_clear() {
  for (int i=0;i<NCATHODES; i++) display_state[i] = 0;
  scan_phase = 0;
  tlastscan = millis();
}
void sevensegment_setup() {
  sevensegment_off();
  sevensegment_clear();
}
void sevensegment_scan() {
  if (millis()-tlastscan<10) return;
  tlastscan = millis();
  int code = display_state[scan_phase];
  sevensegment_off();
  for (int i=0;i<NSEGMENTS; i++) {
    digitalWrite(CELL[i], (code&1)? HIGH : LOW );
    code>>=1;
  }
  sevensegment_on();
  digitalWrite(LINE[scan_phase],LOW);
  pinMode(LINE[scan_phase],OUTPUT);
  scan_phase = (scan_phase+1)%NCATHODES;
}
void writeCode(int code, int cycle) {
  display_state[NCATHODES-1-cycle%NCATHODES]=code;
}
void writeDigitHex( int number, int cycle ) {
  writeCode(DIGITS[number&15],cycle);
}
void writeNumberHex( int number, int cycle ) {
  number>>=(4*(cycle&3));
  writeCode(DIGITS[number&15],cycle);
}
void writeNumberDec(int number, int cycle) {
  cycle %= NCATHODES;
  for(int i=0; i<cycle; i++) number/=10;
  writeCode(DIGITS[number%10], cycle);
}
void writeChar(char code,int cycle) {
  writeCode(ASCII[ code<32 || code>126? 0 : code-32 ], cycle);
}

/** scrollString will scroll a string across the display from left to right
 *
 *  @param string string to scroll
 *  @param length length of string to scroll
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




#endif //__sevensegment_h__