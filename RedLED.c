#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>  // Include SPI if you're using SPI
#include <SFE_MicroOLED.h>  // Include the SFE_MicroOLED library
#include <HTTPClient.h> 

const char* ssid     = "RPi3";
const char* password = "smartmeters";

int Count = 23;

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 12  // Connect RST to pin 9 (req. for SPI and I2C)
#define PIN_DC    13  // Connect DC to pin 8 (required for SPI)
#define PIN_CS    5 // Connect CS to pin 10 (required for SPI)
#define DC_JUMPER 0
// Also connect pin 13 to SCK and pin 11 to MOSI

Adafruit_INA219 ina219;
int OutputPin = 14;

WiFiServer server(80);

//////////////////////////////////
// MicroOLED Object Declaration //
//////////////////////////////////
MicroOLED oled(PIN_RESET, PIN_DC, PIN_CS);

void setup()
{
    pinMode(OutputPin, OUTPUT);
    Serial.begin(115200);
    
    while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
    }
    
    pinMode(5, OUTPUT);      // set the LED pin mode
    delay(10);
    digitalWrite(OutputPin, HIGH);
    // We start by connecting to a WiFi network

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
  // clear(ALL) will clear out the OLED's graphic memory.
  // clear(PAGE) will clear the Arduino's display buffer.
  oled.clear(ALL);  // Clear the display's memory (gets rid of artifacts)
  // To actually draw anything on the display, you must call the
  // display() function. 
  oled.print("Hello, world");
  oled.display();   
}

int value = 0;

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
 // float power = 0;

 if(Count != 0)
 {
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
 // power = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  Serial.println("Data from the current sensor:");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
//Serial.print("Power:       "); Serial.print(power); Serial.println(" mW");
  Serial.println("");
  delay(1000);
  oled.begin();
  // clear(ALL) will clear out the OLED's graphic memory.
  // clear(PAGE) will clear the Arduino's display buffer.
  oled.clear(ALL);  // Clear the display's memory (gets rid of artifacts)
  // To actually draw anything on the display, you must call the
  // display() function. 
  oled.println("Voltage:           "); oled.print(loadvoltage); oled.print(" V    ");
  oled.print("Current:  "); 
  oled.print(current_mA); 
  oled.print(" mA            ");
  oled.println(" ");
  oled.display();  


// Http Server  
  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED off.<br>");
            client.println();            
            client.print("Load Voltage:  "); client.print(loadvoltage); client.println(" V");
            client.print("Current:       "); client.print(current_mA); client.println(" mA");
            
          //client.print("Power:       "); client.print(power); client.println(" mW");
          //client.print("Bus Voltage:   "); client.print(busvoltage); client.println(" V");
          //client.print("Shunt Voltage: "); client.print(shuntvoltage); client.println(" mV");
  
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(OutputPin, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(OutputPin, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
    //delay(2000);
  }
 }
else
{
// HTTP client of the node - Blue Strip 
   if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin("http://192.168.42.9/"); //Specify the URL
   // int httpCode = http.GET();                                        //Make the request
 
    while (http.GET()) 
    { //Check for the returning code
 
        String payload = http.getString();
        Serial.println("Data from neighbour device - Blue Strip lamp");
        //Serial.println(httpCode);
        Serial.println(payload);
        break;
      }
 
    http.end(); //Free the resources
  }
  //   delay(2000);
  // HTTP client of the node - Red Strip 
   if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
 
    http.begin("http://192.168.42.5/"); //Specify the URL
   // int httpCode = http.GET();                                        //Make the request
 
    while(http.GET()) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println("Data from neighbour device - Red Strip");
        //Serial.println(httpCode);
        Serial.println(payload);
        break;
      }
 
    http.end(); //Free the resources
  }
  Count = 10;
}
Count = Count - 1;
}
