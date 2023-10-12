/* Initial Parameter and Programs ------------------------------------------------------------------------------------------------------ ♥ --*/
//include used libary
#include <ArduinoHttpClient.h> 
#include <WiFiNINA.h>
#include <stdio.h>
#include <LiquidCrystal_I2C.h>

//Initial dip-switch input
#define dip1 8
// #define dip2 3
// #define dip3 4
// #define dip4 5
// #define dip5 6
#define mc 9
#define mbc 10
//Initial LEDs output
#define led01 13 // Green
#define led02 12 // Yellow
#define led03 11 // Red
//Initial control button
#define adP 5  //adjust Plus           //sw3
#define adM 4  //adjust Minus          //sw4
#define scl 6  //adjust scale setting  //sw2
#define vals 7 //values selection      //sw1
#define conf 3 //cconfirm setting      //sw5
#define upl 2  //upload                //sw6
//Initial sensor input
#define ulS1 A0
#define ulS2 A1
#define ulS3 A2
#define ulS4 A3
#define ulS5 A6

//Setup wifi
char ssid[] = "KEMET Manufacturing";
char pass[] = "P0lym3riZ@t!on";

//Setup HTTP client
int port = 443; // server port of goole **don't change**
const char serverAddress[] = "script.google.com"; // don't change
WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);

//Set google script key
String GOOGLE_SCRIPT_ID = "AKfycbySZ6uks-eW8CxqMdealYmWoLy26Nc-EP2TGKCWCLaLtfCZ4b9kC1zI_60uAIECiew";

//Set the payload type to json.
String contentType = "application/json";

//Set string for json object to parse in google script & parameter 
//Parameter == command:...(choosing case in g-script), sheet_name:...(your sheet name), values:...(what you want to send out)
String payload_base_s =  "{\"command\": \"append_row\", \"sheet_name\": \"Dashboard\", \"values\": "; //base of the payload
String payload_base_r =  "{\"command\": \"change_value\", \"sheet_name\": \"Dashboard\", \"values\": "; //base of the payload

//String for append values on payload_base  _↑_
String payload_0 = "";
String payload_0_c = "";
String payload_1 = "";
String payload_2 = "";

//Creating URL from g-script key for each purpose as below ↓
String url_0 = String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec";
String url_1 = String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec?master=N2&tole=O2";

//Initial LCD display
LiquidCrystal_I2C lcd(0x27,20,4); //7 - mean adress of the LCD ..... use google for finding the address code na krub ♥

//Master value define
float master_value = 15.0;
float tole = 2;
float ref_height = 75; // reference height of the sensor to end of stainless probe (Can set according to the amplifier)

//Variable for machine No
int maNo = 25;
int baNo = 1;
int batchNo = 1;
String machineNo = String("Z") + maNo;
String bathNo = baNo + String(" - ") + batchNo;

//Define input value for assigning the input & output
int dip1_state = 0;
// int dip2_state = 0;
// int dip3_state = 0;
// int dip4_state = 0;
// int dip5_state = 0;
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

//Create array for Ulsonic sensor input
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

int analogS[5] = {0, 0, 0, 0, 0};
float sensors[5] = {0.0, 0.0, 0.0, 0.0, 0.0};

/* Void Setup() ------------------------------------------------------------------------------------------------------------------------ ♥ --*/
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  pinMode(dip1, INPUT);
  // pinMode(dip2, INPUT);
  // pinMode(dip3, INPUT);
  // pinMode(dip4, INPUT);
  // pinMode(dip5, INPUT);
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
  
  //Blink LEDs to let operator know is gonna happen...
  //(actaully useless but nahh bro, it would be bored XD)
  StatusLed(3);

  //Connect WIFI
  WiFi.begin(ssid, pass);
  String str_wifi_connect = "Connecting WIFI...";
  int wifi_connect_midline = (20 - str_wifi_connect.length())/2;
  lcd.setCursor(wifi_connect_midline, 0); lcd.print(str_wifi_connect);

  while ( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  String str_ssid = WiFi.SSID();
  int ssid_midline = (20 - str_ssid.length())/2;
  lcd.setCursor(ssid_midline,2);   //Set cursor to character 2 on line 0
  lcd.print(str_ssid);

  String str_ip = String("IP: ") + WiFi.localIP();
  int ip_midline = (20 - str_ip.length())/2;
  lcd.setCursor(ip_midline,3);   //Set cursor to character 2 on line 0
  lcd.print(str_ip);
  delay(500);
  lcd.clear();
  Lcd_secure_show("Standard: ", "Receiving", "Tolerance: ", "Receiving", "");
  Serial.println("Receiving....");
  Recieve_From_Sheet();
  StatusLed(4);
  lcd.clear();
}

/* Void Loop() ------------------------------------------------------------------------------------------------------------------------- ♥ --*/
void loop() {
  // while(1);
  while( WiFi.status() == WL_CONNECTED){
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
  String str_ssid = WiFi.SSID();
  int ssid_midline = (20 - str_ssid.length())/2;
  lcd.setCursor(ssid_midline,2);   //Set cursor to character 2 on line 0
  lcd.print(str_ssid);

  String str_ip = String("IP: ") + WiFi.localIP();
  int ip_midline = (20 - str_ip.length())/2;
  lcd.setCursor(ip_midline,3);   //Set cursor to character 2 on line 0
  lcd.print(str_ip);
  delay(500);
}

/*Measuring Function |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ↓ |||||| ↓ ||*/
//Measure function: Use to activate the measure function which will work when dip-switch 6,7 is off
//(master_tolerance and machine_bath configure switch.)
void Measure_Now(){
  dip1_state = digitalRead(dip1);
  // dip2_state = digitalRead(dip2);
  // dip3_state = digitalRead(dip3);
  // dip4_state = digitalRead(dip4);
  // dip5_state = digitalRead(dip5);
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
  int messi = 0;
  Serial.println("Start..............");
  if(mastole_state == 0 && machbath_state == 0) messi = 1;
  if(mastole_state == 1 && machbath_state == 1) messi = 1;
  lcd.clear();
  while(messi){
    dip1_state = digitalRead(dip1);
    //use referance height minus with measured height will result in the chemical height.
    analogS[0] = analogRead(ulS1);
    analogS[1] = analogRead(ulS2);
    analogS[2] = analogRead(ulS3);
    analogS[3] = analogRead(ulS4);
    analogS[4] = analogRead(ulS5);
  
    sensors[0] = ref_height - InputFilter(0, analogConvertor(analogS[0]));
    sensors[1] = ref_height - InputFilter(1, analogConvertor(analogS[1]));
    sensors[2] = ref_height - InputFilter(2, analogConvertor(analogS[2]));
    sensors[3] = ref_height - InputFilter(3, analogConvertor(analogS[3]));
    sensors[4] = ref_height - InputFilter(4, analogConvertor(analogS[4]));
    // sensors[0] = ref_height - analogConvertor(analogS[0]);
    // sensors[1] = ref_height - analogConvertor(analogS[1]);
    // sensors[2] = ref_height - analogConvertor(analogS[2]);
    // sensors[3] = ref_height - analogConvertor(analogS[3]);
    // sensors[4] = ref_height - analogConvertor(analogS[4]);
    delay(5);
    if(dip1_state == 0){sensors[2] = 0;}
    Checking_Ind(sensors[0], sensors[1], sensors[2], sensors[3], sensors[4]);
    Checking_Avg(sensors[0], sensors[1], sensors[2], sensors[3], sensors[4]);
    mastole_state = digitalRead(mc);
    machbath_state = digitalRead(mbc);
    upload_press = digitalRead(upl);
    if(upload_press > 0){Send_To_Sheet(machineNo, bathNo, sensors[0], sensors[1], sensors[2], sensors[3], sensors[4], master_value, tole);}
    if(mastole_state == 0 && machbath_state == 1) messi = 0;
    if(mastole_state == 1 && machbath_state == 0) messi = 0;
  }
  // Activate configurefunction when switch is triggered.
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
  //If dip-switch 6 || 7 is on reset the LEDs 
  StatusLed(4);
}

//Function use to convert analog to mm.
float analogConvertor(int x){
//  /**/ Serial.print("analoginput: "); Serial.println(x);
  // x % 5 < 3 ? x = x - x % 5: x = (x - x % 5) +5;
  float y = x * (2.9027/912);
//  /**/ Serial.print("Voltage: "); Serial.println(y);
  // float z = -3.7566 + (7.7526*y) + ((24.5944) * pow(y, 2)) + (-3.1582* pow(y,3)); //polynomial regression - code in colab(gradient descent) || use excel(nah bro it not cool na)
  float z = 85.8*y - 50.6;    
//  /**/ Serial.print("Converted: "); Serial.println(z);                                    //linear regression - hand calculation..... XD
  return z;
}

float InputFilter(int id, float x){
  prev_re_average[id] = re_average[id];
  re_numReading[id] = numReading;
  ulReading[id][readIndex[id]] = x;
  delay(50);

  if (readIndex[id] >= numReading) {
    // Serial.println(readIndex[id]);
    Serial.println("Id Total: " + String(readtotal[id]));
    average[id] = readtotal[id] / numReading;
    Serial.print("Id Average: ");
    Serial.println(average[id]);
    for(int i=0;i < numReading;i++){
      ind_deviation[id][i] = abs(ulReading[id][i] - average[id]);
      deviation_total[id] = deviation_total[id] + ind_deviation[id][i];
      Serial.println("individual deviation: " + String(id) + " " + String(ind_deviation[id][i]) + " " + String(ulReading[id][i]) +" " +String(i));
      Serial.println("total deviation: " + String(id) + " " + String(deviation_total[id]));
    }
    avgdeviation[id] = deviation_total[id]/numReading;
    Serial.println("Id AverageDeviation: " + String(avgdeviation[id]));
    for(int i=0;i < numReading;i++){
      if(ind_deviation[id][i] >= avgdeviation[id]){
        ulReading[id][i] = 0;
        re_numReading[id] = re_numReading[id] - 1;
      }
      re_readtotal[id] = re_readtotal[id] + ulReading[id][i];
    }
    Serial.println("No. filtered: " + String(re_numReading[id]));
    re_average[id] = re_readtotal[id] / re_numReading[id];
    Serial.print("Return value: ");
    Serial.println(re_average[id]);
    readtotal[id] = 0;
    readIndex[id] = 0;
    deviation_total[id] = 0;
    re_readtotal[id] = 0;
    avgdeviation[id] = 0;
    return re_average[id];
  }
  else if (readIndex[id] < numReading) {
    Serial.print("FilterInput: "); Serial.print(id); Serial.print(", "); Serial.println(x);
    readtotal[id] = readtotal[id] + ulReading[id][readIndex[id]];
    Serial.println("Id Total: " + String(readtotal[id]));
    readIndex[id] = readIndex[id] + 1; 
    return prev_re_average[id];
  }
}

//Function for creating the interval for return the chemical checking status.
int IntervalCheck(float x){
  int result = 0;
  
  if(master_value-tole <= x && x <= master_value+tole){result = 0;} // Green
  if(master_value+tole < x && x < master_value-tole){result = 1;}   // Yellow

  //Retuning the status of checking 0 == good, 1 == not met standard 
  return result;
}

//Function for checking each sensor individualy
void Checking_Ind(float x, float y, float z, float w, float a){
  
  //Checking for selected sensors if the dip-switch is off that sensor value will be 0.
  IntervalCheck(x);
  IntervalCheck(y);
  IntervalCheck(z);
  IntervalCheck(w);
  IntervalCheck(a);
  Serial.println("Interval for what:...");
  Lcd_measured_show(x, y, z, w, a);
  Serial.println("It Happening");
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
}

//Function for checking overall measured sensor to lighting the LEDs.
void Checking_Avg(float x, float y, float z, float w, float a){
  int numberAllsensor = 5;
  if(z == 0){numberAllsensor = 4;}

  //Finding the average value of the sensor.
  // int numAllSensor = dip1_state + dip2_state + dip3_state + dip4_state + dip5_state;
  float avg = (x + y + z + w + a)/numberAllsensor;
  Serial.print("Overalll Status: ");
  Serial.println(IntervalCheck(avg));
  //Used function StatusLed to light up the LEDs according to the output of the function.
  StatusLed(IntervalCheck(avg)); //In this case use the return output from IntervalCheck as input for StatusLed.
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
}

/*Configuring Function |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ↓ |||||| ↓ ||*/
// This is the part where you cloud confuse AF even the one who write it got blind eye XD.
// Configure Master & tolerance value function _________________________________________________________________ ♂
void Configure_page(){
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
  Serial.println("Configure page");
  
  int valueState = 0;
  int scaleState = 0;
  int confirmed = 0;

  int scl_org = 0;
  float changeU1 = master_value; // Variable for storing the changes value before change the real one
  float changeU2 = tole;        // Same as above but no idea on the name Hehee ~~~~♪♫

  while(confirmed == 0 && mastole_state == 1 && machbath_state == 0){
    mastole_state = digitalRead(mc);
    machbath_state = digitalRead(mbc);
    Serial.println("confirmed = 0");
    lcd.clear();

    while(valueState == 0 && mastole_state == 1 && machbath_state == 0 && confirmed == 0){ // change master value ------→
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);

      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);

      Serial.println("master changing");
      //Create condition to cycle the scale and value selection
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;

      if(scaleState == 0){ //1 digit......x1..........
      scl_org = 1;
        if(adjust_plus > 0 && adjust_minus == 0)changeU1 = changeU1 + 1;
        if(adjust_plus == 0 && adjust_minus > 0)changeU1 = changeU1 - 1;
      }
      if(scaleState == 1){ //2 digit......x10.........
      scl_org = 10;
        if(adjust_plus > 0 && adjust_minus == 0)changeU1 = changeU1 + 10;
        if(adjust_plus == 0 && adjust_minus > 0)changeU1 = changeU1 - 10;
      }

      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 1;} //reset scale................
      if(confirm_set > 0)confirmed += 1;

      Lcd_setting_show("Standard Configure", scl_org, "Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "<-", "");
      delay(100);
    }
    lcd.clear();
    while(valueState == 1 && mastole_state == 1 && machbath_state == 0 && confirmed == 0){ // change tole value ------→
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);

      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);
      
      Serial.println("tole changing");
      //Create condition to cycle the scale and value selection
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;

      if(scaleState == 0){ //1 digit......x1..........
      scl_org = 1;
        if(adjust_plus > 0 && adjust_minus == 0)changeU2 = changeU2 + 1;
        if(adjust_plus == 0 && adjust_minus > 0)changeU2 = changeU2 - 1;
      }
      if(scaleState == 1){ //2 digit......x10.........
      scl_org = 10;
        if(adjust_plus > 0 && adjust_minus == 0)changeU2 = changeU2 + 10;
        if(adjust_plus == 0 && adjust_minus > 0)changeU2 = changeU2 - 10;
      }

      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 1;} //reset scale................
      if(confirm_set > 0)confirmed += 1;
      // String str_changeU2 = String(changeU2);
      Lcd_setting_show("Standard Configure", scl_org, "Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "", "<-");
      delay(100);
    }
    if(valueState > 1)valueState = 0;
  }
  if(confirmed >= 1 && mastole_state == 1 && machbath_state == 0){ // Confirm configure............
    Serial.println("setting secure");
    lcd.clear();
    Lcd_secure_show("Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "");
    master_value = changeU1;
    tole = changeU2;

    //Send the changes configuration in google sheet
    payload_0_c = payload_base_r + "\"" + master_value + "," + tole + "\"}";
    Serial.println("Publishing data...");
    Serial.println(payload_0_c);
    //Convert string into json object and send to google sheet.
    client.post(url_0, contentType, payload_0_c);
    int statusCode = client.responseStatusCode();
    Serial.println(statusCode);
      if (statusCode == 302 || statusCode == 303) {
          String location;
          while (client.headerAvailable()) {
            if (client.readHeaderName() == "Location") {
              location = client.readHeaderValue();
              break;
            }
          }
          if (location.length()) {
            uint16_t path_begin = location.substring(8).indexOf('/');
            String server = location.substring(8, path_begin + 8);
            String path = location.substring(path_begin + 8);
            client = HttpClient(wifi, server, port);
            client.get(path);
            int statusCode_re = client.responseStatusCode();
            Serial.print("Status code: after Redirect is: ");
            Serial.println(statusCode);
            if(statusCode_re == 200){
              String response_text = client.responseBody();
              Serial.println(response_text);
              Lcd_secure_show("Standard: ", String(changeU1), "Tolerance: ", String(changeU2), response_text);
            }
            else if(statusCode_re != 200){Lcd_secure_show("Standard: ", String(changeU1), "Tolerance: ", String(changeU2), "FAIL");}
          }
          
      }
    // If sent success g-script will return "success" on serial monitor.
    confirmed = 0;
    
    delay(1000); // Delay for a 0.5 sec to make sure the configure was changes.
  }
}

// Choose Machine & Bath number function _________________________________________________________________ ♀
void Choose_machine(){
  mastole_state = digitalRead(mc);
  machbath_state = digitalRead(mbc);
  Serial.println("Choose machine");
  
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
    Serial.println("confirmed = 0");

    while(valueState == 0 && mastole_state == 0 && machbath_state == 1 && confirmed == 0){ // change machine number....................................
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);

      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);

      Serial.println("machine changing");
      //Create condition to cycle the scale and value selection
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;

      if(scaleState == 0){ //1 digit......x1..........
      scl_org = 1;
        if(adjust_plus > 0 && adjust_minus == 0)maNo = maNo + 1;
        if(adjust_plus == 0 && adjust_minus > 0)maNo = maNo - 1;
      }
      if(scaleState == 1){ //2 digit......x10.........
      scl_org = 10;
        if(adjust_plus > 0 && adjust_minus == 0)maNo = maNo + 10;
        if(adjust_plus == 0 && adjust_minus > 0)maNo = maNo - 10;
      }
      if(maNo > 55) maNo = 1;
      if(maNo < 1) maNo = 55;
      
      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 1;} //reset scale................
      if(confirm_set > 0)confirmed += 1;
      maChange = String("Z") + maNo;
      Lcd_setting_show("Machine Setting", scl_org, "Machine No: ", maChange, "Bath No: ", baChange, "<-", "");
      delay(100);
    }
    lcd.clear();
    while(valueState == 1 && mastole_state == 0 && machbath_state == 1 && confirmed == 0){ // change bath number....................................
      adjust_plus = digitalRead(adP);
      adjust_minus = digitalRead(adM);
      scale_set = digitalRead(scl);
      values_select = digitalRead(vals);
      confirm_set = digitalRead(conf);

      mastole_state = digitalRead(mc);
      machbath_state = digitalRead(mbc);

      Serial.println("bath changing");
      //Create condition to cycle the scale and value selection
      if(scale_set > 0)scaleState+=1; if(values_select > 0)valueState+=1;

      if (scaleState == 0){ //Bath No....................
      scl_org = 6;
        if(adjust_plus > 0 && adjust_minus == 0){baNo = baNo + 1;}
        if(adjust_plus == 0 && adjust_minus > 0){baNo = baNo - 1;}
        if(baNo > 6) baNo = 1;
      	if(baNo < 1) baNo = 6;
      }
      if (scaleState == 1){ //Batch No.................
      scl_org = 3;
        if(adjust_plus > 0 && adjust_minus == 0){batchNo = batchNo + 1;}
        if(adjust_plus == 0 && adjust_minus > 0){batchNo = batchNo - 1;}
        if(batchNo > 3) batchNo = 1;
      	if(batchNo < 1) batchNo = 3;
      }

      if(scaleState > 1){scaleState = 0; lcd.clear(); scl_org = 6;} //reset scale................
      if(confirm_set > 0)confirmed += 1;
      baChange = baNo + String(" - ") + batchNo;
      Lcd_setting_show("Machine Setting", scl_org, "Machine No: ", maChange, "Bath No: ", baChange, "", "<-");
      delay(100);
    }
    if(valueState > 1)valueState = 0;
  }
  if(confirmed >= 1 && mastole_state == 0 && machbath_state == 1){ //Confirm configure............
    Serial.println("setting secure");
    lcd.clear();
    Lcd_secure_show("Machine No: ", maChange, "Bath No: ", baChange, "DONE");
    machineNo = maChange;
    bathNo = baChange;
    confirmed = 0;
    delay(1000); // Delay for a 0.5 sec to make sure the configure was changes.
  }
}

//Function for lighting the LEDs according the the input value
//(The return value of the choosen function)
void StatusLed(int x){
  if(x == 0){digitalWrite(led01, 1); digitalWrite(led02, 0); digitalWrite(led03, 0);}  //Green
  if(x == 1){digitalWrite(led01, 0); digitalWrite(led02, 1); digitalWrite(led03, 0);}  //Yellow
  if(x == 2){digitalWrite(led01, 0); digitalWrite(led02, 0); digitalWrite(led03, 1);}  //Red
  if(x == 3){digitalWrite(led01, 1); digitalWrite(led02, 1); digitalWrite(led03, 1);}  //Full
  else if(x == 4){digitalWrite(led02, 0);digitalWrite(led03, 0);digitalWrite(led01, 0);}          //Nothing  
}

void Lcd_measured_show(float x, float y, float z, float w, float a){
  lcd.clear();
  Serial.println("Input LCD: " + String(x) + " " + String(y) + " " + String(z) + " " + String(w) + " " + String(a));
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
  delay(145);
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


  String str_sv = a + b;
  lcd.setCursor(0, 3);
  lcd.print(str_sv);
  lcd.setCursor(18, 3);
  lcd.print(s2);
}

void Lcd_secure_show(String z, String w, String a, String b, String dd){
  String x = "Confirm setting";
  int set_header_midline = (20 - x.length())/2;
  lcd.setCursor(set_header_midline, 0);
  lcd.print(x);            

  String y = String("Changes value - ") + dd;
  lcd.setCursor(0, 1);
  lcd.print(y);            

  String str_fv = z + w;
  lcd.setCursor(0, 2);
  lcd.print(str_fv);

  String str_sv = a + b;
  lcd.setCursor(0, 3);
  lcd.print(str_sv);
}

void Lcd_upload_show(String xy, String yz, float z, float w, float a, float b, float c, float d, float e, String ss){
  String x = String("Uploading - ") + ss;
  int set_header_midline = (20 - x.length())/2;
  lcd.setCursor(set_header_midline, 0);
  lcd.print(x);           

  String y = String("Machine: ") + xy + " " + yz;
  lcd.setCursor(0, 1);
  lcd.print(y);           

  String str_fv = String("Values:") + d + ", " + e;
  lcd.setCursor(0, 2);
  lcd.print(str_fv);

  String str_sv = int(z) + String(", ") + int(w) + String(", ") + int(a) + String(", ") + int(b) + String(", ") + int(c);
  lcd.setCursor(0, 3);
  lcd.print(str_sv);
}

/*HTTP Function |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||| ↓ |||||| ↓ ||*/
// Function for uploading the data to google sheet via http protocal. มันโง่ดีครับอย่าไปใส่ใจ
void Send_To_Sheet(String mo, String bo, float x, float y, float z, float w, float a, float b, float c){
    lcd.clear();
    Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, "");
    payload_0 = payload_base_s + "\"" + mo + "," + bo + "," + x + "," + y + "," + z + "," + w + "," + a + "," + b + "," + c + "\"}";
    HttpClient client = HttpClient(wifi, serverAddress, port);
    Serial.println("Publishing data...");
    Serial.println(payload_0);
    //Setting string into json object and send to google sheet.
    client.post(url_0, contentType, payload_0);
    int statusCode = client.responseStatusCode();
    Serial.println(statusCode);
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
          uint16_t path_begin = location.substring(8).indexOf('/');
          String server = location.substring(8, path_begin + 8);
          String path = location.substring(path_begin + 8);
          client = HttpClient(wifi, server, port);
          client.get(path);
          int statusCode_re = client.responseStatusCode();
          Serial.print("Status code: after Redirect is: ");
          Serial.println(statusCode_re);
          lcd.clear();
          Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, String(statusCode_re));
          if(statusCode_re == 200 || statusCode_re == 302){
            String response_text = client.responseBody();
            Serial.println(response_text);
            lcd.clear();
            Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, response_text);
          }
          else if(statusCode_re == -2){Lcd_upload_show(mo, bo, x, y, z, w, a, b, c, "DONE");}
        }
    }
    // If sent success g-script will return "success" on serial monitor.
    delay(1000);
    lcd.clear();
    // a delay of several seconds is required before publishing again  
}

// Function for recieving the master &tolerance value from google sheet via http protocal.
void Recieve_From_Sheet(){
  int response_int[3];
  //In this part we will reassign the client to make sure it a empty client then use GET Function.
  HttpClient client = HttpClient(wifi, serverAddress, port);
  client.get(url_1);
  // Here affter use get function it will return the status of the response
  // For google script will use redirect http to create a response body instead of give a client a solid response         
  int statusCode = client.responseStatusCode();
  Serial.println(statusCode);
  lcd.clear();
  Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), String(statusCode));
  if (statusCode == 302 || statusCode == 303) {     // Now checking the tatus of the http wheter it needs to be redirect or not
      String location;
      while (client.headerAvailable()) {
        if (client.readHeaderName() == "Location") {  // As the given new path of http to get the response body from google sheet.
          location = client.readHeaderValue();        // We need to the variable first to parse it into server and key for initiate Http client.
          break;
        }
      }
      if (location.length()) {      // Parse the Location header for assigning the server and key.
        uint16_t path_begin = location.substring(8).indexOf('/');
        String server = location.substring(8, path_begin + 8);
        String path = location.substring(path_begin + 8);
        client = HttpClient(wifi, server, port);
        client.get(path);           // Use get aagin to get a response body of the redirect path.
        int statusCode_re = client.responseStatusCode();
        Serial.print("Status code: after Redirect is: ");
        Serial.println(statusCode_re);
        lcd.clear();
        Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), String(statusCode_re));
        if(statusCode_re == 200){
          String response_text = client.responseBody();   // Now we got the response body then we need to parse and assign into int value.
          uint16_t respo = response_text.indexOf("/");
          response_int[1] = response_text.substring(0, respo).toInt();
          response_int[2] = response_text.substring(respo+1).toInt();
          master_value = response_int[1];
          tole = response_int[2];
          Serial.println(response_int[1]);
          Serial.println(response_int[2]);
          lcd.clear();
          Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), "DONE");
          delay(1000);
        }
        else if(statusCode_re == -2){
          lcd.clear(); 
          master_value = response_int[1];
          tole = response_int[2];
          Lcd_secure_show("Standard: ", String(master_value), "Tolerance: ", String(tole), "FAIL");
          delay(1000);
          }
      }
      
  }
  

}



