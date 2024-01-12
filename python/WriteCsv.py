#!/usr/bin/pythona3
# -*- coding:utf-8 -*-
"""
A program to read data from the High-Precision-AD-HAT
and save to a CSV file
"""

import time
import sys
import numpy as np

from MiniNirAdc import MiniNirAdc

NUM_SAMPLES = 6000
NUM_CHANNELS = 6

timestr = time.strftime("%Y%m%d-%H%M%S")

if len(sys.argv) > 1:
    filename = str(sys.argv[1])
else:
    filename = 'dados_test1/BI_' + timestr + '.csv'

try:
    adc = MiniNirAdc()
    if (adc.open() == -1):
        print('Open Error')
        exit()

    adcVals = adc.read(num_samples=60, num_channels=6)
    adc.close()
    
    print('Acquisition Done. Saving Data to ' + filename)
    np.savetxt(filename, adcVals, fmt='%.5f', delimiter=',')

except IOError as e:
    print(e)
   
except KeyboardInterrupt:
    print("ctrl + c:")
    print("Program end")
    exit()

