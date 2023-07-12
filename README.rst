=========================
Allied Vision V4L2 Viewer
=========================
Allied Vision V4L2 Viewer was designed for easy evaluation of 
`Alvium CSI-2 cameras <https://www.alliedvision.com/en/products/embedded-vision-solutions/>`_. 

THE SOFTWARE IS PRELIMINARY AND STILL IN TESTING AND VERIFICATION PHASE AND IS PROVIDED ON AN “AS IS” AND “AS AVAILABLE” BASIS AND IS BELIEVED TO CONTAIN DEFECTS. A PRIMARY PURPOSE OF THIS EARLY ACCESS IS TO OBTAIN FEEDBACK ON PERFORMANCE AND THE IDENTIFICATION OF DEFECT SOFTWARE, HARDWARE AND DOCUMENTATION.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Compatibility
-------------
New as of V2.2.0: support for IMX8MPlus EVK 2.2.0-avt1.0, see https://github.com/alliedvision/alvium-manifest-imx8mpevk

 Tests were performed on the following NVIDIA Jetson SOMs: 

  - NVIDIA AGX Orin Developer Kit
  - JetPack 5.1.0 (L4T 35.1.0), see https://github.com/alliedvision/linux_nvidia_jetson

Additionally, you can use the viewer with any other CSI-2 camera or another board.

Tested cameras:

-  Alvium MIPI CSI-2 cameras with firmware version 12


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


