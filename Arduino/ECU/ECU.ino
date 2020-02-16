//*******ECU**********
//
//Author: Li Yi
//Date: 02/15/2020
//Description:
//The code receives reading from Ultrasonic Sensor, and converts to distance in centimeter unit.
//Tt uses CAN BUS library to transmit distance data to IP_CAN_Gateway.
//Through CAN, it receives LED command from IP_CAN_Gateway to turn on/off LED light which is Pin 8.


#include <mcp_can.h>
#include <SPI.h>

int trigPin = 7;
int echoPin = 6;
int ledPin = 8;
int distance; // centermeter (cm)
long duration;
int ledValue;


MCP_CAN CAN0(10); // set MCP's CS pin to 10
#define CAN0_INT 2 // set INT to pin 2

long unsigned rxId;
unsigned char len = 0;
unsigned char rxBuf[4];
char msgString[128];        // array to store serial string


void setup() {
  
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(CAN0_INT, INPUT);
  pinMode(ledPin, OUTPUT);
  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) 
      Serial.println("MCP2515 Initialized Successfully!");
  else 
      Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted

}

unsigned char stmp[5];

void loop() {
  
  ledValue = digitalRead(ledPin);
  // clearing trigpin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2;

  Serial.print(distance);
  Serial.println(" cm");
  Serial.println(ledValue);
 
  // converting distance(int) into byte array
  // unsigned char stmp[8];
  stmp[0] = (int)((distance >> 24) & 0xFF);
  stmp[1] = (int)((distance >> 16) & 0xFF);
  stmp[2] = (int)((distance >> 8) & 0XFF);
  stmp[3] = (int)((distance & 0XFF));
  stmp[4] = (int)(ledValue);
  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 5, stmp);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
 
  if(!digitalRead(CAN0_INT)) {              // read recieve buffer if CAN0_INT is low
    CAN0.readMsgBuf(&rxId, &len, rxBuf);    // read data: len = data length, buf = # data byte(s)

    if(rxBuf[0] == 0x00){
      digitalWrite(ledPin, HIGH);
      }
    else{
      digitalWrite(ledPin, LOW);
      }

    if ((rxId & 0x80000000) == 0x80000000) {  // determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    } else {
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
    }

    Serial.print(msgString);

    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
    Serial.println(); 
  }
  delay(1000);   // send data per 1000ms
}
