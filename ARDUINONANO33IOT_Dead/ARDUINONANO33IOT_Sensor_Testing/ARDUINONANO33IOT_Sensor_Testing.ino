#define ulS1 A0
#define ulS2 A1
#define ulS3 A2
#define ulS4 A3
#define ulS5 A6

float sensors[] = {0.0, 0.0, 0.0, 0.0, 0.0};
const int numReading = 10;
float ulReading[5][numReading];
int readIndex[5] = {0,0,0,0,0};
float readtotal[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
float average[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
float ind_deviation[5][numReading];
float avgdeviation[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
float deviation_total[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
int re_numReading[5] = {0,0,0,0,0};
float prev_re_average[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
float re_average[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
float re_readtotal[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
int n = 0;

void setup() {
  Serial.begin(9600);
  // pinMode(ulS1, INPUT);
  // pinMode(ulS2, INPUT);
  // pinMode(ulS3, INPUT);
  // pinMode(ulS4, INPUT);
  // pinMode(ulS5, INPUT);
}

void loop() {
//  sensors[1] = analogRead(ulS2);
//  sensors[2] = analogRead(ulS3);
//  sensors[3] = analogRead(ulS4);
//  sensors[4] = analogRead(ulS5);
//  char textSum[100];
//  sprintf(textSum,"Selected Swtich: %d", sensors[0]);
//  Serial.println(textSum);
while( n < 11){
sensors[0] = analogRead(ulS2);
Serial.println("Raw value: " + String(sensors[0]));
Serial.println("Voltage value: " + String((sensors[0]) * (2.9028/912)));
Serial.println("Filter: " + String(analogConvertor(InputFilter(0, sensors[0]))));
n = n + 1;
delay(500);
}
delay(5000);
n = 0;
}

float analogConvertor(int x){
  // x % 5 < 3 ? x = x - x % 5: x = (x - x % 5) +5;
  // float y = x * (2.92/914);
  // // float z = -3.7566 + (7.7526*y) + ((24.5944) * pow(y, 2)) + (-3.1582* pow(y,3)); //polynomial regression - code in colab(gradient descent) || use excel(nah bro it not cool na)
  // float z = 85.8*y - 50.6;  
  float y = 0.275 * x - 50.95;                                       //linear regression - hand calculation..... XD
  return y;
}

float InputFilter(int id, float x){
  prev_re_average[id] = re_average[id];
  re_numReading[id] = numReading;
  ulReading[id][readIndex[id]] = x;
  delay(5);
  if (readIndex[id] >= numReading) {
    average[id] = readtotal[id] / numReading;
    for(int i=0;i < numReading;i++){
      ind_deviation[id][i] = abs(ulReading[id][i] - average[id]);
      deviation_total[id] = deviation_total[id] + ind_deviation[id][i];
    }
    avgdeviation[id] = deviation_total[id]/numReading;
    Serial.println("AVG STD: " + String(avgdeviation[id]));
    for(int i=0;i < numReading;i++){
      if(ind_deviation[id][i] >= avgdeviation[id]){
        ulReading[id][i] = 0;
        re_numReading[id] = re_numReading[id] - 1;
      }
      re_readtotal[id] = re_readtotal[id] + ulReading[id][i];
    }
    Serial.println("Filtered: " + String(re_numReading[id]));
    re_average[id] = re_readtotal[id] / re_numReading[id];
    Serial.println("AVG: " + String(re_average[id]));
    readtotal[id] = 0;
    readIndex[id] = 0;
    deviation_total[id] = 0;
    re_readtotal[id] = 0;
    avgdeviation[id] = 0;
    return re_average[id];
  }
  else if (readIndex[id] < numReading) {
    readtotal[id] = readtotal[id] + ulReading[id][readIndex[id]];
    readIndex[id] = readIndex[id] + 1; 
    return prev_re_average[id];
  }
}