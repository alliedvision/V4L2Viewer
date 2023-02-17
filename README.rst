=========================
Allied Vision V4L2 Viewer
=========================
Allied Vision V4L2 Viewer was designed for easy evaluation of 
`Alvium CSI-2 cameras <https://www.alliedvision.com/en/products/embedded-vision-solutions/>`_. 

Compatibility
-------------
Allied Vision V4L2 Viewer v2.1.0 was tested with JetPack 5.1.0 (L4T 35.1.0), see https://github.com/alliedvision/linux_nvidia_jetson

 Tests were performed on the following NVIDIA Jetson SOMs: 

  - AGX Orin Developer Kit
  - AGX Xavier DevKit
  - Xavier NX DevKit
  - Auvidea carrier JNX30-PD with Xavier NX 

Additionally, you can use the viewer with any other CSI-2 camera or another board.

Tested cameras:

-  Alvium MIPI CSI-2 cameras with firmware version 11


Installation
------------
Method A:
^^^^^^^^^
Use the binaries for Jetson provided in this release version. 

Additionally, install some Qt packages that are missing in JetPack 5.1.0:

.. code-block:: bash

   sudo apt-get install libqt5widgets5



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


