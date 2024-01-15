# Software to read data from Sarspec Res+ Spectrographs on Linux

## Install libftd2xx (see other instructions)  
You may need to unload Linux standard system drivers
```console
sudo rmmod ftdi_sio
sudo rmmod usbserial
```
## Compile and run Cpp program
1. If needed, install [cmake](https://cmake.org) tool.
2. Run
```console
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./spec-res -h     # to see help screen
$ ./spec-res -i 3 -b # to save in binary format for ploting 
```

## Python3 plots
1. Install [PyQtGraph](https://www.pyqtgraph.org)
2. Run:
```console
$ cd ..
$ python3 pyqtSarspecBin.py build/data.bin
```
