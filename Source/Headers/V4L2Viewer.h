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
#include "AboutWidget.h"
#include "SettingsActionWidget.h"
#include "ui_V4L2Viewer.h"
#include "ControlsHolderWidget.h"

#include <list>

#include <QMainWindow>
#include <QtWidgets>
#include <QTimer>
#include <QTranslator>

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
    QWidgetAction *m_NumberOfFixedFrameRateWidgetAction;
    QLineEdit *m_NumberOfFixedFrameRate;
    // A list of known camera IDs
    std::vector<uint32_t> m_cameras;
    // Is a camera open?
    bool m_bIsOpen;
    // The current streaming state
    bool m_bIsStreaming;
    //// Our Qt image to display
    double m_dScaleFactor;
    // Timer to show the frames received from the frame observer
    QTimer m_FramesReceivedTimer;
    // Graphics scene to show the image
    QSharedPointer<QGraphicsScene> m_pScene;
    // Pixel map for the graphics scene
    QGraphicsPixmapItem *m_PixmapItem;
    // store radio buttons for blocking/non-blocking mode in a group
    QButtonGroup* m_BlockingModeRadioButtonGroup;

    QTranslator *m_pGermanTranslator;
    AboutWidget *m_pAboutWidget;

    SettingsActionWidget *m_pSettingsActionWidget;
    QMenu *m_pSettingsMenu;

    bool m_bIsFixedRate;

    int32_t m_MinimumExposure;
    int32_t m_MaximumExposure;

    int32_t m_sliderGainValue;
    int32_t m_sliderBlackLevelValue;
    int32_t m_sliderGammaValue;
    int32_t m_sliderExposureValue;

    ControlsHolderWidget m_EnumerationControlWidget;

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

    // Official QT dialog close event callback
    virtual void closeEvent(QCloseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void changeEvent(QEvent *event);

    // called by master viewer window
    void RemoteClose();
    void GetImageInformation();
    // Check if IO Read was checked and remove it when not capable
    void Check4IOReadAbility();
    void SetTitleText();
    void UpdateCameraFormat();

    // The function for get device info
    QString GetDeviceInfo();

private slots:
    void OnLogToFile();
    void OnShowFrames();
    // The event handler to close the program
    void OnMenuCloseTriggered();
    // The event handler to set IO Read
    void OnUseRead();
    // The event handler to set IO MMAP
    void OnUseMMAP();
    // The event handler to set IO USERPTR
    void OnUseUSERPTR();
    // The event handler to set VIDIOC_TRY_FMT
    void OnUseVIDIOC_TRY_FMT();
    // The event handler to open a next viewer
    void OnMenuOpenNextViewer();
    // The event handler for open / close camera
    void OnOpenCloseButtonClicked();
    // The event handler for the camera list changed event
    void OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info);
    // The event handler for starting acquisition
    void OnStartButtonClicked();
    // The event handler for stopping acquisition
    void OnStopButtonClicked();
    // The event handler to resize the image to fit to window
    void OnZoomFitButtonClicked();
    // The event handler for resize the image
    void OnZoomInButtonClicked();
    void OnZoomOutButtonClicked();
    void OnSaveImageClicked();
    // The event handler to show the frames received
    void OnUpdateFramesReceived();
    // The event handler to show the processed frame
    void OnFrameReady(const QImage &image, const unsigned long long &frameId);
    // The event handler to show the processed frame ID
    void OnFrameID(const unsigned long long &frameId);
    // The event handler to open a camera on double click event
    void OnListBoxCamerasItemDoubleClicked(QListWidgetItem * item);

    void OnSavePNG();
    void OnSaveRAW();

    void OnCropXOffset();
    void OnCropYOffset();
    void OnCropWidth();
    void OnCropHeight();

    void OnWidth();
    void OnHeight();
    void OnPixelFormat();
    void OnGain();
    void OnAutoGain();
    void OnExposure();
    void OnAutoExposure();
    void OnExposureAbs();
    void OnPixelFormatChanged(const QString &item);
    void UpdateCurrentPixelFormatOnList(QString pixelFormat);
    void OnFrameSizesDBLClick(QListWidgetItem *);
    void OnGamma();
    void OnBrightness();
    void OnContrast();
    void OnSaturation();
    void OnHue();
    void OnContinousWhiteBalance();
    void OnRedBalance();
    void OnBlueBalance();
    void OnFrameRate();
    void OnCropCapabilities();
    void OnReadAllValues();

    void OnCameraPixelFormat(const QString &);
    void OnCameraFrameSize(const QString &);
    void OnLanguageChange();

    void OnUpdateAutoExposure(int32_t value);
    void OnUpdateAutoGain(int32_t value);

    void OnSliderExposureValueChange(int value);
    void OnSliderGainValueChange(int value);
    void OnSliderGammaValueChange(int value);
    void OnSliderBlackLevelValueChange(int value);

    void OnSlidersReleased();
    void UpdateSlidersPositions(QSlider *slider, int32_t value);

    void OnCameraListButtonClicked();
    void OnMenuSplitterMoved(int pos, int index);

    void OnFixedFrameRateButtonClicked();
    void CheckAquiredFixedFrames(int framesCount);

    int32_t GetSliderValueFromLog(int32_t value);

    void ShowHideEnumerationControlWidget();

    void GetIntDataToEnumerationWidget(int32_t id,  int32_t step, int32_t min, int32_t max, int32_t value, QString name);
    void GetIntDataToEnumerationWidget(int32_t id,  int64_t step, int64_t min, int64_t max, int64_t value, QString name);
    void GetBoolDataToEnumerationWidget(int32_t id, bool value, QString name);
    void GetButtonDataToEnumerationWidget(int32_t id, QString name);
    void GetListDataToEnumerationWidget(int32_t id, QList<QString> list, QString name);
    void GetListDataToEnumerationWidget(int32_t id, QList<int64_t> list, QString name);
};

#endif // V4L2VIEWER_H
