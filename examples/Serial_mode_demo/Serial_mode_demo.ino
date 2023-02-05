/*
  RCWL-1005 and RCWL-1605 serial mode demo

  Solder a 10K resistor to the M1 (=R8) solder bridge and leave M2 (=R6) open to put the module into UART/serial mode.

  Connections for Arduino Uno/Nano etc.:

  ECHO_TX_SDA     --> D10
  Trig_RX_SCL_I/O --> D11
  GND             --> GND
  Vcc             --> 5V

  The company and version info is composed of 12 Chinese encoded bytes, a zero 0x00 byte, and a subsequent ASCII part.
  
  My 1005 info:
  Chinese part (hex) = [ CE DE CE FD C8 D5 B3 BF CE EF C1 AA 00 ] ascii part = [RCWL-1005]
  
  My 1605 info:
  Chinese part (hex) = [ CE DE CE FD C8 D5 B3 BF CE EF C1 AA 00 ] ascii part = [RCWL-1605]

  The hex part decodes as "无锡日晨物联" in GB18030, which Google translates as "Wuxi Richen IoT". 
  The "datasheet" mentions "Wuxi Richen Wulian Technology Co., Ltd.", so this is nothing more than the company name.
  
  Note: It seems the HC-SR04_P_ module reports the same company info and the ascii part as [RCWL-2020], see https://www.mikrocontroller.net/topic/530175

  (c) juh, 2023

*/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX


void setup() {
  Serial.begin(115200);
  while (!Serial) {    
  }

  mySerial.begin(9600); // 9600 N 1 according to RCWL "datasheet"
  delay(100);
}


void loop() {

  Serial.print("Measuring..., result is: ");
  mySerial.write(0xA0); // initiate measurement
  while (mySerial.available() < 3) {} // wait until module sends result data
  
  uint8_t bytes[3]; uint8_t i = 0;
  while (mySerial.available() > 0) {
    bytes[i] = mySerial.read();
    printHex(bytes[i]);
    i++;
  }
  
  Serial.print(" (raw)  equals  ");
  Serial.print(((uint32_t(bytes[0]) << 16) + (uint32_t(bytes[1]) << 8) + uint32_t(bytes[2])) / 1000);
  Serial.println(" mm");
  delay(1000);

  Serial.print("company and version info, hex part = [ ");
  mySerial.write(0xF1);
  while (mySerial.available() == 0) {}
  delay(20); // wait for all data to come in
  i = 0;
  
  while (mySerial.available() > 0) {
    uint8_t c = mySerial.read();
    if (i++ < 13) { // first 13 bytes are (probably) Chinese GB18030 encoded, we'll output them as HEX
      printHex(c); Serial.print(" ");
    } else { // remaining part is ASCII
      Serial.print(char(c));
    }
    if (i == 13) {
      Serial.print("] ascii part = [");
    }
  }
  
  Serial.print("]\n\n");
  delay(1000);
}



void printHex(uint8_t c) {
  if (c < 0x10) {
    Serial.print("0");
  }
  Serial.print(c, HEX);
}
