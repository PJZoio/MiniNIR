#!/usr/bin/env python3
"""
Use pyqtgraph lib to plot Sparspec data
"""

import numpy as np
import sys
import pyqtgraph as pg
#from pyqtgraph.Qt import QtCore

FILENAME = 'build/data.bin'
if len(sys.argv) > 1:
    FILENAME = str(sys.argv[1])

data =  np.fromfile(FILENAME, dtype='float64')
sparc_data = np.reshape(data, (2, -1), order='C')

pg.setConfigOption('background', 'w')  # before loading widget
app = pg.mkQApp("Plotting Sparsec Data")
#mw = QtWidgets.QMainWindow()
#mw.resize(800,800)

win = pg.GraphicsLayoutWidget(show=True, title="Basic plotting examples")
win.resize(1000,600)
win.setWindowTitle('pyqtgraph example: Plotting')

# Enable antialiasing for prettier plots
pg.setConfigOptions(antialias=True)

x = sparc_data[0]
y = sparc_data[1]
p1 = win.addPlot(title="Spectrum")
p1.plot(x, y, pen=(255,0,0), name="Red curve")
p1.setLabel('left', "Y Axis", units='Counts')
p1.setLabel('bottom', "X Axis", units='Pixel')

if __name__ == '__main__':
    pg.exec()

