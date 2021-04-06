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

#include <QtCore>
#include <QtGlobal>
#include <QStringList>

#include <ctime>
#include <limits>
#include <sstream>

#define NUM_COLORS 3
#define BIT_DEPTH 8

#define MAX_ZOOM_IN  (16.0)
#define MAX_ZOOM_OUT (1/8.0)
#define ZOOM_INCREMENT (2.0)

#define MANUF_NAME_AV       "Allied Vision"
#define APP_NAME            "Video4Linux2 Testtool"
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
    , m_bFitToScreen(false)
    , m_dScaleFactor(1.0)
    , m_DirectAccessData(0)
    , m_SaveFileDialog(0)
    , m_BLOCKING_MODE(true)
    , m_MMAP_BUFFER(IO_METHOD_MMAP) // use mmap by default
    , m_VIDIOC_TRY_FMT(true) // use VIDIOC_TRY_FMT by default
    , m_ExtendedControls(false)
    , m_ShowFrames(true)
    , m_nDroppedFrames(0)
    , m_nStreamNumber(0)
    , m_ReferenceImageDialog(0)
    , m_SaveFileDir(QDir::currentPath())
{
    srand((unsigned)time(0));

    Logger::InitializeLogger("V4L2ViewerLog.log");

    ui.setupUi(this);

    // connect the menu actions
    connect(ui.m_MenuClose, SIGNAL(triggered()), this, SLOT(OnMenuCloseTriggered()));
    if (VIEWER_MASTER == m_nViewerNumber)
        connect(ui.m_MenuOpenNextViewer, SIGNAL(triggered()), this, SLOT(OnMenuOpenNextViewer()));
    else
        ui.m_MenuOpenNextViewer->setEnabled(false);

    // set event font
    ui.m_EventsLogEditWidget->setFont(QFont("Courier", 10));
    ui.m_EventsLogEditWidget->setReadOnly(true);
    ui.m_EventsLogEditWidget->clear();

    // Connect GUI events with event handlers
    connect(ui.m_OpenCloseButton,             SIGNAL(clicked()),         this, SLOT(OnOpenCloseButtonClicked()));
    connect(ui.m_GetDeviceInfoButton,         SIGNAL(clicked()),         this, SLOT(OnGetDeviceInfoButtonClicked()));
    connect(ui.m_GetStreamStatisticsButton,   SIGNAL(clicked()),         this, SLOT(OnGetStreamStatisticsButtonClicked()));
    connect(ui.m_StartButton,                 SIGNAL(clicked()),         this, SLOT(OnStartButtonClicked()));
    connect(ui.m_ToggleButton,                SIGNAL(clicked()),         this, SLOT(OnToggleButtonClicked()));
    connect(ui.m_StopButton,                  SIGNAL(clicked()),         this, SLOT(OnStopButtonClicked()));
    connect(ui.m_ZoomFitButton,               SIGNAL(clicked()),         this, SLOT(OnZoomFitButtonClicked()));
    connect(ui.m_ZoomInButton,                SIGNAL(clicked()),         this, SLOT(OnZoomInButtonClicked()));
    connect(ui.m_ZoomOutButton,               SIGNAL(clicked()),         this, SLOT(OnZoomOutButtonClicked()));

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
    connect(&m_Camera, SIGNAL(OnCameraRecordFrame_Signal(const QSharedPointer<MyFrame>&)), this, SLOT(OnCameraRecordFrame(const QSharedPointer<MyFrame>&)));
    connect(&m_Camera, SIGNAL(OnCameraPixelFormat_Signal(const QString &)), this, SLOT(OnCameraPixelFormat(const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameSize_Signal(const QString &)), this, SLOT(OnCameraFrameSize(const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraLiveDeviationCalc_Signal(int)), this, SLOT(OnCalcLiveDeviationFromFrameObserver(int)));

    // Setup blocking mode radio buttons
    m_BlockingModeRadioButtonGroup = new QButtonGroup();
    m_BlockingModeRadioButtonGroup->setExclusive(true);
    m_BlockingModeRadioButtonGroup->addButton(ui.m_radioBlocking);
    m_BlockingModeRadioButtonGroup->addButton(ui.m_radioNonBlocking);
    if (m_BLOCKING_MODE)
    {
        ui.m_radioBlocking->setChecked(true);
    }
    else
    {
        ui.m_radioNonBlocking->setChecked(true);
    }

    connect(m_BlockingModeRadioButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnBlockingMode(QAbstractButton*)));
    connect(ui.m_chkUseRead, SIGNAL(clicked()), this, SLOT(OnUseRead()));
    connect(ui.m_TitleUseRead, SIGNAL(triggered()), this, SLOT(OnUseRead()));
    connect(ui.m_chkUseMMAP, SIGNAL(clicked()), this, SLOT(OnUseMMAP()));
    connect(ui.m_TitleUseMMAP, SIGNAL(triggered()), this, SLOT(OnUseMMAP()));
    connect(ui.m_chkUseUSERPTR, SIGNAL(clicked()), this, SLOT(OnUseUSERPTR()));
    connect(ui.m_TitleUseUSERPTR, SIGNAL(triggered()), this, SLOT(OnUseUSERPTR()));
    connect(ui.m_TitleEnable_VIDIOC_TRY_FMT, SIGNAL(triggered()), this, SLOT(OnUseVIDIOC_TRY_FMT()));
    connect(ui.m_TitleEnableExtendedControls, SIGNAL(triggered()), this, SLOT(OnUseExtendedControls()));
    connect(ui.m_TitleShowFrames, SIGNAL(triggered()), this, SLOT(OnShowFrames()));
    connect(ui.m_TitleClearOutputListbox, SIGNAL(triggered()), this, SLOT(OnClearOutputListbox()));
    connect(ui.m_TitleLogtofile, SIGNAL(triggered()), this, SLOT(OnLogToFile()));

    int err = m_Camera.DeviceDiscoveryStart();

    connect(ui.m_CamerasListBox, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnListBoxCamerasItemDoubleClicked(QListWidgetItem *)));

    // Connect the handler to show the frames per second
    connect(&m_FramesReceivedTimer, SIGNAL(timeout()), this, SLOT(OnUpdateFramesReceived()));

    // Connect the handler to toggle the stream randomly
    connect(&m_StreamToggleTimer, SIGNAL(timeout()), this, SLOT(OnStreamToggleTimeout()));

    // Connect the handler to clear the event list
    connect(ui.m_ClearEventLogButton, SIGNAL(clicked()), this, SLOT(OnClearEventLogButtonClicked()));

    // Connect the buttons for the direct register access
    connect(ui.m_DirectRegisterAccessReadButton, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessReadButtonClicked()));
    connect(ui.m_DirectRegisterAccessWriteButton, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessWriteButtonClicked()));

    // connect the buttons for the Recording
    connect(ui.m_StartRecordButton, SIGNAL(clicked()), this, SLOT(OnStartRecording()));
    connect(ui.m_StopRecordButton, SIGNAL(clicked()), this, SLOT(OnStopRecording()));
    connect(ui.m_DeleteRecording, SIGNAL(clicked()), this, SLOT(OnDeleteRecording()));
    connect(ui.m_DisplaySaveFrame, SIGNAL(clicked()), this, SLOT(OnSaveFrame()));
    connect(ui.m_ExportRecordedFrameButton, SIGNAL(clicked()), this, SLOT(OnExportFrame()));
    connect(ui.m_SaveFrameSeriesButton, SIGNAL(clicked()), this, SLOT(OnSaveFrameSeries()));
    connect(ui.m_CalcDeviationButton, SIGNAL(clicked()), this, SLOT(OnCalcDeviation()));
    connect(ui.m_CalcLiveDeviationButton, SIGNAL(clicked()), this, SLOT(OnCalcLiveDeviation()));
    connect(ui.m_GetReferenceButton, SIGNAL(clicked()), this, SLOT(OnGetReferenceImage()));
    connect(ui.m_FrameRecordTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(OnRecordTableSelectionChanged(const QItemSelection &, const QItemSelection &)));


    // disable table editing from user
    ui.m_FrameRecordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.m_FrameRecordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.m_FrameRecordTable->setSelectionMode(QAbstractItemView::SingleSelection);

    // register meta type for QT signal/slot mechanism
    qRegisterMetaType<QSharedPointer<MyFrame> >("QSharedPointer<MyFrame>");

    // make calc live deviation button checakbel
    ui.m_CalcLiveDeviationButton->setCheckable(true);
    ui.m_meanDeviationLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // connect the buttons for Image m_ControlRequestTimer
    connect(ui.m_edWidth, SIGNAL(returnPressed()), this, SLOT(OnWidth()));
    connect(ui.m_edHeight, SIGNAL(returnPressed()), this, SLOT(OnHeight()));
    connect(ui.m_edPixelFormat, SIGNAL(returnPressed()), this, SLOT(OnPixelFormat()));
    connect(ui.m_edGain, SIGNAL(returnPressed()), this, SLOT(OnGain()));
    connect(ui.m_chkAutoGain, SIGNAL(clicked()), this, SLOT(OnAutoGain()));
    connect(ui.m_edExposure, SIGNAL(returnPressed()), this, SLOT(OnExposure()));
    connect(ui.m_edExposureAbs, SIGNAL(returnPressed()), this, SLOT(OnExposureAbs()));
    connect(ui.m_chkAutoExposure, SIGNAL(clicked()), this, SLOT(OnAutoExposure()));
    connect(ui.m_liPixelFormats, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnPixelFormatDBLClick(QListWidgetItem *)));
    connect(ui.m_liFrameSizes, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnFrameSizesDBLClick(QListWidgetItem *)));
    connect(ui.m_edGamma, SIGNAL(returnPressed()), this, SLOT(OnGamma()));
    connect(ui.m_edReverseX, SIGNAL(returnPressed()), this, SLOT(OnReverseX()));
    connect(ui.m_edReverseY, SIGNAL(returnPressed()), this, SLOT(OnReverseY()));
    connect(ui.m_edSharpness, SIGNAL(returnPressed()), this, SLOT(OnSharpness()));
    connect(ui.m_edBrightness, SIGNAL(returnPressed()), this, SLOT(OnBrightness()));
    connect(ui.m_edContrast, SIGNAL(returnPressed()), this, SLOT(OnContrast()));
    connect(ui.m_edSaturation, SIGNAL(returnPressed()), this, SLOT(OnSaturation()));
    connect(ui.m_edHue, SIGNAL(returnPressed()), this, SLOT(OnHue()));
    connect(ui.m_chkContWhiteBalance, SIGNAL(clicked()), this, SLOT(OnContinousWhiteBalance()));
    connect(ui.m_butWhiteBalanceOnce, SIGNAL(clicked()), this, SLOT(OnWhiteBalanceOnce()));
    connect(ui.m_edRedBalance, SIGNAL(returnPressed()), this, SLOT(OnRedBalance()));
    connect(ui.m_edBlueBalance, SIGNAL(returnPressed()), this, SLOT(OnBlueBalance()));
    connect(ui.m_edFrameRate, SIGNAL(returnPressed()), this, SLOT(OnFrameRate()));
    connect(ui.m_edCropXOffset, SIGNAL(returnPressed()), this, SLOT(OnCropXOffset()));
    connect(ui.m_edCropYOffset, SIGNAL(returnPressed()), this, SLOT(OnCropYOffset()));
    connect(ui.m_edCropWidth, SIGNAL(returnPressed()), this, SLOT(OnCropWidth()));
    connect(ui.m_edCropHeight, SIGNAL(returnPressed()), this, SLOT(OnCropHeight()));
    connect(ui.m_butCropCapabilities, SIGNAL(clicked()), this, SLOT(OnCropCapabilities()));
    connect(ui.m_butReadValues, SIGNAL(clicked()), this, SLOT(OnReadAllValues()));

    // Set the splitter stretch factors
    ui.m_Splitter1->setStretchFactor(0, 25);
    ui.m_Splitter1->setStretchFactor(1, 50);
    ui.m_Splitter1->setStretchFactor(2, 25);
    ui.m_Splitter3->setStretchFactor(0, 75);
    ui.m_Splitter3->setStretchFactor(1, 25);

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

    ///////////////////// Dump range /////////////////////
    // add the number of used frames option to the menu
    m_LogFrameRangeLineEdit = new QLineEdit(this);
    m_LogFrameRangeLineEdit->setText("");

    // prepare the layout
    QHBoxLayout *layoutDump = new QHBoxLayout;
    QLabel *labelDump = new QLabel("Dump frames to log file (range 1 or 1-3):");
    layoutDump->addWidget(labelDump);
    layoutDump->addWidget(m_LogFrameRangeLineEdit);

    // put the layout into a widget
    QWidget *widgetDump = new QWidget(this);
    widgetDump->setLayout(layoutDump);

    ///////////////////// Dump byte range /////////////////////
    // add the number of used frames option to the menu
    m_DumpByteFrameRangeLineEdit = new QLineEdit(this);
    m_DumpByteFrameRangeLineEdit->setText("");

    // prepare the layout
    QHBoxLayout *layoutByteDump = new QHBoxLayout;
    QLabel *labelByteDump = new QLabel("Dump frames to binary file (range 1 or 1-3):");
    layoutByteDump->addWidget(labelByteDump);
    layoutByteDump->addWidget(m_DumpByteFrameRangeLineEdit);

    // put the layout into a widget
    QWidget *widgetByteDump = new QWidget(this);
    widgetByteDump->setLayout(layoutByteDump);

    ///////////////////// CSV File /////////////////////
    // add the number of used frames option to the menu
    m_CSVFileLineEdit = new QLineEdit(this);
    m_CSVFileLineEdit->setText("");

    // prepare the layout
    QHBoxLayout *layoutCSVFile = new QHBoxLayout;
    QLabel *labelCSVFile = new QLabel("CSV File:");
    layoutCSVFile->addWidget(labelCSVFile);
    layoutCSVFile->addWidget(m_CSVFileLineEdit);

    // put the layout into a widget
    QWidget *widgetCSVFile = new QWidget(this);
    widgetCSVFile->setLayout(layoutCSVFile);

    ///////////////////// Toggle stream Delay Random /////////////////////
    // add the number of used frames option to the menu
    m_ToggleStreamDelayRandLineEdit = new QLineEdit(this);
    m_ToggleStreamDelayRandLineEdit->setText("500");
    m_ToggleStreamDelayRandLineEdit->setValidator(new QIntValidator(1, 10000, this));

    m_ToggleStreamDelayRandCheckBox = new QCheckBox(this);
    m_ToggleStreamDelayRandCheckBox->setChecked(true);
    connect(m_ToggleStreamDelayRandCheckBox, SIGNAL(clicked()), this, SLOT(OnToggleStreamDelayRand()));

    // prepare the layout
    QHBoxLayout *layoutToggleStreamDelayRand = new QHBoxLayout;
    QLabel *labelToggleStreamDelayRand = new QLabel("Toggle Stream Delay Random/ms 1-");
    layoutToggleStreamDelayRand->addWidget(m_ToggleStreamDelayRandCheckBox);
    layoutToggleStreamDelayRand->addWidget(labelToggleStreamDelayRand);
    layoutToggleStreamDelayRand->addWidget(m_ToggleStreamDelayRandLineEdit);

    // put the layout into a widget
    QWidget *widgetToggleStreamDelayRand = new QWidget(this);
    widgetToggleStreamDelayRand->setLayout(layoutToggleStreamDelayRand);

    ///////////////////// Toggle stream Delay Random /////////////////////
    // add the number of used frames option to the menu
    m_ToggleStreamDelayLineEdit = new QLineEdit(this);
    m_ToggleStreamDelayLineEdit->setText("1000");
    m_ToggleStreamDelayLineEdit->setValidator(new QIntValidator(1, 10000, this));

    m_ToggleStreamDelayCheckBox = new QCheckBox(this);
    m_ToggleStreamDelayCheckBox->setChecked(false);
    connect(m_ToggleStreamDelayCheckBox, SIGNAL(clicked()), this, SLOT(OnToggleStreamDelay()));

    // prepare the layout
    QHBoxLayout *layoutToggleStreamDelay = new QHBoxLayout;
    QLabel *labelToggleStreamDelay = new QLabel("Toggle Stream Delay/ms ");
    layoutToggleStreamDelay->addWidget(m_ToggleStreamDelayCheckBox);
    layoutToggleStreamDelay->addWidget(labelToggleStreamDelay);
    layoutToggleStreamDelay->addWidget(m_ToggleStreamDelayLineEdit);

    // put the layout into a widget
    QWidget *widgetToggleStreamDelay = new QWidget(this);
    widgetToggleStreamDelay->setLayout(layoutToggleStreamDelay);

    // add the widget into the menu bar
    m_NumberOfUsedFramesWidgetAction = new QWidgetAction(this);
    m_NumberOfUsedFramesWidgetAction->setDefaultWidget(widgetNum);
    ui.m_MenuOptions->addAction(m_NumberOfUsedFramesWidgetAction);

    // add the widget into the menu bar
    m_LogFrameRangeWidgetAction = new QWidgetAction(this);
    m_LogFrameRangeWidgetAction->setDefaultWidget(widgetDump);
    ui.m_MenuOptions->addAction(m_LogFrameRangeWidgetAction);

    // add the widget into the menu bar
    m_DumpByteFrameRangeWidgetAction = new QWidgetAction(this);
    m_DumpByteFrameRangeWidgetAction->setDefaultWidget(widgetByteDump);
    ui.m_MenuOptions->addAction(m_DumpByteFrameRangeWidgetAction);

    // add the widget into the menu bar
    m_CSVFileWidgetAction = new QWidgetAction(this);
    m_CSVFileWidgetAction->setDefaultWidget(widgetCSVFile);
    ui.m_MenuOptions->addAction(m_CSVFileWidgetAction);

    // add the widget into the menu bar
    m_ToggleStreamDelayRandWidgetAction = new QWidgetAction(this);
    m_ToggleStreamDelayRandWidgetAction->setDefaultWidget(widgetToggleStreamDelayRand);
    ui.m_MenuTest->addAction(m_ToggleStreamDelayRandWidgetAction);

    // add the widget into the menu bar
    m_ToggleStreamDelayWidgetAction = new QWidgetAction(this);
    m_ToggleStreamDelayWidgetAction->setDefaultWidget(widgetToggleStreamDelay);
    ui.m_MenuTest->addAction(m_ToggleStreamDelayWidgetAction);

    ui.menuBar->setNativeMenuBar(false);

    ui.m_FeatureTabWidget->setCurrentIndex(0);

    QMainWindow::showMaximized();

    UpdateViewerLayout();
    UpdateZoomButtons();

    // set check boxes state for mmap according to variable m_MMAP_BUFFER
    if (IO_METHOD_READ == m_MMAP_BUFFER)
    {
        ui.m_chkUseRead->setChecked(true);
        ui.m_TitleUseRead->setChecked(true);
    }
    else
    {
        ui.m_chkUseRead->setChecked(false);
        ui.m_TitleUseRead->setChecked(false);
    }

    if (IO_METHOD_USERPTR == m_MMAP_BUFFER)
    {
        ui.m_chkUseUSERPTR->setChecked(true);
        ui.m_TitleUseUSERPTR->setChecked(true);
    }
    else
    {
        ui.m_chkUseUSERPTR->setChecked(false);
        ui.m_TitleUseUSERPTR->setChecked(false);
    }

    if (IO_METHOD_MMAP == m_MMAP_BUFFER)
    {
        ui.m_chkUseMMAP->setChecked(true);
        ui.m_TitleUseMMAP->setChecked(true);
    }
    else
    {
        ui.m_chkUseMMAP->setChecked(false);
        ui.m_TitleUseMMAP->setChecked(false);
    }

    ui.m_TitleEnable_VIDIOC_TRY_FMT->setChecked((m_VIDIOC_TRY_FMT));
    ui.m_TitleEnableExtendedControls->setChecked((m_ExtendedControls));

    // setup table widgets for record listing
    InitializeTableWidget();
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
    m_BLOCKING_MODE = ui.m_radioBlocking->isChecked();
    OnLog(QString("Use BLOCKING_MODE = %1").arg((m_BLOCKING_MODE) ? "TRUE" : "FALSE"));
}

void V4L2Viewer::OnUseRead()
{
    m_MMAP_BUFFER = IO_METHOD_READ;
    OnLog(QString("Use IO Read"));

    ui.m_chkUseUSERPTR->setChecked(false);
    ui.m_chkUseRead->setChecked(true);
    ui.m_chkUseMMAP->setChecked(false);
    ui.m_TitleUseMMAP->setChecked(false);
    ui.m_TitleUseUSERPTR->setChecked(false);
    ui.m_TitleUseRead->setChecked(true);
}

void V4L2Viewer::OnUseMMAP()
{
    m_MMAP_BUFFER = IO_METHOD_MMAP;
    OnLog(QString("Use IO MMAP"));

    ui.m_chkUseUSERPTR->setChecked(false);
    ui.m_chkUseRead->setChecked(false);
    ui.m_chkUseMMAP->setChecked(true);
    ui.m_TitleUseMMAP->setChecked(true);
    ui.m_TitleUseUSERPTR->setChecked(false);
    ui.m_TitleUseRead->setChecked(false);
}

void V4L2Viewer::OnUseUSERPTR()
{
    m_MMAP_BUFFER = IO_METHOD_USERPTR;
    OnLog(QString("Use IO USERPTR"));

    ui.m_chkUseUSERPTR->setChecked(true);
    ui.m_chkUseRead->setChecked(false);
    ui.m_chkUseMMAP->setChecked(false);
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

void V4L2Viewer::OnUseExtendedControls()
{
    m_ExtendedControls = !m_ExtendedControls;
    OnLog(QString("Use Extended Controls = %1").arg((m_ExtendedControls) ? "TRUE" : "FALSE"));

    ui.m_TitleEnableExtendedControls->setChecked(m_ExtendedControls);
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

void V4L2Viewer::mousePressEvent(QMouseEvent *event)
{
    // Detect if the click is in the view.
    QPoint imageViewPoint = ui.m_ImageView->mapFrom( this, event->pos() );
    if (ui.m_ImageView->rect().contains(imageViewPoint))
    {
        QTransform transformation = ui.m_ImageView->transform();

        if (m_dScaleFactor >= 1)
        {
            QImage image = m_PixmapItem->pixmap().toImage();
            int offsetX = 0;
            int offsetY = 0;

            if (ui.m_ImageView->horizontalScrollBar()->width() > image.width())
                offsetX = (ui.m_ImageView->horizontalScrollBar()->width() - image.width()) / 2;
            if (ui.m_ImageView->verticalScrollBar()->height() > image.height())
                offsetY = (ui.m_ImageView->verticalScrollBar()->width() - image.height()) / 2;

            QPoint scalePoint = QPoint((imageViewPoint.x() - offsetX + ui.m_ImageView->horizontalScrollBar()->value()) / m_dScaleFactor,
                                       (imageViewPoint.y() - offsetY + ui.m_ImageView->verticalScrollBar()->value()) / m_dScaleFactor);
            QColor myPixel = image.pixel(scalePoint);

            QToolTip::showText(ui.m_ImageView->mapToGlobal(imageViewPoint), QString("x:%1, y:%2, r:%3/g:%4/b:%5").arg(scalePoint.x()).arg(scalePoint.y()).arg(myPixel.red()).arg(myPixel.green()).arg(myPixel.blue()));
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
        QString devName = ui.m_CamerasListBox->item(nRow)->text();
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
                OnLog("Camera closed successfully");

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
    ui.m_radioBlocking->setEnabled( !m_bIsOpen );
    ui.m_radioNonBlocking->setEnabled( !m_bIsOpen );
    ui.m_chkUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_chkUseRead->setEnabled( !m_bIsOpen );
    ui.m_chkUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_TitleUseRead->setEnabled( !m_bIsOpen );
    ui.m_TitleEnable_VIDIOC_TRY_FMT->setEnabled( !m_bIsOpen );
    ui.m_TitleEnableExtendedControls->setEnabled( !m_bIsOpen );

    // enable/disable record buttons accordingly
    if ( m_bIsOpen )
    {
        ui.m_StartRecordButton->setEnabled(true);
        if(m_ReferenceImage)
        {
            ui.m_CalcLiveDeviationButton->setEnabled(true);
        }
    }
    else
    {
        ui.m_CalcLiveDeviationButton->setChecked(false);
        OnCalcLiveDeviation();
        OnStopRecording();
        ui.m_StartRecordButton->setEnabled(false);
        ui.m_CalcLiveDeviationButton->setEnabled(false);
    }
}

// The event handler for get device info
void V4L2Viewer::OnGetDeviceInfoButtonClicked()
{
    int nRow = ui.m_CamerasListBox->currentRow();

    if (-1 != nRow)
    {
        QString devName = ui.m_CamerasListBox->item(nRow)->text();
        std::string deviceName;
        std::string tmp;

        devName = devName.right(devName.length() - devName.indexOf(':') - 2);
        deviceName = devName.left(devName.indexOf('(') - 1).toStdString();

        if ( !m_bIsOpen )
            m_Camera.OpenDevice(deviceName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT, m_ExtendedControls);

        OnLog("---------------------------------------------");
        OnLog("---- Device Info ");
        OnLog("---------------------------------------------");

        OnLog(QString("Camera FW Version = %1").arg(QString::fromStdString(m_Camera.getAvtDeviceFirmwareVersion())));
        OnLog(QString("Camera Device Temperature = %1C").arg(QString::fromStdString(m_Camera.getAvtDeviceTemperature())));
        OnLog(QString("Camera Serial Number = %1").arg(QString::fromStdString(m_Camera.getAvtDeviceSerialNumber())));


        m_Camera.GetCameraDriverName(tmp);
        OnLog(QString("Driver name = %1").arg(tmp.c_str()));
        m_Camera.GetCameraDeviceName(tmp);
        OnLog(QString("Device name = %1").arg(tmp.c_str()));
        m_Camera.GetCameraBusInfo(tmp);
        OnLog(QString("Bus info = %1").arg(tmp.c_str()));
        m_Camera.GetCameraDriverVersion(tmp);
        OnLog(QString("Driver version = %1").arg(tmp.c_str()));
        m_Camera.GetCameraCapabilities(tmp);
        OnLog(QString("Capabilities = %1").arg(tmp.c_str()));

        OnLog("---------------------------------------------");

        if ( !m_bIsOpen )
            m_Camera.CloseDevice();
    }
}

// The event handler for stream statistics
void V4L2Viewer::OnGetStreamStatisticsButtonClicked()
{
    OnLog("---------------------------------------------");
    OnLog("---- Stream Statistics ");
    OnLog("---------------------------------------------");

    uint64_t FramesCount;
    uint64_t PacketCRCError;
    uint64_t FramesUnderrun;
    uint64_t FramesIncomplete;
    double CurrentFrameRate;

    if( m_Camera.getDriverStreamStat(FramesCount, PacketCRCError, FramesUnderrun, FramesIncomplete, CurrentFrameRate) )
    {
        OnLog("Stream statistics as reported from kernel driver:");
        OnLog(QString("FramesCount=%1").arg(FramesCount));
        OnLog(QString("PacketCRCError=%1").arg(PacketCRCError));
        OnLog(QString("FramesUnderrun=%1").arg(FramesUnderrun));
        OnLog(QString("FramesIncomplete=%1").arg(FramesIncomplete));
        OnLog(QString("CurrentFrameRate=")+QString::number(CurrentFrameRate, 'f', 2));
    }
    else
    {
        OnLog("Driver does not support stream statistics custom ioctl VIDIOC_STREAMSTAT.");
    }

    OnLog("---------------------------------------------");
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

void V4L2Viewer::OnToggleButtonClicked()
{
    uint32_t payloadSize = 0;
    int result = 0;

    // Get the payload size first to setup the streaming channel
    result = m_Camera.ReadPayloadSize(payloadSize);

    OnLog(QString("Received payload size = %1").arg(payloadSize));

    if (result == 0)
    {
        int timeout = 1000;
        if (m_ToggleStreamDelayRandCheckBox->isChecked())
        {
            int randomBase = m_ToggleStreamDelayRandLineEdit->text().toInt();
            timeout = (rand() % randomBase) + 1;
        }
        else
        {
            timeout = m_ToggleStreamDelayLineEdit->text().toInt();
        }

        m_StreamToggleTimer.start(timeout);
    }
}

void V4L2Viewer::OnStreamToggleTimeout()
{
    int timeout = 1000;
    if (m_ToggleStreamDelayRandCheckBox->isChecked())
    {
        int randomBase = m_ToggleStreamDelayRandLineEdit->text().toInt();
        timeout = (rand() % randomBase) + 1;
    }
    else
    {
        timeout = m_ToggleStreamDelayLineEdit->text().toInt();
    }

    m_StreamToggleTimer.stop();

    if (m_bIsStreaming)
    {
        m_Camera.StopStreaming();

        int err = m_Camera.StopStreamChannel();
        if (0 != err)
            OnLog("Stop stream channel failed.");

        m_bIsStreaming = false;
        UpdateViewerLayout();

        m_FramesReceivedTimer.stop();
        m_Camera.DeleteUserBuffer();
    }
    else
        OnStartButtonClicked();

    m_StreamToggleTimer.start(timeout);
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

// Event will be called when the a frame is recorded
void V4L2Viewer::OnCameraRecordFrame(const QSharedPointer<MyFrame>& frame)
{
    if(m_FrameRecordVector.size() >= 100)
    {
        OnLog(QString("The following frames are not recorded, more than %1 would freeze the system.").arg(MAX_RECORD_FRAME_VECTOR_SIZE));
        OnStopRecording();
    }
    else
    {
        m_FrameRecordVector.push_back(frame);
        ui.m_FrameIDRecording->setText(QString("FrameID: %1").arg(frame->GetFrameId()));

        ui.m_FramesInQueue->setText(QString("Frames in Queue: %1").arg(m_FrameRecordVector.size()));

        if (1 == m_FrameRecordVector.size())
        {
            ui.m_FrameIDStartedRecord->setText(QString("FrameID start: %1").arg(frame->GetFrameId()));
        }
        ui.m_FrameIDStoppedRecord->setText(QString("FrameID stopped: %1").arg(frame->GetFrameId()));
    }

    UpdateRecordTableWidget();
}

void V4L2Viewer::OnCameraPixelFormat(const QString& pixelFormat)
{
    ui.m_liPixelFormats->addItem(pixelFormat);
}

void V4L2Viewer::OnCameraFrameSize(const QString& frameSize)
{
    ui.m_liFrameSizes->addItem(frameSize);
}

void V4L2Viewer::StartStreaming(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine)
{
    int err = 0;
    int nRow = ui.m_CamerasListBox->currentRow();

    // disable the start button to show that the start acquisition is in process
    ui.m_StartButton->setEnabled(false);
    ui.m_ToggleButton->setEnabled(false);
    QApplication::processEvents();

    m_nDroppedFrames = 0;

    // prepare the frame ascii log range
    QString logFrameRange = m_LogFrameRangeLineEdit->text();
    QStringList logFrameRangeList = logFrameRange.split('-');

    if (logFrameRangeList.size() > 3)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("Missing parameter. Format: 1 or 1-2!") );
        return;
    }

    int32_t logFrameRangeStart = -1;
    if (m_LogFrameRangeLineEdit->text() != "" && logFrameRangeList.size() >= 1)
        logFrameRangeStart = logFrameRangeList.at(0).toInt();

    int32_t logFrameRangeEnd = logFrameRangeStart;
    if (logFrameRangeList.size() == 2)
        logFrameRangeEnd = logFrameRangeList.at(1).toInt();

    // prepare the frame byte log range
    QString dumpByteFrameRange = m_DumpByteFrameRangeLineEdit->text();
    QStringList dumpByteFrameRangeList = dumpByteFrameRange.split('-');

    if (dumpByteFrameRangeList.size() > 3)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("Missing parameter. Format: 1 or 1-2!") );
        return;
    }

    int32_t dumpByteFrameRangeStart = -1;
    if (m_DumpByteFrameRangeLineEdit->text() != "" && dumpByteFrameRangeList.size() >= 1)
        dumpByteFrameRangeStart = dumpByteFrameRangeList.at(0).toInt();
    int32_t dumpByteFrameRangeEnd = dumpByteFrameRangeStart;
    if (dumpByteFrameRangeList.size() == 2)
        dumpByteFrameRangeEnd = dumpByteFrameRangeList.at(1).toInt();

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
        err = m_Camera.StartStreamChannel(m_CSVFileLineEdit->text().toStdString().c_str(),
                                          pixelFormat,
                                          payloadSize,
                                          width,
                                          height,
                                          bytesPerLine,
                                          NULL,
                                          ui.m_TitleLogtofile->isChecked(),
                                          logFrameRangeStart,
                                          logFrameRangeEnd,
                                          dumpByteFrameRangeStart,
                                          dumpByteFrameRangeEnd,
                                          ui.m_TitleCorrectIncomingRAW10Image->isChecked());

        if (0 != err)
            OnLog("Start Acquisition failed during SI Start channel.");
        else
        {
            OnLog("Acquisition started ...");
            m_bIsStreaming = true;
            ui.m_StreamLabel->setText(QString("Stream#%1").arg(++m_nStreamNumber));
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

void V4L2Viewer::InitializeTableWidget()
{
    // set number of columns and rows
    ui.m_FrameRecordTable->setRowCount(0);
    ui.m_FrameRecordTable->setColumnCount(9);

    // set table headers
    QStringList header;
    header << "Frame ID" << "Buffer index" << "Width" << "Height" << "Payload size" << "Pixel Format" << "Buffer length" << "Bytes per line" << "Reference Image: #Unequal Bytes";
    ui.m_FrameRecordTable->setHorizontalHeaderLabels(header);
    ui.m_FrameRecordTable->resizeColumnsToContents();
}

// Returns the frame object that is selected in the gui table, or a null pointer if nothing is selected
QSharedPointer<MyFrame> V4L2Viewer::getSelectedRecordedFrame()
{
    QSharedPointer<MyFrame> result;
    QList<QTableWidgetItem*> selectedItems = ui.m_FrameRecordTable->selectedItems();

    if (selectedItems.size() > 0)
    {
        QTableWidgetItem* item = selectedItems[0];
        int index = item->row();

        if (index >= 0 && index < m_FrameRecordVector.size())
        {
            result = m_FrameRecordVector[index];
        }
    }

    return result;
}

void V4L2Viewer::UpdateRecordTableWidget()
{
    ui.m_FrameRecordTable->setRowCount(m_FrameRecordVector.size());

    for (int i = 0; i < m_FrameRecordVector.size(); i++)
    {
        QSharedPointer<MyFrame> frame = m_FrameRecordVector.at(i);

        // get frame information
        unsigned long long frameId = frame->GetFrameId();
        uint32_t bufferIndex = frame->GetBufferIndex();
        uint32_t width = frame->GetWidth();
        uint32_t height = frame->GetHeight();
        uint32_t payloadSize = frame->GetPayloadSize();
        uint32_t pixelFormat = frame->GetPixelFormat();
        uint32_t bufferLength = frame->GetBufferlength();
        uint32_t bytesPerLine = frame->GetBytesPerLine();

        // frame id
        QTableWidgetItem* item_frameId = new QTableWidgetItem();
        item_frameId->setText(QString::number(frameId));
        ui.m_FrameRecordTable->setItem(i, 0, item_frameId);

        // buffer index
        QTableWidgetItem* item_bufferIndex = new QTableWidgetItem();
        item_bufferIndex->setText(QString::number(bufferIndex));
        ui.m_FrameRecordTable->setItem(i, 1, item_bufferIndex);

        // width
        QTableWidgetItem* item_width = new QTableWidgetItem();
        item_width->setText(QString::number(width));
        ui.m_FrameRecordTable->setItem(i, 2, item_width);

        // height
        QTableWidgetItem* item_height = new QTableWidgetItem();
        item_height->setText(QString::number(height));
        ui.m_FrameRecordTable->setItem(i, 3, item_height);

        // payload size
        QTableWidgetItem* item_payloadSize = new QTableWidgetItem();
        item_payloadSize->setText(QString::number(payloadSize));
        ui.m_FrameRecordTable->setItem(i, 4, item_payloadSize);

        // pixel format
        QTableWidgetItem* item_pixelFormat = new QTableWidgetItem();
        item_pixelFormat->setText(QString::fromStdString(v4l2helper::ConvertPixelFormat2EnumString(pixelFormat)));
        ui.m_FrameRecordTable->setItem(i, 5, item_pixelFormat);

        // buffer length
        QTableWidgetItem* item_bufferLength = new QTableWidgetItem();
        item_bufferLength->setText(QString::number(bufferLength));
        ui.m_FrameRecordTable->setItem(i, 6, item_bufferLength);

        // bytes per line
        QTableWidgetItem* item_bytesPerLine = new QTableWidgetItem();
        item_bytesPerLine->setText(QString::number(bytesPerLine));
        ui.m_FrameRecordTable->setItem(i, 7, item_bytesPerLine);
    }

    ui.m_FrameRecordTable->resizeColumnsToContents();
}


// The event handler for stopping acquisition
void V4L2Viewer::OnStopButtonClicked()
{
    // disable the stop button to show that the stop acquisition is in process
    ui.m_StopButton->setEnabled(false);

    int err = m_Camera.StopStreamChannel();
    if (0 != err)
        OnLog("Stop stream channel failed.");

    m_StreamToggleTimer.stop();

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

// The event handler to show the processed frame
void V4L2Viewer::OnFrameReady(const QImage &image, const unsigned long long &frameId)
{
    if (m_ShowFrames)
    {
        if (!image.isNull())
        {
            if (ui.m_TitleFlipHorizontal->isChecked() || ui.m_TitleFlipVertical->isChecked())
            {
                QImage tmpImage;
                if (ui.m_TitleFlipHorizontal->isChecked())
                    tmpImage = image.mirrored(false, true);
                else
                    tmpImage = image;
                if (ui.m_TitleFlipVertical->isChecked())
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

    ui.m_EventsLogEditWidget->appendPlainText(eventText);
    ui.m_EventsLogEditWidget->verticalScrollBar()->setValue(ui.m_EventsLogEditWidget->verticalScrollBar()->maximum());
}

// The event handler to clear the event list
void V4L2Viewer::OnClearEventLogButtonClicked()
{
    ui.m_EventsLogEditWidget->clear();
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
    ui.m_radioBlocking->setEnabled( !m_bIsOpen );
    ui.m_radioNonBlocking->setEnabled( !m_bIsOpen );
    ui.m_chkUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_chkUseRead->setEnabled( !m_bIsOpen );
    ui.m_chkUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_TitleUseRead->setEnabled( !m_bIsOpen );
    ui.m_TitleEnable_VIDIOC_TRY_FMT->setEnabled( !m_bIsOpen );
    ui.m_TitleEnableExtendedControls->setEnabled( !m_bIsOpen );
}

// The event handler to open a camera on double click event
void V4L2Viewer::OnListBoxCamerasItemDoubleClicked(QListWidgetItem * item)
{
    OnOpenCloseButtonClicked();
}

// Queries and lists all known camera
void V4L2Viewer::UpdateCameraListBox(uint32_t cardNumber, uint64_t cameraID, const QString &deviceName, const QString &info)
{
    std::string strCameraName;

    strCameraName = "Camera#";

    ui.m_CamerasListBox->addItem(QString::fromStdString(strCameraName + ": ") + deviceName + QString(" (") + info + QString(")"));
    m_cameras.push_back(cardNumber);

    // select the first camera if there is no camera selected
    if ((-1 == ui.m_CamerasListBox->currentRow()) && (0 < ui.m_CamerasListBox->count()))
    {
        ui.m_CamerasListBox->setCurrentRow(0, QItemSelectionModel::Select);
    }

    ui.m_OpenCloseButton->setEnabled((0 < m_cameras.size()) || m_bIsOpen);
    ui.m_GetDeviceInfoButton->setEnabled((0 < m_cameras.size()) || m_bIsOpen);
    ui.m_GetStreamStatisticsButton->setEnabled((0 < m_cameras.size()) || m_bIsOpen);
    ui.m_radioBlocking->setEnabled( !m_bIsOpen );
    ui.m_radioNonBlocking->setEnabled( !m_bIsOpen );
    ui.m_chkUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_chkUseRead->setEnabled( !m_bIsOpen );
    ui.m_chkUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
    ui.m_TitleUseRead->setEnabled( !m_bIsOpen );
    ui.m_TitleEnable_VIDIOC_TRY_FMT->setEnabled( !m_bIsOpen );
    ui.m_TitleEnableExtendedControls->setEnabled( !m_bIsOpen );
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

    ui.m_CamerasListBox->setEnabled(!m_bIsOpen);

    ui.m_StartButton->setEnabled(m_bIsOpen && !m_bIsStreaming);
    ui.m_ToggleButton->setEnabled(m_bIsOpen && !m_bIsStreaming);
    ui.m_StopButton->setEnabled(m_bIsOpen && m_bIsStreaming);
    ui.m_FramesPerSecondLabel->setEnabled(m_bIsOpen && m_bIsStreaming);
    ui.m_FrameIdLabel->setEnabled(m_bIsOpen && m_bIsStreaming);
    ui.m_StreamLabel->setEnabled(m_bIsOpen && m_bIsStreaming);

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
    err = m_Camera.OpenDevice(devName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT, m_ExtendedControls);

    if (0 != err)
        OnLog("Open device failed");
    else
    {
        m_nStreamNumber = 0;

        char buff[32];
        memset(buff, 0, sizeof(buff));

        if (m_Camera.ReadRegister(CCI_GCPRM_16R, buff, 2, true) >= 0)
        {
            uint16_t val = *(uint16_t*)buff;
            ui.m_GCPRMOffset->setText( QString("0x%1").arg(val, 4, 16, QChar('0')));
        }
        else
        {
            ui.m_GCPRMOffset->setText("-");
        }

        if (m_Camera.ReadRegister(CCI_BCRM_16R, buff, 2, true) >= 0)
        {
            uint16_t val = *(uint16_t*)buff;
            ui.m_BCRMOffset->setText( QString("0x%1").arg(val, 4, 16, QChar('0')));
        }
        else
        {
            ui.m_BCRMOffset->setText("-");
        }
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

// The event handler to show the frames received
void V4L2Viewer::OnControllerResponseTimeout()
{
    OnLog("Payload request timed out. Cannot start streaming.");

    m_ControlRequestTimer.stop();
}

// The event handler to read a register per direct access
void V4L2Viewer::OnDirectRegisterAccessReadButtonClicked()
{
    // define the base of the given string
    int base = 10;
    if (0 <= ui.m_DirectRegisterAccessAddressLineEdit->text().indexOf("x"))
    {
        base = 16;
    }
    // convert the register address value
    bool converted;
    uint16_t address = (uint16_t)ui.m_DirectRegisterAccessAddressLineEdit->text().toInt(&converted, base);

    if (converted)
    {
        char *pBuffer = NULL;
        uint32_t nSize = 0;

        if (ui.m_DataTypeCombo->currentText() == "Int8")
        {
            nSize = 1;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt8")
        {
            nSize = 1;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "Int16")
        {
            nSize = 2;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt16")
        {
            nSize = 2;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "Int32")
        {
            nSize = 4;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt32")
        {
            nSize = 4;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "Int64")
        {
            nSize = 8;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt64")
        {
            nSize = 8;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "String64")
        {
            nSize = 64;
        }
        else
        {
            // Error
            return;
        }

        pBuffer = (char*)malloc(nSize+1);
        memset(pBuffer, 0, nSize+1);
        int iRet = m_Camera.ReadRegister(address, pBuffer, nSize, true);
        QString strValue("");

        if (iRet == 0)
        {
            if (ui.m_DataTypeCombo->currentText() == "Int8")
            {
                int8_t nVal = *(int8_t*)pBuffer;
                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 2, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "UInt8")
            {
                uint8_t nVal = *(uint8_t*)pBuffer;
                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 2, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "Int16")
            {
                int16_t nVal = *(int16_t*)pBuffer;

                if (ui.m_BigEndianessRadio->isChecked())
                    nVal = qToBigEndian(nVal);

                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 4, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "UInt16")
            {
                uint16_t nVal = *(uint16_t*)pBuffer;

                if (ui.m_BigEndianessRadio->isChecked())
                    nVal = qToBigEndian(nVal);

                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 4, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "Int32")
            {
                int32_t nVal = *(int32_t*)pBuffer;

                if (ui.m_BigEndianessRadio->isChecked())
                    nVal = qToBigEndian(nVal);

                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 8, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "UInt32")
            {
                uint32_t nVal = *(uint32_t*)pBuffer;

                if (ui.m_BigEndianessRadio->isChecked())
                    nVal = qToBigEndian(nVal);

                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 8, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "Int64")
            {
                qint64 nVal = *(qint64*)pBuffer;

                if (ui.m_BigEndianessRadio->isChecked())
                    nVal = qToBigEndian(nVal);

                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 16, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "UInt64")
            {
                quint64 nVal = *(quint64*)pBuffer;

                if (ui.m_BigEndianessRadio->isChecked())
                    nVal = qToBigEndian(nVal);

                strValue = ui.m_DataHexRadio->isChecked() ?
                    QString("0x%1").arg(nVal, 16, 16, QChar('0')) :
                    QString("%1").arg(nVal);
            }
            else
            if (ui.m_DataTypeCombo->currentText() == "String64")
            {
                strValue = QString("%1").arg(pBuffer);
            }

            OnLog(QString("Register '0x%1' read returned %2.").arg(address, 4, 16, QChar('0')).arg(strValue));
        }
        else
        {
            OnLog(QString("Error while reading register '0x%1'").arg(address, 4, 16, QChar('0')));
        }

        ui.m_DirectRegisterAccessDataLineEdit->setText(strValue);

        free(pBuffer);
        pBuffer = NULL;
    }
    else
    {
        OnLog("Could not convert the register address from the given string.");
    }
}


// The event handler to write a register per direct access
void V4L2Viewer::OnDirectRegisterAccessWriteButtonClicked()
{
    // define the base of the given strings
    int base = 10;
    if (0 <= ui.m_DirectRegisterAccessAddressLineEdit->text().indexOf("x"))
    {
        base = 16;
    }

    // convert the register address value
    bool converted;
    uint16_t address = (uint16_t)ui.m_DirectRegisterAccessAddressLineEdit->text().toInt(&converted, base);

    if (converted)
    {
        uint64_t nVal = 0;
        bool bValid = false;
        char *pBuffer = NULL;
        uint32_t nSize = 0;
        pBuffer = (char*)malloc(65);
        memset(pBuffer, 0, 65);

        if (ui.m_DataTypeCombo->currentText() != "String64")
        {
            nVal = ui.m_DataHexRadio->isChecked() ?
                    ui.m_DirectRegisterAccessDataLineEdit->text().toLongLong(&converted, 16) :
                    ui.m_DirectRegisterAccessDataLineEdit->text().toLongLong(&converted, 10);
        }
        else
        {
            QByteArray ba = ui.m_DirectRegisterAccessDataLineEdit->text().toLocal8Bit();
            memcpy(pBuffer, ba.data(), ba.length());
            nSize = 64;
            bValid = true;
        }

        if (ui.m_DataTypeCombo->currentText() == "Int8")
        {
            memcpy(pBuffer, (char*)&nVal, 1);
            nSize = 1;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt8")
        {
            memcpy(pBuffer, (char*)&nVal, 1);
            nSize = 1;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "Int16")
        {
            if (ui.m_BigEndianessRadio->isChecked())
                nVal = qToBigEndian((int16_t)nVal);

            memcpy(pBuffer, (char*)&nVal, 2);
            nSize = 2;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt16")
        {
            if (ui.m_BigEndianessRadio->isChecked())
                nVal = qToBigEndian((uint16_t)nVal);
            memcpy(pBuffer, (char*)&nVal, 2);
            nSize = 2;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "Int32")
        {
            if (ui.m_BigEndianessRadio->isChecked())
                nVal = qToBigEndian((int32_t)nVal);
            memcpy(pBuffer, (char*)&nVal, 4);
            nSize = 4;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt32")
        {
            if (ui.m_BigEndianessRadio->isChecked())
                nVal = qToBigEndian((uint32_t)nVal);
            memcpy(pBuffer, (char*)&nVal, 4);
            nSize = 4;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "Int64")
        {
            if (ui.m_BigEndianessRadio->isChecked())
                nVal = qToBigEndian((qint64)nVal);
            memcpy(pBuffer, (char*)&nVal, 8);
            nSize = 8;
            bValid = true;
        }
        else
        if (ui.m_DataTypeCombo->currentText() == "UInt64")
        {
            if (ui.m_BigEndianessRadio->isChecked())
                nVal = qToBigEndian((quint64)nVal);
            memcpy(pBuffer, (char*)&nVal, 8);
            nSize = 8;
            bValid = true;
        }

        if (bValid && converted)
        {
            int iRet = m_Camera.WriteRegister(address, pBuffer, nSize, true);
            if (iRet == 0)
            {
                OnLog(QString("Register '0x%1' set to %2.").arg(address, 4, 16, QChar('0')).arg(pBuffer));
            }
            else
            {
                OnLog(QString("Error while writing register '0x%1'").arg(address, 4, 16, QChar('0')));
            }
        }

        free(pBuffer);
        pBuffer = NULL;
    }
    else
    {
        OnLog("Could not convert the register address from the given string.");
    }
}

void V4L2Viewer::OnStartRecording()
{
    m_Camera.SetRecording(true);

    ui.m_CalcLiveDeviationButton->setEnabled(false);
    ui.m_StartRecordButton->setEnabled(false);
    ui.m_StopRecordButton->setEnabled(true);
    ui.m_CalcDeviationButton->setEnabled(false);
    ui.m_DeleteRecording->setEnabled(false);
    ui.m_SaveFrameSeriesButton->setEnabled(false);
}

void V4L2Viewer::OnStopRecording()
{
    m_Camera.SetRecording(false);

    UpdateRecordTableWidget();

    ui.m_StartRecordButton->setEnabled(true);
    ui.m_StopRecordButton->setEnabled(false);

    if (m_FrameRecordVector.size() > 0)
    {
        ui.m_DeleteRecording->setEnabled(true);
        ui.m_SaveFrameSeriesButton->setEnabled(true);

        if (m_ReferenceImage)
        {
            ui.m_CalcDeviationButton->setEnabled(true);
            ui.m_CalcLiveDeviationButton->setEnabled(true);
        }
    }
}

void V4L2Viewer::OnDeleteRecording()
{
    if(m_bIsStreaming)
    {
        OnStopRecording();
    }

    m_FrameRecordVector.clear();
    UpdateRecordTableWidget();

    ui.m_FrameIDRecording->setText(QString("FrameID: -"));
    ui.m_FrameIDStartedRecord->setText(QString("FrameID start: -"));
    ui.m_FrameIDStoppedRecord->setText(QString("FrameID stopped: -"));
    ui.m_FramesInQueue->setText(QString("Frames in Queue: 0"));

    if (m_FrameRecordVector.size() == 0)
    {
        ui.m_DeleteRecording->setEnabled(false);
        ui.m_SaveFrameSeriesButton->setEnabled(false);
        ui.m_CalcDeviationButton->setEnabled(false);
        ui.m_meanDeviationLabel->setText(QString("Mean #Unequal Bytes: -"));
    }
}

void V4L2Viewer::OnRecordTableSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    QSharedPointer<MyFrame> selectedFrame = getSelectedRecordedFrame();

    // enable save frame if there is a frame selected
    ui.m_DisplaySaveFrame->setEnabled(!(!selectedFrame));
    ui.m_ExportRecordedFrameButton->setEnabled(!(!selectedFrame));

    // Display selected recorded frame if camera is not streaming
    if (!m_bIsStreaming)
    {
        if (selectedFrame)
        {
            QImage image = selectedFrame->GetImage();

            m_pScene->setSceneRect(0, 0, image.width(), image.height());
            m_PixmapItem->setPixmap(QPixmap::fromImage(image));
            ui.m_ImageView->show();

            ui.m_FrameIdLabel->setText(QString("Recorded Frame ID: %1, W: %2, H: %3").arg(selectedFrame->GetFrameId()).arg(image.width()).arg(image.height()));
        }
    }
}

void V4L2Viewer::OnGetReferenceImage()
{
    // if dialog already exist, delete it
    if ( NULL != m_ReferenceImageDialog )
    {
        delete m_ReferenceImageDialog;
        m_ReferenceImageDialog = NULL;
    }

    // create new file dialog
    QString fileExtensions = GetImageFormatString();
    m_ReferenceImageDialog = new QFileDialog( this, tr("Open reference image"), m_SaveFileDir, "" );
    m_ReferenceImageDialog->selectNameFilter( m_SelectedExtension );
    m_ReferenceImageDialog->setAcceptMode( QFileDialog::AcceptOpen );

    // open file dialog
    if ( m_ReferenceImageDialog->exec() )
    {
        m_SelectedExtension = m_ReferenceImageDialog->selectedNameFilter();
        m_SaveFileDir = m_ReferenceImageDialog->directory().absolutePath();
        QStringList selectedFiles = m_ReferenceImageDialog->selectedFiles();

        // get selected file
        if ( !selectedFiles.isEmpty() )
        {
            QString filePath = selectedFiles.at(0);
            QFile file(filePath);

            if (file.open(QIODevice::ReadOnly))
            {
                m_ReferenceImage.clear();
                m_ReferenceImage = QSharedPointer<QByteArray>(new QByteArray(file.readAll()));

                QFileInfo fileInfo(file.fileName());
                QString filename(fileInfo.fileName());
                ui.m_referenceLabel->setText(QString("Reference Image: %1 (%2 Bytes)").arg(filename).arg(m_ReferenceImage->size()));

                file.close();

                if(m_bIsOpen)
                {
                    ui.m_CalcLiveDeviationButton->setEnabled(true);

                    if (m_FrameRecordVector.size())
                    {
                        ui.m_CalcDeviationButton->setEnabled(true);
                    }
                }
            }
            else
            {
                QMessageBox::warning( this, tr("V4L2 Test"), tr("Could not load image! \nCheck access rights."), tr("") );
            }

        }
    }
}


void V4L2Viewer::SaveFrame(QSharedPointer<MyFrame> frame, QString fileName, bool raw)
{
    // dump frame binary to file
    if (raw)
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly))
        {
            QByteArray databuf = QByteArray((char*)(frame->GetBuffer()), frame->GetBufferlength());
            file.write(databuf);
            file.flush();
            file.close();

            OnLog(QString("Saving RAW image successful: %1").arg(fileName));
        }
        else
        {
            QMessageBox::warning( this, tr("V4L2 Test"), tr("Failed to save image! \nCheck access rights."), tr("") );
        }

    }
    // save png
    else
    {
        QImage image = frame->GetImage();
        if (image.save(fileName))
        {
            OnLog(QString("Saving PNG image successful: %1").arg(fileName));
        }
        else
        {
            QMessageBox::warning( this, tr("V4L2 Test"), tr("Failed to save image! \nCheck access rights."), tr("") );
        }
    }
}


void V4L2Viewer::SaveFrameDialog(bool raw)
{
    QSharedPointer<MyFrame> selectedFrame = getSelectedRecordedFrame();

    if (selectedFrame)
    {
        QString fileExtensions = raw ? ".raw" : ".png";

        QString pixelFormat = QString::fromStdString(v4l2helper::ConvertPixelFormat2EnumString(selectedFrame->GetPixelFormat()));
        QString width = QString::number(selectedFrame->GetWidth());
        QString height = QString::number(selectedFrame->GetHeight());
        QString frameId = QString::number(selectedFrame->GetFrameId());
        QString defaultFileName = QString("Frame") + frameId + QString("_") + width + QString("x") + height + QString("_") + pixelFormat;

        if ( NULL != m_SaveFileDialog )
        {
            delete m_SaveFileDialog;
            m_SaveFileDialog = NULL;
        }

        m_SaveFileDialog = new QFileDialog ( this, tr((raw ? "Save Image" : "Export Image")), m_SaveFileDir + QString("/") + defaultFileName, fileExtensions );
        m_SaveFileDialog->selectNameFilter(m_SelectedExtension);
        m_SaveFileDialog->setAcceptMode(QFileDialog::AcceptSave);

        if (m_SaveFileDialog->exec())
        {
            m_SelectedExtension = m_SaveFileDialog->selectedNameFilter();
            m_SaveFileDir = m_SaveFileDialog->directory().absolutePath();
            QStringList files = m_SaveFileDialog->selectedFiles();

            if (!files.isEmpty())
            {
                QString fileName = files.at(0);

                if (!fileName.endsWith(m_SelectedExtension))
                {
                    fileName.append(m_SelectedExtension);
                }

                SaveFrame(selectedFrame, fileName, raw);
            }
        }
    }
}

void V4L2Viewer::OnSaveFrame()
{
    SaveFrameDialog(true);
}

void V4L2Viewer::OnSaveFrameSeries()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), m_SaveFileDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // check if user has hit the cancel button in the file dialog
    if (!dir.isEmpty() && !dir.isNull())
    {
        // iterate through table
        for (unsigned int tableRow = 0; tableRow < ui.m_FrameRecordTable->rowCount(); ++tableRow)
        {
            if (tableRow < m_FrameRecordVector.size())
            {
                QSharedPointer<MyFrame> frame = m_FrameRecordVector[tableRow];

                QString pixelFormat = QString::fromStdString(v4l2helper::ConvertPixelFormat2EnumString(frame->GetPixelFormat()));
                QString width = QString::number(frame->GetWidth());
                QString height = QString::number(frame->GetHeight());
                QString frameId = QString::number(frame->GetFrameId());
                QString filename = QString::number(tableRow) + QString("_Frame") + frameId + QString("_") + width + QString("x") + height + QString("_") + pixelFormat + QString(".raw");
                QString absoluteFilePath = dir + QString("/") + filename;

                SaveFrame(frame, absoluteFilePath, true);
            }
        }
    }
}

void V4L2Viewer::OnExportFrame()
{
    SaveFrameDialog(false);
}


void V4L2Viewer::OnCalcDeviationReady(unsigned int tableRow, int numberOfUnequalBytes, bool done)
{
    if(numberOfUnequalBytes < 0)
    {
        m_deviationErrors++;
    }
    else
    {
        m_MeanNumberOfUnequalBytes += numberOfUnequalBytes;

        QTableWidgetItem* item_unequalBytes = new QTableWidgetItem();
        item_unequalBytes->setText(QString::number(numberOfUnequalBytes));
        ui.m_FrameRecordTable->setItem(tableRow, 8, item_unequalBytes);
    }

    if(done)
    {
        // clear loading animation
        ui.m_meanDeviationLabel->clear();
        delete m_LoadingAnimation;

        if(m_deviationErrors < ui.m_FrameRecordTable->rowCount())
        {
            ui.m_meanDeviationLabel->setText(QString("Mean #Unequal Bytes: %1").arg(m_MeanNumberOfUnequalBytes / ui.m_FrameRecordTable->rowCount()));
        }
        else
        {
            ui.m_meanDeviationLabel->setText(QString("Mean #Unequal Bytes: -"));
            QMessageBox::warning( this, tr("V4L2 Test"), tr("Invalid reference image!\nPlease make sure to use a reference image in RAW format\nwith the same resolution and pixel format!"), tr("") );
        }

        ui.m_StartRecordButton->setEnabled(true);
        ui.m_CalcDeviationButton->setEnabled(true);
        ui.m_DeleteRecording->setEnabled(true);
        ui.m_GetReferenceButton->setEnabled(true);
        ui.m_CalcLiveDeviationButton->setEnabled(true);
        m_CalcThread.clear();
    }
}

void V4L2Viewer::OnCalcDeviation()
{
    ui.m_StartRecordButton->setEnabled(false);
    ui.m_CalcDeviationButton->setEnabled(false);
    ui.m_DeleteRecording->setEnabled(false);
    ui.m_GetReferenceButton->setEnabled(false);
    ui.m_CalcLiveDeviationButton->setEnabled(false);

    m_MeanNumberOfUnequalBytes = 0;
    m_deviationErrors = 0;

    std::map<unsigned int, QSharedPointer<MyFrame> > rowToFrameTable;

    // iterate through table
    for (unsigned int tableRow = 0; tableRow < ui.m_FrameRecordTable->rowCount(); ++tableRow)
    {
        // delete deviation column
        delete ui.m_FrameRecordTable->item(tableRow, 8);

        // populate map for deviation calculator thread
        if (tableRow < m_FrameRecordVector.size())
        {
            QSharedPointer<MyFrame> frame = m_FrameRecordVector[tableRow];
            rowToFrameTable.insert(std::make_pair(tableRow, frame));
        }
    }

    // show loading animation
    m_LoadingAnimation = new QMovie(":/V4L2Viewer/loading-animation.gif");
    ui.m_meanDeviationLabel->setMovie(m_LoadingAnimation);
    ui.m_meanDeviationLabel->show();
    m_LoadingAnimation->start();

    // calculate in separate thread
    m_CalcThread = QSharedPointer<DeviationCalculator>(new DeviationCalculator(m_ReferenceImage, rowToFrameTable));

    connect(m_CalcThread.data(), SIGNAL(OnCalcDeviationReady_Signal(unsigned int, int, bool)),
            this, SLOT(OnCalcDeviationReady(unsigned int, int, bool)));

    m_CalcThread->start();
}

void V4L2Viewer::OnCalcLiveDeviation()
{
    // start calc live deviation
    if(ui.m_CalcLiveDeviationButton->isChecked())
    {
        ui.m_StartRecordButton->setEnabled(false);
        ui.m_StopRecordButton->setEnabled(false);
        ui.m_CalcDeviationButton->setEnabled(false);
        ui.m_DeleteRecording->setEnabled(false);
        ui.m_SaveFrameSeriesButton->setEnabled(false);
        ui.m_DisplaySaveFrame->setEnabled(false);
        ui.m_ExportRecordedFrameButton->setEnabled(false);
        ui.m_GetReferenceButton->setEnabled(false);
        ui.m_FrameRecordTable->setDisabled(true);

        m_LiveDeviationNumberOfErrorFrames = 0;
        m_LiveDeviationFrameCount = 0;
        m_LiveDeviationUnequalBytes = 0;
        m_Camera.SetLiveDeviationCalc(m_ReferenceImage);
    }
    // stop calc live deviation
    else
    {
        // pass a QShared-Null-Pointer to disable live deviation calc
        m_Camera.SetLiveDeviationCalc(QSharedPointer<QByteArray>());

        ui.m_StartRecordButton->setEnabled(true);
        ui.m_StopRecordButton->setEnabled(false);
        ui.m_GetReferenceButton->setEnabled(true);
        ui.m_FrameRecordTable->setDisabled(false);

        if(m_FrameRecordVector.size())
        {
            ui.m_DeleteRecording->setEnabled(true);
            ui.m_SaveFrameSeriesButton->setEnabled(true);

            if(m_ReferenceImage)
            {
                ui.m_CalcDeviationButton->setEnabled(true);
            }
        }
    }
}

void V4L2Viewer::OnCalcLiveDeviationFromFrameObserver(int numberOfUnequalBytes)
{
    // invalid reference image
    if(numberOfUnequalBytes < 0)
    {
        // stop calc live deviation
        ui.m_CalcLiveDeviationButton->setChecked(false);
        OnCalcLiveDeviation();

        QMessageBox::warning( this, tr("V4L2 Test"), tr("Invalid reference image!\nPlease make sure to use a reference image in RAW format\nwith the same resolution and pixel format!"), tr("") );
    }
    // valid reference image
    else
    {
        m_LiveDeviationFrameCount++;
        m_LiveDeviationUnequalBytes += numberOfUnequalBytes;
        if(numberOfUnequalBytes > 0)
        {
            m_LiveDeviationNumberOfErrorFrames++;
        }

        ui.m_FrameIDRecording->setText(QString(""));
        ui.m_FrameIDStartedRecord->setText(QString("Processed frames: %1").arg(m_LiveDeviationFrameCount));
        ui.m_FrameIDStoppedRecord->setText(QString("Error Frames: %1").arg(m_LiveDeviationNumberOfErrorFrames));
        ui.m_FramesInQueue->setText(QString("Unequal Bytes: %1").arg(m_LiveDeviationUnequalBytes));
        ui.m_meanDeviationLabel->setText(QString("Live Comparison: Average %3 Unequal Bytes/Frame").arg(((double)m_LiveDeviationUnequalBytes)/m_LiveDeviationFrameCount));
    }
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
        ui.m_edPayloadSize->setText(QString("%1").arg(payloadSize));
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
        ui.m_edPayloadSize->setText(QString("%1").arg(payloadSize));
    }

    m_Camera.ReadFrameSize(width, height);
    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));
}

void V4L2Viewer::OnPixelFormat()
{
    if (m_Camera.SetPixelFormat(ui.m_edPixelFormat->text().toInt(), "") < 0)
    {
        uint32_t pixelFormat = 0;
        uint32_t bytesPerLine = 0;
        QString pixelFormatText;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE pixel format!") );
        m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
        ui.m_edPixelFormat->setText(QString("%1").arg(pixelFormat));
        ui.m_edPixelFormatText->setText(QString("%1").arg(pixelFormatText));
    }
    else
    {
        uint32_t pixelFormat = 0;
        uint32_t bytesPerLine = 0;
        QString pixelFormatText;
        // Readback to verify
        m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
        ui.m_edPixelFormat->setText(QString("%1").arg(pixelFormat));
        ui.m_edPixelFormatText->setText(QString("%1").arg(pixelFormatText));

        OnLog(QString("Pixel format set to %1").arg(ui.m_edPixelFormat->text().toInt()));
    }

    OnReadAllValues();
}



void V4L2Viewer::OnGain()
{
    int32_t gain = 0;
    bool autogain = false;

    int32_t nVal = int64_2_int32(ui.m_edGain->text().toLongLong());

    m_Camera.SetGain(nVal);

    if (m_Camera.ReadGain(gain) != -2)
    {
        ui.m_edGain->setEnabled(true);
        ui.m_edGain->setText(QString("%1").arg(gain));
    }
    else
        ui.m_edGain->setEnabled(false);
}

void V4L2Viewer::OnAutoGain()
{
    int32_t gain = 0;
    bool autogain = false;

    m_Camera.SetAutoGain(ui.m_chkAutoGain->isChecked());

    if (m_Camera.ReadAutoGain(autogain) != -2)
    {
        ui.m_chkAutoGain->setEnabled(true);
        ui.m_chkAutoGain->setChecked(autogain);
    }
    else
        ui.m_chkAutoGain->setEnabled(false);
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
        ui.m_chkAutoExposure->setEnabled(false);
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
        ui.m_chkAutoExposure->setEnabled(false);
}

void V4L2Viewer::OnAutoExposure()
{
    int32_t exposure = 0;
    bool autoexposure = false;

    m_Camera.SetAutoExposure(ui.m_chkAutoExposure->isChecked());

    if (m_Camera.ReadAutoExposure(autoexposure) != -2)
    {
        ui.m_chkAutoExposure->setEnabled(true);
        ui.m_chkAutoExposure->setChecked(autoexposure);
    }
    else
        ui.m_chkAutoExposure->setEnabled(false);
}

void V4L2Viewer::OnPixelFormatDBLClick(QListWidgetItem *item)
{
    std::string tmp = item->text().toStdString();
    char *s = (char*)tmp.c_str();
    uint32_t result = 0;

    if (tmp.size() == 4)
    {
        result += *s++;
        result += *s++ << 8;
        result += *s++ << 16;
        result += *s++ << 24;
    }

    ui.m_edPixelFormat->setText(QString("%1").arg(result));
    ui.m_edPixelFormatText->setText(QString("%1").arg(v4l2helper::ConvertPixelFormat2EnumString(result).c_str()));

    OnPixelFormat();
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

void V4L2Viewer::OnReverseX()
{
    int32_t nVal = int64_2_int32(ui.m_edReverseX->text().toLongLong());

    if (m_Camera.SetReverseX(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE reverse x!") );
        m_Camera.ReadReverseX(tmp);
        ui.m_edReverseX->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("ReverseX set to %1").arg(nVal));

        m_Camera.ReadReverseX(tmp);
        ui.m_edReverseX->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnReverseY()
{
    int32_t nVal = int64_2_int32(ui.m_edReverseY->text().toLongLong());

    if (m_Camera.SetReverseY(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE reverse y!") );
        m_Camera.ReadReverseY(tmp);
        ui.m_edReverseY->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("ReverseY set to %1").arg(nVal));

        m_Camera.ReadReverseY(tmp);
        ui.m_edReverseY->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnSharpness()
{
    int32_t nVal = int64_2_int32(ui.m_edSharpness->text().toLongLong());

    if (m_Camera.SetSharpness(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE sharpness!") );
        m_Camera.ReadSharpness(tmp);
        ui.m_edSharpness->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Sharpness set to %1").arg(nVal));

        m_Camera.ReadSharpness(tmp);
        ui.m_edSharpness->setText(QString("%1").arg(tmp));
    }
}

void V4L2Viewer::OnBrightness()
{
    int32_t nVal = int64_2_int32(ui.m_edBrightness->text().toLongLong());

    if (m_Camera.SetBrightness(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE brightness!") );
        m_Camera.ReadBrightness(tmp);
        ui.m_edBrightness->setText(QString("%1").arg(tmp));
    }
    else
    {
        int32_t tmp = 0;
        OnLog(QString("Brightness set to %1").arg(nVal));

        m_Camera.ReadBrightness(tmp);
        ui.m_edBrightness->setText(QString("%1").arg(tmp));
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
        ui.m_edBoundsX->setEnabled(true);
        ui.m_edBoundsX->setText(QString("%1").arg(boundsx));
        ui.m_edBoundsY->setEnabled(true);
        ui.m_edBoundsY->setText(QString("%1").arg(boundsy));
        ui.m_edBoundsW->setEnabled(true);
        ui.m_edBoundsW->setText(QString("%1").arg(boundsw));
        ui.m_edBoundsH->setEnabled(true);
        ui.m_edBoundsH->setText(QString("%1").arg(boundsh));
        ui.m_edDefrectX->setEnabled(true);
        ui.m_edDefrectX->setText(QString("%1").arg(defrectx));
        ui.m_edDefrectY->setEnabled(true);
        ui.m_edDefrectY->setText(QString("%1").arg(defrecty));
        ui.m_edDefrectW->setEnabled(true);
        ui.m_edDefrectW->setText(QString("%1").arg(defrectw));
        ui.m_edDefrectH->setEnabled(true);
        ui.m_edDefrectH->setText(QString("%1").arg(defrecth));
        ui.m_edAspect->setEnabled(true);
        ui.m_edAspect->setText(QString("%1/%2").arg(aspectnum).arg(aspectdenum));
    }
    else
    {
        ui.m_edBoundsX->setEnabled(false);
        ui.m_edBoundsY->setEnabled(false);
        ui.m_edBoundsW->setEnabled(false);
        ui.m_edBoundsH->setEnabled(false);
        ui.m_edDefrectX->setEnabled(false);
        ui.m_edDefrectY->setEnabled(false);
        ui.m_edDefrectW->setEnabled(false);
        ui.m_edDefrectH->setEnabled(false);
        ui.m_edAspect->setEnabled(false);
    }
}

void V4L2Viewer::OnReadAllValues()
{
    GetImageInformation();
    UpdateCameraFormat();
}

void V4L2Viewer::OnToggleStreamDelayRand()
{
    m_ToggleStreamDelayCheckBox->setChecked(false);
    m_ToggleStreamDelayRandCheckBox->setChecked(true);
}

void V4L2Viewer::OnToggleStreamDelay()
{
    m_ToggleStreamDelayCheckBox->setChecked(true);
    m_ToggleStreamDelayRandCheckBox->setChecked(false);
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
    int result = 0;
    uint32_t nUVal;
    int32_t nSVal;
    uint32_t numerator = 0;
    uint32_t denominator = 0;
    uint32_t pixelFormat = 0;
    QString pixelFormatText;
    uint32_t bytesPerLine = 0;


    ui.m_chkAutoGain->setChecked(false);
    ui.m_chkAutoExposure->setChecked(false);

    UpdateCameraFormat();

    ui.m_butWhiteBalanceOnce->setEnabled(m_Camera.IsWhiteBalanceOnceSupported());
    ui.m_chkContWhiteBalance->setEnabled(m_Camera.IsAutoWhiteBalanceSupported());

    if (m_Camera.ReadGain(gain) != -2)
    {
        ui.m_edGain->setEnabled(true);
        ui.m_edGain->setText(QString("%1").arg(gain));
    }
    else
        ui.m_edGain->setEnabled(false);

    if (m_Camera.ReadAutoGain(autogain) != -2)
    {
        ui.m_chkAutoGain->setEnabled(true);
        ui.m_chkAutoGain->setChecked(autogain);
    }
    else
        ui.m_chkAutoGain->setEnabled(false);

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
        ui.m_chkAutoExposure->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadGamma(nSVal) != -2)
    {
        ui.m_edGamma->setEnabled(true);
        ui.m_edGamma->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edGamma->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadReverseX(nSVal) != -2)
    {
        ui.m_edReverseX->setEnabled(true);
        ui.m_edReverseX->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edReverseX->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadReverseY(nSVal) != -2)
    {
        ui.m_edReverseY->setEnabled(true);
        ui.m_edReverseY->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edReverseY->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadSharpness(nSVal) != -2)
    {
        ui.m_edSharpness->setEnabled(true);
        ui.m_edSharpness->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edSharpness->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadBrightness(nSVal) != -2)
    {
        ui.m_edBrightness->setEnabled(true);
        ui.m_edBrightness->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edBrightness->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadContrast(nSVal) != -2)
    {
        ui.m_edContrast->setEnabled(true);
        ui.m_edContrast->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edContrast->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadSaturation(nSVal) != -2)
    {
        ui.m_edSaturation->setEnabled(true);
        ui.m_edSaturation->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edSaturation->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadHue(nSVal) != -2)
    {
        ui.m_edHue->setEnabled(true);
        ui.m_edHue->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edHue->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadRedBalance(nSVal) != -2)
    {
        ui.m_edRedBalance->setEnabled(true);
        ui.m_edRedBalance->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edRedBalance->setEnabled(false);

    nSVal = 0;
    if (m_Camera.ReadBlueBalance(nSVal) != -2)
    {
        ui.m_edBlueBalance->setEnabled(true);
        ui.m_edBlueBalance->setText(QString("%1").arg(nSVal));
    }
    else
        ui.m_edBlueBalance->setEnabled(false);

    nUVal = 0;
    m_Camera.ReadFrameSize(width, height);
    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    if (m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat) != -2)
    {
        ui.m_edFrameRate->setEnabled(true);
        ui.m_edFrameRate->setText(QString("%1/%2").arg(numerator).arg(denominator));
    }
    else
        ui.m_edFrameRate->setEnabled(false);

    nUVal = 0;
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
    {
        ui.m_edCropXOffset->setEnabled(true);
        ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
        ui.m_edCropYOffset->setEnabled(true);
        ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
        ui.m_edCropWidth->setEnabled(true);
        ui.m_edCropWidth->setText(QString("%1").arg(width));
        ui.m_edCropHeight->setEnabled(true);
        ui.m_edCropHeight->setText(QString("%1").arg(height));
    }
    else
    {
        ui.m_edCropXOffset->setEnabled(false);
        ui.m_edCropYOffset->setEnabled(false);
        ui.m_edCropWidth->setEnabled(false);
        ui.m_edCropHeight->setEnabled(false);
    }
    OnCropCapabilities();

    if (0 != m_Camera.EnumAllControlNewStyle())
    {
        OnCameraWarning("Didn't get Controls with new style.");
        m_Camera.EnumAllControlOldStyle();
    }
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

    ui.m_liPixelFormats->clear();
    ui.m_liFrameSizes->clear();

    // Get the payload size first to setup the streaming channel
    result = m_Camera.ReadPayloadSize(payloadSize);
    ui.m_edPayloadSize->setText(QString("%1").arg(payloadSize));

    result = m_Camera.ReadFrameSize(width, height);
    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));

    result = m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    ui.m_edPixelFormat->setText(QString("%1").arg(pixelFormat));
    ui.m_edPixelFormatText->setText(QString("%1").arg(pixelFormatText));

    result = m_Camera.ReadFormats();
}

// Check if IO Read was checked and remove it when not capable
void V4L2Viewer::Check4IOReadAbility()
{
    int nRow = ui.m_CamerasListBox->currentRow();

    if (-1 != nRow)
    {
        QString devName = ui.m_CamerasListBox->item(nRow)->text();
        std::string deviceName;
        std::string tmp;

        devName = devName.right(devName.length() - devName.indexOf(':') - 2);
        deviceName = devName.left(devName.indexOf('(') - 1).toStdString();
        bool ioRead;

        m_Camera.OpenDevice(deviceName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT, m_ExtendedControls);
        m_Camera.GetCameraReadCapability(ioRead);
        m_Camera.CloseDevice();

        if (!ioRead)
        {
            ui.m_chkUseRead->setEnabled( false );
            ui.m_TitleUseRead->setEnabled( false );
        }
        else
        {
            ui.m_chkUseRead->setEnabled( true );
            ui.m_TitleUseRead->setEnabled( true );
        }
        if (!ioRead && ui.m_chkUseRead->isChecked())
        {
            QMessageBox::warning( this, tr("V4L2 Test"), tr("IO Read not available with this camera. V4L2_CAP_VIDEO_CAPTURE not set. Switched to IO MMAP."));

            ui.m_chkUseRead->setChecked( false );
            ui.m_TitleUseRead->setChecked( false );
            ui.m_chkUseMMAP->setEnabled( true );
            ui.m_TitleUseMMAP->setEnabled( true );
            ui.m_chkUseMMAP->setChecked( true );
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

