# LIPSedge SDK samples
Here we collect several samples using OpenNI2 programming interface to stream depth and color images from LIPSedge cameras.
There are some examples showing device management handled by OpenNI2 framework.

### Requirements
  * Install LIPSedge SDK, check [here](https://www.lips-hci.com/lipssdk) to download latest one.
  * Important build tools, like CMake, make, g++, git, are ready in your Linux system.

### Build samples
  * After SDk installation completed, you have to setup OpenNI2 software development environment.

     * ``$ source OpenNIDevEnvironment``

  * Steps to build samples

      ``
	  $ mkdir build
	  $ cd build
	  $ cmake ..
	  $ make
	  ``

### Running samples
After CMake build completed, you can use `make install` to install required OpenNI2 libraries to the output folder.

    * ``$ make install``

This step will copy everything OpenNI needs to the folder 'Bin'. Switch to 'Bin' to run sample programs.

For example, you can run Ni2PointCloud-gl.

    * ``$ cd Bin``
    * ``Bin$ ./Ni2PointCloud-gl``

<p align="center"><img src="res/Screenshot-ni2-pointcloud-viewer.png" /></p>

Enjoy our samples and happy coding!
