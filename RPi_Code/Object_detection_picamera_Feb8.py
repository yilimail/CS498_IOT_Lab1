######## Picamera Object Detection Feb 08 #########
#
# Author: Li Yi
# Date: 02/08/2020
# Description: 
# This program uses a TensorFlow classifier to perform object detection.
# It loads the classifier to perform object detection on a Picamera feed.
# It draws boxes and scores around the objects of interest in each frame from the Picamera.
# It detect if human presents, and receives distance data from CAN_IP_Gateway. 
# It a person presents and distance less than 50 CM, it sends command to CAN_IP_Gateway to turn on LED light on ECU
# It shows FPS, Distance, LED status on frame
# it uses UDP to receive distance data and send LED command 

# Import packages
import os
import cv2
import numpy as np
from picamera.array import PiRGBArray
from picamera import PiCamera
import tensorflow as tf
import argparse
import sys
import socket
import json

UDP_IP = "169.254.190.122"  #Local IP
UDP_IP_SVR = "169.254.190.10" #UNO IP
UDP_PORT = 54322   #Local port number, as long as the port is free and could be used by user.
UDP_PORT_SVR = 54321 # UNO listening port
sock = socket.socket(socket.AF_INET, # Internet
                    socket.SOCK_DGRAM) # UDP
clientSock = socket.socket(socket.AF_INET, # Internet
                    socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
#data, addr = sock.recvfrom(1024)
#data = json.loads(data.decode())

# Set up camera constants
# Use smaller resolution for slightly faster framerate
IM_WIDTH = 608
IM_HEIGHT = 608

# Select camera type
camera_type = 'picamera'

# This is needed since the working directory is the object_detection folder.
# Object_detectopm_picamera.py need to be put into
# /home/pi/tensorflow1/models/research/object_detection
sys.path.append('..')

# Import utilites
from utils import label_map_util
from utils import visualization_utils as vis_util

# Name of the directory containing the object detection module we're using
#MODEL_NAME = 'ssdlite_mobilenet_v2_coco_2018_05_09'

# Option 
MODEL_NAME = 'ssd_mobilenet_v1_0.75_depth_300x300_coco14_sync_2018_07_03'

# Grab path to current working directory
CWD_PATH = os.getcwd()

# Path to frozen detection graph .pb file, contains models that is used for object detection.
#PATH_TO_CKPT = os.path.join(CWD_PATH,MODEL_NAME,'frozen_inference_graph.pb')

# Option 
PATH_TO_CKPT = os.path.join(CWD_PATH,MODEL_NAME,'frozen_inference_graph.pb')

# Path to label map file
#PATH_TO_LABELS = os.path.join(CWD_PATH,'data','mscoco_label_map.pbtxt')

# Option 
PATH_TO_LABELS = os.path.join(CWD_PATH,'data','mscoco_label_map.pbtxt')

# Number of classes the object detector can identify
NUM_CLASSES = 90

## Load the label map.
# Label maps map indices to category names, so that when the convolution
# network predicts `5`, we know that this corresponds to `airplane`.
# Here we use internal utility functions, but anything that returns a
# dictionary mapping integers to appropriate string labels would be fine
label_map = label_map_util.load_labelmap(PATH_TO_LABELS)
categories = label_map_util.convert_label_map_to_categories(label_map, max_num_classes=NUM_CLASSES, use_display_name=True)
category_index = label_map_util.create_category_index(categories)

# Load the Tensorflow model into memory.
detection_graph = tf.Graph()
with detection_graph.as_default():
    od_graph_def = tf.GraphDef()
    with tf.gfile.GFile(PATH_TO_CKPT, 'rb') as fid:
        serialized_graph = fid.read()
        od_graph_def.ParseFromString(serialized_graph)
        tf.import_graph_def(od_graph_def, name='')

    sess = tf.Session(graph=detection_graph)


# Define input and output tensors (i.e. data) for the object detection classifier

# Input tensor is the image
image_tensor = detection_graph.get_tensor_by_name('image_tensor:0')

# Output tensors are the detection boxes, scores, and classes
# Each box represents a part of the image where a particular object was detected
detection_boxes = detection_graph.get_tensor_by_name('detection_boxes:0')

# Each score represents level of confidence for each of the objects.
# The score is shown on the result image, together with the class label.
detection_scores = detection_graph.get_tensor_by_name('detection_scores:0')
detection_classes = detection_graph.get_tensor_by_name('detection_classes:0')

# Number of objects detected
num_detections = detection_graph.get_tensor_by_name('num_detections:0')

# Initialize frame rate calculation
frame_rate_calc = 1
freq = cv2.getTickFrequency()
font = cv2.FONT_HERSHEY_SIMPLEX

# Initialize camera and perform object detection.

### Picamera ###
if camera_type == 'picamera':
    # Initialize Picamera and grab reference to the raw capture
    camera = PiCamera()
    camera.resolution = (IM_WIDTH,IM_HEIGHT)
    camera.framerate = 10
    rawCapture = PiRGBArray(camera, size=(IM_WIDTH,IM_HEIGHT))
    rawCapture.truncate(0)

    for frame1 in camera.capture_continuous(rawCapture, format="bgr",use_video_port=True):

        t1 = cv2.getTickCount()
        distance, addr = sock.recvfrom(1024)
        distance = json.loads(distance.decode())        
        # Acquire frame and expand frame dimensions to have shape: [1, None, None, 3]
        frame = np.copy(frame1.array)
        frame.setflags(write=1)
        frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        frame_expanded = np.expand_dims(frame_rgb, axis=0)

        # Perform the actual detection by running the model with the image as input
        (boxes, scores, classes, num) = sess.run(
            [detection_boxes, detection_scores, detection_classes, num_detections],
            feed_dict={image_tensor: frame_expanded})
        # Detect if person is in the frame (Human = Classes--1)
        if (int(classes[0][0]) ==1) and (distance <= 50):
            led = " ON"
            clientSock.sendto(bytes(1), (UDP_IP_SVR, UDP_PORT_SVR))
        else:
            led = " OFF"
            clientSock.sendto(bytes(0), (UDP_IP_SVR, UDP_PORT_SVR))
        # Draw the results of the detection (aka 'visulaize the results')
        vis_util.visualize_boxes_and_labels_on_image_array(
            frame,
            np.squeeze(boxes),
            np.squeeze(classes).astype(np.int32),
            np.squeeze(scores),
            category_index,
            use_normalized_coordinates=True,
            line_thickness=8,
            min_score_thresh=0.40)

        cv2.putText(frame,"FPS: {0:.2f}".format(frame_rate_calc),(30,50),font,1,(255,255,255),2,cv2.LINE_AA)
        cv2.putText(frame, "Distance: {0} cm".format(distance), (30,80),font,1,(255,255,255),2,cv2.LINE_AA)
        cv2.putText(frame, ("LED is"+ led), (30,110),font,1,(255,255,255),2,cv2.LINE_AA)
        # All the results have been drawn on the frame, so it's time to display it.
        cv2.imshow('Object detector', frame)

        t2 = cv2.getTickCount()
        time1 = (t2-t1)/freq
        frame_rate_calc = 1/time1

        # Press 'q' to quit
        if cv2.waitKey(1) == ord('q'):
            break

        rawCapture.truncate(0)

    camera.close()

cv2.destroyAllWindows()
