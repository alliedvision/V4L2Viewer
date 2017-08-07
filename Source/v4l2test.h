/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        v4l2test.h

  Description: 

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef v4l2test_H
#define v4l2test_H

#include <list>
#include <QtGui>
#include <QMainWindow>
#include <QTimer>
#include "ui_v4l2test.h"

#include <MyFrame.h>
#include <Camera.h>
#include <FrameObserver.h>

using namespace AVT::Tools::Examples;

#define VIEWER_MASTER       0

class v4l2test : public QMainWindow
{
    Q_OBJECT

public:
    v4l2test( QWidget *parent = 0, Qt::WindowFlags flags = 0, int viewerNumber = VIEWER_MASTER );
    ~v4l2test();

private:
    // Graphics scene to show the image
	std::list<QSharedPointer<v4l2test> > m_pViewer;
    int m_nViewerNumber;
    
    bool m_BLOCKING_MODE;
    bool m_MMAP_BUFFER;
    bool m_ShowFrames;
	
    // The currently streaming camera
	Camera m_Camera;
    
    uint32_t m_nDroppedFrames;
    
	// The Qt GUI
    Ui::v4l2testTestClass ui;
	// The menu widget to setup the number of used frames
	QWidgetAction *m_NumberOfUsedFramesWidgetAction;
	// The line which holds the number of used frames
	QLineEdit *m_NumberOfUsedFramesLineEdit;
	// The menu widget to setup the number of dumped frames
	QWidgetAction *m_LogFrameRangeWidgetAction;
	// The line which holds the number dumped frames
	QLineEdit *m_LogFrameRangeLineEdit;
	// The menu widget to setup the number of dumped frames
	QWidgetAction *m_DumpByteFrameRangeWidgetAction;
	// The line which holds the number dumped frames
	QLineEdit *m_DumpByteFrameRangeLineEdit;
	// The menu widget to setup the number of CSV File
	QWidgetAction *m_CSVFileWidgetAction;
	// The line which holds the number CSV File
	QLineEdit *m_CSVFileLineEdit;
	// The menu widget to setup the number of CSV File
	QWidgetAction *m_ToggleStreamDelayRandWidgetAction;
	// The line which holds the number CSV File
	QLineEdit *m_ToggleStreamDelayRandLineEdit;
	QCheckBox *m_ToggleStreamDelayRandCheckBox;
	// The menu widget to setup the number of CSV File
	QWidgetAction *m_ToggleStreamDelayWidgetAction;
	// The line which holds the number CSV File
	QLineEdit *m_ToggleStreamDelayLineEdit;
	QCheckBox *m_ToggleStreamDelayCheckBox;
	// A list of known camera IDs
    std::vector<uint32_t> m_cameras;
	// Is a camera open?
	bool m_bIsOpen;
	// The current streaming state
	bool m_bIsStreaming;
    //// Our Qt image to display
	double m_dScaleFactor;
	//// Our Qt image to display
	bool m_dFitToScreen;
	// Timer to togle the stream
	QTimer m_StreamToggleTimer;
    // Timer to show the frames received from the frame observer
	QTimer m_FramesReceivedTimer;
    // Timer to check for controller timeouts
	QTimer m_ControlRequestTimer;
	// Graphics scene to show the image
	QSharedPointer<QGraphicsScene> m_pScene;
	// Pixel map for the graphics scene
	QGraphicsPixmapItem *m_PixmapItem;
	// Direct access value
	uint64_t m_DirectAccessData;
    // save a frame dialog
    QFileDialog *m_saveFileDialog;
    // save a frame directory
    QString m_SaveFileDir;
    // save a frame extension
    QString m_SelectedExtension;
    // save a frame name
    QString m_SaveImageName;

    // Queries and lists all known cameras
    void UpdateCameraListBox(uint32_t cardNumber, uint64_t cameraID, const QString &deviceName, const QString &info);
	// Update the viewer range
	void UpdateViewerLayout();
	// Update the zoom buttons
	void UpdateZoomButtons();
	// Open/Close the camera
	int OpenAndSetupCamera(const uint32_t cardNumber, const QString &deviceName);
    int CloseCamera(const uint32_t cardNumber);
	// called by OnCameraPayloadSizeReady when payloadsize arrieved
    void StartStreaming(uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine);
    
    // Official QT dialog close event callback
    virtual void closeEvent(QCloseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

    // called by master viewer window
    void RemoteClose();
	
	void GetImageInformation();
	
	void SetTitleText(QString additionalText);
	
	void UpdateCameraFormat();

private slots:
    void OnLogToFile();
      void OnClearOutputListbox();
      void OnShowFrames();
	// The event handler to close the program
	void OnMenuCloseTriggered();
	// The event handler to set blocking mode
	void OnBlockingMode();
	// The event handler to set MMAP
	void OnUseMMAP();
    // The event handler to open a next viewer
    void OnMenuOpenNextViewer();
	// Prints out some logging
	void OnLog(const QString &strMsg);
    // The event handler for open / close camera
    void OnOpenCloseButtonClicked();
    // The event handler for get device info
    void OnGetDeviceInfoButtonClicked();
    // The event handler for stream statistics
    void OnGetStreamStatisticsButtonClicked();
    // The event handler for the camera list changed event
    void OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info);
	// The event handler for starting acquisition
	void OnStartButtonClicked();
    // The event handler for toggeling acquisition
	void OnToggleButtonClicked();
	// The event handler for stopping acquisition
	void OnStopButtonClicked();
	// The event handler to resize the image to fit to window
	void OnZoomFitButtonClicked();
	// The event handler for resize the image
	void OnZoomInButtonClicked();
	void OnZoomOutButtonClicked();
	// The event handler to show the frames received
	void OnUpdateFramesReceived();
    // The event handler to check for controller timeouts
	void OnControllerResponseTimeout();
    // The event handler to receive timer messages
    void OnStreamToggleTimeout();
	// The event handler to show the processed frame
	void OnFrameReady(const QImage &image, const unsigned long long &frameId);
	// The event handler to show the processed frame ID
	void OnFrameID(const unsigned long long &frameId);
	// The event handler to show the event data
	void OnCameraEventReady(const QString &eventText);
    // The event handler to clear the event list
	void OnClearEventLogButtonClicked();
	// The event handler to show the received values
	void OnCameraRegisterValueReady(unsigned long long value);
    // Event will be called on error
	void OnCameraError(const QString &text);
    // Event will be called on message
	void OnCameraMessage(const QString &text);
    // Event will be called when the a frame is recorded
    void OnCameraRecordFrame(const unsigned long long &frameID, const unsigned long long &framesInQueue);
	// Event will be called when the a frame is displayed
    void OnCameraDisplayFrame(const unsigned long long &frameID);
    // The event handler to open a camera on double click event
	void OnListBoxCamerasItemDoubleClicked(QListWidgetItem * item);
	// The event handler to read a register per direct access
	void OnDirectRegisterAccessReadButtonClicked();
	// The event handler to write a register per direct access
	void OnDirectRegisterAccessWriteButtonClicked();
	// The event handler to update the direct register access value
	void OnDirectRegisterAccessUpdateData();

	void OnCropXOffset();
	void OnCropYOffset();
	void OnCropWidth();
	void OnCropHeight();

    
    void OnStartRecording();
    void OnStopRecording();
    void OnBackwardDisplay();
    void OnForwardDisplay();
    void OnDeleteRecording();
    void OnSaveFrame();
	
	void OnWidth();
	void OnHeight();
	void OnPixelformat();
	void OnGain();
	void OnAutoGain();
	void OnExposure();
	void OnAutoExposure();
    void OnExposureAbs();
	void OnPixelformatDBLClick(QListWidgetItem *);
	void OnFramesizesDBLClick(QListWidgetItem *);
	void OnGamma();
	void OnReverseX();
	void OnReverseY();
	void OnSharpness();
	void OnBrightness();
	void OnContrast();
	void OnSaturation();
	void OnHue();
	void OnContinousWhiteBalance();
	void OnWhiteBalanceOnce();
	void OnRedBalance();
	void OnBlueBalance();
	void OnFramerate();
	void OnCropCapabilities();
	void OnReadAllValues();

	
	void OnCameraPixelformat(const QString &);
	void OnCameraFramesize(const QString &);
	
	void OnToggleStreamDelayRand();
	void OnToggleStreamDelay();
};

#endif // v4l2test_H
