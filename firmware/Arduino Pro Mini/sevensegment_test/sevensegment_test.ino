
// Routines to identify 7 segment pinout

// Used for LEDs: 
// 3 5 6 9 10 11
// I don't think we ised TS or RX (0, 1) either

void announce() {
  pinMode(13,OUTPUT);
  for (int i=0; i<10; i++) {
    digitalWrite(13,LOW ); delay(20);
    digitalWrite(13,HIGH); delay(20);
  }
}

#define NPINTEST 10
int testpins[NPINTEST] = {2,4,7,8,12,13,14,15,16,17};
//                        1 2 3 4 5  6  7  8  9  10
void test_segments() {
  announce();
  for (int i=0;i<NPINTEST;i++) pinMode(testpins[i],OUTPUT);
  for (int i=0;i<NPINTEST;i++) {
    PORTB=PORTC=PORTD=0;
    digitalWrite(testpins[i],HIGH);
    delay(2000);
  }
}
/*
Result: 
 6
5 7
 3
1 4
 2
*/

void setup() {
  for (int i=0; i<20; i++) pinMode(i,INPUT);
  for (int i=0; i<20; i++) digitalWrite(i,LOW);
  const int apin = 4;
  digitalWrite(apin,1);
  digitalWrite(15,0);
  digitalWrite(16,0);
  pinMode(apin,OUTPUT);
  pinMode(15,OUTPUT);
  pinMode(16,OUTPUT);
  while(1);
}

void test_common_cathode() {
  announce();
  int testcathodes[3] = {15,16,17};
  for (int i=0; i<3; i++)  pinMode(testcathodes[i],INPUT);
  int CELL[7] = {13,12,7,14,2,4,8};
  for (int i=0;i<7;i++) pinMode(CELL[i],OUTPUT);
  for (int i=0;i<7;i++) digitalWrite(CELL[i],HIGH);
  for (int i=0; i<3; i++) {
    pinMode(testcathodes[i],OUTPUT);
    digitalWrite(testcathodes[i],LOW);
    delay(2000);
    pinMode(testcathodes[i],INPUT);
  }  
}

void loop() {
  test_segments();
  test_common_cathode();
}
