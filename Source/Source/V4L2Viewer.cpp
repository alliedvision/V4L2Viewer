/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        V4L2Viewer.cpp

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

#include <V4L2Helper.h>
#include "Logger.h"
#include "V4L2Viewer.h"
#include "CameraListCustomItem.h"

#include <QtCore>
#include <QtGlobal>
#include <QStringList>
#include <QFontDatabase>

#include <ctime>
#include <limits>
#include <sstream>

#define NUM_COLORS 3
#define BIT_DEPTH 8

#define MAX_ZOOM_IN  (16.0)
#define MAX_ZOOM_OUT (1/8.0)
#define ZOOM_INCREMENT (2.0)

#define MANUF_NAME_AV       "Allied Vision"
#define APP_NAME            "Video4Linux2 Viewer"
#define APP_VERSION_MAJOR   1
#define APP_VERSION_MINOR   41
#define APP_VERSION_PATCH   0
#ifndef SCM_REVISION
#define SCM_REVISION        0
#endif

// CCI registers
#define CCI_GCPRM_16R                               0x0010
#define CCI_BCRM_16R                                0x0014


static const QStringList GetImageFormats()
{
    QStringList formats;
    unsigned int count = QImageReader::supportedImageFormats().count();

    for ( int i = count - 1; i >= 0; i-- )
    {
        QString format = QString(QImageReader::supportedImageFormats().at(i)).toLower();
        formats << format;
    }

    return formats;
}

static const QString GetImageFormatString()
{
    QString formatString;
    unsigned int count = QImageReader::supportedImageFormats().count();

    for ( int i = count - 1; i >= 0; i-- )
    {
        formatString += ".";
        formatString += QString(QImageReader::supportedImageFormats().at(i)).toLower();
        if ( 0 != i )
        {
            formatString += ";;";
        }
    }

    return formatString;
}

static int32_t int64_2_int32(const int64_t value)
{
    if (value > 0)
    {
        return std::min(value, (int64_t)std::numeric_limits<int32_t>::max());
    }
    else
    {
        return std::max(value, (int64_t)std::numeric_limits<int32_t>::min());
    }
}

V4L2Viewer::V4L2Viewer(QWidget *parent, Qt::WindowFlags flags, int viewerNumber)
    : QMainWindow(parent, flags)
    , m_nViewerNumber(viewerNumber)
    , m_bIsOpen(false)
    , m_bIsStreaming(false)
    , m_dScaleFactor(1.0)
    , m_BLOCKING_MODE(true)
    , m_MMAP_BUFFER(IO_METHOD_MMAP) // use mmap by default
    , m_VIDIOC_TRY_FMT(true) // use VIDIOC_TRY_FMT by default
    , m_ShowFrames(true)
    , m_nDroppedFrames(0)
    , m_nStreamNumber(0)
{
    QFontDatabase::addApplicationFont(":/Fonts/Open_Sans/OpenSans-Regular.ttf");
    QFont font;
    font.setFamily("Open Sans");
    qApp->setFont(font);

    m_pGermanTranslator = new QTranslator(this);
    m_pGermanTranslator->load(":/Translations/Translations/german.qm");

    srand((unsigned)time(0));

    Logger::InitializeLogger("V4L2ViewerLog.log");

    ui.setupUi(this);

    // connect the menu actions
    connect(ui.m_MenuClose, SIGNAL(triggered()), this, SLOT(OnMenuCloseTriggered()));
    if (VIEWER_MASTER == m_nViewerNumber)
        connect(ui.m_MenuOpenNextViewer, SIGNAL(triggered()), this, SLOT(OnMenuOpenNextViewer()));
    else
        ui.m_MenuOpenNextViewer->setEnabled(false);

    // Connect GUI events with event handlers
    connect(ui.m_OpenCloseButton,             SIGNAL(clicked()),         this, SLOT(OnOpenCloseButtonClicked()));
    connect(ui.m_StartButton,                 SIGNAL(clicked()),         this, SLOT(OnStartButtonClicked()));
    connect(ui.m_StopButton,                  SIGNAL(clicked()),         this, SLOT(OnStopButtonClicked()));
    connect(ui.m_ZoomFitButton,               SIGNAL(clicked()),         this, SLOT(OnZoomFitButtonClicked()));
    connect(ui.m_ZoomInButton,                SIGNAL(clicked()),         this, SLOT(OnZoomInButtonClicked()));
    connect(ui.m_ZoomOutButton,               SIGNAL(clicked()),         this, SLOT(OnZoomOutButtonClicked()));
    connect(ui.m_SaveImageButton,             SIGNAL(clicked()),         this, SLOT(OnSaveImageClicked()));

    OnLog("Starting Application");

    SetTitleText("");

    // Start Camera
    connect(&m_Camera, SIGNAL(OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameReady_Signal(const QImage &, const unsigned long long &)), this, SLOT(OnFrameReady(const QImage &, const unsigned long long &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameID_Signal(const unsigned long long &)), this, SLOT(OnFrameID(const unsigned long long &)));
    connect(&m_Camera, SIGNAL(OnCameraEventReady_Signal(const QString &)), this, SLOT(OnCameraEventReady(const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraRegisterValueReady_Signal(unsigned long long)), this, SLOT(OnCameraRegisterValueReady(unsigned long long)));
    connect(&m_Camera, SIGNAL(OnCameraError_Signal(const QString &)), this, SLOT(OnCameraError(const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraMessage_Signal(const QString &)), this, SLOT(OnCameraMessage(const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraPixelFormat_Signal(const QString &)), this, SLOT(OnCameraPixelFormat(const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameSize_Signal(const QString &)), this, SLOT(OnCameraFrameSize(const QString &)));

    // Setup blocking mode radio buttons
    m_BlockingModeRadioButtonGroup = new QButtonGroup(this);
    m_BlockingModeRadioButtonGroup->setExclusive(true);


    connect(m_BlockingModeRadioButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnBlockingMode(QAbstractButton*)));
    connect(ui.m_TitleUseRead, SIGNAL(triggered()), this, SLOT(OnUseRead()));
    connect(ui.m_TitleUseMMAP, SIGNAL(triggered()), this, SLOT(OnUseMMAP()));
    connect(ui.m_TitleUseUSERPTR, SIGNAL(triggered()), this, SLOT(OnUseUSERPTR()));
    connect(ui.m_TitleEnable_VIDIOC_TRY_FMT, SIGNAL(triggered()), this, SLOT(OnUseVIDIOC_TRY_FMT()));
    connect(ui.m_DisplayImagesCheckBox, SIGNAL(clicked()), this, SLOT(OnShowFrames()));
    connect(ui.m_TitleClearOutputListbox, SIGNAL(triggered()), this, SLOT(OnClearOutputListbox()));
    connect(ui.m_TitleLogtofile, SIGNAL(triggered()), this, SLOT(OnLogToFile()));
    connect(ui.m_TitleLangEnglish, SIGNAL(triggered()), this, SLOT(OnLanguageChange()));
    connect(ui.m_TitleLangGerman, SIGNAL(triggered()), this, SLOT(OnLanguageChange()));
    connect(ui.m_TitleSavePNG, SIGNAL(triggered()), this, SLOT(OnSavePNG()));
    connect(ui.m_TitleSaveRAW, SIGNAL(triggered()), this, SLOT(OnSaveRAW()));

    m_Camera.DeviceDiscoveryStart();
    connect(ui.m_CamerasListBox, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnListBoxCamerasItemDoubleClicked(QListWidgetItem *)));

    // Connect the handler to show the frames per second
    connect(&m_FramesReceivedTimer, SIGNAL(timeout()), this, SLOT(OnUpdateFramesReceived()));

    // register meta type for QT signal/slot mechanism
    qRegisterMetaType<QSharedPointer<MyFrame> >("QSharedPointer<MyFrame>");

    // connect the buttons for Image m_ControlRequestTimer
    connect(ui.m_edWidth, SIGNAL(returnPressed()), this, SLOT(OnWidth()));
    connect(ui.m_edHeight, SIGNAL(returnPressed()), this, SLOT(OnHeight()));


    connect(ui.m_edGain, SIGNAL(returnPressed()), this, SLOT(OnGain()));
    connect(ui.m_chkAutoGain, SIGNAL(clicked()), this, SLOT(OnAutoGain()));
    connect(ui.m_edExposure, SIGNAL(returnPressed()), this, SLOT(OnExposure()));
    connect(ui.m_edExposureAbs, SIGNAL(returnPressed()), this, SLOT(OnExposureAbs()));
    connect(ui.m_chkAutoExposure, SIGNAL(clicked()), this, SLOT(OnAutoExposure()));

    connect(ui.m_pixelFormats, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnPixelFormatChanged(const QString &)));

    connect(ui.m_edGamma, SIGNAL(returnPressed()), this, SLOT(OnGamma()));

    connect(ui.m_edBlackLevel, SIGNAL(returnPressed()), this, SLOT(OnBrightness()));
    connect(ui.m_edContrast, SIGNAL(returnPressed()), this, SLOT(OnContrast()));
    connect(ui.m_edSaturation, SIGNAL(returnPressed()), this, SLOT(OnSaturation()));
    connect(ui.m_edHue, SIGNAL(returnPressed()), this, SLOT(OnHue()));
    connect(ui.m_chkContWhiteBalance, SIGNAL(clicked()), this, SLOT(OnContinousWhiteBalance()));
    connect(ui.m_edRedBalance, SIGNAL(returnPressed()), this, SLOT(OnRedBalance()));
    connect(ui.m_edBlueBalance, SIGNAL(returnPressed()), this, SLOT(OnBlueBalance()));
    connect(ui.m_edFrameRate, SIGNAL(returnPressed()), this, SLOT(OnFrameRate()));
    connect(ui.m_edCropXOffset, SIGNAL(returnPressed()), this, SLOT(OnCropXOffset()));
    connect(ui.m_edCropYOffset, SIGNAL(returnPressed()), this, SLOT(OnCropYOffset()));
    connect(ui.m_edCropWidth, SIGNAL(returnPressed()), this, SLOT(OnCropWidth()));
    connect(ui.m_edCropHeight, SIGNAL(returnPressed()), this, SLOT(OnCropHeight()));
    connect(ui.m_butReadValues, SIGNAL(clicked()), this, SLOT(OnReadAllValues()));

    // Set the splitter stretch factors
    ui.m_Splitter1->setStretchFactor(0, 45);
    ui.m_Splitter1->setStretchFactor(1, 55);
    ui.m_Splitter2->setStretchFactor(0, 75);
    ui.m_Splitter2->setStretchFactor(1, 25);

    // Set the viewer scene
    m_pScene = QSharedPointer<QGraphicsScene>(new QGraphicsScene());

    m_PixmapItem = new QGraphicsPixmapItem();

    ui.m_ImageView->setScene(m_pScene.data());

    m_pScene->addItem(m_PixmapItem);

    ///////////////////// Number of frames /////////////////////
    // add the number of used frames option to the menu
    m_NumberOfUsedFramesLineEdit = new QLineEdit(this);
    m_NumberOfUsedFramesLineEdit->setText("5");
    m_NumberOfUsedFramesLineEdit->setValidator(new QIntValidator(1, 500000, this));

    // prepare the layout
    QHBoxLayout *layoutNum = new QHBoxLayout;
    QLabel *labelNum = new QLabel("Number of used Frames:");
    layoutNum->addWidget(labelNum);
    layoutNum->addWidget(m_NumberOfUsedFramesLineEdit);

    // put the layout into a widget
    QWidget *widgetNum = new QWidget(this);
    widgetNum->setLayout(layoutNum);
    widgetNum->setStyleSheet("QWidget{background:transparent; color:white;} QWidget::disabled{color:rgb(79,79,79);}");

    // add the widget into the menu bar
    m_NumberOfUsedFramesWidgetAction = new QWidgetAction(this);
    m_NumberOfUsedFramesWidgetAction->setDefaultWidget(widgetNum);
    ui.m_MenuBuffer->addAction(m_NumberOfUsedFramesWidgetAction);

    // add about widget to the menu bar
    m_AboutWidget = new AboutWidget(this);
    QWidgetAction *aboutWidgetAction = new QWidgetAction(this);
    aboutWidgetAction->setDefaultWidget(m_AboutWidget);
    ui.m_MenuAbout->addAction(aboutWidgetAction);

    ui.menuBar->setNativeMenuBar(false);

    QMainWindow::showMaximized();

    UpdateViewerLayout();
    UpdateZoomButtons();

    // set check boxes state for mmap according to variable m_MMAP_BUFFER
    if (IO_METHOD_READ == m_MMAP_BUFFER)
    {
        ui.m_TitleUseRead->setChecked(true);
    }
    else
    {
        ui.m_TitleUseRead->setChecked(false);
    }

    if (IO_METHOD_USERPTR == m_MMAP_BUFFER)
    {
        ui.m_TitleUseUSERPTR->setChecked(true);
    }
    else
    {
        ui.m_TitleUseUSERPTR->setChecked(false);
    }

    if (IO_METHOD_MMAP == m_MMAP_BUFFER)
    {
        ui.m_TitleUseMMAP->setChecked(true);
    }
    else
    {
        ui.m_TitleUseMMAP->setChecked(false);
    }

    ui.m_TitleEnable_VIDIOC_TRY_FMT->setChecked((m_VIDIOC_TRY_FMT));
}

V4L2Viewer::~V4L2Viewer()
{
    m_Camera.DeviceDiscoveryStop();

    // if we are streaming stop streaming
    if ( true == m_bIsOpen )
        OnOpenCloseButtonClicked();

}

// The event handler to close the program
void V4L2Viewer::OnMenuCloseTriggered()
{
    close();
}

void V4L2Viewer::OnBlockingMode(QAbstractButton* button)
{
    // m_BLOCKING_MODE = ui.m_radioBlocking->isChecked();
    OnLog(QString("Use BLOCKING_MODE = %1").arg((m_BLOCKING_MODE) ? "TRUE" : "FALSE"));
}

void V4L2Viewer::OnUseRead()
{
    m_MMAP_BUFFER = IO_METHOD_READ;
    OnLog(QString("Use IO Read"));

    ui.m_TitleUseMMAP->setChecked(false);
    ui.m_TitleUseUSERPTR->setChecked(false);
    ui.m_TitleUseRead->setChecked(true);
}

void V4L2Viewer::OnUseMMAP()
{
    m_MMAP_BUFFER = IO_METHOD_MMAP;
    OnLog(QString("Use IO MMAP"));

    ui.m_TitleUseMMAP->setChecked(true);
    ui.m_TitleUseUSERPTR->setChecked(false);
    ui.m_TitleUseRead->setChecked(false);
}

void V4L2Viewer::OnUseUSERPTR()
{
    m_MMAP_BUFFER = IO_METHOD_USERPTR;
    OnLog(QString("Use IO USERPTR"));

    ui.m_TitleUseMMAP->setChecked(false);
    ui.m_TitleUseUSERPTR->setChecked(true);
    ui.m_TitleUseRead->setChecked(false);
}

void V4L2Viewer::OnUseVIDIOC_TRY_FMT()
{
    m_VIDIOC_TRY_FMT = !m_VIDIOC_TRY_FMT;
    OnLog(QString("Use VIDIOC_TRY_FMT = %1").arg((m_VIDIOC_TRY_FMT) ? "TRUE" : "FALSE"));

    ui.m_TitleEnable_VIDIOC_TRY_FMT->setChecked(m_VIDIOC_TRY_FMT);
}

void V4L2Viewer::OnShowFrames()
{
    m_ShowFrames = !m_ShowFrames;
    OnLog(QString("Show Frames = %1").arg((m_ShowFrames) ? "TRUE" : "FALSE"));

    m_Camera.SwitchFrameTransfer2GUI(m_ShowFrames);
}

void V4L2Viewer::OnClearOutputListbox()
{
    ui.m_LogTextEdit->clear();
}

void V4L2Viewer::OnLogToFile()
{
    Logger::LogSwitch(ui.m_TitleLogtofile->isChecked());
}

void V4L2Viewer::RemoteClose()
{
    if ( true == m_bIsOpen )
        OnOpenCloseButtonClicked();
}

// The event handler to open a next viewer
void V4L2Viewer::OnMenuOpenNextViewer()
{
    QSharedPointer<V4L2Viewer> viewer(new V4L2Viewer(0, 0, (int)(m_pViewer.size() + 2)));
    m_pViewer.push_back(viewer);
    viewer->show();
}

void V4L2Viewer::closeEvent(QCloseEvent *event)
{
    if (VIEWER_MASTER == m_nViewerNumber)
    {
        std::list<QSharedPointer<V4L2Viewer> >::iterator xIter;
        for (xIter = m_pViewer.begin(); xIter != m_pViewer.end(); xIter++)
        {
            QSharedPointer<V4L2Viewer> viewer = *xIter;
            viewer->RemoteClose();
            viewer->close();
        }
        QApplication::quit();
    }

    event->accept();
}

void V4L2Viewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier))
    {
        event->accept();
        if (event->delta() > 0)
            OnZoomOutButtonClicked();
        else
            OnZoomInButtonClicked();
    }
    else
        event->ignore();
}

void V4L2Viewer::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui.retranslateUi(this);
        m_AboutWidget->UpdateStrings();
    }
    else
    {
        QMainWindow::changeEvent(event);
    }
}

void V4L2Viewer::mousePressEvent(QMouseEvent *event)
{
    QPoint imageViewPoint = ui.m_ImageView->mapFrom(this, event->pos());
    QPointF imageScenePointF = ui.m_ImageView->mapToScene(imageViewPoint);

    if (m_PixmapItem != nullptr && m_PixmapItem->pixmap().rect().contains(imageScenePointF.toPoint()))
    {
        if (m_dScaleFactor >= 1)
        {
            QImage image = m_PixmapItem->pixmap().toImage();
            QColor myPixel = image.pixel(static_cast<int>(imageScenePointF.x()), static_cast<int>(imageScenePointF.y()));
            QToolTip::showText(ui.m_ImageView->mapToGlobal(imageViewPoint), QString("x:%1, y:%2, r:%3/g:%4/b:%5")
                               .arg(static_cast<int>(imageScenePointF.x())).arg(static_cast<int>(imageScenePointF.y()))
                               .arg(myPixel.red())
                               .arg(myPixel.green())
                               .arg(myPixel.blue()), this);
        }
    }
}

// The event handler for open / close camera
void V4L2Viewer::OnOpenCloseButtonClicked()
{
    if (false == m_bIsOpen)
    {
        // check if IO parameter are correct
        Check4IOReadAbility();
    }

    // Disable the open/close button and redraw it
    ui.m_OpenCloseButton->setEnabled(false);
    QApplication::processEvents();

    int err;
    int nRow = ui.m_CamerasListBox->currentRow();

    if (-1 < nRow)
    {
        QString devName = dynamic_cast<CameraListCustomItem*>(ui.m_CamerasListBox->itemWidget(ui.m_CamerasListBox->item(nRow)))->GetCameraName();
        QString deviceName = devName.right(devName.length() - devName.indexOf(':') - 2);

        deviceName = deviceName.left(deviceName.indexOf('(') - 1);

        if (false == m_bIsOpen)
        {
            // Start
            err = OpenAndSetupCamera(m_cameras[nRow], deviceName);
            // Set up Qt image
            if (0 == err)
            {
                OnLog("Camera opened successfully");
                GetImageInformation();
                SetTitleText(deviceName);
                ui.m_CamerasListBox->removeItemWidget(ui.m_CamerasListBox->item(nRow));
                CameraListCustomItem *newItem = new CameraListCustomItem(devName, this);
                QString devInfo = GetDeviceInfo();
                newItem->SetCameraInformation(devInfo);
                ui.m_CamerasListBox->item(nRow)->setSizeHint(newItem->sizeHint());
                ui.m_CamerasListBox->setItemWidget(ui.m_CamerasListBox->item(nRow), newItem);
            }
            else
                CloseCamera(m_cameras[nRow]);

            m_bIsOpen = (0 == err);
        }
        else
        {
            m_bIsOpen = false;

            // Stop
            if (true == m_bIsStreaming)
            {
                OnStopButtonClicked();
            }

            m_dScaleFactor = 1.0;

            err = CloseCamera(m_cameras[nRow]);
            if (0 == err)
            {
                OnLog("Camera closed successfully");
                ui.m_CamerasListBox->removeItemWidget(ui.m_CamerasListBox->item(nRow));
                CameraListCustomItem *newItem = new CameraListCustomItem(devName, this);
                ui.m_CamerasListBox->item(nRow)->setSizeHint(newItem->sizeHint());
                ui.m_CamerasListBox->setItemWidget(ui.m_CamerasListBox->item(nRow), newItem);
            }

            SetTitleText("");
        }

        if (false == m_bIsOpen)
        {
            ui.m_OpenCloseButton->setText(QString("Open Camera"));
        }
        else
        {
            ui.m_OpenCloseButton->setText(QString("Close Camera"));
        }

        UpdateViewerLayout();
    }

    ui.m_OpenCloseButton->setEnabled( 0 <= m_cameras.size() || m_bIsOpen );

    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_TitleUseRead->setEnabled( !m_bIsOpen );
    ui.m_TitleEnable_VIDIOC_TRY_FMT->setEnabled( !m_bIsOpen );
}

// The event handler for starting
void V4L2Viewer::OnStartButtonClicked()
{
    uint32_t payloadSize = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pixelFormat = 0;
    uint32_t bytesPerLine = 0;
    QString pixelFormatText;
    int result = 0;

    // Get the payload size first to setup the streaming channel
    result = m_Camera.ReadPayloadSize(payloadSize);
    OnLog(QString("Received payload size = %1").arg(payloadSize));
    result = m_Camera.ReadFrameSize(width, height);
    OnLog(QString("Received width = %1").arg(width));
    OnLog(QString("Received height = %1").arg(height));
    result = m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    OnLog(QString("Received pixel format = %1 = 0x%2 = %3").arg(pixelFormat).arg(pixelFormat, 8, 16, QChar('0')).arg(pixelFormatText));

    if (result == 0)
        StartStreaming(pixelFormat, payloadSize, width, height, bytesPerLine);
}

void V4L2Viewer::OnCameraRegisterValueReady(unsigned long long value)
{
    OnLog(QString("Received value = %1").arg(value));
}

void V4L2Viewer::OnCameraError(const QString &text)
{
    OnLog(QString("Error = %1").arg(text));
}

void V4L2Viewer::OnCameraWarning(const QString &text)
{
    OnLog(QString("Warning = %1").arg(text));
}

void V4L2Viewer::OnCameraMessage(const QString &text)
{
    OnLog(QString("Message = %1").arg(text));
}

void V4L2Viewer::OnCameraPixelFormat(const QString& pixelFormat)
{
    ui.m_pixelFormats->blockSignals(true);
    ui.m_pixelFormats->addItem(pixelFormat);
    ui.m_pixelFormats->blockSignals(false);
}

void V4L2Viewer::OnCameraFrameSize(const QString& frameSize)
{
    //ui.m_liFrameSizes->addItem(frameSize);
}

void V4L2Viewer::OnLanguageChange()
{
    QAction *senderAction = qobject_cast<QAction*>(sender());
    if (QString::compare(senderAction->text(), "German", Qt::CaseInsensitive) == 0)
    {
        qApp->installTranslator(m_pGermanTranslator);
    }
    else if (QString::compare(senderAction->text(), "Englisch", Qt::CaseInsensitive) == 0)
    {
        qApp->removeTranslator(m_pGermanTranslator);
    }
    else
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("Language has been already set!") );
    }
}

void V4L2Viewer::StartStreaming(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine)
{
    int err = 0;
    int nRow = ui.m_CamerasListBox->currentRow();

    // disable the start button to show that the start acquisition is in process
    ui.m_StartButton->setEnabled(false);
    ui.m_SaveImageButton->setEnabled(false);
    QApplication::processEvents();

    m_nDroppedFrames = 0;

    // start streaming

    if (m_Camera.CreateUserBuffer(m_NumberOfUsedFramesLineEdit->text().toLong(), payloadSize) == 0)
    {
        m_Camera.QueueAllUserBuffer();
        OnLog("Starting Stream...");
        if (m_Camera.StartStreaming() == 0)
        {
            OnLog("Start Stream OK.");
        }
        else
        {
            OnLog("Start Stream failed.");
        }
        err = m_Camera.StartStreamChannel(pixelFormat,
                                          payloadSize,
                                          width,
                                          height,
                                          bytesPerLine,
                                          NULL,
                                          ui.m_TitleLogtofile->isChecked());

        if (0 != err)
            OnLog("Start Acquisition failed during SI Start channel.");
        else
        {
            OnLog("Acquisition started ...");
            m_bIsStreaming = true;
        }

        UpdateViewerLayout();

        m_FramesReceivedTimer.start(1000);
        m_Camera.QueueAllUserBuffer();

        OnLog("Starting Stream...");
    }
    else
    {
        OnLog("Creating user buffers failed.");
    }
}

// The event handler for stopping acquisition
void V4L2Viewer::OnStopButtonClicked()
{
    // disable the stop button to show that the stop acquisition is in process
    ui.m_StopButton->setEnabled(false);
    ui.m_SaveImageButton->setEnabled(true);

    int err = m_Camera.StopStreamChannel();
    if (0 != err)
        OnLog("Stop stream channel failed.");

    OnLog("Stopping Stream...");
    if (m_Camera.StopStreaming() == 0)
    {
        OnLog("Stop Stream OK.");
    }
    else
    {
        OnLog("Stop Stream failed.");
    }

    QApplication::processEvents();

    m_bIsStreaming = false;
    UpdateViewerLayout();
    OnLog("Stream channel stopped ...");

    m_FramesReceivedTimer.stop();

    m_Camera.DeleteUserBuffer();
}

// The event handler to resize the image to fit to window
void V4L2Viewer::OnZoomFitButtonClicked()
{
    if (ui.m_ZoomFitButton->isChecked())
    {
        ui.m_ImageView->fitInView(m_pScene->sceneRect(), Qt::KeepAspectRatio);
    }
    else
    {
        QTransform transformation;
        transformation.scale(m_dScaleFactor, m_dScaleFactor);
        ui.m_ImageView->setTransform(transformation);
    }

    UpdateZoomButtons();
}

// The event handler for resize the image
void V4L2Viewer::OnZoomInButtonClicked()
{
    m_dScaleFactor *= ZOOM_INCREMENT;

    QTransform transformation;
    transformation.scale(m_dScaleFactor, m_dScaleFactor);
    ui.m_ImageView->setTransform(transformation);

    UpdateZoomButtons();
}

// The event handler for resize the image
void V4L2Viewer::OnZoomOutButtonClicked()
{
    m_dScaleFactor *= (1 / ZOOM_INCREMENT);

    QTransform transformation;
    transformation.scale(m_dScaleFactor, m_dScaleFactor);
    ui.m_ImageView->setTransform(transformation);

    UpdateZoomButtons();
}

void V4L2Viewer::OnSaveImageClicked()
{
    if (ui.m_TitleSavePNG->isChecked())
    {
        QString format;
        format = "png";
        QPixmap pixmap = m_PixmapItem->pixmap();
        QImage image = pixmap.toImage();
        QString filename = QFileDialog::getSaveFileName(this, tr("Save file"), QDir::homePath(), ".png");
        image.save(filename,"png");
    }
    else
    {
        QString format;
        format = "raw";
        QPixmap pixmap = m_PixmapItem->pixmap();
        QImage image = pixmap.toImage();
        int size = image.sizeInBytes();
        QByteArray data(reinterpret_cast<const char*>(image.bits()), size);
        QString filename = QFileDialog::getSaveFileName(this, tr("Save file"), QDir::homePath(), ".raw");
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
    }
}

// The event handler to show the processed frame
void V4L2Viewer::OnFrameReady(const QImage &image, const unsigned long long &frameId)
{
    if (m_ShowFrames)
    {
        if (!image.isNull())
        {
            if (ui.m_FlipHorizontalCheckBox->isChecked() || ui.m_FlipVerticalCheckBox->isChecked())
            {
                QImage tmpImage;
                if (ui.m_FlipVerticalCheckBox->isChecked())
                    tmpImage = image.mirrored(false, true);
                else
                    tmpImage = image;
                if (ui.m_FlipHorizontalCheckBox->isChecked())
                    tmpImage = tmpImage.mirrored(true, false);
                m_pScene->setSceneRect(0, 0, tmpImage.width(), tmpImage.height());
                m_PixmapItem->setPixmap(QPixmap::fromImage(tmpImage));
                ui.m_ImageView->show();

                ui.m_FrameIdLabel->setText(QString("Frame ID: %1, W: %2, H: %3").arg(frameId).arg(tmpImage.width()).arg(tmpImage.height()));
            }
            else
            {
                m_pScene->setSceneRect(0, 0, image.width(), image.height());
                m_PixmapItem->setPixmap(QPixmap::fromImage(image));
                ui.m_ImageView->show();

                ui.m_FrameIdLabel->setText(QString("Frame ID: %1, W: %2, H: %3").arg(frameId).arg(image.width()).arg(image.height()));
            }
        }
        else
            m_nDroppedFrames++;
    }
    else
        ui.m_FrameIdLabel->setText(QString("FrameID: %1").arg(frameId));
}

// The event handler to show the processed frame
void V4L2Viewer::OnFrameID(const unsigned long long &frameId)
{
    ui.m_FrameIdLabel->setText(QString("FrameID: %1").arg(frameId));
}

// The event handler to show the event data
void V4L2Viewer::OnCameraEventReady(const QString &eventText)
{
    OnLog("Event received.");
}

// This event handler is triggered through a Qt signal posted by the camera observer
void V4L2Viewer::OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
    bool bUpdateList = false;

    // We only react on new cameras being found and known cameras being unplugged
    if (UpdateTriggerPluggedIn == reason)
    {
        bUpdateList = true;

        std::string manuName;
        std::string devName = deviceName.toLatin1().data();
        OnLog(QString("Camera list changed. A new camera was discovered, cardNumber=%1, deviceID=%2, cardName=%3.").arg(cardNumber).arg(deviceID).arg(info));
    }
    else if (UpdateTriggerPluggedOut == reason)
    {
        OnLog(QString("Camera list changed. A camera was disconnected, cardNumber=%1, deviceID=%2.").arg(cardNumber).arg(deviceID));
        if ( true == m_bIsOpen )
        {
            OnOpenCloseButtonClicked();
        }
        bUpdateList = true;
    }

    if ( true == bUpdateList )
    {
        UpdateCameraListBox(cardNumber, deviceID, deviceName, info);
    }

    ui.m_OpenCloseButton->setEnabled( 0 < m_cameras.size() || m_bIsOpen );
    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_TitleUseRead->setEnabled( !m_bIsOpen );
    ui.m_TitleEnable_VIDIOC_TRY_FMT->setEnabled( !m_bIsOpen );
}

// The event handler to open a camera on double click event
void V4L2Viewer::OnListBoxCamerasItemDoubleClicked(QListWidgetItem * item)
{
    OnOpenCloseButtonClicked();
}

void V4L2Viewer::OnSavePNG()
{
    if (ui.m_TitleSavePNG->isChecked())
    {
        ui.m_TitleSaveRAW->setChecked(false);
    }
    else
    {
        ui.m_TitleSavePNG->setChecked(true);
    }
}

void V4L2Viewer::OnSaveRAW()
{
    if (ui.m_TitleSaveRAW->isChecked())
    {
        ui.m_TitleSavePNG->setChecked(false);
    }
    else
    {
        ui.m_TitleSaveRAW->setChecked(true);
    }
}

// Queries and lists all known camera
void V4L2Viewer::UpdateCameraListBox(uint32_t cardNumber, uint64_t cameraID, const QString &deviceName, const QString &info)
{
    std::string strCameraName;

    strCameraName = "Camera#";

    QListWidgetItem *item = new QListWidgetItem(ui.m_CamerasListBox);
    CameraListCustomItem *customItem = new CameraListCustomItem(QString::fromStdString(strCameraName + ": ") + deviceName + QString(" (") + info + QString(")"), this);
    item->setSizeHint(customItem->sizeHint());
    ui.m_CamerasListBox->setItemWidget(item, customItem);

    m_cameras.push_back(cardNumber);

    // select the first camera if there is no camera selected
    if ((-1 == ui.m_CamerasListBox->currentRow()) && (0 < ui.m_CamerasListBox->count()))
    {
        ui.m_CamerasListBox->setCurrentRow(0, QItemSelectionModel::Select);
    }

    ui.m_OpenCloseButton->setEnabled((0 < m_cameras.size()) || m_bIsOpen);
    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_TitleUseRead->setEnabled( !m_bIsOpen );
    ui.m_TitleEnable_VIDIOC_TRY_FMT->setEnabled( !m_bIsOpen );
}

// Update the viewer range
void V4L2Viewer::UpdateViewerLayout()
{
    m_NumberOfUsedFramesWidgetAction->setEnabled(!m_bIsOpen);

    if (!m_bIsOpen)
    {
        QPixmap pix(":/V4L2Viewer/Viewer.png");
        m_pScene->setSceneRect(0, 0, pix.width(), pix.height());
        m_PixmapItem->setPixmap(pix);
        ui.m_ImageView->show();
        m_bIsStreaming = false;
    }

    // Hide all cameras when not needed and block signals to avoid
    // unnecessary reopening of the current camera
    ui.m_CamerasListBox->blockSignals(m_bIsOpen);
    int currentIndex = ui.m_CamerasListBox->currentRow();
    if (currentIndex >= 0)
    {
        unsigned int items = ui.m_CamerasListBox->count();
        for (unsigned int i=0; i<items; i++)
        {
            if (i == currentIndex)
            {
                continue;
            }
            else
            {
                ui.m_CamerasListBox->item(i)->setHidden(m_bIsOpen);
            }
        }
    }

    ui.m_StartButton->setEnabled(m_bIsOpen && !m_bIsStreaming);
    ui.m_StopButton->setEnabled(m_bIsOpen && m_bIsStreaming);
    ui.m_FramesPerSecondLabel->setEnabled(m_bIsOpen && m_bIsStreaming);
    ui.m_FrameIdLabel->setEnabled(m_bIsOpen && m_bIsStreaming);

    UpdateZoomButtons();
}

// Update the zoom buttons
void V4L2Viewer::UpdateZoomButtons()
{
    ui.m_ZoomFitButton->setEnabled(m_bIsOpen);

    if (m_dScaleFactor >= MAX_ZOOM_IN)
    {
        ui.m_ZoomInButton->setEnabled(false);
    }
    else
    {
        ui.m_ZoomInButton->setEnabled(true && m_bIsOpen && !ui.m_ZoomFitButton->isChecked());
    }

    if (m_dScaleFactor <= MAX_ZOOM_OUT)
    {
        ui.m_ZoomOutButton->setEnabled(false);
    }
    else
    {
        ui.m_ZoomOutButton->setEnabled(true && m_bIsOpen && !ui.m_ZoomFitButton->isChecked());
    }

    ui.m_ZoomLabel->setText(QString("%1%").arg(m_dScaleFactor * 100));
    ui.m_ZoomLabel->setEnabled(m_bIsOpen && m_bIsStreaming && !ui.m_ZoomFitButton->isChecked());
}

// Prints out some logging without error
void V4L2Viewer::OnLog(const QString &strMsg)
{
    if (ui.m_TitleEnableMessageListbox->isChecked())
    {
        ui.m_LogTextEdit->appendPlainText(strMsg);
        ui.m_LogTextEdit->verticalScrollBar()->setValue(ui.m_LogTextEdit->verticalScrollBar()->maximum());
    }
}

// Open/Close the camera
int V4L2Viewer::OpenAndSetupCamera(const uint32_t cardNumber, const QString &deviceName)
{
    int err = 0;

    std::string devName = deviceName.toStdString();
    err = m_Camera.OpenDevice(devName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT);

    if (0 != err)
    {
        OnLog("Open device failed");
    }

    return err;
}

int V4L2Viewer::CloseCamera(const uint32_t cardNumber)
{
    int err = 0;

    err = m_Camera.CloseDevice();
    if (0 != err)
        OnLog("Close device failed.");

    return err;
}

// The event handler to show the frames received
void V4L2Viewer::OnUpdateFramesReceived()
{
    unsigned int fpsReceived = m_Camera.GetReceivedFramesCount();
    unsigned int fpsRendered = m_Camera.GetRenderedFramesCount();
    unsigned int uncompletedFrames = m_Camera.GetDroppedFramesCount() + m_nDroppedFrames;

    ui.m_FramesPerSecondLabel->setText(QString("fps: %1 received/%2 rendered [drops %3]").arg(fpsReceived).arg(fpsRendered).arg(uncompletedFrames));
}

void V4L2Viewer::OnWidth()
{
    if (m_Camera.SetFrameSize(ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt()) < 0)
    {
        uint32_t width = 0;
        uint32_t height = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE frame size!") );
        m_Camera.ReadFrameSize(width, height);
        ui.m_edWidth->setText(QString("%1").arg(width));
        ui.m_edHeight->setText(QString("%1").arg(height));
    }
    else
    {
        uint32_t payloadSize = 0;
        OnLog(QString("Frame size set to %1x%2").arg(ui.m_edWidth->text().toInt()).arg(ui.m_edHeight->text().toInt()));

        m_Camera.ReadPayloadSize(payloadSize);
    }
}

void V4L2Viewer::OnHeight()
{
    uint32_t width = 0;
    uint32_t height = 0;

    if (m_Camera.SetFrameSize(ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt()) < 0)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE frame size!") );
    }
    else
    {
        uint32_t payloadSize = 0;
        OnLog(QString("Frame size set to %1x%2").arg(ui.m_edWidth->text().toInt()).arg(ui.m_edHeight->text().toInt()));

        m_Camera.ReadPayloadSize(payloadSize);
    }

    m_Camera.ReadFrameSize(width, height);
    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));
}

void V4L2Viewer::OnPixelFormat()
{

}

void V4L2Viewer::OnGain()
{
    int32_t gain = 0;
    int32_t nVal = int64_2_int32(ui.m_edGain->text().toLongLong());

    m_Camera.SetGain(nVal);

    if (m_Camera.ReadGain(gain) != -2)
    {
        ui.m_edGain->setEnabled(true);
        ui.m_edGain->setText(QString("%1").arg(gain));
    }
    else
    {
        ui.m_edGain->setEnabled(false);
    }
}

void V4L2Viewer::OnAutoGain()
{
    bool autogain = false;

    m_Camera.SetAutoGain(ui.m_chkAutoGain->isChecked());

    if (m_Camera.ReadAutoGain(autogain) != -2)
    {
        ui.m_chkAutoGain->setEnabled(true);
        ui.m_chkAutoGain->setChecked(autogain);
    }
    else
    {
        ui.m_chkAutoGain->setEnabled(false);
    }
}

void V4L2Viewer::OnExposure()
{
    int32_t exposure = 0;
    int32_t exposureAbs = 0;
    bool autoexposure = false;

    int32_t nVal = int64_2_int32(ui.m_edExposure->text().toLongLong());

    m_Camera.SetExposure(nVal);

    if (m_Camera.ReadExposure(exposure) != -2)
    {
        ui.m_edExposure->setEnabled(true);
        ui.m_edExposure->setText(QString("%1").arg(exposure));
    }
    else
        ui.m_edExposure->setEnabled(false);
    if (m_Camera.ReadExposureAbs(exposureAbs) != -2)
    {
        ui.m_edExposureAbs->setEnabled(true);
        ui.m_edExposureAbs->setText(QString("%1").arg(exposureAbs));
    }
    else
        ui.m_edExposureAbs->setEnabled(false);

    if (m_Camera.ReadAutoExposure(autoexposure) != -2)
    {
        ui.m_chkAutoExposure->setEnabled(true);
        ui.m_chkAutoExposure->setChecked(autoexposure);
    }
    else
    {
        ui.m_chkAutoExposure->setEnabled(false);
    }
}

void V4L2Viewer::OnExposureAbs()
{
    int32_t exposure = 0;
    int32_t exposureAbs = 0;
    bool autoexposure = false;

    int32_t nVal = int64_2_int32(ui.m_edExposureAbs->text().toLongLong());

    m_Camera.SetExposureAbs(nVal);

    if (m_Camera.ReadExposureAbs(exposureAbs) != -2)
    {
        ui.m_edExposureAbs->setEnabled(true);
        ui.m_edExposureAbs->setText(QString("%1").arg(exposureAbs));
    }
    else
        ui.m_edExposureAbs->setEnabled(false);
    if (m_Camera.ReadExposure(exposure) != -2)
    {
        ui.m_edExposure->setEnabled(true);
        ui.m_edExposure->setText(QString("%1").arg(exposure));
    }
    else
        ui.m_edExposure->setEnabled(false);

    if (m_Camera.ReadAutoExposure(autoexposure) != -2)
    {
        ui.m_chkAutoExposure->setEnabled(true);
        ui.m_chkAutoExposure->setChecked(autoexposure);
    }
    else
    {
        ui.m_chkAutoExposure->setEnabled(false);
    }
}

void V4L2Viewer::OnAutoExposure()
{
    bool autoexposure = false;

    m_Camera.SetAutoExposure(ui.m_chkAutoExposure->isChecked());

    if (m_Camera.ReadAutoExposure(autoexposure) != -2)
    {
        ui.m_chkAutoExposure->setEnabled(true);
        ui.m_chkAutoExposure->setChecked(autoexposure);
    }
    else
    {
        ui.m_chkAutoExposure->setEnabled(false);
    }
}

void V4L2Viewer::OnPixelFormatChanged(const QString &item)
{
    std::string tmp = item.toStdString();
    char *s = (char*)tmp.c_str();
    uint32_t result = 0;

    if (tmp.size() == 4)
    {
        result += *s++;
        result += *s++ << 8;
        result += *s++ << 16;
        result += *s++ << 24;
    }

    if (m_Camera.SetPixelFormat(result, "") < 0)
    {
        OnLog(QString("Failed to save pixel format to %1").arg(item));
    }
    else
    {
        OnLog(QString("Pixel format changed to %1").arg(item));
    }
}

void V4L2Viewer::OnFrameSizesDBLClick(QListWidgetItem *item)
{
    QString tmp = item->text();
    QStringList list1 = tmp.split('x').first().split(':');
    QStringList list2 = tmp.split('x');

    ui.m_edWidth->setText(QString("%1").arg(list1.at(1)));
    ui.m_edHeight->setText(QString("%1").arg(list2.at(1)));
}

void V4L2Viewer::OnGamma()
{
    int32_t nVal = int64_2_int32(ui.m_edGamma->text().toLongLong());

    if (m_Camera.SetGamma(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE gamma!") );
        m_Camera.ReadGamma(tmp);
        ui.m_edGamma->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Gamma set to %1").arg(nVal));

        m_Camera.ReadGamma(tmp);
        ui.m_edGamma->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnBrightness()
{
    int32_t nVal = int64_2_int32(ui.m_edBlackLevel->text().toLongLong());

    if (m_Camera.SetBrightness(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE brightness!") );
        m_Camera.ReadBrightness(tmp);
        ui.m_edBlackLevel->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Brightness set to %1").arg(nVal));

        m_Camera.ReadBrightness(tmp);
        ui.m_edBlackLevel->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnContrast()
{
    int32_t nVal = int64_2_int32(ui.m_edContrast->text().toLongLong());

    if (m_Camera.SetContrast(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE contrast!") );
        m_Camera.ReadContrast(tmp);
        ui.m_edContrast->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Contrast set to %1").arg(nVal));

        m_Camera.ReadContrast(tmp);
        ui.m_edContrast->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnSaturation()
{
    int32_t nVal = int64_2_int32(ui.m_edSaturation->text().toLongLong());

    if (m_Camera.SetSaturation(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE saturation!") );
        m_Camera.ReadSaturation(tmp);
        ui.m_edSaturation->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Saturation set to %1").arg(nVal));

        m_Camera.ReadSaturation(tmp);
        ui.m_edSaturation->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnHue()
{
    int32_t nVal = int64_2_int32(ui.m_edHue->text().toLongLong());

    if (m_Camera.SetHue(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE hue!") );
        m_Camera.ReadHue(tmp);
        ui.m_edHue->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Hue set to %1").arg(nVal));

        m_Camera.ReadHue(tmp);
        ui.m_edHue->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnContinousWhiteBalance()
{
    m_Camera.SetContinousWhiteBalance(ui.m_chkContWhiteBalance->isChecked());
}

void V4L2Viewer::OnWhiteBalanceOnce()
{
    m_Camera.DoWhiteBalanceOnce();
}

void V4L2Viewer::OnRedBalance()
{
    int32_t nVal = int64_2_int32(ui.m_edRedBalance->text().toLongLong());

    if (m_Camera.SetRedBalance((uint32_t)nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE red balance!") );
        m_Camera.ReadRedBalance(tmp);
        ui.m_edRedBalance->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("RedBalance set to %1").arg(nVal));

        m_Camera.ReadRedBalance(tmp);
        ui.m_edRedBalance->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnBlueBalance()
{
    int32_t nVal = int64_2_int32(ui.m_edBlueBalance->text().toLongLong());

    if (m_Camera.SetBlueBalance((uint32_t)nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE blue balance!") );
        m_Camera.ReadBlueBalance(tmp);
        ui.m_edBlueBalance->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("BlueBalance set to %1").arg(nVal));

        m_Camera.ReadBlueBalance(tmp);
        ui.m_edBlueBalance->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnFrameRate()
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pixelFormat = 0;
    uint32_t bytesPerLine = 0;
    QString pixelFormatText;
    QString frameRate = ui.m_edFrameRate->text();
    QStringList frameRateList = frameRate.split('/');

    if (frameRateList.size() < 2)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("Missing parameter. Format: 1/100!") );
        return;
    }

    uint32_t numerator = frameRateList.at(0).toInt();
    uint32_t denominator = frameRateList.at(1).toInt();

    m_Camera.ReadFrameSize(width, height);
    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);

    if (m_Camera.SetFrameRate(numerator, denominator) < 0)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE frame rate!") );
        m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat);
        ui.m_edFrameRate->setText(QString("%1/%2").arg(numerator).arg(denominator));
    }
    else
    {
        OnLog(QString("Frame rate set to %1").arg(ui.m_edFrameRate->text() + QString(" (") + QString::number((double)numerator/(double)denominator, 'f', 3) + QString(")") ));
        m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat);
        ui.m_edFrameRate->setText(QString("%1/%2").arg(numerator).arg(denominator));
    }
}

void V4L2Viewer::OnCropXOffset()
{
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;
    uint32_t tmp;

    OnLog("Read org cropping values");
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        xOffset = ui.m_edCropXOffset->text().toInt();
        tmp = xOffset;
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            OnLog("Verifing new cropping values");
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
                if (tmp != xOffset)
                    OnLog("Error: value not set !!!");

                UpdateCameraFormat();
            }
        }
    }
}

void V4L2Viewer::OnCropYOffset()
{
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;
    uint32_t tmp;

    OnLog("Read org cropping values");
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        yOffset = ui.m_edCropYOffset->text().toInt();
        tmp = yOffset;
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            OnLog("Verifing new cropping values");
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
                if (tmp != yOffset)
                    OnLog("Error: value not set !!!");
            }
        }
    }
}

void V4L2Viewer::OnCropWidth()
{
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;
    uint32_t tmp;

    OnLog("Read org cropping values");
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        width = ui.m_edCropWidth->text().toInt();
        tmp = width;
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            OnLog("Verifing new cropping values");
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
                if (tmp != width)
                    OnLog("Error: value not set !!!");
            }
        }
    }

    OnReadAllValues();
}

void V4L2Viewer::OnCropHeight()
{
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;
    uint32_t tmp;

    OnLog("Read org cropping values");
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        height = ui.m_edCropHeight->text().toInt();
        tmp = height;
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            OnLog("Verifing new cropping values");
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
                if (tmp != height)
                    OnLog("Error: value not set !!!");
            }
        }
    }

    OnReadAllValues();
}

void V4L2Viewer::OnCropCapabilities()
{
    uint32_t boundsx;
    uint32_t boundsy;
    uint32_t boundsw;
    uint32_t boundsh;
    uint32_t defrectx;
    uint32_t defrecty;
    uint32_t defrectw;
    uint32_t defrecth;
    uint32_t aspectnum;
    uint32_t aspectdenum;

    OnLog("Read cropping capabilities");
    if (m_Camera.ReadCropCapabilities(boundsx, boundsy, boundsw, boundsh,
                                      defrectx, defrecty, defrectw, defrecth,
                                      aspectnum, aspectdenum) == 0)
    {
//        ui.m_edBoundsX->setEnabled(true);
//        ui.m_edBoundsX->setText(QString("%1").arg(boundsx));
//        ui.m_edBoundsY->setEnabled(true);
//        ui.m_edBoundsY->setText(QString("%1").arg(boundsy));

//        ui.m_edBoundsW->setEnabled(true);
//        ui.m_edBoundsW->setText(QString("%1").arg(boundsw));
//        ui.m_edBoundsH->setEnabled(true);
//        ui.m_edBoundsH->setText(QString("%1").arg(boundsh));

//        ui.m_edDefrectX->setEnabled(true);
//        ui.m_edDefrectX->setText(QString("%1").arg(defrectx));
//        ui.m_edDefrectY->setEnabled(true);
//        ui.m_edDefrectY->setText(QString("%1").arg(defrecty));
//        ui.m_edDefrectW->setEnabled(true);
//        ui.m_edDefrectW->setText(QString("%1").arg(defrectw));
//        ui.m_edDefrectH->setEnabled(true);
//        ui.m_edDefrectH->setText(QString("%1").arg(defrecth));
//        ui.m_edAspect->setEnabled(true);
//        ui.m_edAspect->setText(QString("%1/%2").arg(aspectnum).arg(aspectdenum));
    }
    else
    {
//        ui.m_edBoundsX->setEnabled(false);
//        ui.m_edBoundsY->setEnabled(false);
//        ui.m_edBoundsW->setEnabled(false);
//        ui.m_edBoundsH->setEnabled(false);
//        ui.m_edDefrectX->setEnabled(false);
//        ui.m_edDefrectY->setEnabled(false);
//        ui.m_edDefrectW->setEnabled(false);
//        ui.m_edDefrectH->setEnabled(false);
//        ui.m_edAspect->setEnabled(false);
    }
}

void V4L2Viewer::OnReadAllValues()
{
    GetImageInformation();
    UpdateCameraFormat();
}

/////////////////////// Tools /////////////////////////////////////

void V4L2Viewer::GetImageInformation()
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t xOffset = 0;
    uint32_t yOffset = 0;
    int32_t gain = 0;
    bool autogain = false;
    int32_t exposure = 0;
    int32_t exposureAbs = 0;
    bool autoexposure = false;
    int32_t nSVal;
    uint32_t numerator = 0;
    uint32_t denominator = 0;
    uint32_t pixelFormat = 0;
    QString pixelFormatText;
    uint32_t bytesPerLine = 0;


    ui.m_chkAutoGain->setChecked(false);
    ui.m_chkAutoExposure->setChecked(false);

    UpdateCameraFormat();

    ui.m_chkContWhiteBalance->setEnabled(m_Camera.IsAutoWhiteBalanceSupported());

    if (m_Camera.ReadGain(gain) != -2)
    {
        ui.m_edGain->setEnabled(true);
        ui.m_labelGain->setEnabled(true);
        ui.m_edGain->setText(QString("%1").arg(gain));
    }
    else
    {
        ui.m_edGain->setEnabled(false);
        ui.m_labelGain->setEnabled(false);
    }

    if (m_Camera.ReadAutoGain(autogain) != -2)
    {
        ui.m_chkAutoGain->setEnabled(true);
        ui.m_chkAutoGain->setChecked(autogain);
    }
    else
    {
        ui.m_chkAutoGain->setEnabled(false);
    }

    if (m_Camera.ReadExposure(exposure) != -2)
    {
        ui.m_edExposure->setEnabled(true);
        ui.m_labelExposure->setEnabled(true);
        ui.m_edExposure->setText(QString("%1").arg(exposure));
    }
    else
    {
        ui.m_edExposure->setEnabled(false);
        ui.m_labelExposure->setEnabled(false);
    }

    if (m_Camera.ReadExposureAbs(exposureAbs) != -2)
    {
        ui.m_edExposureAbs->setEnabled(true);
        ui.m_labelExposureAbs->setEnabled(true);
        ui.m_edExposureAbs->setText(QString("%1").arg(exposureAbs));
    }
    else
    {
        ui.m_edExposureAbs->setEnabled(false);
        ui.m_labelExposureAbs->setEnabled(false);
    }

    if (m_Camera.ReadAutoExposure(autoexposure) != -2)
    {
        ui.m_chkAutoExposure->setEnabled(true);
        ui.m_chkAutoExposure->setChecked(autoexposure);
    }
    else
    {
        ui.m_chkAutoExposure->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadGamma(nSVal) != -2)
    {
        ui.m_edGamma->setEnabled(true);
        ui.m_labelGamma->setEnabled(true);
        ui.m_edGamma->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edGamma->setEnabled(false);
        ui.m_labelGamma->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadBrightness(nSVal) != -2)
    {
        ui.m_edBlackLevel->setEnabled(true);
        ui.m_labelBlackLevel->setEnabled(true);
        ui.m_edBlackLevel->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edBlackLevel->setEnabled(false);
        ui.m_labelBlackLevel->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadContrast(nSVal) != -2)
    {
        ui.m_edContrast->setEnabled(true);
        ui.m_labelContrast->setEnabled(true);
        ui.m_edContrast->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edContrast->setEnabled(false);
        ui.m_labelContrast->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadSaturation(nSVal) != -2)
    {
        ui.m_edSaturation->setEnabled(true);
        ui.m_labelSaturation->setEnabled(true);
        ui.m_edSaturation->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edSaturation->setEnabled(false);
        ui.m_labelSaturation->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadHue(nSVal) != -2)
    {
        ui.m_edHue->setEnabled(true);
        ui.m_labelHue->setEnabled(true);
        ui.m_edHue->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edHue->setEnabled(false);
        ui.m_labelHue->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadRedBalance(nSVal) != -2)
    {
        ui.m_edRedBalance->setEnabled(true);
        ui.m_labelRedBalance->setEnabled(true);
        ui.m_edRedBalance->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edRedBalance->setEnabled(false);
        ui.m_labelRedBalance->setEnabled(false);
    }

    nSVal = 0;
    if (m_Camera.ReadBlueBalance(nSVal) != -2)
    {
        ui.m_edBlueBalance->setEnabled(true);
        ui.m_labelBlueBalance->setEnabled(true);
        ui.m_edBlueBalance->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edBlueBalance->setEnabled(false);
        ui.m_labelBlueBalance->setEnabled(false);
    }

    m_Camera.ReadFrameSize(width, height);
    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    if (m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat) != -2)
    {
        ui.m_edFrameRate->setEnabled(true);
        ui.m_labelFrameRate->setEnabled(true);
        ui.m_edFrameRate->setText(QString("%1/%2").arg(numerator).arg(denominator));
    }
    else
    {
        ui.m_edFrameRate->setEnabled(false);
        ui.m_labelFrameRate->setEnabled(false);
    }

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
    {
        ui.m_edCropXOffset->setEnabled(true);
        ui.m_labelCropXOffset->setEnabled(true);
        ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
        ui.m_edCropYOffset->setEnabled(true);
        ui.m_labelCropYOffset->setEnabled(true);
        ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
        ui.m_edCropWidth->setEnabled(true);
        ui.m_labelCropWidth->setEnabled(true);
        ui.m_edCropWidth->setText(QString("%1").arg(width));
        ui.m_edCropHeight->setEnabled(true);
        ui.m_labelCropHeight->setEnabled(true);
        ui.m_edCropHeight->setText(QString("%1").arg(height));
    }
    else
    {
        ui.m_edCropXOffset->setEnabled(false);
        ui.m_edCropYOffset->setEnabled(false);
        ui.m_edCropWidth->setEnabled(false);
        ui.m_edCropHeight->setEnabled(false);
        ui.m_labelCropXOffset->setEnabled(false);
        ui.m_labelCropYOffset->setEnabled(false);
        ui.m_labelCropWidth->setEnabled(false);
        ui.m_labelCropHeight->setEnabled(false);
    }
    OnCropCapabilities();
    m_Camera.EnumAllControlNewStyle();
}

void V4L2Viewer::UpdateCameraFormat()
{
    int result = -1;
    uint32_t payloadSize = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pixelFormat = 0;
    QString pixelFormatText;
    uint32_t bytesPerLine = 0;

    ui.m_pixelFormats->clear();
    result = m_Camera.ReadPayloadSize(payloadSize);

    result = m_Camera.ReadFrameSize(width, height);
    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));

    result = m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    result = m_Camera.ReadFormats();
}

QString V4L2Viewer::GetDeviceInfo()
{
    std::string tmp;

    QString firmware = QString("Camera FW Version = %1").arg(QString::fromStdString(m_Camera.getAvtDeviceFirmwareVersion()));
    QString devTemp = QString("Camera Device Temperature = %1C").arg(QString::fromStdString(m_Camera.getAvtDeviceTemperature()));
    QString devSerial = QString("Camera Serial Number = %1").arg(QString::fromStdString(m_Camera.getAvtDeviceSerialNumber()));

    m_Camera.GetCameraDriverName(tmp);
    QString driverName = QString("Driver name = %1").arg(tmp.c_str());
    m_Camera.GetCameraBusInfo(tmp);
    QString busInfo = QString("Bus info = %1").arg(tmp.c_str());
    m_Camera.GetCameraDriverVersion(tmp);
    QString driverVer = QString("Driver version = %1").arg(tmp.c_str());
    m_Camera.GetCameraCapabilities(tmp);
    QString capabilities = QString("Capabilities = %1").arg(tmp.c_str());
    return QString(firmware + "<br>" + devTemp + "<br>" + devSerial + "<br>" + driverName + "<br>" + busInfo + "<br>" + driverVer + "<br>" + capabilities);

}

// Check if IO Read was checked and remove it when not capable
void V4L2Viewer::Check4IOReadAbility()
{
    int nRow = ui.m_CamerasListBox->currentRow();

    if (-1 != nRow)
    {
        QString devName = dynamic_cast<CameraListCustomItem*>(ui.m_CamerasListBox->itemWidget(ui.m_CamerasListBox->item(nRow)))->GetCameraName();
        std::string deviceName;

        devName = devName.right(devName.length() - devName.indexOf(':') - 2);
        deviceName = devName.left(devName.indexOf('(') - 1).toStdString();
        bool ioRead;

        m_Camera.OpenDevice(deviceName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT);
        m_Camera.GetCameraReadCapability(ioRead);
        m_Camera.CloseDevice();

        if (!ioRead)
        {
            ui.m_TitleUseRead->setEnabled( false );
        }
        else
        {
            ui.m_TitleUseRead->setEnabled( true );
        }
        if (!ioRead && ui.m_TitleUseRead->isChecked())
        {
            QMessageBox::warning( this, tr("V4L2 Test"), tr("IO Read not available with this camera. V4L2_CAP_VIDEO_CAPTURE not set. Switched to IO MMAP."));

            ui.m_TitleUseRead->setChecked( false );
            ui.m_TitleUseMMAP->setEnabled( true );
            ui.m_TitleUseMMAP->setChecked( true );
            m_MMAP_BUFFER = IO_METHOD_MMAP;
        }
    }
}

void V4L2Viewer::SetTitleText(QString additionalText)
{
    if (VIEWER_MASTER == m_nViewerNumber)
        setWindowTitle(QString("%1 V%2.%3.%4 - master view  %5").arg(APP_NAME).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH).arg(additionalText));
    else
        setWindowTitle(QString("%1 V%2.%3.%4 - %5. viewer %6").arg(APP_NAME).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH).arg(m_nViewerNumber).arg(additionalText));
}

