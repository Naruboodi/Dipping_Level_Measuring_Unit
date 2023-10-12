#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// https://randomnerdtutorials.com/esp32-http-get-post-arduino/
// https://www.electroniclinic.com/send-data-from-google-spreadsheet-or-google-sheets-to-esp8266-or-esp32/
#define dip1 5
#define dip2 35
#define mc 9
#define mbc 10
#define led01 25
#define led02 33
#define led03 32
#define adP 4
#define adM 2
#define scl 16
#define vals 17
#define conf 15
#define upl 0

#define ulS1 26
#define ulS2 27
#define ulS3 14
#define ulS4 12
#define ulS5 13

char wifi_id[] = "KEMET Manufacturing";
char wifi_pass[] = "P0lym3riZ@t!on";

const char* serverAddress = "script.google.com";
const int port = 443;
String GOOGLE_SCRIPT_ID = "AKfycbxGIHyBaO-i5JUZOk8FeeN82W5_BNz3OcvO2tDcSg5OVeGwtUYjUeKRtuHxrzCKLDp0";
String contentType = "application/json";
String payload_base_s =  "{\"command\": \"append_row\", \"sheet_name\": \"Dashboard\", \"values\": ";
String payload_base_r =  "{\"command\": \"change_value\", \"sheet_name\": \"Dashboard\", \"values\": ";
String payload_0 = "";
String payload_0_c = "";
String payload_1 = "";
String payload_2 = "";
String url_0 = String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec";
String url_1 = String("/macros/s/") + GOOGLE_SCRIPT_ID + "/exec?master=N2&tole=O2";

int retval = 0;
bool flag = false;

LiquidCrystal_I2C lcd(0x27,20,4);

float master_value = 15.0;
float tole = 2;
float ref_height = 75;
int maNo = 25;
int baNo = 1;
int batchNo = 1;
String machineNo = String("Z") + maNo;
String bathNo = baNo + String(" - ") + batchNo;

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

const int numReading = 40;
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

int count = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.clear();
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

  WiFi.begin(wifi_id, wifi_pass);
  while ( WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void loop() {
  while(WiFi.status() == WL_CONNECTED){
    Serial.println("Started");
    int meas01 = random(320,380);
    int meas02 = random(320,380);
    int meas03 = random(320,380);
    int meas04 = random(320,380);
    int meas05 = random(320,380);

    maNo = int(random(1, 55));
    baNo = int(random(1, 7));
    batchNo = int(random(1, 4));
    String maChange = String("Z") + maNo;
    String baChange = String(baNo) + String(" - ") + batchNo;
    Send_To_Sheet(maChange, baChange, master_value, tole, meas01, meas02, meas03, meas04, meas05);
    Recieve_From_Sheet();
  }
    WiFi.begin(wifi_id, wifi_pass);
    while ( WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
}

void Send_To_Sheet(String mo, String bo, float x, float y, float z, float w, float a, float b, float c){
  Serial.println("Started sending");
  WiFiClientSecure wifi;
  wifi.setInsecure();
  payload_0 = payload_base_s + "\"" + mo + "," + bo + "," + x + "," + y + "," + z + "," + w + "," + a + "," + b + "," + c + "\"}";
  HttpClient client = HttpClient(wifi, serverAddress, port);
  client.post(url_0, contentType, payload_0);
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
        client.stop();
        uint16_t path_begin = location.substring(8).indexOf('/');
        String server_re = location.substring(8, path_begin + 8);
        String path_re = location.substring(path_begin + 8);
        HttpClient client_re = HttpClient(wifi, server_re, port);
        Serial.println(path_re);
        client_re.get(path_re);
        int statusCode_re = client_re.responseStatusCode();
        Serial.println(statusCode_re);
        if(statusCode_re == 200 || statusCode_re == 302){
          String response_text = client_re.responseBody();
          Serial.println(response_text);
          client_re.stop();
          delay(1000);
        }
        else if(statusCode_re == -2){
          Serial.println("Why fail..");
          client_re.stop();
          delay(1000);}
      }
  }
  delay(1000);
  lcd.clear();
}
void Recieve_From_Sheet(){
  Serial.println("Started reading");
  WiFiClientSecure wifi;
  wifi.setInsecure();
  int response_int[3];
  HttpClient client = HttpClient(wifi, serverAddress, port);
  client.get(url_1);     
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
        client.stop();
        uint16_t path_begin = location.substring(8).indexOf('/');
        String server_re = location.substring(8, path_begin + 8);
        Serial.println(server_re);
        String path_re = location.substring(path_begin + 8);
        HttpClient client_re = HttpClient(wifi, server_re, port);
        Serial.println(path_re);
        client_re.get(path_re);
        int statusCode_re = client_re.responseStatusCode();
        Serial.println(statusCode_re);
         if(statusCode_re == 200){
          String response_text = client_re.responseBody();
          uint16_t respo = response_text.indexOf("/");
          response_int[1] = response_text.substring(0, respo).toInt();
          response_int[2] = response_text.substring(respo+1).toInt();
          master_value = response_int[1];
          tole = response_int[2];
          Serial.println("Recieved: " + String(master_value) + ", " + String(tole));
          client_re.stop();
          delay(1000);
        }
        else {
          master_value = response_int[1];
          tole = response_int[2];
          Serial.println("Why fail..");
          client_re.stop();
          delay(1000);
        }
      }
  }
}