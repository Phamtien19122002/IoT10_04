#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ESP32Servo.h>

const char *ssid = "Pham Van Tien";
const char *password = "emkhongbiet";

WiFiServer server(80);

String header;
Servo myServo;

// Đèn sân
String state_led_light = "off";
int mode_led_light = 0;  // 0: Manual, 1: Auto
#define led_light 2
#define light_sensor 34

// Đèn phòng tắm
String state_led_motion = "off";
int mode_led_motion = 0;  // 0: Manual, 1: Auto
#define  pirPin 18
#define  led_motion 19

// Mưa - Dây phơi
String state_motor_wire = "off";
int mode_motor_wire = 0, goc;
#define  motor_wire 4
#define  rain_sensor 16

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  pinMode(led_light, OUTPUT);
  pinMode(led_motion, OUTPUT);
  pinMode(rain_sensor, INPUT);
  analogReadResolution(12);
  digitalWrite(led_light, LOW);
  digitalWrite(led_motion, LOW);
  myServo.attach(motor_wire);
}

// Hàm điều khiển đèn sân
void control_led_denSan() {
  if (mode_led_light == 1) {
    int lightValue = analogRead(light_sensor);
    Serial.println(lightValue);
    if (lightValue > 2048) {
      Serial.println("Low light, turning GPIO 2 on");
      digitalWrite(led_light, HIGH);
    } else {
      Serial.println("Sufficient light, turning GPIO 2 off");
      digitalWrite(led_light, LOW);
    }
  }
}

// Hàm điều khiển đèn phòng tắm
void control_led_motion() {
  if (mode_led_motion == 1) {
    int motionDetected = digitalRead(pirPin);
    if (motionDetected == HIGH) {
      Serial.println("Motion detected, turning LED in bathroom on");
      digitalWrite(led_motion, HIGH);
    } else {
      Serial.println("No motion, turning LED in bathroom off");
      digitalWrite(led_motion, LOW);
    }
  }
}

//Hàm điều khiển dây phơi
void control_motor_wire() {
  if (mode_motor_wire == 1) {
    int rain = digitalRead(rain_sensor);
    if (rain == 1) {
      Serial.println("It's not rains, clothes are drying");
      myServo.write(0);
    } else {
      Serial.println("It's raining, clothes have been packed");
      myServo.write(90);
    }
  }
}

void loop() {
  control_led_denSan();
  control_led_motion();
  control_motor_wire();
  WiFiClient client = server.available();
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (mode_led_light == 0) {
              if (header.indexOf("GET /2/on") >= 0) {
                Serial.println("GPIO 2 on");
                state_led_light = "on";
                digitalWrite(led_light, HIGH);
              } else if (header.indexOf("GET /2/off") >= 0) {
                Serial.println("GPIO 2 off");
                state_led_light = "off";
                digitalWrite(led_light, LOW);
              }
            }
            if (mode_led_motion == 0) {
              if (header.indexOf("GET /19/on") >= 0) {
                Serial.println("LED in bathroom on");
                state_led_motion = "on";
                digitalWrite(led_motion, HIGH);
              } else if (header.indexOf("GET /19/off") >= 0) {
                Serial.println("LED in bathroom off");
                state_led_motion = "off";
                digitalWrite(led_motion, LOW);
              }
            }
            if (mode_motor_wire == 0) {
              if (header.indexOf("GET /4/on") >= 0) {
                Serial.println("clothes are drying");
                state_motor_wire = "on";
                myServo.write(0);
              } else if (header.indexOf("GET /4/off") >= 0) {
                Serial.println("clothes have been packed");
                state_motor_wire = "off";
                myServo.write(90);
              }
            }
            if (header.indexOf("GET /auto/denSan/on") >= 0) {
              Serial.println("Auto mode for Den San on");
              mode_led_light = 1;
            } else if (header.indexOf("GET /auto/denSan/off") >= 0) {
              Serial.println("Auto mode for Den San off");
              mode_led_light = 0;
            }
            if (header.indexOf("GET /auto/denPhongTam/on") >= 0) {
              Serial.println("Auto mode for Den Phong Tam on");
              mode_led_motion = 1;
            } else if (header.indexOf("GET /auto/denPhongTam/off") >= 0) {
              Serial.println("Auto mode for Den Phong Tam off");
              mode_led_motion = 0;
            }
            if (header.indexOf("GET /auto/day/on") >= 0) {
              Serial.println("Auto mode for day on");
              mode_motor_wire = 1;
            } else if (header.indexOf("GET /auto/day/off") >= 0) {
              Serial.println("Auto mode for day off");
              mode_motor_wire = 0;
            }
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset=\"UTF-8\" /><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />");
            client.println("<link rel=\"icon\" href=\"data:,\" />");
            client.println("<style>");
            client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }");
            client.println("body { background-color:rgb(31, 31, 81); }");
            client.println(".button-container { display: flex; justify-content: space-around; }");
            client.println(".button-container a { flex: 1; }");
            client.println(".button { background-color: rgb(199, 186, 6); border: none; color: #fff; padding: 10px 20px; text-decoration: none; font-size: 20px; margin: 5px; cursor: pointer; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); transition: 0.3s; width: 200px; }");
            client.println(".button:hover { box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2); }");
            client.println(".button2 { background-color: #fff; color:rgb(199, 186, 6) }");
            client.println(".button3 { background-color:rgb(139, 139, 216); color: #bdc5bd;}");
            client.println(".icon { font-size: 50px; margin-bottom: 10px; }");
            client.println(".thena {width: 250px;background-color:rgb(93, 93, 152) ;border-radius: 20px ; }");
            client.println("</style></head>");
            client.println("<body>");
            client.println("<h1 style=\"color: rgb(93, 93, 152)\" >CONTROL</h1>");
            client.println("<div class=\"container\">");
            client.println("<div class=\"button-container\">");

            // code của led và cảm biến ánh sáng
            client.println("<div class=\"thena\">");
            client.println("<span class=\"icon\">&#128161;</span>");
            client.println("<p>Led_light</p>");
            client.println("<p>Chế độ: " + String(mode_led_light == 1 ? "Tự động" : "Thủ công") + "</p>");
            if (mode_led_light == 0) {
              if (state_led_light == "off") {
                client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
                client.println("<p><a href=\"/auto/denSan/on\"><button class=\"button\">Auto</button></a></p>");
              } else {
                client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
                client.println("<p><a href=\"/auto/denSan/on\"><button class=\"button\">Auto</button></a></p>");
              }
            } else {
              client.println("<p><button class=\"button button3\">ON</button></p>");
              client.println("<p><a href=\"/auto/denSan/off\"><button class=\"button button2\">Off Auto</button></a></p>");
            }
            client.println("</div>");

            // code của led cảm biến chuyển động
            client.println("<div class=\"thena\">");
            client.println("<span class=\"icon\">&#128161;</span>");
            client.println("<p>Led_motion</p>");
            client.println("<p>Chế độ: " + String(mode_led_motion == 1 ? "Tự động" : "Thủ công") + "</p>");
            if (mode_led_motion == 0) {
              if (state_led_motion == "off") {
                client.println("<p><a href=\"/19/on\"><button class=\"button\">On</button></a></p>");
                client.println("<p><a href=\"/auto/denPhongTam/on\"><button class=\"button\">Auto</button></a></p>");
              } else {
                client.println("<p><a href=\"/19/off\"><button class=\"button button2\">OFF</button></a></p>");
                client.println("<p><a href=\"/auto/denPhongTam/on\"><button class=\"button\">Auto</button></a></p>");
              }
            } else {
              client.println("<p><button class=\"button button3\">ON</button></p>");
              client.println("<p><a href=\"/auto/denPhongTam/off\"><button class=\"button button2\">Off Auto</button></a></p>");
            }
            client.println("</div>");

            // Cảm biến mưa và dây phơi
            client.println("<div class=\"thena\">");
            client.println("<span class=\"icon\">&#128161;</span>");
            client.println("<p>motor_wire</p>");
            client.println("<p>Chế độ: " + String(mode_motor_wire == 1 ? "Phơi" : "Thu") + "</p>");
            if (mode_motor_wire == 0) {
              if (state_motor_wire == "off") {
                client.println("<p><a href=\"/4/on\"><button class=\"button\">On</button></a></p>");
                client.println("<p><a href=\"/auto/day/on\"><button class=\"button\">Auto</button></a></p>");
              } else {
                client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
                client.println("<p><a href=\"/auto/day/on\"><button class=\"button\">Auto</button></a></p>");
              }
            } else {
              client.println("<p><button class=\"button button3\">ON</button></p>");
              client.println("<p><a href=\"/auto/day/off\"><button class=\"button button2\">Off Auto</button></a></p>");
            }
            client.println("</div>");

            client.println("</div>");
            client.println("</div>");
            client.println("</div></body></html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}