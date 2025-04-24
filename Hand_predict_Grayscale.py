# -- coding: utf-8 --
"""
Created on Tue Jan 30 01:06:35 2024

@author: cdmor
"""

#%%
import numpy as np
import cv2
import torch
from ultralytics import YOLO
from ultralytics.utils.benchmarks import benchmark

from IPython.display import display, Image
from IPython import display
display.clear_output()
import ultralytics
ultralytics.checks()

import time
import sys
import os
os.environ['KMP_DUPLICATE_LIB_OK']='True'

#configurations for YOLO
device1 = 'cuda' if torch.cuda.is_available() else 'cpu'
model = YOLO('/home/pi/LeptonModule-master/software/raspberrypi_video/prueba2.onnx')

#infinte loop to always be infering
while True:
#we add handlers for the file error to determine if we restart the code or end it.
       try:
              
              #path of the file 
              csv_file = "/home/Thermal_Camera/AI_Temps.csv"

              #read matrix
              matrix = np.genfromtxt(csv_file, delimiter=',')
              
              #we create an rgb image duplicating the matrix in all the 3 dims
              image_rgb = np.stack((matrix,matrix,matrix),axis=-1)

# save all the iformations of the inference in results 
              results=model(source=image_rgb,
                            task='detect',
                            save=False, 
                            imgsz=(160,128),
                            augment=True,
                            flipud=0.5, # image flip up-down (probability)
                            fliplr=0.5,  # image flip left-right (probability)
                            degrees=180,
                            name='Test',
                            device=device1,
                            conf=0.5)
# in a empty list we save the bounding boes as a numpy array 
              boxes=[]
              for result in results:
                     boxes.append(result.cpu().numpy().boxes.xyxy)
                     
# then iterate in the boxes and save them in the file
# if there is no box hen we write a 0,0,0,0 array
              for box in boxes:
                     for i in box:
                            with open('/home/Thermal_Camera/output.csv', 'w') as f:
                                   temp = [round(num) for num in i]
                                   print(*temp,file=f)
                     if not box:
                            with open('/home/Thermal_Camera/output.csv', 'w') as f:
                                   temp = (0,0,0,0)
                                   print(*temp,file=f)
       # we add a little delay because the infering is so fast that the cpu gets too hot
              time.sleep(0.1)
#if we can't find the file means the AI in the interface is ended
# so we delete the file with the bounding box and finish python
       except IOError as e:
              os.remove('/home/Thermal_Camera/output.csv')
              sys.exit()
              
# Because both codes are fast sometimes when python is reading the image file
# c++ start to write again in the same file and provoke and error so 
# is easier if we just add a little time to wite and start again
       except Exception as e:
              time.sleep(0.1)
