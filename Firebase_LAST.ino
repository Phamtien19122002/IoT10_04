

#include <ESP32Firebase.h>
#include <DHT.h>
#include <ArduinoJson.h> 
#include <ESP32Servo.h>

String header;
Servo myServo;

unsigned long currentTime = 0;
unsigned long previousTime = 0;
const unsigned long interval = 1000;
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float temp , hum;

#define ssid "PiTit"          // Your WiFi SSID
#define password "motdentambo1"      // Your WiFi Password
#define REFERENCE_URL "https://esp32-4862b-default-rtdb.firebaseio.com"  // Your Firebase project reference url

Firebase firebase(REFERENCE_URL);


int state_led_light = 0;
int mode_led_light =0;
#define led_light 2
#define light_sensor 34


int state_led_motion = 0;
int mode_led_motion = 0;  
#define  pirPin 18
#define  led_motion 19


int state_motor_wire = 0;
int mode_motor_wire = 0, goc;
#define  motor_wire 4
#define  rain_sensor 16


#define gas_sensor 35
int state_gas =0;

void setup() {
  Serial.begin(115200);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  dht.begin();
   pinMode(led_light,OUTPUT);
   pinMode(led_motion,OUTPUT);
   pinMode(rain_sensor, INPUT);
   myServo.attach(motor_wire);
   pinMode(gas_sensor, INPUT);

}




void Control_led_light()
{
  if (mode_led_light == 1) {
    int lightValue = analogRead(light_sensor);
    if (lightValue > 2048) {
      digitalWrite(led_light, HIGH);
    } else {
      digitalWrite(led_light, LOW);
    }
   }  
}

void Control_led_motion()
{
  if (mode_led_motion == 1) {
    int motionDetected = digitalRead(pirPin);
    if (motionDetected == HIGH) {
      digitalWrite(led_motion, HIGH);
    } else {
      digitalWrite(led_motion, LOW);
    }
  }
  
}



void control_motor_wire() {
  if (mode_motor_wire == 1) {
    int rain = digitalRead(rain_sensor);
    if (rain == 1) {
      myServo.write(0);
    } else {
      myServo.write(90);
    }
  }
}


void jsondata()
{
  state_gas = digitalRead(gas_sensor);
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  Serial.println(temp);
  Serial.println(hum);
  firebase.setFloat("/iot_1/temp",temp);
  firebase.setFloat("/iot_1/hum",hum);
  firebase.setFloat("/iot_1/gas",state_gas);




  firebase.json(true);
  String data = firebase.getString("/iot");
  const size_t capacity = JSON_OBJECT_SIZE(6) + 50;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc,data);
  Serial.println(data);

  mode_led_light = doc["led_2"];
  state_led_light = doc["led_1"];
  mode_led_motion = doc["led_4"];
  state_led_motion = doc["led_3"];
  mode_motor_wire = doc["led_6"];
  state_motor_wire = doc["led_5"];
   Serial.println(mode_motor_wire);

  if(mode_led_light == 1){
    Serial.println("led1 auto");
  } else {
    if(state_led_light ==1) {
      digitalWrite(led_light,HIGH);
      Serial.println("led1 mo");
    }
    else {
      digitalWrite(led_light,LOW);
      Serial.println("led1 tat");
    }
  }

  if(mode_led_motion == 1){
    Serial.println("led_2 auto");
  } else {
    if(state_led_motion ==1) {
      digitalWrite(led_motion,HIGH);
      Serial.println("led2 mo");
    }
    else {
      digitalWrite(led_motion,LOW);
      Serial.println("led2 dong");
    }
  }

  if(mode_motor_wire == 1){
    Serial.println("door_auto");
  } else {
    if(state_motor_wire ==1) {
      myServo.write(0);
      Serial.println("cua mo");
    }
    else {
      myServo.write(90);
      Serial.println("cuadong");
    }
  }


}

void loop() {

  Control_led_light();
  Control_led_motion();
  control_motor_wire();

    jsondata();

  
  
}
