#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>  
#include <SFE_MicroOLED.h>  
#include <HTTPClient.h> 

const char* ssid     = "RPi3";
const char* password = "smartmeters";

int Count = 5;
#define PIN_RESET 12  
#define PIN_DC    13  
#define PIN_CS    5 
#define DC_JUMPER 0

Adafruit_INA219 ina219;
int OutputPin = 11;

WiFiServer server(80);
MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS);

void setup()
{
    pinMode(OutputPin, OUTPUT);
    Serial.begin(115200);
    
    while (!Serial) {
      delay(1);
    }
    
    pinMode(5, OUTPUT);      
    delay(10);
    digitalWrite(OutputPin, HIGH);
 
    //connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();
    uint32_t currentFrequency;  
    Serial.println("Hello!");
    ina219.begin();
    Serial.println("Measuring voltage and current with INA219 ...");
 
    oled.begin();
    oled.clear(ALL);  
    oled.print("Hello, world");
    oled.display();   
}

int value = 0;

void loop(){
 WiFiClient client = server.available();   
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;

 if(Count != 0)
 {
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  Serial.println("Data from the current sensor:");
  Serial.print("Load Voltage:  "); 
  Serial.print(loadvoltage); 
  Serial.println(" V");
  Serial.print("Current:       "); 
  Serial.print(current_mA); 
  Serial.println(" mA");
  Serial.println("");
  delay(1000);
  oled.begin();
  oled.clear(ALL);   
  oled.println("Voltage:           "); 
  oled.print(loadvoltage); 
  oled.print(" V    ");
  oled.print("Current:  "); 
  oled.print(current_mA); 
  oled.print(" mA            ");
  oled.println(" ");
  oled.display();  

// Http Server  
  if (client) {                             
    Serial.println("New Client.");            
    String currentLine = "";                 
    while (client.connected()) {            
      if (client.available()) {              
        char c = client.read();            
        Serial.write(c);                    
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("Click <a href=\"/H\">here</a> to turn the LED on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED off.<br>");
            client.println();            
            client.print("Load Voltage:  "); 
            client.print(loadvoltage); 
            client.println(" V");
            client.print("Current:       "); 
            client.print(current_mA); 
            client.println(" mA");
            client.println();
            break;
          } 
         else {    
            currentLine = "";
          }
        } 
       else if (c != '\r') { 
          currentLine += c;      
        }
       
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(OutputPin, HIGH);               
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(OutputPin, LOW);                
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
 }
else
{
// HTTP client of the node - Red LED 
   if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin("http://192.168.42.18/"); //Specify the URL
    int httpCode = http.GET();                                        //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println("Data from neighbour device - Red LED lamp");
        //Serial.println(httpCode);
        Serial.println(payload);
      }
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }

  // HTTP client of the node - Blue Strip 
   if ((WiFi.status() == WL_CONNECTED)) 
   { 
    HTTPClient http;
    http.begin("http://192.168.42.9/"); 
    int httpCode = http.GET();                                        
    if (httpCode > 0) { 
        String payload = http.getString();
        Serial.println("Data from neighbour device - Blue Strip");
        Serial.println(payload);
      }
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); 
  }
  Count = 5;
}
Count = Count - 1;
}
