# CS498_IOT_Lab1
This repo contains files related to CS 498 Lab1 - IOT Devices(Spring 2020)
Contributors: 

Alexey Burlakov (NetID: alexeyb2)
Christopher Lewis (NetID: calewis4)
Li Yi (NetID: liyi2)
Pui Sze Pansy Ng (NetID: ppn2)
Zhijie Wang (NetID: zhijiew2)

Description:
In this lab, we implemented a vehicular network and computation infrastructure which is modeling 2019 Honda Civic. 
The infrastructure contains: 
Raspberry PI 3 B+ as ADAS, 
two Arduino Uno as microcontrollers, 
Pi camera as back end camera, 
ultrasonic sensor and LED light as tail light. 
The infrastructure performs object detection via pi camera and uses the ultrasonic sensor to calculate the distance. 
The LED light will turn on if a human is detected and the distance less than 50CM.

To get the code work:
1. Upoad IP_CAN_Gateway.ino to the Arduino board with Ethernet Shield
2. Upload ECU.ino to the Arduino board connected with LED and Ultrasonic sensor
3. Copy Files in RPi folder to /home/pi/tensorflow/models/research/object_detection
4. Cd /home/pi/tensorflow1/models/research/object_detection
5. Run Object_Detection_picamera_Feb8.py
6. Enjoy
