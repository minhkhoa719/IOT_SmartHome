
// Fill-in information from your Blynk Template here
// Đây là phần thêm vào trong code từ template nãy bạn khởi tạo
#define BLYNK_TEMPLATE_ID "TMPLoDSsdks7"
#define BLYNK_DEVICE_NAME "smart home"
#define BLYNK_FIRMWARE_VERSION "0.1.3"
#define BLYNK_PRINT Serial
#define USE_NODE_MCU_BOARD
#include "BlynkEdgent.h"
#include <SimpleTimer.h>
#include "DHT.h"
#include <Servo.h>
#include <SoftwareSerial.h>
#include <ESP_Mail_Client.h>
#define DHTPIN D3 
#define DHTTYPE DHT11
#define LED1 D8
#define lightSensor D6

Servo myServo;
SoftwareSerial NodeMCU(D1,D2);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

float t, h;
char c;
String dataIn;
int8_t indexOfA, indexOfB,indexOfC;
String data1,data2, data3;
String str;
int lightValue = 0;
int flagLight = 0;
int flagTP = 0;
int flagOTP = 0;
int flagDoor = 0;
int flagWindow = 0 ;
long timePass;
long oneTimePassword;

// đọc giá trị cảm biến nhiệt độ, độ ẩm và gửi len Blynk
void sendSensor()
{
  h = dht.readHumidity();
  t = dht.readTemperature();  
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V1, t);
}
// turn Light
BLYNK_WRITE(V3){
  int p = param.asInt();
  if(p == 1) {
    digitalWrite(LED1,1);  
  } else {
    digitalWrite(LED1,0);
  }
}
//Remote door
BLYNK_WRITE(V9){
  int p = param.asInt();
  if(p==1){
    flagDoor = 1; 
  } else {
    flagDoor = 0;  
  }
}
//Time pass
BLYNK_WRITE(V4){
  int p = param.asInt();
  if(p == 1){
    flagTP = 1;
    timePass = random(100000,999999);
    Blynk.virtualWrite(V5,timePass);
  } else {
    flagTP = 0;
    timePass = 0;
    Blynk.virtualWrite(V5,0);
  }
}
//OneTimePass
BLYNK_WRITE(V7){
  int p = param.asInt();
  if(p == 1){
    flagOTP = 1;
    oneTimePassword = random(100000,999999);
    Blynk.virtualWrite(V8,oneTimePassword);
  } else {
    flagOTP = 0;
    oneTimePassword = 0;
    Blynk.virtualWrite(V8,0);
  }
}
// xử lí dữ liệu
void Parse_the_Data(){
  indexOfA = dataIn.indexOf("A");
  indexOfB = dataIn.indexOf("B");
  indexOfC = dataIn.indexOf("C");

  data1 = dataIn.substring(0, indexOfA);
  data2 = dataIn.substring(indexOfA + 1, indexOfB);
  data3 = dataIn.substring(indexOfB + 1, indexOfC);
}
// gửi dữ liệu tới Arduino
void sendDatatoArduino(){  
   if(flagDoor == 1){
    NodeMCU.print((String)timePass); NodeMCU.print("A");
    NodeMCU.print((String)oneTimePassword); NodeMCU.print("B");
    NodeMCU.print((String)flagDoor); NodeMCU.print("C");
    NodeMCU.print("\n");  
  }else if(flagTP == 1  || flagOTP == 1 ){
    NodeMCU.print((String)timePass); NodeMCU.print("A");
    NodeMCU.print((String)oneTimePassword); NodeMCU.print("B");
    NodeMCU.print((String)0); NodeMCU.print("C");
    NodeMCU.print("\n");
  }
  else {
    NodeMCU.print((String)0); NodeMCU.print("A");
    NodeMCU.print((String)0); NodeMCU.print("B");
    NodeMCU.print((String)0); NodeMCU.print("C");
    NodeMCU.print("\n");
  }
  // đọc dữ liệu từ arduino
  while(NodeMCU.available()>0){
     c = NodeMCU.read();
     if(c == '\n'){break;}
     else {dataIn+=c;} 
  }
  if(c== '\n'){
    Parse_the_Data();
    //Show the data
    Serial.println(data1);
    Serial.println(data2);
    Serial.println(data3);
    Serial.println("===================");
    if(data1.toInt() == 0){
      oneTimePassword = data1.toInt();
      Blynk.virtualWrite(V8,0);
      Blynk.virtualWrite(V7,0);  
    }
    if( data2.toInt() == 0){
      flagDoor = 0;
      Blynk.virtualWrite(V9,flagDoor);  
    }     
    if(data3.toInt() == 1){
      Blynk.virtualWrite(V11,1);  
    }else {
      Blynk.virtualWrite(V11,0);
    }
    c=0;
    dataIn="";     
  }
}
// open window
BLYNK_WRITE(V6){
  int p = param.asInt();
  if(p == 1){
    myServo.write(180);
  }else {
    myServo.write(0);
  }
}
//openWindowWithLightSensor
BLYNK_WRITE(V10){
  int p = param.asInt();
  if(p == 1){
    flagWindow = 1;  
  }else {
    flagWindow = 0;  
  }
}
// kiểm tra giá trị từ Light sensor để bật tắt đèn LED
void checkLightValue(){
  lightValue =  digitalRead(lightSensor);
  if(flagWindow == 1){
    if(lightValue == 1){
      myServo.write(180);
      Serial.println(lightValue);
      Blynk.virtualWrite(V6,1);  
    } else {
      myServo.write(0);
      Blynk.virtualWrite(V6,0);
      Serial.println(lightValue);
    }  
  }   
}

void setup()
{
  BlynkEdgent.begin();
  Serial.begin(57600);
  NodeMCU.begin(9600);
  dht.begin();
  myServo.attach(14);
  myServo.write(0);
  pinMode(LED1,OUTPUT);
  pinMode(lightSensor, INPUT);
  digitalWrite(LED1,LOW);
  randomSeed(analogRead(A0));
  timer.setInterval(100L,sendSensor);
  timer.setInterval(100L,checkLightValue);
  timer.setInterval(100L,sendDatatoArduino);
}

void loop() 
{
  BlynkEdgent.run();
  timer.run();
  
}
