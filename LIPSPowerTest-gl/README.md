# LIPSPowerTest-gl
This sample tool LIPSPowerTest-gl is for power mode switching test on LIPSedge camera.
We can put camera to sleep mode by turnning off projector/emittor to save power.

Support device:
  * LIPSedge DL and M3

Revision History:
  * 2021.10.20: update to OpenGL version

## Build steps:
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```

## Run sample:
Run program with command:
```
$ cd build
$ ./bin/Ni2PowerTest
```

1. Wait for window pop up and shows Depth/IR images, you will see input command on the console:
```
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
```

2. Type l or L to put device into Low-Power Mode

  * IR emitters and LED indicator will be turned off and Depth/IR streaming cannot continue

3. Type n or N to bring back device to Normal Mode

  * IR emitters and LED indicator will be turned on and Depth/IR streaming can continue

4. Finally press q or Q to exit program.
> NOTE: this only works when your window focus is on the Viewer GUI, NOT on the console.
