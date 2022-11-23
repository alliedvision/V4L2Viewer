=========================
Allied Vision V4L2 Viewer
=========================
Allied Vision V4L2 Viewer was designed for easy evaluation of 
`Alvium CSI-2 cameras <https://www.alliedvision.com/en/products/embedded-vision-solutions/>`_ and
NVIDIA Jetson boards. 
Additionally, you can use the viewer with any other CSI-2 camera or another board.

Compatibility
-------------
Allied Vision V4L2 Viewer v1.1.0 was tested with:

-  NVIDIA Jetson Nano, AGX Xavier, Xavier NX, TX2, TX2 NX
-  JetPack 4.6.1 (L4T 32.7.1)
-  Allied Vision driver version 4.0.0, see https://github.com/alliedvision/linux_nvidia_jetson
-  Alvium CSI-2 cameras with firmware version 6.0 or newer


Installation
------------
Method A:
^^^^^^^^^
Use the binaries for Jetson provided in this release version. 

Method B:
^^^^^^^^^
Use the sources on Jetson boards:

.. code-block:: bash

   apt install qt5-default cmake
   mkdir build
   cd build
   cmake ..
   make


Usage
-----
List cameras
^^^^^^^^^^^^
To view the camera list while the camera is open, click the camera icon.

Settings
^^^^^^^^
Go to *Settings* to enable or disable *Log to file*.

All features
^^^^^^^^^^^^
When you open *All Features*, all camera and driver features are shown. 

| Camera features are listed in the Alvium MIPI CSI-2 Cameras Direct Register Access Controls Reference, see: 
| https://www.alliedvision.com/en/support/technical-documentation/alvium-csi-2-documentation/

For NVIDIA's driver features, see NVIDIA's documentation.

Known issues
------------
Known issues:

-  Reverse X/Y in combination with a Bayer pixel format causes images with incorrect colors or an incorrect pixel format.
-  Reverse X/Y cannot be set during image acquisition.
-  Saving images during image acquisition is not always possible. 


