#include <mcp_can.h>
#include <SPI.h>
const int trigPin = 7;
const int echoPin = 6;

long duration;
int distance;

MCP_CAN CAN0(10);     // Set CS to pin 10

byte data[4];


void setup(){
  Serial.begin(115200);

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop(){
 
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration*0.034/2;
  // convert long to bytes
  data[0] = (byte) distance;
  data[1] = (byte) distance >> 8;
  data[2] = (byte) distance >> 16;
  data[3] = (byte) distance >> 24;


  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 4, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(100);   // send data per 100ms
  Serial.print("Distance = ");
  Serial.println(distance);
   
}
