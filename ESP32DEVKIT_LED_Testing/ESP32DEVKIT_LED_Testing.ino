#define led_d2 25
#define led_d3 33
#define led_d4 32

int led_1_stat = 0;
int led_2_stat = 0;
int led_3_stat = 0;

void setup() {
  pinMode(led_d2, OUTPUT);
  pinMode(led_d3, OUTPUT);
  pinMode(led_d4, OUTPUT);

}

void loop() {
  digitalWrite(led_d2, 1);
  delay(500);
  digitalWrite(led_d2, 0);
  delay(500);
  digitalWrite(led_d3, 1);
  delay(500);
  digitalWrite(led_d3, 0);
  delay(500);
  digitalWrite(led_d4, 1);
  delay(500);
  digitalWrite(led_d4, 0);
  delay(500);

}
