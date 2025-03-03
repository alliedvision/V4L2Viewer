=========================
Allied Vision V4L2 Viewer
=========================
Allied Vision V4L2 Viewer was designed for easy evaluation of 
`Alvium CSI-2 cameras <https://www.alliedvision.com/en/products/embedded-vision-solutions/>`_. 


Compatibility
-------------

Tests were performed on the following NVIDIA Jetson SOMs: 

- JetPack 6.2.0 (L4T 36.4.3), see https://github.com/alliedvision/alvium-jetson-driver-release
- All SoMs + carriers supported by the driver

Additionally, you can use the viewer with any other CSI-2 camera or another board.

Tested cameras:

-  Alvium C, GM2 and FP3 cameras


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

Beta Disclaimer
---------------

Please be aware that all code revisions not explicitly listed in the Github Release section are
considered a **Beta Version**.

For Beta Versions, the following applies in addition to the GPLv2 License:

THE SOFTWARE IS PRELIMINARY AND STILL IN TESTING AND VERIFICATION PHASE AND IS PROVIDED ON AN “AS
IS” AND “AS AVAILABLE” BASIS AND IS BELIEVED TO CONTAIN DEFECTS. THE PRIMARY PURPOSE OF THIS EARLY
ACCESS IS TO OBTAIN FEEDBACK ON PERFORMANCE AND THE IDENTIFICATION OF DEFECTS IN THE SOFTWARE,
HARDWARE AND DOCUMENTATION.



