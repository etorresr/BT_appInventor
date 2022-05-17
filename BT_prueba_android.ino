#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
bool men = false;
String message1 = "";
String mess = "e";


void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
SerialBT.begin("ESP32pumitas"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  
}

void loop() {
  // put your main code here, to run repeatedly:
if (SerialBT.available()) {
    
    char incomingChar1 = SerialBT.read();
    
    if (incomingChar1!= '\n'){
      message1 += String(incomingChar1);
      //Serial.println(message1);
    }
    else{
      men=true;
      
    }
  }
  if (men){
    Serial.println(message1);
    if (message1 == "adios"){
      SerialBT.println(mess);
    }
    men = false;
    message1="";
  }
}
