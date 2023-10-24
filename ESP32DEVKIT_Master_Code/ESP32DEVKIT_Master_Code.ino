#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define dip1 27
#define dip2 14
#define mc 12
#define mbc 13
#define led01 4
#define led02 2
#define led03 15
#define adP 5
#define adM 18
#define scl 17
#define vals 16
#define conf 1
#define upl 3

#define ulS1 36
#define ulS2 39
#define ulS3 34
#define ulS4 35
#define ulS5 32

char ssid[] = "KEMET Manufacturing";
char pass[] = "P0lym3riZ@t!on";

int port = 443;
const char serverAddress[] = "script.google.com";
String GOOGLE_SCRIPT_ID = "AKfycbxGIHyBaO-i5JUZOk8FeeN82W5_BNz3OcvO2tDcSg5OVeGwtUYjUeKRtuHxrzCKLDp0";
String contentType = "application/json";
String payload_base_s =  "{\"command\": \"append_row\", \"sheet_name\": \"Dashboard\", \"values\": ";
String payload_base_r =  "{\"command\": \"change_value\", \"sheet_name\": \"Dashboard\", \"values\": ";
String payload_0 = "";
String payload_0_c = "";

String url_0 = String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec";
String url_1 = String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec?master=N2&tole=O2";

float master_value = 15.0;
float tole = 2;
float ref_height = 75;
int maNo = 25;
int baNo = 1;
int batchNo = 1;
String machineNo = String("Z") + maNo;
String bathNo = baNo + String(" - ") + batchNo;

LiquidCrystal_I2C lcd(39,20,4);
uint8_t lcd_adr = 0;

int dip1_state = 0;
int dip2_state = 0;
int mastole_state = 0;
int machbath_state = 0;
int adjust_plus = 0;
int adjust_minus = 0;
int scale_set = 0;
int values_select = 0;
int confirm_set = 0;
int upload_press = 0;
int led01_state = 0;
int led02_state = 0;
int led03_state = 0;

const int numReading = 20;
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

int analogS[5] = {0, 0, 0, 0, 0};
float sensors[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

void setup() {
  Serial.begin(9600);

  Serial.println ("I2C scanner. Scanning ...");
  delay(2500);
  byte count = 0;
  
  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      lcd_adr = i, DEC;
      Serial.print (lcd_adr);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);
      }
  }
  lcd.init();
  lcd.backlight();
  pinMode(dip1, INPUT);
  pinMode(dip2, INPUT);
  pinMode(mc, INPUT);
  pinMode(mbc, INPUT);
  pinMode(adP, INPUT);
  pinMode(adM, INPUT);
  pinMode(scl, INPUT);
  pinMode(vals, INPUT);
  pinMode(conf, INPUT);
  pinMode(upl, INPUT);
  pinMode(led01, OUTPUT);
  pinMode(led02, OUTPUT);
  pinMode(led03, OUTPUT);
  
  lcd.clear();
  StatusLed(3);
  delay(500);
  String str_wifi_connect = "Connecting WIFI...";
  int wifi_connect_midline = (20 - str_wifi_connect.length())/2;
  lcd.setCursor(wifi_connect_midline, 0); lcd.print(str_wifi_connect);
  delay(500);
  WiFi.begin(ssid, pass);

  while ( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  String str_ssid = WiFi.SSID();
  int ssid_midline = (20 - str_ssid.length())/2;
  lcd.setCursor(ssid_midline,2);
  lcd.print(str_ssid);

  String str_ip = String("IP: ") + WiFi.localIP();
  int ip_midline = (20 - str_ip.length())/2;
  lcd.setCursor(ip_midline,3);
  lcd.print(str_ip);
  delay(500);
  lcd.clear();
  Lcd_secure_show("Standard:  ", "Receiving", "Tolerance: ", "Receiving", "");
  delay(500);
  Recieve_From_Sheet();
}

void loop() {
  while(WiFi.status() == WL_CONNECTED){
    Measure_Now();
  }
  WiFi.begin(ssid, pass);
  String str_wifi_connect = "Connecting WIFI...";
  int wifi_connect_midline = (20 - str_wifi_connect.length())/2;
  lcd.setCursor(wifi_connect_midline, 0); lcd.print(str_wifi_connect);

  while ( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  String str_ssidL = WiFi.SSID();
  int ssidL_midline = (20 - str_ssidL.length())/2;
  lcd.setCursor(ssidL_midline,2);
  lcd.print(str_ssidL);

  String str_ipL = String("IP: ") + WiFi.localIP();
  int ipL_midline = (20 - str_ipL.length())/2;
  lcd.setCursor(ipL_midline,3);
  lcd.print(str_ipL);
  delay(500);
}

void Measure_Now(){
  dip1_state = digitalRead(dip1);
  dip2_state = digitalRead(dip2);
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
  
  float temp_error = 0.0;
  float stand_ford = 0.0;
  int messi = 0;
  if(mastole_state == 0 && machbath_state == 0) messi = 1;
  if(mastole_state == 1 && machbath_state == 1) messi = 1;
  lcd.clear();
  while(messi){
    float error_stand_ford[5] = {-0.365, 0.18, 0.16, 0.28, -0.82};
    dip1_state = digitalRead(dip1);
    dip2_state = digitalRead(dip2);
    if(dip2_state == 0) temp_error = 0.0;
    if(dip2_state == 1) temp_error = 0.675;
    if(dip1_state == 0){ref_height = 75; stand_ford = 1.0;}
    if(dip1_state == 1){
      ref_height = 0; 
      stand_ford = -1.0;
      }

    analogS[0] = analogRead(ulS1);
    analogS[1] = analogRead(ulS2);
    analogS[2] = analogRead(ulS3);
    analogS[3] = analogRead(ulS4);
    analogS[4] = analogRead(ulS5);
  
    sensors[0] = stand_ford * (ref_height - (analogConvertor(InputFilter(0, analogS[0])) + error_stand_ford[0]));
    sensors[1] = stand_ford * ((ref_height - (analogConvertor(InputFilter(1, analogS[1])) + error_stand_ford[1])) - temp_error);
    sensors[2] = stand_ford * (ref_height - (analogConvertor(InputFilter(2, analogS[2])) + error_stand_ford[2]));
    sensors[3] = stand_ford * ((ref_height - (analogConvertor(InputFilter(3, analogS[3])) + error_stand_ford[3])) - temp_error);
    sensors[4] = stand_ford * (ref_height - (analogConvertor(InputFilter(4, analogS[4])) + error_stand_ford[4]));

    sensors[0] > 1024 ? sensors[0] = 0: sensors[0] = sensors[0];
    sensors[1] > 1024 ? sensors[1] = 0: sensors[1] = sensors[1];
    sensors[2] > 1024 ? sensors[2] = 0: sensors[2] = sensors[2];
    sensors[3] > 1024 ? sensors[3] = 0: sensors[3] = sensors[3];
    sensors[4] > 1024 ? sensors[4] = 0: sensors[4] = sensors[4];
    delay(5);
    Serial.println("Raw value 0: " + String(ref_height - sensors[0]));
    Serial.println("Raw value 1: " + String(ref_height - sensors[1]));
    Serial.println("Raw value 2: " + String(ref_height - sensors[2]));
    Serial.println("Raw value 3: " + String(ref_height - sensors[3]));
    Serial.println("Raw value 4: " + String(ref_height - sensors[4]));
    Checking_Ind(sensors[0], sensors[1], sensors[2], sensors[3], sensors[4]);
    Checking_Avg(sensors[0], sensors[1], sensors[2], sensors[3], sensors[4]);
    mastole_state = digitalRead(mc);
    machbath_state = digitalRead(mbc);
    upload_press = digitalRead(upl);
    if(upload_press > 0){Send_To_Sheet(machineNo, bathNo, sensors[0], sensors[1], sensors[2], sensors[3], sensors[4], master_value, tole);}
    if(mastole_state == 0 && machbath_state == 1) messi = 0;
    if(mastole_state == 1 && machbath_state == 0) messi = 0;
  }
  if(mastole_state == 1 && machbath_state == 0){
    messi = 0;
    lcd.clear();
    StatusLed(3);
    Configure_page();
    if(upload_press > 0){Send_To_Sheet(machineNo, bathNo, sensors[0], sensors[1], sensors[2], sensors[3], sensors[4], master_value, tole);}
  }
  if(mastole_state == 0 && machbath_state == 1){
    messi = 0;
    lcd.clear();
    StatusLed(3);
    Choose_machine();
    if(upload_press > 0){Send_To_Sheet(machineNo, bathNo, sensors[0], sensors[1], sensors[2], sensors[3], sensors[4], master_value, tole);}
  }
    StatusLed(4);
}

float analogConvertor(int x){
  float y = -10.25 + (0.0212*x) + (2.68E-05 * pow(x,2)) + (-4.61E-09 * pow(x,3));
  return y;
}

float InputFilter(int id, float x){
  prev_re_average[id] = re_average[id];
  re_numReading[id] = numReading;
  ulReading[id][readIndex[id]] = x;
  delay(25);
  if (readIndex[id] >= numReading) {
    average[id] = readtotal[id] / numReading;
    for(int i=0;i < numReading;i++){
      ind_deviation[id][i] = abs(ulReading[id][i] - average[id]);
      deviation_total[id] = deviation_total[id] + ind_deviation[id][i];
    }
    avgdeviation[id] = deviation_total[id]/numReading;
    for(int i=0;i < numReading;i++){
      if(ind_deviation[id][i] >= avgdeviation[id]){
        ulReading[id][i] = 0;
        re_numReading[id] = re_numReading[id] - 1;
      }
      re_readtotal[id] = re_readtotal[id] + ulReading[id][i];
    }
    re_average[id] = re_readtotal[id] / re_numReading[id];
    readtotal[id] = 0;
    readIndex[id] = 0;
    deviation_total[id] = 0;
    re_readtotal[id] = 0;
    avgdeviation[id] = 0;
    lcd.clear();
    return re_average[id];
  }
  else if (readIndex[id] < numReading) {
    readtotal[id] = readtotal[id] + ulReading[id][readIndex[id]];
    readIndex[id] = readIndex[id] + 1; 
    return prev_re_average[id];
  }
}

int IntervalCheck(float x){
  int result = 0;
  if(x >=master_value-tole && x <= master_value+tole){return result = 0;}
  if(x > master_value+tole || x < master_value-tole){return result = 1;}
}

void Checking_Ind(float x, float y, float z, float w, float a){
  Lcd_measured_show(x, y, z, w, a);
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
}

void Checking_Avg(float x, float y, float z, float w, float a){
  int numberAllsensor = 5;
  if(z == 0){numberAllsensor = 4;}
  float avg = (x + y + z + w + a)/numberAllsensor;
  StatusLed(IntervalCheck(avg));
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
}

void Configure_page(){
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
  int valueState = 0;
  int scaleState = 0;
  int confirmed = 0;
  int scl_org = 0;
  float changeU1 = master_value;
  float changeU2 = tole;
  while(confirmed == 0 && mastole_state == 1 && machbath_state == 0){
    mastole_state = digitalRead(mc);
    machbath_state = digitalRead(mbc);
    lcd.clear();
    while(valueState == 0 && mastole_state == 1 && machbath_state == 0 && confirmed == 0){
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);
      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;
      if(scaleState == 0){
      scl_org = 1;
        if(adjust_plus > 0 && adjust_minus == 0)changeU1 = changeU1 + 1;
        if(adjust_plus == 0 && adjust_minus > 0)changeU1 = changeU1 - 1;
      }
      if(scaleState == 1){
      scl_org = 10;
        if(adjust_plus > 0 && adjust_minus == 0)changeU1 = changeU1 + 10;
        if(adjust_plus == 0 && adjust_minus > 0)changeU1 = changeU1 - 10;
      }
      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 1;}
      if(confirm_set > 0)confirmed += 1;
      Lcd_setting_show("Standard Configure", scl_org, "Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "<-", "");
      delay(100);
    }
    lcd.clear();
    while(valueState == 1 && mastole_state == 1 && machbath_state == 0 && confirmed == 0){
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);
      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;
      if(scaleState == 0){
      scl_org = 1;
        if(adjust_plus > 0 && adjust_minus == 0)changeU2 = changeU2 + 1;
        if(adjust_plus == 0 && adjust_minus > 0)changeU2 = changeU2 - 1;
      }
      if(scaleState == 1){
      scl_org = 10;
        if(adjust_plus > 0 && adjust_minus == 0)changeU2 = changeU2 + 10;
        if(adjust_plus == 0 && adjust_minus > 0)changeU2 = changeU2 - 10;
      }
      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 1;}
      if(confirm_set > 0)confirmed += 1;
      Lcd_setting_show("Standard Configure", scl_org, "Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "", "<-");
      delay(100);
    }
    if(valueState > 1)valueState = 0;
  }
  if(confirmed >= 1 && mastole_state == 1 && machbath_state == 0){
    lcd.clear();
    Lcd_secure_show("Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "");
    master_value = changeU1;
    tole = changeU2;

    WiFiClientSecure wifi;
    wifi.setInsecure();
    payload_0_c = payload_base_r + "\"" + master_value + "," + tole + "\"}";
    HttpClient client = HttpClient(wifi, serverAddress, port);
    client.post(url_0, contentType, payload_0_c);
    int statusCode = client.responseStatusCode();
      if (statusCode == 302 || statusCode == 303) {
          String location;
          while (client.headerAvailable()) {
            if (client.readHeaderName() == "Location") {
              location = client.readHeaderValue();
              break;
            }
          }
          if (location.length()) {
            client.stop();
            uint16_t path_begin = location.substring(8).indexOf('/');
            String server_re = location.substring(8, path_begin + 8);
            String path_re = location.substring(path_begin + 8);
            HttpClient client_re = HttpClient(wifi, server_re, port);
            client_re.get(path_re);
            int statusCode_re = client_re.responseStatusCode();
            if(statusCode_re == 200){
              String response_text = client_re.responseBody();
              Lcd_secure_show("Standard: ", String(changeU1), "Tolerance: ", String(changeU2), response_text);
              client_re.stop();
            }
            else if(statusCode_re != 200){
              Lcd_secure_show("Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "FAIL");
              client_re.stop();
            }
          }
      }
    confirmed = 0;
    delay(850);
  }
}
void Choose_machine(){
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
  int valueState = 0;
  int scaleState = 0;
  int confirmed = 0;
  int scl_org = 0;
  String maChange = String("Z") + maNo;
  String baChange = baNo + String(" - ") + batchNo;
  while(confirmed == 0 && mastole_state == 0 && machbath_state == 1){
    mastole_state = digitalRead(mc);
    machbath_state = digitalRead(mbc);
    lcd.clear();
    while(valueState == 0 && mastole_state == 0 && machbath_state == 1 && confirmed == 0){
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);
      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;
      if(scaleState == 0){
      scl_org = 1;
        if(adjust_plus > 0 && adjust_minus == 0)maNo = maNo + 1;
        if(adjust_plus == 0 && adjust_minus > 0)maNo = maNo - 1;
      }
      if(scaleState == 1){
      scl_org = 10;
        if(adjust_plus > 0 && adjust_minus == 0)maNo = maNo + 10;
        if(adjust_plus == 0 && adjust_minus > 0)maNo = maNo - 10;
      }
      if(maNo > 55) maNo = 1;
      if(maNo < 1) maNo = 55;
      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 1;}
      if(confirm_set > 0)confirmed += 1;
      maChange = String("Z") + maNo;
      Lcd_setting_show("Machine Setting", scl_org, "Machine No: ", maChange, "Bath No: ", baChange, "<-", "");
      delay(100);
    }
    lcd.clear();
    while(valueState == 1 && mastole_state == 0 && machbath_state == 1 && confirmed == 0){
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);
      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);
      Serial.println("bath changing");
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;
      if (scaleState == 0){
      scl_org = 6;
        if(adjust_plus > 0 && adjust_minus == 0){baNo = baNo + 1;}
        if(adjust_plus == 0 && adjust_minus > 0){baNo = baNo - 1;}
        if(baNo > 6) baNo = 1;
      	if(baNo < 1) baNo = 6;
      }
      if (scaleState == 1){
      scl_org = 3;
        if(adjust_plus > 0 && adjust_minus == 0){batchNo = batchNo + 1;}
        if(adjust_plus == 0 && adjust_minus > 0){batchNo = batchNo - 1;}
        if(batchNo > 3) batchNo = 1;
      	if(batchNo < 1) batchNo = 3;
      }
      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 6;}
      if(confirm_set > 0)confirmed += 1;
      baChange = baNo + String(" - ") + batchNo;
      Lcd_setting_show("Machine Setting", scl_org, "Machine No: ", maChange, "Bath No: ", baChange, "", "<-");
      delay(100);
    }
    if(valueState > 1)valueState = 0;
  }
  if(confirmed >= 1 && mastole_state == 0 && machbath_state == 1){
    lcd.clear();
    Lcd_secure_show("Machine No: ", maChange, "Bath No: ", baChange, "DONE");
    machineNo = maChange;
    bathNo = baChange;
    confirmed = 0;
    delay(850);
  }
}
void StatusLed(int x){
  if(x == 0){digitalWrite(led01, 1); digitalWrite(led02, 0); digitalWrite(led03, 0);}
  if(x == 1){digitalWrite(led01, 0); digitalWrite(led02, 1); digitalWrite(led03, 0);}
  if(x == 2){digitalWrite(led01, 0); digitalWrite(led02, 0); digitalWrite(led03, 1);}
  if(x == 3){digitalWrite(led01, 1); digitalWrite(led02, 1); digitalWrite(led03, 1);}
  else if(x == 4){digitalWrite(led02, 0);digitalWrite(led03, 0);digitalWrite(led01, 0);} 
}

void Lcd_measured_show(float x, float y, float z, float w, float a){
  String str_meas_header = "Measured Values (mm)";
  int meas_mideline = (20 - str_meas_header.length())/2;
  lcd.setCursor(meas_mideline, 0);
  lcd.print(str_meas_header);
  lcd.setCursor(0,1);   
  lcd.print("L1: ");
  lcd.print(String(x,1));
  String str_l2 = String("L2: ") + String(y, 1);
  int l2_backline = 20 - str_l2.length();
  lcd.setCursor(l2_backline,1);   
  lcd.print(str_l2);
  String str_l3 = String("L3: ") + String(z,1);
  int l3_midline = (20 - str_l3.length())/2;
  lcd.setCursor(l3_midline,2);   
  lcd.print(str_l3);
  lcd.setCursor(0,3);   
  lcd.print("L5: ");
  lcd.print(String(a,1));
  String str_l4 = String("L4: ") + String(w,1);
  int l4_backline = 20 - str_l4.length();
  lcd.setCursor(l4_backline,3);   
  lcd.print(str_l4);
  delay(15);
}

void Lcd_setting_show(String x, float y, String z, String w, String a, String b, String s1, String s2){
  int set_header_midline = (20 - x.length())/2;
  lcd.setCursor(set_header_midline, 0);
  lcd.print(x);           
  String str_scl = String("Setting Scale: X") + int(y);
  lcd.setCursor(0, 1);
  lcd.print(str_scl);            
  String str_fv = z + w;
  lcd.setCursor(0, 2);
  lcd.print(str_fv);
  lcd.setCursor(18, 2);
  lcd.print(s1);
  String str_sv = a + String(" ") + b;
  lcd.setCursor(0, 3);
  lcd.print(str_sv);
  lcd.setCursor(18, 3);
  lcd.print(s2);
}

void Lcd_secure_show(String z, String w, String a, String b, String dd){
  String x = "Confirm setting";
  int set_header_midline_ss = (20 - x.length())/2;
  lcd.setCursor(set_header_midline_ss, 0);
  lcd.print(x);            
  String y = String("Changes value - ") + dd;
  lcd.setCursor(0, 1);
  lcd.print(y);            
  String str_fv_ss = z + w;
  lcd.setCursor(0, 2);
  lcd.print(str_fv_ss);
  String str_sv_ss = a + b;
  lcd.setCursor(0, 3);
  lcd.print(str_sv_ss);
}

void Lcd_upload_show(String xy, String yz, float z, float w, float a, float b, float c, float d, float e, String ss){
  String x = String("Uploading - ") + ss;
  int set_header_midline_us = (20 - x.length())/2;
  lcd.setCursor(set_header_midline_us, 0);
  lcd.print(x);           
  String y = String("Machine: ") + xy + " " + yz;
  lcd.setCursor(0, 1);
  lcd.print(y);           
  String str_fv_us = String("Values:") + d + ", " + e;
  lcd.setCursor(0, 2);
  lcd.print(str_fv_us);
  String str_sv_us = int(z) + String(", ") + int(w) + String(", ") + int(a) + String(", ") + int(b) + String(", ") + int(c);
  lcd.setCursor(0, 3);
  lcd.print(str_sv_us);
}

void Send_To_Sheet(String mo, String bo, float x, float y, float z, float w, float a, float b, float c){
    StatusLed(2);
    lcd.clear();
    Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, "");
    WiFiClientSecure wifi;
    wifi.setInsecure();
    payload_0 = payload_base_s + "\"" + mo + "," + bo + "," + x + "," + y + "," + z + "," + w + "," + a + "," + b + "," + c + "\"}";
    HttpClient client = HttpClient(wifi, serverAddress, port);
    client.post(url_0, contentType, payload_0);
    int statusCode = client.responseStatusCode();
    lcd.clear();
    Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, String(statusCode));
    if (statusCode == 302 || statusCode == 303) {
        String location;
        while (client.headerAvailable()) {
          if (client.readHeaderName() == "Location") {
            location = client.readHeaderValue();
            break;
          }
        }
        if (location.length()) {
          client.stop();
          uint16_t path_begin = location.substring(8).indexOf('/');
          String server_re = location.substring(8, path_begin + 8);
          String path_re = location.substring(path_begin + 8);
          HttpClient client_re = HttpClient(wifi, server_re, port);
          client_re.get(path_re);
          int statusCode_re = client_re.responseStatusCode();
          lcd.clear();
          Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, String(statusCode_re));
          if(statusCode_re == 200 || statusCode_re == 302){
            String response_text = client_re.responseBody();
            Serial.println(response_text);
            lcd.clear();
            Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, response_text);
            client_re.stop();
          }
          else if(statusCode_re == -2){
            Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, "DONE");
            client_re.stop();
            }
        }
    }
    delay(1000);
    lcd.clear();
}
void Recieve_From_Sheet(){
  int response_int[3];
  WiFiClientSecure wifi;
  wifi.setInsecure();
  HttpClient client = HttpClient(wifi, serverAddress, port);
  client.get(url_1);     
  int statusCode = client.responseStatusCode();
  lcd.clear();
  Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), String(statusCode));
  if (statusCode == 302 || statusCode == 303) {
      String location;
      while (client.headerAvailable()) {
        if (client.readHeaderName() == "Location") {
          location = client.readHeaderValue();
          break;
        }
      }
      if (location.length()) {
        client.stop();
        uint16_t path_begin = location.substring(8).indexOf('/');
        String server_re = location.substring(8, path_begin + 8);
        String path_re = location.substring(path_begin + 8);
        HttpClient client_re = HttpClient(wifi, server_re, port);
        client_re.get(path_re);
        int statusCode_re = client_re.responseStatusCode();
        lcd.clear();
        Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), String(statusCode_re));
        if(statusCode_re == 200){
          String response_text = client_re.responseBody();
          uint16_t respo = response_text.indexOf("/");
          response_int[1] = response_text.substring(0, respo).toInt();
          response_int[2] = response_text.substring(respo+1).toInt();
          master_value = response_int[1];
          tole = response_int[2];
          lcd.clear();
          Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), "DONE");
          client_re.stop();
          delay(1000);
        }
        else if(statusCode_re == -2){
          lcd.clear(); 
          master_value = response_int[1];
          tole = response_int[2];
          Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), "FAIL");
          client_re.stop();
          delay(1000);
          }
      }
  }
}