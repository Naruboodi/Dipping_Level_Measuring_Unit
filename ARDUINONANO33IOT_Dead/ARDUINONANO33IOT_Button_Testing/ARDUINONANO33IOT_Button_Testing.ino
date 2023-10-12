#include <stdio.h>
#define sw1 5
#define sw2 4
#define sw3 3
#define sw4 2
#define sw5 13
#define sw6 21
#define dip1 6
#define dip2 7

int sw1_state = 0;
int sw2_state = 0;
int sw3_state = 0;
int sw4_state = 0;
int sw5_state = 0;
int sw6_state = 0;

int dip1_state = 0;
int dip2_state = 0;

void setup() {
  Serial.begin(9600);
  pinMode(sw1, INPUT);
  pinMode(sw2, INPUT);
  pinMode(sw3, INPUT);
  pinMode(sw4, INPUT);
  pinMode(sw5, INPUT);
  pinMode(sw6, INPUT);
  pinMode(dip1, INPUT);
  pinMode(dip2, INPUT);
}

void loop() {
  sw1_state = digitalRead(sw1);
  sw2_state = digitalRead(sw2);
  sw3_state = digitalRead(sw3);
  sw4_state = digitalRead(sw4);
  sw5_state = digitalRead(sw5);
  sw6_state = digitalRead(sw6);
  dip1_state = digitalRead(dip1);
  dip2_state = digitalRead(dip2);
  char textSum[1000];
  sprintf(textSum,"Selected Swtich: %d, %d, %d, %d, %d, %d __ Dips: %d, %d", sw1_state, sw2_state, sw3_state, sw4_state, sw5_state, sw6_state, dip1_state, dip2_state);
  Serial.println(textSum);

}
