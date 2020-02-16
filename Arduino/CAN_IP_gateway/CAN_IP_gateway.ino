//*******CAN_IP_Gateway**********
//
//Author: Li Yi
//Date: 02/15/2020
//Description:
//The code use CAN BUS library and Ethernet UDP library to perform LED and distance data transmision
//Through UDP, it transmits distance data to RPi and receives LED command from RPi.
//Through CAN, it receives distance data from ECU and transmits LED command to RPi.
//Some of the code is copied from examples of Ethernet and mcp_can libaraies





#include <mcp_can.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];   //buffer for receive from raspberry pi
char ledcmd[1];
char msgString2[UDP_TX_PACKET_MAX_SIZE]; 
byte ledStatus;
int distStatus;
//int ledStatus;
#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

byte mac[] = {0x00, 0x55, 0x66, 0xEE, 0xFF, 0xFF};
IPAddress ip(169,254,190,10);   //Ardunio Gateway IP
IPAddress dest(169,254,190,122); //RPi IP

unsigned int locPort = 54321;
unsigned int remPort = 54322;

EthernetUDP UDP;

void setup()
{
  Serial.begin(115200);
  
  // Initialize MCP2515 running at 8MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
 
  CAN0.setMode(MCP_NORMAL); // Set operation mode to normal so the MCP2515 sends acks to received data.
  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
  
  Ethernet.begin(mac,ip);  // Initialize Ethernet                        
  UDP.begin(locPort);
}

void loop()
{
  if(!digitalRead(CAN0_INT))                  // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    ledStatus = rxBuf[4];
    distStatus = (rxBuf[2] << 8) + rxBuf[3];
    Serial.println(distStatus);
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString); //Debug Message
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } 
    else
    {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
     }
      
     UDP.beginPacket(dest, remPort);
     UDP.print(distStatus);
     UDP.endPacket();
    }
    Serial.println();
  }
  int packetSize = UDP.parsePacket();
  ledStatus = "";
  if (packetSize) {
    // read the packet into packetBufffer
    Serial.println("Packsize loop");
    UDP.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    ledStatus = 0x00;
    //Serial.print("ledcmd:");
    sprintf(ledcmd, "0x%.2X", packetBuffer[0]);
    //Serial.println(ledcmd);
  }
      
    byte sndStat = CAN0.sendMsgBuf(0x100, 0, 1, ledStatus);
    if(sndStat != CAN_OK){
    Serial.println("Error Receiving Message...");
    }
  delay(1500);
}
