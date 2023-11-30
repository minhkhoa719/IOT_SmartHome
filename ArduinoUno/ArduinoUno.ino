#include <Servo.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SimpleTimer.h>
#include <SoftwareSerial.h>
Servo ServoMotor;
SimpleTimer timer;
SoftwareSerial ArduinoUno(10,11); //Rx,Tx
#define PassWordLength 6 
// khai báo biến
char defaultPassword[6] = "147719"; 
char c;
char key;
char keyPass;
String dataIn;
int8_t indexOfA, indexOfB, indexOfC;
String data1, data2, data3;
int flagDoor;
char oneTimePass[7] = "";
char timePass[7] = "";
char userInputPass[6];
char eepromPass[6];
char newPass[6];
int position = 0;
int checkWrongPassTime = 0;
//khai báo biến bàn phím
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
{'1','2','3'},
{'4','5','6'},
{'7','8','9'},
{'*','0','#'}
};
byte rowPins[ROWS] = { 8, 7, 6, 5 };
byte colPins[COLS] = { 4, 3, 2 };
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(57600);
  ArduinoUno.begin(9600);
  ServoMotor.attach(9);
  ServoMotor.write(0);
  timer.setInterval(100L,checkPassWord);
}
// hàm phân tích data gửi từ esp
void Parse_the_Data(){
  indexOfA = dataIn.indexOf("A");
  indexOfB = dataIn.indexOf("B");
  indexOfC = dataIn.indexOf("C"); 

  data1 = dataIn.substring(0, indexOfA);
  data2 = dataIn.substring(indexOfA + 1, indexOfB);
  data3 = dataIn.substring(indexOfB + 1, indexOfC);  
}
// kiểm tra password
void checkPassWord(){
  while(ArduinoUno.available() > 0){
     c = ArduinoUno.read();
     if(c == '\n') break;
     else dataIn += c;
  }
  if(c == '\n'){
    Parse_the_Data();    
    c = 0;
    dataIn = "";     
  }
  data1.toCharArray(timePass,7);
  data2.toCharArray(oneTimePass,7);
  flagDoor = data3.toInt();

  key = keypad.getKey();
  switch(key){
    case '#':
    data1.toCharArray(timePass,7);
    data2.toCharArray(oneTimePass,7);
    Serial.println(timePass);
    Serial.println(oneTimePass);
  while (position < 6)
      {
        keyPass = keypad.getKey();
        if (keyPass)
        {
          userInputPass[position++] = keyPass;
          Serial.print(keyPass);
        }
        keyPass = 0;
      }
      if( position == 6){
        for (int j = 0; j < 6; j++)
          eepromPass[j] = EEPROM.read(j);
      if (!(strncmp(userInputPass, eepromPass, 6)) )
        {
          openDoor(1);
          ArduinoUno.print((String)oneTimePass); ArduinoUno.print("A");
          ArduinoUno.print("B");
          ArduinoUno.print(String(0)); ArduinoUno.print("C");
          ArduinoUno.print("\n");
          checkWrongPassTime = 0;
          //Write your logic for correct password
        }
      else if (!(strncmp(userInputPass, timePass, 6)))
        {
          openDoor(1);
          ArduinoUno.print((String)oneTimePass); ArduinoUno.print("A");
          ArduinoUno.print("B");
          ArduinoUno.print(String(0)); ArduinoUno.print("C");
          ArduinoUno.print("\n");
          checkWrongPassTime = 0;
          //Write your logic for correct password
        }
      else if (!(strncmp(userInputPass, oneTimePass, 6)))
        {
          openDoor(1);
          oneTimePass[7] = "";
          ArduinoUno.print((String) 0); ArduinoUno.print("A");
          ArduinoUno.print("B");
          ArduinoUno.print(String(0)); ArduinoUno.print("C");
          ArduinoUno.print("\n");
          checkWrongPassTime = 0;
        }
        else {
          Serial.println();
          Serial.println("Wrong password");
          checkWrongPassTime++;  
        }
        if(checkWrongPassTime == 3){
          Serial.println("Sai pass 3 lần");
          checkWrongPassTime = 0;
          ArduinoUno.print((String) 0); ArduinoUno.print("A");
          ArduinoUno.print("B");
          ArduinoUno.print((String) 1); ArduinoUno.print("C");
          ArduinoUno.print("\n");  
        }
        position = 0;
        }
        break;
        
        case '*':
        Serial.println("Change your Password:");
        change();
        break;
              
        case '0':
        Serial.println("Set Default Password Done....");
        reset_password();
        break;
  }
  if(flagDoor == 1){    
    openDoor(1);
    flagDoor = 0;
    ArduinoUno.print((String) oneTimePass); ArduinoUno.print("A");
    ArduinoUno.print((String) flagDoor); ArduinoUno.print("B");
    ArduinoUno.print(String(0)); ArduinoUno.print("C");
    ArduinoUno.print("\n");  
    }
}

void change()
{
  int j = 0;
  Serial.println("Please Enter Your Current Password");
  while (j < 6)
  {
    char changeKey = keypad.getKey();
    if (changeKey)
    {
      userInputPass[j++] = changeKey;
      Serial.print(changeKey);
    }
    changeKey = 0;
  }
  delay(500);

  if ((strncmp(userInputPass, eepromPass, 6)))
  {
    Serial.println();
    Serial.println("Wrong Password!");
    Serial.println("Try Again!");
    delay(1000);

    key = 0;
  }
  else
  {
    j = 0;
    Serial.println();
    Serial.println("Enter New Password:");
    while (j < 6)
    {
      char changeKey = keypad.getKey();
      if (changeKey)
      {
        newPass[j] = changeKey;
        Serial.print(changeKey);
        EEPROM.write(j, newPass[j]);
        j++;
      }
    }
    Serial.println();
    Serial.println("Password Successfully Updated...");
    delay(1000);
  }
}

void reset_password() {
  strncpy(eepromPass, defaultPassword, 6);
  for (int j = 0; j < 6; j++) {
    EEPROM.write(j, defaultPassword[j]);
  }
}

void loop(){
  timer.run();
}
  
void openDoor(int locked){
  if (locked == 1){
    ServoMotor.write(180);
    delay(1000);
    ServoMotor.write(0);
  }
}
