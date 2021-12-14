#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FreeRTOS.h>
#include <HTTPClient.h>

const char *SSID = "ISIMA_Etudiants";
const char *PWD = "test1234";

const byte TRIGGER_PIN = 12; // Broche TRIGGER
const byte ECHO_PIN = 13;    // Broche 
float  distance ,duration ;
//int const buzzPin = 2;


// Web server running on port 80 (http port)
WebServer server(80);

 

// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

// connection to the wifi mba3id ndeha fil setup
void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  WiFi.begin(SSID, PWD);
  // wifi for connection 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    // we can even make the ESP32 to sleep
  }
 
  Serial.print("Connected. IP address: ");
  Serial.println(WiFi.localIP());
}

// setup API resources
void setup_routing() {     
  server.on("/distance", getDistance);
  // start server    
  server.begin(); 
}

void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
 
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}

void read_sensor_data(void * parameter) {
   for (;;) {
      
      Serial.println("Read sensor data");
      digitalWrite(TRIGGER_PIN, HIGH);
      delayMicroseconds(1000);
      digitalWrite(TRIGGER_PIN, LOW);

      duration = pulseIn(ECHO_PIN, HIGH); // receive reflected waves
      distance = duration / 58.2;   // convert to distance

  /* if (distance <= 50 && distance >= 0) {
       // Buzz
        digitalWrite(buzzPin, HIGH);
        } else {
       // Don't buzz
       digitalWrite(buzzPin, LOW);
        }
       // Waiting 60 ms won't hurt any one
        delay(60);*/
      delay(1000); 
     // delay the task
     vTaskDelay(60000 / portTICK_PERIOD_MS);
   }
}

// implement the Rest API
void getDistance() {
  Serial.println("Get distance");
  create_json("distance",distance, "CM");
  server.send(200, "application/json", buffer);
}

void handlePost() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  
  // Respond to the client
  server.send(200, "application/json", "{}");
}

void setup_task() {    
  xTaskCreate(     
  read_sensor_data,      
  "Read sensor data",       
  1000,      
  NULL,      
  1,     
  NULL     
  );     
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);    
     
  // Sensor setup    
  connectToWiFi();     
  setup_task();  
  /*
   *  if (distance <= 50 && distance >= 0) {
       // Buzz
        digitalWrite(buzzPin, HIGH);
        } else {
       // Don't buzz
       digitalWrite(buzzPin, LOW);
        }
       // Waiting 60 ms won't hurt any one
        delay(60);
   */  
  setup_routing(); 
  /* Initialise les broches */
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, LOW); // La broche TRIGGER doit être à LOW au repos
  pinMode(ECHO_PIN, INPUT);
 // pinMode(buzzPin, OUTPUT); // buzz pin is output to control buzzering

}

void loop() {
  server.handleClient();
}
