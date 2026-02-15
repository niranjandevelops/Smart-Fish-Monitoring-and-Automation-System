#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "your-ID"
#define BLYNK_TEMPLATE_NAME "name"
#define BLYNK_AUTH_TOKEN "your-token"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "admin123";
char pass[] = "admin123";

#define TRIG_PIN 23
#define ECHO_PIN 25
#define CT_MTR   27
#define FT_MTR   26
long duration;
float distance;

// ---------------- Servo ----------------
Servo myservo; 
int pos = 0;
int servoPin = 17;

// ---------------- DS18B20 ----------------
#define ONE_WIRE_BUS 15
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ---------------- pH ----------------
float calibration_value = 20.24 - 0.7; //21.34 - 0.7
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;
float ph_act;


BLYNK_WRITE(V2) {  // servo control FishFeed
  int FF = param.asInt();
  if(FF==1){
  Servo_FEED();
  }
}
BLYNK_WRITE(V3) {  // CLEAN
  int CT = param.asInt();
  if(CT==1){
   do{
     Read_Level();
     digitalWrite(CT_MTR,HIGH);
     Serial.println("CT_ON");
      if(distance>40){
      digitalWrite(CT_MTR,LOW); 
      Serial.println("CT_OFF");
      }
     }while(distance<40);
  }
}

BLYNK_WRITE(V4) {  // CLEAN
  int FT = param.asInt();
  if(FT==1){
   do{
     Read_Level();
      digitalWrite(FT_MTR,HIGH);
      Serial.println("FT_ON");
      if(distance<25){
      digitalWrite(FT_MTR,LOW); 
      Serial.println("FT_OFF");
      }
     }while(distance>25);
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(CT_MTR  , OUTPUT);
  pinMode(FT_MTR  , OUTPUT);
  // DS18B20 init
  sensors.begin();
  myservo.attach(servoPin, 1000, 2000);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();
  // Temperature
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  Serial.print("Temperature:");
  Serial.print(temperatureC);
  Serial.println(" C ");
  Blynk.virtualWrite(V0, temperatureC);
  // Ultrasonic
  Read_Level();
  // pH
  Read_pH();
  digitalWrite(CT_MTR,LOW);
  digitalWrite(FT_MTR,LOW);
}





void Read_Level(){
    // Clear trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Read echo
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculate distance (speed of sound = 343 m/s)
  distance = duration * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(500);
}

void Read_pH(){
 for(int i=0;i<10;i++) 
 { 
 buffer_arr[i]=analogRead(35);
 delay(30);
 }
 for(int i=0;i<9;i++)
 {
 for(int j=i+1;j<10;j++)
 {
 if(buffer_arr[i]>buffer_arr[j])
 {
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*3.3/4096.0/6;  
 //Serial.print("Voltage: ");
 //Serial.println(volt);
 ph_act = -5.70 * volt + calibration_value;
 Serial.print("pH Val: ");
 Serial.println(ph_act);
 Blynk.virtualWrite(V1, ph_act);
}

void Servo_FEED(){
   for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
  delay(500);
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(15);             // waits 15ms for the servo to reach the position
  }
}