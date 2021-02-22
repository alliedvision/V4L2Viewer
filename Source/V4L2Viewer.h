/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        V4L2Viewer.h

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

#ifndef V4L2VIEWER_H
#define V4L2VIEWER_H

#include "Camera.h"
#include "DeviationCalculator.h"
#include "FrameObserver.h"
#include "MyFrame.h"

#include "ui_V4L2Viewer.h"

#include <list>

#include <QMainWindow>
#include <QtGui>
#include <QTimer>

#define VIEWER_MASTER       0

class V4L2Viewer : public QMainWindow
{
    Q_OBJECT

public:
    V4L2Viewer( QWidget *parent = 0, Qt::WindowFlags flags = 0, int viewerNumber = VIEWER_MASTER );
    ~V4L2Viewer();

private:

    // Graphics scene to show the image
    std::list<QSharedPointer<V4L2Viewer> > m_pViewer;
    int m_nViewerNumber;

    bool m_BLOCKING_MODE;
    IO_METHOD_TYPE m_MMAP_BUFFER;
    bool m_VIDIOC_TRY_FMT;
    bool m_ShowFrames;
    bool m_ExtendedControls;

    // The currently streaming camera
    Camera m_Camera;

    uint32_t m_nDroppedFrames;
    uint32_t m_nStreamNumber;

    // The Qt GUI
    Ui::V4L2ViewerClass ui;
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
    bool m_bFitToScreen;
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
    QFileDialog *m_SaveFileDialog;
    // save a frame directory
    QString m_SaveFileDir;
    // save a frame extension
    QString m_SelectedExtension;
    // save a frame name
    QString m_SaveImageName;
    // store radio buttons for blocking/non-blocking mode in a group
    QButtonGroup* m_BlockingModeRadioButtonGroup;
    // load reference image dialog
    QFileDialog *m_ReferenceImageDialog;
    // reference image
    QSharedPointer<QByteArray> m_ReferenceImage;
    // thread to calculate deviation
    QSharedPointer<DeviationCalculator> m_CalcThread;
    // for calculating the mean deviation to the reference image
    int m_MeanNumberOfUnequalBytes;
    // for checking if deviation calculation was successful
    unsigned int m_deviationErrors;
    // show loading gif during deviation calculation
    QMovie *m_LoadingAnimation;
    // vector for recorded frames
    QVector<QSharedPointer<MyFrame> > m_FrameRecordVector;
    // max size of framerecordvector
    const static int MAX_RECORD_FRAME_VECTOR_SIZE = 100;
    // frames received for live deviation calc
    unsigned int m_LiveDeviationFrameCount;
    // number of unequal bytes of live deviation calc
    unsigned long long m_LiveDeviationUnequalBytes;
    // number of frames with unequal bytes
    unsigned int m_LiveDeviationNumberOfErrorFrames;

    // Queries and lists all known cameras
    void UpdateCameraListBox(uint32_t cardNumber, uint64_t cameraID, const QString &deviceName, const QString &info);
    // Update the viewer range
    void UpdateViewerLayout();
    // Update the zoom buttons
    void UpdateZoomButtons();
    // Open/Close the camera
    int OpenAndSetupCamera(const uint32_t cardNumber, const QString &deviceName);
    int CloseCamera(const uint32_t cardNumber);
    // called by OnCameraPayloadSizeReady when payload size arrived
    void StartStreaming(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine);

    // update record listing
    void InitializeTableWidget();
    void UpdateRecordTableWidget();
    QSharedPointer<MyFrame> getSelectedRecordedFrame();


    // Official QT dialog close event callback
    virtual void closeEvent(QCloseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    // called by master viewer window
    void RemoteClose();

    void GetImageInformation();

    // Check if IO Read was checked and remove it when not capable
    void Check4IOReadAbility();

    void SetTitleText(QString additionalText);

    void UpdateCameraFormat();
    // returns the file destination for saving the frame
    void SaveFrameDialog(bool raw);
    // Saves the given frame to the given destination
    void SaveFrame(QSharedPointer<MyFrame> frame, QString filepath, bool raw);

private slots:
    void OnLogToFile();
    void OnClearOutputListbox();
    void OnShowFrames();
    // The event handler to close the program
    void OnMenuCloseTriggered();
    // The event handler to set blocking mode
    void OnBlockingMode(QAbstractButton* button);
    // The event handler to set IO Read
    void OnUseRead();
    // The event handler to set IO MMAP
    void OnUseMMAP();
    // The event handler to set IO USERPTR
    void OnUseUSERPTR();
    // The event handler to set VIDIOC_TRY_FMT
    void OnUseVIDIOC_TRY_FMT();
    // The event handler to set Extended Controls
    void OnUseExtendedControls();
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
    // Event will be called on warning
    void OnCameraWarning(const QString &text);
    // Event will be called on message
    void OnCameraMessage(const QString &text);
    // Event will be called when the a frame is recorded
    void OnCameraRecordFrame(const QSharedPointer<MyFrame>& frame);
    // The event handler to open a camera on double click event
    void OnListBoxCamerasItemDoubleClicked(QListWidgetItem * item);
    // The event handler to read a register per direct access
    void OnDirectRegisterAccessReadButtonClicked();
    // The event handler to write a register per direct access
    void OnDirectRegisterAccessWriteButtonClicked();

    void OnCropXOffset();
    void OnCropYOffset();
    void OnCropWidth();
    void OnCropHeight();

    // Button start recording clicked
    void OnStartRecording();
    // Button stop recording clicked
    void OnStopRecording();
    // User has selected a row in the record table
    void OnRecordTableSelectionChanged(const QItemSelection &, const QItemSelection &);
    // Button delete recording clicked
    void OnDeleteRecording();
    // Button save frame clicked
    void OnSaveFrame();
    // Button save frame series clicked
    void OnSaveFrameSeries();
    // Button calc deviation clicked
    void OnCalcDeviation();
    // Button calc live deviation clicked
    void OnCalcLiveDeviation();

    void OnCalcLiveDeviationFromFrameObserver(int numberOfUnequalBytes);

    // Button load reference image clicked
    void OnGetReferenceImage();
    // Deviation calculator thread has calculated the deviation of one frame
    void OnCalcDeviationReady(unsigned int tableRow, int numberOfUnequalBytes, bool done);
    // Button export frame clicked
    void OnExportFrame();

    void OnWidth();
    void OnHeight();
    void OnPixelFormat();
    void OnGain();
    void OnAutoGain();
    void OnExposure();
    void OnAutoExposure();
    void OnExposureAbs();
    void OnPixelFormatDBLClick(QListWidgetItem *);
    void OnFrameSizesDBLClick(QListWidgetItem *);
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
    void OnFrameRate();
    void OnCropCapabilities();
    void OnReadAllValues();


    void OnCameraPixelFormat(const QString &);
    void OnCameraFrameSize(const QString &);

    void OnToggleStreamDelayRand();
    void OnToggleStreamDelay();
};

#endif // V4L2VIEWER_H
