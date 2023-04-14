# Ni2PowerTest-gl #

This sample tool Ni2PowerTest-gl is for power mode switching test on LIPSedge devices

Support device: DL and M3

Revision:
2021.10.20: update to OpenGL version

Build steps:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

Run program with command:

```
$ cd build
$ ./Bin/Ni2PowerTest
```

1. Wait for Depth/IR image window pop up, you will see input command on console like below

...
2019-08-08 15:28:25.492 INFO    Camera init completed.
2019-08-08 15:28:25.494 INFO    productName = [LIPSedge M3-ToF]
2019-08-08 15:28:25.496 INFO    productSN   = [0310HI5100058]
2019-08-08 15:28:25.498 INFO    calibration = [030110020300]
depth video mode - FPS=30, X=80, Y=60
2019-08-08 15:28:25.904 INFO    ALPU 8 bytes mode is not support
2019-08-08 15:28:25.904 INFO    Process LIPSedge M3-ToF(2df2:0214) init...
Power Test:
  l/L Low-Power Mode
  n/N Normal Mode
Please input your option:

2. Type l or L to put device into Low-Power Mode

You will find IR emitters and LED indicator turn off and Depth/IR streaming does not continue

3. Type n or N to bring back device to Normal Mode

You will find IR emitters and LED indicator turn of and Depth/IR streaming continues

4. Finally press q or Q on the Viewer GUI, NOT the console, to exit the program.
