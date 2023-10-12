/*
This example creates a client object that connects and transfers
data using always SSL.

It is compatible with the methods normally related to plain
connections, like client.connect(host, port).

Written by Arturo Guadalupi
last revision November 2015

*/
#include <ArduinoHttpClient.h> 
#include <Arduino.h>
#include <WiFiNINA.h>


///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = "KEMET Manufacturing";
char pass[] = "P0lym3riZ@t!on";   // your network password (use for WPA, or use as key for WEP)

//Setup HTTP client
int port = 443; // server port of goole **don't change**
const char serverAddress[] = "script.google.com"; // don't change
WiFiSSLClient wifi;

//Set google script key
String GOOGLE_SCRIPT_ID = "AKfycbxjCAVbmlXu9-tmt_mxFm3kSe7MdttxvLh9wCCYZZ8Tq9H-IKAPoRkacARwuGOqikc4";

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
String url_0 = String("/macros/s/") + GOOGLE_SCRIPT_ID_SEND + "/exec";
String url_1 = String("/macros/s/") + GOOGLE_SCRIPT_ID_GET + "/exec?master=N2&tole=O2";

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  Serial.println("\n\n\n\n\n\n\n\n\n\n");

  //Connect WIFI
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED) {  
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.SSID());
  Serial.println(WiFi.localIP());
}
void loop() {
  // Serial.println("....Ok....");
  //   Serial.print("Enter you values: ");
  //   while(Serial.available()==0){
  //   }
  //   int sensor_now = Serial.parseInt();
  //   if(sensor_now == 1){
  //     Send_To_Sheet("Z32", "2 - 2", 17, 17, 16, 16, 16, 16, 2);
  //   }
  //   if(sensor_now == 2){
  //     Recieve_From_Sheet();
  //   }
  Serial.println("Start sending: ");
  Send_To_Sheet("Z32", "2 - 2", 17, 17, 16, 16, 16, 16, 2);
  Serial.println("Start reading: ");
  Serial.println(GOOGLE_SCRIPT_ID_SEND.length());
  Recieve_From_Sheet();
  delay(750);
  while(1);
}
// Function for recieving the master &tolerance value from google sheet via http protocal. มันโง่ดีครับอย่าไปใส่ใจ
void Recieve_From_Sheet(){
  //I don't know how to explain this thing cause the line of code have straight meaning ;-;.
  HttpClient client = HttpClient(wifi, serverAddress, port);
  client.get(url_1);         //
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
        statusCode = client.responseStatusCode();
        Serial.print("Status code: after Redirect is: ");
        Serial.println(statusCode);
      }
  }
  String response_text = client.responseBody();
  uint16_t respo = response_text.indexOf("/");
  int response_int[3];
  response_int[1] = response_text.substring(0, respo).toInt();
  response_int[2] = response_text.substring(respo+1).toInt();
  Serial.println(response_int[1]);
  Serial.println(response_int[2]);

}

  
    

void Send_To_Sheet(String mo, String bo, float x, float y, float z, float w, float a, float b, float c){
    HttpClient client = HttpClient(wifi, serverAddress, port);
    // Create string to send to Google Sheets
    // Parameter is values of 5 sensors x-a
    payload_0 = payload_base_s + "\"" + mo + "," + bo + "," + x + "," + y + "," + z + "," + w + "," + a + "," + b + "," + c + "\"}";
    
    Serial.println("Publishing data...");
    Serial.println(payload_0);
    //Convert string into json object and send to google sheet.
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
            uint16_t path_begin = location.substring(8).indexOf('/');
            String server = location.substring(8, path_begin + 8);
            String path = location.substring(path_begin + 8);
            client = HttpClient(wifi, server, port);
            client.get(path);
            statusCode = client.responseStatusCode();
            Serial.print("Status code: after Redirect is: ");
            Serial.println(statusCode);
          }
          Serial.println(client.responseBody());
      }
    // If sent success g-script will return "success" on serial monitor.
    delay(500);
    // a delay of several seconds is required before publishing again  
}

