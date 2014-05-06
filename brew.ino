#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <services.h>

const int waterPin = A0;
const int brewPin =  2;
const int lightPin = 3;
const int buttonPin = 4;

int lightState = LOW;
boolean armed = false;
unsigned long previousTime = 0;
const long interval = 1000;
const int threshold = 500;

void setup() {
  delay(1000);
  
  ble_set_pins(9, 8);
  ble_set_name("Coffee Pot");
  ble_begin();
  
  pinMode(brewPin, OUTPUT); 
  pinMode(lightPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  
  digitalWrite(lightPin, LOW);
  Serial.begin(9600);
  Serial.flush();
}

void errorBlink() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(lightPin, HIGH);
    delay(50);
    digitalWrite(lightPin, LOW);
    delay(50);
  }
}

void tryArm() {
  if (!armed) {
    int waterValue = analogRead(waterPin);
    if (waterValue > threshold) {
      armed = true;
    } else {
      errorBlink();
    }
  }
}

void loop() {
    
  //  Collect and react to messages
  while (Serial.available()) {
    char message = (char)Serial.read();
    if (message == 'b') {
       if (armed) {
         digitalWrite(brewPin, HIGH);
       } else {
         errorBlink();
       }
    }
  }
  
  while (ble_available()) {
    char message = (char)ble_read();
    if (message == 'b') {
       if (armed) {
         digitalWrite(brewPin, HIGH);
       } else {
         errorBlink();
       }
    } else if (message == 'a') {
      tryArm(); 
    }
  }
  
  // Collect device state
  int waterValue = analogRead(waterPin);
  int buttonValue = digitalRead(buttonPin);
  int brewValue = digitalRead(brewPin);
    
  // Arm if button is pressed
  if (buttonValue == LOW) {
    tryArm();
  }
  
  // Configure light
  if (brewValue == HIGH) {
    unsigned long currentTime = millis();
    if(currentTime - previousTime >= interval) {
      previousTime = currentTime;   
      if (lightState == LOW) {
        lightState = HIGH;
      } else {
        lightState = LOW;
      }
      digitalWrite(lightPin, lightState);
    }
  } else if (armed) {
     digitalWrite(lightPin, HIGH);    
  } else {
     digitalWrite(lightPin, LOW);
  }
  
  //  Stop brewing if out of water
  if (waterValue <= threshold) {
     digitalWrite(brewPin, LOW);
     armed = false;
  }
  
  // Recollect state
  waterValue = analogRead(waterPin);    
  brewValue = digitalRead(brewPin);
  
  //  Message state
  Serial.print((int)armed);
  Serial.print(",");
  Serial.print(waterValue);
  Serial.print(",");
  Serial.println(brewValue);
  
  ble_write(0x0A);
  if (armed) {
    ble_write(0x01);
  } else {
    ble_write(0x00);
  }
  ble_write(0x00);
  
  ble_write(0x0B);
  ble_write(waterValue >> 8);
  ble_write(waterValue);
 
  ble_write(0x0C);
  if (brewValue) {
    ble_write(0x01);
  } else {
    ble_write(0x00);
  }
    
  ble_do_events();
  delay(100);
}
