
/*=============================================================================
Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        v4l2test.cpp

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

#include <sstream>

#include <QtGlobal>
#include <QtCore>
#include <QStringList>

#include "v4l2test.h"
#include "V4l2Helper.h"

#include "Logger.h"
#include <ctime>

#define NUM_COLORS 3
#define BIT_DEPTH 8

#define MAX_ZOOM_IN  (16.0)
#define MAX_ZOOM_OUT (1/8.0)
#define ZOOM_INCREMENT (2.0)

#define MANUF_NAME_AV "Allied Vision"

//#define PROGRAM_NAME    "Video4Linux2 Testtool"
//#define PROGRAM_VERSION "v1.35"
#define APP_NAME    "Video4Linux2 Testtool"
#define APP_VERSION_MAJOR   1
#define APP_VERSION_MINOR   36
#define APP_VERSION_PATCH   0
#ifndef SVN_REV
#define SVN_REV             0
#endif


/*
* 1.0: base version
* 1.1: Horizontal and vertical image flip
* 1.2: Framerate implementation
* 1.3: Frameinterval implementation instead of Framerate
* 1.4: Fix for select MMAP, USERPTR
* 1.5: revert Fix and rename the GUI controls to make handling more clear.
		FrameObserver issue didn't queue all frames
* 1.6: VIDIOC_TRY_FMT set formats first again
* 1.7: Queue buffer even though buffer has errors
		put card name into camera list to destinguish the cameras
* 1.8: V4L2_PIX_FMT_GREY conversion added.
* 1.9: Cropping added
* 1.10: Update camera formats after set cropping
		bayer8 to rgb24
		VIDIOC_TRY_FMT return value wrong check
* 1.11: AVTImageTransform added
* 1.12: ImageTransform use bug in VmbImageTransformHelper fix
* 1.13: FrameObserver ReadFrame DQBUF return value watch added
* 1.14: Crop set get and capabilities improved
* 1.15: Read all Values button added to update the shown values
* 1.16: Additional pixelformat conversions
* 1.17: Under Options new log options added
* 1.18: Compare with CSV file added
* 1.19: Fixed bug when app is closed unexpected
* 1.20: Added configurable stream toggle delays
* 1.21: user buffer list now threadsafe
* 1.22: VIDIOC_S_FMT reads back the resolution if result is OK.
		The resolution could have been adjusted.
		RAW10 and RAW12 conversion bug fix
* 1.23: threadsafety fix is buggy rollback
* 1.24: second try for threadsafety
		frame byte dump to disk added
* 1.25: Memoryleak fixed
		Message Listbox can be disabled now
		number of stream now available in GUI
* 1.26: VIDIOC_TRY_FMT can be now switched off
* 1.27: Extended Controls added
	min max default values added to outputbox
* 1.28: Extended controls get by camera class id
	get standard controls by user class id
	controls converter id -> string added
* 1.29: image conversion from RAW10 in RAW16 to RAW10g implemented,
		because IMX serves only this weird format.
* 1.30: IO Read added and changed the GUI
* 1.31: Modifications of IO Read
* 1.32: Omnivision SBGGR8 10 and 12 added to image conversion but wrong colors
* 1.33: VIDIOC_DQBUF errno EAGAIN disregarded in nonblocking mode
		emit framecount display in FrameObserver 369 disabled.
* 1.34: EnumAllControlOldStyle and NewStyle added. There is a difference
		between these two approaches.
* 1.35  Starts maximized
		Added svn revision number
		Camera::EnumAllControlNewStyle and Camera::EnumAllControlOldStyle now display number of found controls
* 1.36  Fixed OCT-2544: Display of negative feature values for sharpness & hue.
		Changed GUI layout
		Fixed some code indention
*/

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

v4l2test::v4l2test(QWidget *parent, Qt::WindowFlags flags, int viewerNumber)
	: QMainWindow(parent, flags)
	, m_nViewerNumber(viewerNumber)
	, m_bIsOpen(false)
	, m_bIsStreaming(false)
	, m_dFitToScreen(false)
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

	Logger::SetPCIeLogger("v4l2testLog.log");

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
	connect(&m_Camera, SIGNAL(OnCameraRecordFrame_Signal(const unsigned long long &, const unsigned long long &)), this, SLOT(OnCameraRecordFrame(const unsigned long long &, const unsigned long long &)));
	connect(&m_Camera, SIGNAL(OnCameraPixelformat_Signal(const QString &)), this, SLOT(OnCameraPixelformat(const QString &)));
	connect(&m_Camera, SIGNAL(OnCameraFramesize_Signal(const QString &)), this, SLOT(OnCameraFramesize(const QString &)));

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
	//connect(ui.m_chkBlockingMode, SIGNAL(clicked()), this, SLOT(OnBlockingMode()));
	//connect(ui.m_TitleBlockingMode, SIGNAL(triggered()), this, SLOT(OnBlockingMode()));
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
	connect(ui.m_BigEndianessRadio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_LittleEndianessRadio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_DataUintRadio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_DataIntRadio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_DataHexRadio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_DataFloatRadio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_DataType32Radio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	connect(ui.m_DataType64Radio, SIGNAL(clicked()), this, SLOT(OnDirectRegisterAccessUpdateData()));
	OnDirectRegisterAccessUpdateData();

	// connect the buttons for the Recording
	connect(ui.m_StartRecordButton, SIGNAL(clicked()), this, SLOT(OnStartRecording()));
	connect(ui.m_StopRecordButton, SIGNAL(clicked()), this, SLOT(OnStopRecording()));
	connect(ui.m_DeleteRecording, SIGNAL(clicked()), this, SLOT(OnDeleteRecording()));
	connect(ui.m_DisplaySaveFrame, SIGNAL(clicked()), this, SLOT(OnSaveFrame()));
	connect(ui.m_ExportRecordedFrameButton, SIGNAL(clicked()), this, SLOT(OnExportFrame()));
	connect(ui.m_SaveFrameSeriesButton, SIGNAL(clicked()), this, SLOT(OnSaveFrameSeries()));
	connect(ui.m_CalcDeviationButton, SIGNAL(clicked()), this, SLOT(OnCalcDeviation()));
	connect(ui.m_GetReferenceButton, SIGNAL(clicked()), this, SLOT(OnGetReferenceImage()));
	connect(ui.m_FrameRecordTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
	        this, SLOT(OnRecordTableSelectionChanged(const QItemSelection &, const QItemSelection &)));


	// disable table editing from user
	ui.m_FrameRecordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.m_FrameRecordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.m_FrameRecordTable->setSelectionMode(QAbstractItemView::SingleSelection);
	qRegisterMetaType<std::map<unsigned int, double> >("std::map<unsigned int, double>");

	// connect the buttons for Image m_ControlRequestTimer
	connect(ui.m_edWidth, SIGNAL(returnPressed()), this, SLOT(OnWidth()));
	connect(ui.m_edHeight, SIGNAL(returnPressed()), this, SLOT(OnHeight()));
	connect(ui.m_edPixelformat, SIGNAL(returnPressed()), this, SLOT(OnPixelformat()));
	connect(ui.m_edGain, SIGNAL(returnPressed()), this, SLOT(OnGain()));
	connect(ui.m_chkAutoGain, SIGNAL(clicked()), this, SLOT(OnAutoGain()));
	connect(ui.m_edExposure, SIGNAL(returnPressed()), this, SLOT(OnExposure()));
	connect(ui.m_edExposureAbs, SIGNAL(returnPressed()), this, SLOT(OnExposureAbs()));
	connect(ui.m_chkAutoExposure, SIGNAL(clicked()), this, SLOT(OnAutoExposure()));
	connect(ui.m_liPixelformats, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnPixelformatDBLClick(QListWidgetItem *)));
	connect(ui.m_liFramesizes, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnFramesizesDBLClick(QListWidgetItem *)));
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
	connect(ui.m_edFramerate, SIGNAL(returnPressed()), this, SLOT(OnFramerate()));
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
	//m_CSVFileLineEdit->setText("/home/ubuntu/Desktop/1296x968_Mono12p.csv");
	//m_CSVFileLineEdit->setText("/home/ubuntu/Desktop/1296x968_Mono10p.csv");

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

	/*
	// get available image formats
	// and add them to combobox
	QStringList imageFormats = GetImageFormats();
	if( imageFormats.size() )
	{
		ui.m_SelectFileExtensionComboBox->addItems( imageFormats );
	}
	*/

	// setup table widgets for record listing
	InitializeTableWidget();
}

v4l2test::~v4l2test()
{
	m_Camera.DeviceDiscoveryStop();

	// if we are streaming stop streaming
	if ( true == m_bIsOpen )
		OnOpenCloseButtonClicked();

}

// The event handler to close the program
void v4l2test::OnMenuCloseTriggered()
{
	close();
}

void v4l2test::OnBlockingMode(QAbstractButton* button)
{
	m_BLOCKING_MODE = ui.m_radioBlocking->isChecked();
	OnLog(QString("Use BLOCKING_MODE = %1").arg((m_BLOCKING_MODE) ? "TRUE" : "FALSE"));

	//ui.m_chkBlockingMode->setChecked(m_BLOCKING_MODE);
	//ui.m_BlockingMode->setChecked(m_BLOCKING_MODE);
	//ui.m_TitleBlockingMode->setChecked(m_BLOCKING_MODE);
}

void v4l2test::OnUseRead()
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

void v4l2test::OnUseMMAP()
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

void v4l2test::OnUseUSERPTR()
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

void v4l2test::OnUseVIDIOC_TRY_FMT()
{
	m_VIDIOC_TRY_FMT = !m_VIDIOC_TRY_FMT;
	OnLog(QString("Use VIDIOC_TRY_FMT = %1").arg((m_VIDIOC_TRY_FMT) ? "TRUE" : "FALSE"));

	ui.m_TitleEnable_VIDIOC_TRY_FMT->setChecked(m_VIDIOC_TRY_FMT);
}

void v4l2test::OnUseExtendedControls()
{
	m_ExtendedControls = !m_ExtendedControls;
	OnLog(QString("Use Extended Controls = %1").arg((m_ExtendedControls) ? "TRUE" : "FALSE"));

	ui.m_TitleEnableExtendedControls->setChecked(m_ExtendedControls);
}

void v4l2test::OnShowFrames()
{
	m_ShowFrames = !m_ShowFrames;
	OnLog(QString("Show Frames = %1").arg((m_ShowFrames) ? "TRUE" : "FALSE"));

	m_Camera.SwitchFrameTransfer2GUI(m_ShowFrames);
}

void v4l2test::OnClearOutputListbox()
{
	ui.m_LogTextEdit->clear();
}

void v4l2test::OnLogToFile()
{
	Logger::LogSwitch(ui.m_TitleLogtofile->isChecked());
}

void v4l2test::RemoteClose()
{
	if ( true == m_bIsOpen )
		OnOpenCloseButtonClicked();
}

// The event handler to open a next viewer
void v4l2test::OnMenuOpenNextViewer()
{
	QSharedPointer<v4l2test> viewer(new v4l2test(0, 0, (int)(m_pViewer.size() + 2)));
	m_pViewer.push_back(viewer);
	viewer->show();
}

void v4l2test::closeEvent(QCloseEvent *event)
{
	if (VIEWER_MASTER == m_nViewerNumber)
	{
		std::list<QSharedPointer<v4l2test> >::iterator xIter;
		for (xIter = m_pViewer.begin(); xIter != m_pViewer.end(); xIter++)
		{
			QSharedPointer<v4l2test> viewer = *xIter;
			viewer->RemoteClose();
			viewer->close();
		}
		QApplication::quit();
	}

	event->accept();
}

void v4l2test::wheelEvent(QWheelEvent *event)
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

void v4l2test::mousePressEvent(QMouseEvent *event)
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
void v4l2test::OnOpenCloseButtonClicked()
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
	//ui.m_chkBlockingMode->setEnabled( !m_bIsOpen );
	//ui.m_TitleBlockingMode->setEnabled( !m_bIsOpen );
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
	}
	else
	{
		OnStopRecording();
		ui.m_StartRecordButton->setEnabled(false);
	}
}

// The event handler for get device info
void v4l2test::OnGetDeviceInfoButtonClicked()
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
void v4l2test::OnGetStreamStatisticsButtonClicked()
{
	std::string tmp;

	OnLog("---------------------------------------------");
	OnLog("---- Stream Statistics ");
	OnLog("---------------------------------------------");

	//TODO m_Camera.GetStreamStatistics(tmp);
	OnLog("Not implemented");

	OnLog("---------------------------------------------");
}

// The event handler for starting
void v4l2test::OnStartButtonClicked()
{
	uint32_t payloadsize = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t pixelformat = 0;
	uint32_t bytesPerLine = 0;
	QString pixelformatText;
	int result = 0;

	// Get the payloadsize first to setup the streaming channel
	result = m_Camera.ReadPayloadsize(payloadsize);
	OnLog(QString("Received Payloadsize = %1").arg(payloadsize));
	result = m_Camera.ReadFrameSize(width, height);
	OnLog(QString("Received Width = %1").arg(width));
	OnLog(QString("Received Height = %1").arg(height));
	result = m_Camera.ReadPixelformat(pixelformat, bytesPerLine, pixelformatText);
	OnLog(QString("Received Pixelformat = %1 = 0x%2 = %3").arg(pixelformat).arg(pixelformat, 8, 16, QChar('0')).arg(pixelformatText));

	if (result == 0)
		StartStreaming(pixelformat, payloadsize, width, height, bytesPerLine);
}

void v4l2test::OnToggleButtonClicked()
{
	uint32_t payloadsize = 0;
	int result = 0;

	// Get the payloadsize first to setup the streaming channel
	result = m_Camera.ReadPayloadsize(payloadsize);

	OnLog(QString("Received Payloadsize = %1").arg(payloadsize));

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

void v4l2test::OnStreamToggleTimeout()
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

void v4l2test::OnCameraRegisterValueReady(unsigned long long value)
{
	OnLog(QString("Received value = %1").arg(value));
}

void v4l2test::OnCameraError(const QString &text)
{
	OnLog(QString("Error = %1").arg(text));
}

void v4l2test::OnCameraWarning(const QString &text)
{
	OnLog(QString("Warning = %1").arg(text));
}

void v4l2test::OnCameraMessage(const QString &text)
{
	OnLog(QString("Message = %1").arg(text));
}

// Event will be called when the a frame is recorded
void v4l2test::OnCameraRecordFrame(const unsigned long long &frameID, const unsigned long long &framesInQueue)
{
	ui.m_FrameIDRecording->setText(QString("FrameID: %1").arg(frameID));
	ui.m_FramesInQueue->setText(QString("Frames in Queue: %1").arg(framesInQueue));

	if (1 == framesInQueue)
		ui.m_FrameIDStartedRecord->setText(QString("FrameID start: %1").arg(frameID));
	ui.m_FrameIDStoppedRecord->setText(QString("FrameID stopped: %1").arg(frameID));

	UpdateRecordTableWidget();
}

void v4l2test::OnCameraPixelformat(const QString& format)
{
	ui.m_liPixelformats->addItem(format);
}

void v4l2test::OnCameraFramesize(const QString& size)
{
	ui.m_liFramesizes->addItem(size);
}

void v4l2test::StartStreaming(uint32_t pixelformat, uint32_t payloadsize, uint32_t width, uint32_t height, uint32_t bytesPerLine)
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
	err = m_Camera.StartStreamChannel(m_CSVFileLineEdit->text().toStdString().c_str(),
	                                  pixelformat,
	                                  payloadsize,
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

	if (m_Camera.CreateUserBuffer(m_NumberOfUsedFramesLineEdit->text().toLong(), payloadsize) == 0)
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

	}
	else
	{
		OnLog("Creating user buffers failed.");
	}
}

void v4l2test::InitializeTableWidget()
{
	// set number of columns and rows
	ui.m_FrameRecordTable->setRowCount(0);
	ui.m_FrameRecordTable->setColumnCount(9);

	// set table headers
	QStringList header;
	header << "Frame ID" << "Buffer index" << "Width" << "Height" << "Payload size" << "Pixelformat" << "Buffer length" << "Bytes per line" << "Deviation from Reference Image";
	ui.m_FrameRecordTable->setHorizontalHeaderLabels(header);
	ui.m_FrameRecordTable->resizeColumnsToContents();
}

// Returns the frame object that is selected in the gui table, or a null pointer if nothing is selected
QSharedPointer<MyFrame> v4l2test::getSelectedRecordedFrame()
{
	QSharedPointer<MyFrame> result;
	QList<QTableWidgetItem*> selectedItems = ui.m_FrameRecordTable->selectedItems();

	if (selectedItems.size() > 0)
	{
		QTableWidgetItem* item = selectedItems[0];
		int index = item->row();
		QVector<QSharedPointer<MyFrame> > recordedFrames = m_Camera.GetRecordVector();

		if (index >= 0 && index < recordedFrames.size())
		{
			result = recordedFrames[index];
		}
	}

	return result;
}

void v4l2test::UpdateRecordTableWidget()
{
	QVector<QSharedPointer<MyFrame> > recordedFrames = m_Camera.GetRecordVector();

	ui.m_FrameRecordTable->setRowCount(recordedFrames.size());

	for (int i = 0; i < recordedFrames.size(); i++)
	{
		QSharedPointer<MyFrame> frame = recordedFrames.at(i);

		// get frame information
		unsigned long long frameId = frame->GetFrameId();
		uint32_t bufferIndex = frame->GetBufferIndex();
		uint32_t width = frame->GetWidth();
		uint32_t height = frame->GetHeight();
		uint32_t payloadSize = frame->GetPayloadSize();
		uint32_t pixelFormat = frame->GetPixelformat();
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
		item_pixelFormat->setText(QString::fromStdString(V4l2Helper::ConvertPixelformat2EnumString(pixelFormat)));
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
void v4l2test::OnStopButtonClicked()
{
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

	// disable the stop button to show that the stop acquisition is in process
	ui.m_StopButton->setEnabled(false);

	int err = m_Camera.StopStreamChannel();
	if (0 != err)
		OnLog("Stop stream channel failed.");

	QApplication::processEvents();

	m_bIsStreaming = false;
	UpdateViewerLayout();
	OnLog("Stream channel stopped ...");

	m_FramesReceivedTimer.stop();

	m_Camera.DeleteUserBuffer();
}

// The event handler to resize the image to fit to window
void v4l2test::OnZoomFitButtonClicked()
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
void v4l2test::OnZoomInButtonClicked()
{
	m_dScaleFactor *= ZOOM_INCREMENT;

	QTransform transformation;
	transformation.scale(m_dScaleFactor, m_dScaleFactor);
	ui.m_ImageView->setTransform(transformation);

	UpdateZoomButtons();
}

// The event handler for resize the image
void v4l2test::OnZoomOutButtonClicked()
{
	m_dScaleFactor *= (1 / ZOOM_INCREMENT);

	QTransform transformation;
	transformation.scale(m_dScaleFactor, m_dScaleFactor);
	ui.m_ImageView->setTransform(transformation);

	UpdateZoomButtons();
}

// The event handler to show the processed frame
void v4l2test::OnFrameReady(const QImage &image, const unsigned long long &frameId)
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
void v4l2test::OnFrameID(const unsigned long long &frameId)
{
	ui.m_FrameIdLabel->setText(QString("FrameID: %1").arg(frameId));
}

// The event handler to show the event data
void v4l2test::OnCameraEventReady(const QString &eventText)
{
	OnLog("Event received.");

	ui.m_EventsLogEditWidget->appendPlainText(eventText);
	ui.m_EventsLogEditWidget->verticalScrollBar()->setValue(ui.m_EventsLogEditWidget->verticalScrollBar()->maximum());
}

// The event handler to clear the event list
void v4l2test::OnClearEventLogButtonClicked()
{
	ui.m_EventsLogEditWidget->clear();
}

// This event handler is triggered through a Qt signal posted by the camera observer
void v4l2test::OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
	bool bUpdateList = false;

	// We only react on new cameras being found and known cameras being unplugged
	if (UpdateTriggerPluggedIn == reason)
	{
		bUpdateList = true;

		std::string manuName;
		std::string devName = deviceName.toAscii().data();
		OnLog(QString("Camera list changed. A new camera was discovered, number=%1, deviceID=%2, ManufName=%3.").arg(cardNumber).arg(deviceID).arg(manuName.c_str()));
	}
	else if (UpdateTriggerPluggedOut == reason)
	{
		OnLog(QString("Camera list changed. A camera was disconnected, number=%1, deviceID=%2.").arg(cardNumber).arg(deviceID));
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
	//ui.m_chkBlockingMode->setEnabled( !m_bIsOpen );
	//ui.m_TitleBlockingMode->setEnabled( !m_bIsOpen );
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
void v4l2test::OnListBoxCamerasItemDoubleClicked(QListWidgetItem * item)
{
	OnOpenCloseButtonClicked();
}

// Queries and lists all known camera
void v4l2test::UpdateCameraListBox(uint32_t cardNumber, uint64_t cameraID, const QString &deviceName, const QString &info)
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
	//ui.m_chkBlockingMode->setEnabled( !m_bIsOpen );
	//ui.m_TitleBlockingMode->setEnabled( !m_bIsOpen );
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
void v4l2test::UpdateViewerLayout()
{
	m_NumberOfUsedFramesWidgetAction->setEnabled(!m_bIsOpen);

	if (!m_bIsOpen)
	{
		QPixmap pix(":/v4l2test/Viewer.png");

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
void v4l2test::UpdateZoomButtons()
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
void v4l2test::OnLog(const QString &strMsg)
{
	if (ui.m_TitleEnableMessageListbox->isChecked())
	{
		ui.m_LogTextEdit->appendPlainText(strMsg);
		ui.m_LogTextEdit->verticalScrollBar()->setValue(ui.m_LogTextEdit->verticalScrollBar()->maximum());
	}
}

// Open/Close the camera
int v4l2test::OpenAndSetupCamera(const uint32_t cardNumber, const QString &deviceName)
{
	int err = 0;

	std::string devName = deviceName.toStdString();
	err = m_Camera.OpenDevice(devName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT, m_ExtendedControls);

	if (0 != err)
		OnLog("Open device failed");
	else
		m_nStreamNumber = 0;

	return err;
}

int v4l2test::CloseCamera(const uint32_t cardNumber)
{
	int err = 0;

	err = m_Camera.CloseDevice();
	if (0 != err)
		OnLog("Close device failed.");

	return err;
}

// The event handler to show the frames received
void v4l2test::OnUpdateFramesReceived()
{
	unsigned int fpsReceived = m_Camera.GetReceivedFramesCount();
	unsigned int fpsRendered = m_Camera.GetRenderedFramesCount();
	unsigned int uncompletedFrames = m_Camera.GetDroppedFramesCount() + m_nDroppedFrames;

	ui.m_FramesPerSecondLabel->setText(QString("fps: %1 received/%2 rendered [drops %3]").arg(fpsReceived).arg(fpsRendered).arg(uncompletedFrames));
}

// The event handler to show the frames received
void v4l2test::OnControllerResponseTimeout()
{
	OnLog("Payload request timed out. Cannot start streaming.");

	m_ControlRequestTimer.stop();
}

// The event handler to read a register per direct access
void v4l2test::OnDirectRegisterAccessReadButtonClicked()
{
	// define the base of the given string
	int base = 10;
	if (0 <= ui.m_DirectRegisterAccessAddressLineEdit->text().indexOf("x"))
	{
		base = 16;
	}
	// convert the register address value
	bool converted;
	uint64_t address = ui.m_DirectRegisterAccessAddressLineEdit->text().toLongLong(&converted, base);

	if (converted)
	{
		uint64_t value;
		// read the register from the defined address
		//m_Camera.ReadRegister(address, 4, value);
		OnDirectRegisterAccessUpdateData();
		OnLog(QString("Register '%1' read returned %2.").arg(address).arg(value));
	}
	else
	{
		OnLog("Could not convert the register address from the given string.");
	}
}

// The event handler to write a register per direct access
void v4l2test::OnDirectRegisterAccessWriteButtonClicked()
{
	// define the base of the given strings
	int baseAddress = 10;
	if (0 <= ui.m_DirectRegisterAccessAddressLineEdit->text().indexOf("x"))
	{
		baseAddress = 16;
	}

	// convert the register address value
	bool converted;
	uint64_t address = ui.m_DirectRegisterAccessAddressLineEdit->text().toLongLong(&converted, baseAddress);

	if (converted)
	{
		uint64_t data;

		// Hex display interpretation
		if (ui.m_DataHexRadio->isChecked())
		{
			data = ui.m_DirectRegisterAccessDataLineEdit->text().toLongLong(&converted, 16);
		}
		// unsigned integer display interpretation
		else if (ui.m_DataUintRadio->isChecked())
		{
			data = ui.m_DirectRegisterAccessDataLineEdit->text().toULongLong(&converted, 10);
		}
		// integer display interpretation
		else if (ui.m_DataIntRadio->isChecked())
		{
			data = ui.m_DirectRegisterAccessDataLineEdit->text().toLongLong(&converted, 10);
		}
		// float display interpretation
		else
		{
			float value = ui.m_DirectRegisterAccessDataLineEdit->text().toFloat(&converted);
			data = (uint64_t)reinterpret_cast<uint64_t &>(value);
		}

		if (!ui.m_LittleEndianessRadio->isChecked())
		{
			/*
						if (ui.m_DataType32Radio->isChecked())
						{
							data = (uint32_t)qFromBigEndian((uint32_t)data);
						}
						else
						{
							data = qFromBigEndian(data);
						}
			*/
		}

		if (converted)
		{
			m_DirectAccessData = data;
			// write the register to the defined address
			//m_Camera.WriteRegister(address, data);
			OnDirectRegisterAccessUpdateData();
			OnLog(QString("Register '%1' written %2.").arg(address).arg(data));
		}
		else
		{
			OnLog("Could not convert the register data from the given string.");
		}
	}
	else
	{
		OnLog("Could not convert the register address from the given string.");
	}
}

// The event handler to update the direct register access value
void v4l2test::OnDirectRegisterAccessUpdateData()
{
	QString strValue("");
	uint64_t value64u = m_DirectAccessData;
	int64_t value64s = reinterpret_cast<int64_t &>(value64u);
	uint32_t value32u = reinterpret_cast<uint32_t &>(value64u);
	int32_t value32s = reinterpret_cast<int32_t &>(value64u);

	// big or little endian
	if (!ui.m_LittleEndianessRadio->isChecked())
	{
		/*
				value64u = qFromBigEndian(value64u);
				value64s = qFromBigEndian(value64s);
				value32u = qFromBigEndian(value32u);
				value32s = qFromBigEndian(value32s);
		*/
	}

	// Hex display interpretation
	if (ui.m_DataHexRadio->isChecked())
	{
		if (ui.m_DataType32Radio->isChecked())
		{
			strValue = QString("0x%1").arg(value32u, 8, 16, QChar('0'));
		}
		else
		{
			strValue = QString("0x%1").arg(value64u, 16, 16, QChar('0'));
		}
	}
	// unsigned integer interpretation
	else if (ui.m_DataUintRadio->isChecked())
	{
		if (ui.m_DataType32Radio->isChecked())
		{
			strValue = QString("%1").arg(value32u);
		}
		else
		{
			strValue = QString("%1").arg(value64u);
		}
	}
	// integer display interpretation
	else if (ui.m_DataIntRadio->isChecked())
	{
		if (ui.m_DataType32Radio->isChecked())
		{
			strValue = QString("%1").arg(value32s);
		}
		else
		{
			strValue = QString("%1").arg(value64s);
		}
	}
	// float display interpretation
	else
	{
		strValue = QString("%1").arg(reinterpret_cast<float &>(value32u));
	}

	ui.m_DirectRegisterAccessDataLineEdit->setText(strValue);
}

void v4l2test::OnStartRecording()
{
	m_Camera.SetRecording(true);

	ui.m_StartRecordButton->setEnabled(false);
	ui.m_StopRecordButton->setEnabled(true);
	ui.m_CalcDeviationButton->setEnabled(false);
	ui.m_DeleteRecording->setEnabled(false);
	ui.m_SaveFrameSeriesButton->setEnabled(false);
}

void v4l2test::OnStopRecording()
{
	m_Camera.SetRecording(false);

	UpdateRecordTableWidget();

	ui.m_StartRecordButton->setEnabled(true);
	ui.m_StopRecordButton->setEnabled(false);

	if (m_Camera.GetRecordVector().size() > 0)
	{
		ui.m_DeleteRecording->setEnabled(true);
		ui.m_SaveFrameSeriesButton->setEnabled(true);

		if (m_ReferenceImage)
		{
			ui.m_CalcDeviationButton->setEnabled(true);
		}
	}
}

void v4l2test::OnDeleteRecording()
{
	OnStopRecording();

	m_Camera.DeleteRecording();
	UpdateRecordTableWidget();

	ui.m_FrameIDRecording->setText(QString("FrameID: -"));
	ui.m_FrameIDStartedRecord->setText(QString("FrameID start: -"));
	ui.m_FrameIDStoppedRecord->setText(QString("FrameID stopped: -"));
	ui.m_FramesInQueue->setText(QString("Frames in Queue: 0"));

	if (m_Camera.GetRecordVector().size() == 0)
	{
		ui.m_DeleteRecording->setEnabled(false);
		ui.m_SaveFrameSeriesButton->setEnabled(false);
		ui.m_CalcDeviationButton->setEnabled(false);
		ui.m_meanDeviationLabel->setText(QString("Mean Deviation: -"));
	}
}

void v4l2test::OnRecordTableSelectionChanged(const QItemSelection &, const QItemSelection &)
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

void v4l2test::OnGetReferenceImage()
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
	m_ReferenceImageDialog->selectFilter( m_SelectedExtension );
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

				if (m_Camera.GetRecordVector().size())
				{
					ui.m_CalcDeviationButton->setEnabled(true);
				}
			}
			else
			{
				QMessageBox::warning( this, tr("V4L2 Test"), tr("Could not load image! \nCheck access rights."), tr("") );
			}

		}
	}
}


void v4l2test::SaveFrame(QSharedPointer<MyFrame> frame, QString fileName, bool raw)
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


void v4l2test::SaveFrameDialog(bool raw)
{
	QSharedPointer<MyFrame> selectedFrame = getSelectedRecordedFrame();

	if (selectedFrame)
	{
		QString fileExtensions = raw ? ".raw" : ".png";

		QString pixelformat = QString::fromStdString(V4l2Helper::ConvertPixelformat2EnumString(selectedFrame->GetPixelformat()));
		QString width = QString::number(selectedFrame->GetWidth());
		QString height = QString::number(selectedFrame->GetHeight());
		QString frameId = QString::number(selectedFrame->GetFrameId());
		QString defaultFileName = QString("Frame") + frameId + QString("_") + width + QString("x") + height + QString("_") + pixelformat;

		if ( NULL != m_SaveFileDialog )
		{
			delete m_SaveFileDialog;
			m_SaveFileDialog = NULL;
		}

		m_SaveFileDialog = new QFileDialog ( this, tr((raw ? "Save Image" : "Export Image")), m_SaveFileDir + QString("/") + defaultFileName, fileExtensions );
		m_SaveFileDialog->selectFilter(m_SelectedExtension);
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

void v4l2test::OnSaveFrame()
{
	SaveFrameDialog(true);
}

void v4l2test::OnSaveFrameSeries()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), m_SaveFileDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	// check if user has hit the cancel button in the file dialog
	if (!dir.isEmpty() && !dir.isNull())
	{
		QVector<QSharedPointer<MyFrame> > recordedFrames = m_Camera.GetRecordVector();

		// iterate through table
		for (unsigned int tableRow = 0; tableRow < ui.m_FrameRecordTable->rowCount(); ++tableRow)
		{
			if (tableRow < recordedFrames.size())
			{
				QSharedPointer<MyFrame> frame = recordedFrames[tableRow];

				QString pixelformat = QString::fromStdString(V4l2Helper::ConvertPixelformat2EnumString(frame->GetPixelformat()));
				QString width = QString::number(frame->GetWidth());
				QString height = QString::number(frame->GetHeight());
				QString frameId = QString::number(frame->GetFrameId());
				QString filename = QString::number(tableRow) + QString("_Frame") + frameId + QString("_") + width + QString("x") + height + QString("_") + pixelformat + QString(".raw");
				QString absoluteFilePath = dir + QString("/") + filename;

				SaveFrame(frame, absoluteFilePath, true);
			}
		}
	}
}

void v4l2test::OnExportFrame()
{
	SaveFrameDialog(false);
}

void v4l2test::OnCalcDeviationReady(const std::map<unsigned int, double>& tableRowToDeviation)
{
	// clear loading animation
	ui.m_meanDeviationLabel->clear();
	delete m_LoadingAnimation;

	if (tableRowToDeviation.size() > 0)
	{
		double meanDeviation = 0;

		for (std::map<unsigned int, double>::const_iterator it = tableRowToDeviation.begin();
		        it != tableRowToDeviation.end();
		        ++it)
		{
			unsigned int row = it->first;
			double deviation = it->second;

			meanDeviation += deviation;

			QTableWidgetItem* item_deviation = new QTableWidgetItem();
			item_deviation->setText(QString::number(deviation));
			ui.m_FrameRecordTable->setItem(row, 8, item_deviation);
		}

		ui.m_meanDeviationLabel->setText(QString("Mean Deviation: %1 %").arg(meanDeviation / tableRowToDeviation.size() * 100));
	}
	else
	{
		ui.m_meanDeviationLabel->setText(QString("Mean Deviation: -"));
		QMessageBox::warning( this, tr("V4L2 Test"), tr("Invalid reference image!\nPlease make sure to use a reference image in RAW format\nwith the same resolution and pixelformat!"), tr("") );
	}

	ui.m_StartRecordButton->setEnabled(true);
	ui.m_CalcDeviationButton->setEnabled(true);
	m_CalcThread.clear();
}

void v4l2test::OnCalcDeviation()
{
	ui.m_StartRecordButton->setEnabled(false);
	ui.m_CalcDeviationButton->setEnabled(false);

	std::map<unsigned int, QSharedPointer<MyFrame> > tableRowToFrame;
	QVector<QSharedPointer<MyFrame> > recordedFrames = m_Camera.GetRecordVector();

	// iterate through table
	for (unsigned int tableRow = 0; tableRow < ui.m_FrameRecordTable->rowCount(); ++tableRow)
	{
		if (tableRow < recordedFrames.size())
		{
			QSharedPointer<MyFrame> frame = recordedFrames[tableRow];
			tableRowToFrame.insert(std::make_pair(tableRow, frame));
		}
	}

	// show loading animation
	m_LoadingAnimation = new QMovie(":/v4l2test/loading-animation.gif");
	ui.m_meanDeviationLabel->setMovie(m_LoadingAnimation);
	ui.m_meanDeviationLabel->show();
	m_LoadingAnimation->start();

	// calculate in separate thread
	m_CalcThread = QSharedPointer<DeviationCalculator>(new DeviationCalculator(m_ReferenceImage, tableRowToFrame));

	connect(m_CalcThread.data(), SIGNAL(OnCalcDeviationReady_Signal(const std::map<unsigned int, double>&)),
	        this, SLOT(OnCalcDeviationReady(const std::map<unsigned int, double>&)));

	m_CalcThread->start();
}

void v4l2test::OnWidth()
{
	if (m_Camera.SetFrameSize(ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt()) < 0)
	{
		uint32_t width = 0;
		uint32_t height = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Framesize!") );
		m_Camera.ReadFrameSize(width, height);
		ui.m_edWidth->setText(QString("%1").arg(width));
		ui.m_edHeight->setText(QString("%1").arg(height));
	}
	else
	{
		uint32_t payloadsize = 0;
		OnLog(QString("Framesize set to %1x%2").arg(ui.m_edWidth->text().toInt()).arg(ui.m_edHeight->text().toInt()));

		m_Camera.ReadPayloadsize(payloadsize);
		ui.m_edPayloadsize->setText(QString("%1").arg(payloadsize));
	}
}

void v4l2test::OnHeight()
{
	uint32_t width = 0;
	uint32_t height = 0;

	if (m_Camera.SetFrameSize(ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt()) < 0)
	{
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Framesize!") );
	}
	else
	{
		uint32_t payloadsize = 0;
		OnLog(QString("Framesize set to %1x%2").arg(ui.m_edWidth->text().toInt()).arg(ui.m_edHeight->text().toInt()));

		m_Camera.ReadPayloadsize(payloadsize);
		ui.m_edPayloadsize->setText(QString("%1").arg(payloadsize));
	}

	m_Camera.ReadFrameSize(width, height);
	ui.m_edWidth->setText(QString("%1").arg(width));
	ui.m_edHeight->setText(QString("%1").arg(height));
}

void v4l2test::OnPixelformat()
{
	if (m_Camera.SetPixelformat(ui.m_edPixelformat->text().toInt(), "") < 0)
	{
		uint32_t pixelformat = 0;
		uint32_t bytesPerLine = 0;
		QString pixelformatText;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Pixelformat!") );
		m_Camera.ReadPixelformat(pixelformat, bytesPerLine, pixelformatText);
		ui.m_edPixelformat->setText(QString("%1").arg(pixelformat));
		ui.m_edPixelformatText->setText(QString("%1").arg(pixelformatText));
	}
	else
	{
		uint32_t pixelformat = 0;
		uint32_t bytesPerLine = 0;
		QString pixelformatText;
		// Readback to verify
		m_Camera.ReadPixelformat(pixelformat, bytesPerLine, pixelformatText);
		ui.m_edPixelformat->setText(QString("%1").arg(pixelformat));
		ui.m_edPixelformatText->setText(QString("%1").arg(pixelformatText));

		OnLog(QString("Pixelformat set to %1").arg(ui.m_edPixelformat->text().toInt()));
	}
}

void v4l2test::OnGain()
{
	uint32_t gain = 0;
	bool autogain = false;

	m_Camera.SetGain(ui.m_edGain->text().toInt());

	if (m_Camera.ReadGain(gain) != -2)
	{
		ui.m_edGain->setEnabled(true);
		ui.m_edGain->setText(QString("%1").arg(gain));
	}
	else
		ui.m_edGain->setEnabled(false);
}

void v4l2test::OnAutoGain()
{
	uint32_t gain = 0;
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

void v4l2test::OnExposure()
{
	uint32_t exposure = 0;
	uint32_t exposureAbs = 0;
	bool autoexposure = false;

	m_Camera.SetExposure(ui.m_edExposure->text().toInt());

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
}

void v4l2test::OnExposureAbs()
{
	uint32_t exposure = 0;
	uint32_t exposureAbs = 0;
	bool autoexposure = false;

	m_Camera.SetExposureAbs(ui.m_edExposureAbs->text().toInt());

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
}

void v4l2test::OnAutoExposure()
{
	uint32_t exposure = 0;
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

void v4l2test::OnPixelformatDBLClick(QListWidgetItem *item)
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

	ui.m_edPixelformat->setText(QString("%1").arg(result));
	ui.m_edPixelformatText->setText(QString("%1").arg(V4l2Helper::ConvertPixelformat2EnumString(result).c_str()));
}

void v4l2test::OnFramesizesDBLClick(QListWidgetItem *item)
{
	QString tmp = item->text();
	QStringList list1 = tmp.split('x').first().split(':');
	QStringList list2 = tmp.split('x');

	ui.m_edWidth->setText(QString("%1").arg(list1.at(1)));
	ui.m_edHeight->setText(QString("%1").arg(list2.at(1)));
}

void v4l2test::OnGamma()
{
	if (m_Camera.SetGamma(ui.m_edGamma->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Gamma!") );
		m_Camera.ReadGamma(tmp);
		ui.m_edGamma->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("Gamma set to %1").arg(ui.m_edGamma->text().toInt()));

		m_Camera.ReadGamma(tmp);
		ui.m_edGamma->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnReverseX()
{
	if (m_Camera.SetReverseX(ui.m_edReverseX->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE ReverseX!") );
		m_Camera.ReadReverseX(tmp);
		ui.m_edReverseX->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("ReverseX set to %1").arg(ui.m_edReverseX->text().toInt()));

		m_Camera.ReadReverseX(tmp);
		ui.m_edReverseX->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnReverseY()
{
	if (m_Camera.SetReverseY(ui.m_edReverseY->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE ReverseY!") );
		m_Camera.ReadReverseY(tmp);
		ui.m_edReverseY->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("ReverseY set to %1").arg(ui.m_edReverseY->text().toInt()));

		m_Camera.ReadReverseY(tmp);
		ui.m_edReverseY->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnSharpness()
{
	if (m_Camera.SetSharpness(ui.m_edSharpness->text().toInt()) < 0)
	{
		int32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Sharpness!") );
		m_Camera.ReadSharpness(tmp);
		ui.m_edSharpness->setText(QString("%1").arg(tmp));
	}
	else
	{
		int32_t tmp = 0;
		OnLog(QString("Sharpness set to %1").arg(ui.m_edSharpness->text().toInt()));

		m_Camera.ReadSharpness(tmp);
		ui.m_edSharpness->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnBrightness()
{
	if (m_Camera.SetBrightness(ui.m_edBrightness->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Brightness!") );
		m_Camera.ReadBrightness(tmp);
		ui.m_edBrightness->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("Brightness set to %1").arg(ui.m_edBrightness->text().toInt()));

		m_Camera.ReadBrightness(tmp);
		ui.m_edBrightness->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnContrast()
{
	if (m_Camera.SetContrast(ui.m_edContrast->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Contrast!") );
		m_Camera.ReadContrast(tmp);
		ui.m_edContrast->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("Contrast set to %1").arg(ui.m_edContrast->text().toInt()));

		m_Camera.ReadContrast(tmp);
		ui.m_edContrast->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnSaturation()
{
	if (m_Camera.SetSaturation(ui.m_edSaturation->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Saturation!") );
		m_Camera.ReadSaturation(tmp);
		ui.m_edSaturation->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("Saturation set to %1").arg(ui.m_edSaturation->text().toInt()));

		m_Camera.ReadSaturation(tmp);
		ui.m_edSaturation->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnHue()
{
	if (m_Camera.SetHue(ui.m_edHue->text().toInt()) < 0)
	{
		int32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Hue!") );
		m_Camera.ReadHue(tmp);
		ui.m_edHue->setText(QString("%1").arg(tmp));
	}
	else
	{
		int32_t tmp = 0;
		OnLog(QString("Hue set to %1").arg(ui.m_edHue->text().toInt()));

		m_Camera.ReadHue(tmp);
		ui.m_edHue->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnContinousWhiteBalance()
{
	m_Camera.SetContinousWhiteBalance(ui.m_chkContWhiteBalance->isChecked());
}

void v4l2test::OnWhiteBalanceOnce()
{
	m_Camera.DoWhiteBalanceOnce();
}

void v4l2test::OnRedBalance()
{
	if (m_Camera.SetRedBalance(ui.m_edRedBalance->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE RedBalance!") );
		m_Camera.ReadRedBalance(tmp);
		ui.m_edRedBalance->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("RedBalance set to %1").arg(ui.m_edRedBalance->text().toInt()));

		m_Camera.ReadRedBalance(tmp);
		ui.m_edRedBalance->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnBlueBalance()
{
	if (m_Camera.SetBlueBalance(ui.m_edBlueBalance->text().toInt()) < 0)
	{
		uint32_t tmp = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE BlueBalance!") );
		m_Camera.ReadBlueBalance(tmp);
		ui.m_edBlueBalance->setText(QString("%1").arg(tmp));
	}
	else
	{
		uint32_t tmp = 0;
		OnLog(QString("BlueBalance set to %1").arg(ui.m_edBlueBalance->text().toInt()));

		m_Camera.ReadBlueBalance(tmp);
		ui.m_edBlueBalance->setText(QString("%1").arg(tmp));
	}
}

void v4l2test::OnFramerate()
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t pixelformat = 0;
	uint32_t bytesPerLine = 0;
	QString pixelformatText;
	QString framerate = ui.m_edFramerate->text();
	QStringList framerateList = framerate.split('/');

	if (framerateList.size() < 2)
	{
		QMessageBox::warning( this, tr("Video4Linux"), tr("Missing parameter. Format: 1/100!") );
		return;
	}

	uint32_t numerator = framerateList.at(0).toInt();
	uint32_t denominator = framerateList.at(1).toInt();

	m_Camera.ReadFrameSize(width, height);
	m_Camera.ReadPixelformat(pixelformat, bytesPerLine, pixelformatText);

	if (m_Camera.SetFramerate(numerator, denominator) < 0)
	{
		uint32_t denominator = 0;
		uint32_t numerator = 0;
		QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Framerate!") );
		m_Camera.ReadFramerate(numerator, denominator, width, height, pixelformat);
		ui.m_edFramerate->setText(QString("%1/%2").arg(numerator).arg(denominator));
	}
	else
	{
		uint32_t denominator = 0;
		uint32_t numerator = 0;
		OnLog(QString("Framerate set to %1").arg(ui.m_edFramerate->text().toInt()));

		m_Camera.ReadFramerate(numerator, denominator, width, height, pixelformat);
		ui.m_edFramerate->setText(QString("%1/%2").arg(numerator).arg(denominator));
	}
}

void v4l2test::OnCropXOffset()
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

void v4l2test::OnCropYOffset()
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

void v4l2test::OnCropWidth()
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
}

void v4l2test::OnCropHeight()
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
}

void v4l2test::OnCropCapabilities()
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

void v4l2test::OnReadAllValues()
{
	GetImageInformation();
	UpdateCameraFormat();
}

void v4l2test::OnToggleStreamDelayRand()
{
	m_ToggleStreamDelayCheckBox->setChecked(false);
	m_ToggleStreamDelayRandCheckBox->setChecked(true);
}

void v4l2test::OnToggleStreamDelay()
{
	m_ToggleStreamDelayCheckBox->setChecked(true);
	m_ToggleStreamDelayRandCheckBox->setChecked(false);
}

////////////////////////////////////////////////////////////////////////
// Tools
////////////////////////////////////////////////////////////////////////

void v4l2test::GetImageInformation()
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t xOffset = 0;
	uint32_t yOffset = 0;
	uint32_t gain = 0;
	bool autogain = false;
	uint32_t exposure = 0;
	uint32_t exposureAbs = 0;
	bool autoexposure = false;
	int result = 0;
	uint32_t nUVal;
	int32_t nSVal;
	uint32_t numerator = 0;
	uint32_t denominator = 0;
	uint32_t pixelformat = 0;
	QString pixelformatText;
	uint32_t bytesPerLine = 0;


	ui.m_chkAutoGain->setChecked(false);
	ui.m_chkAutoExposure->setChecked(false);

	UpdateCameraFormat();

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

	nUVal = 0;
	if (m_Camera.ReadGamma(nUVal) != -2)
	{
		ui.m_edGamma->setEnabled(true);
		ui.m_edGamma->setText(QString("%1").arg(nUVal));
	}
	else
		ui.m_edGamma->setEnabled(false);

	nUVal = 0;
	if (m_Camera.ReadReverseX(nUVal) != -2)
	{
		ui.m_edReverseX->setEnabled(true);
		ui.m_edReverseX->setText(QString("%1").arg(nUVal));
	}
	else
		ui.m_edReverseX->setEnabled(false);

	nUVal = 0;
	if (m_Camera.ReadReverseY(nUVal) != -2)
	{
		ui.m_edReverseY->setEnabled(true);
		ui.m_edReverseY->setText(QString("%1").arg(nUVal));
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

	nUVal = 0;
	if (m_Camera.ReadBrightness(nUVal) != -2)
	{
		ui.m_edBrightness->setEnabled(true);
		ui.m_edBrightness->setText(QString("%1").arg(nUVal));
	}
	else
		ui.m_edBrightness->setEnabled(false);

	nUVal = 0;
	if (m_Camera.ReadContrast(nUVal) != -2)
	{
		ui.m_edContrast->setEnabled(true);
		ui.m_edContrast->setText(QString("%1").arg(nUVal));
	}
	else
		ui.m_edContrast->setEnabled(false);

	nUVal = 0;
	if (m_Camera.ReadSaturation(nUVal) != -2)
	{
		ui.m_edSaturation->setEnabled(true);
		ui.m_edSaturation->setText(QString("%1").arg(nUVal));
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

	nUVal = 0;
	if (m_Camera.ReadRedBalance(nUVal) != -2)
	{
		ui.m_edRedBalance->setEnabled(true);
		ui.m_edRedBalance->setText(QString("%1").arg(nUVal));
	}
	else
		ui.m_edRedBalance->setEnabled(false);

	nUVal = 0;
	if (m_Camera.ReadBlueBalance(nUVal) != -2)
	{
		ui.m_edBlueBalance->setEnabled(true);
		ui.m_edBlueBalance->setText(QString("%1").arg(nUVal));
	}
	else
		ui.m_edBlueBalance->setEnabled(false);

	nUVal = 0;
	m_Camera.ReadFrameSize(width, height);
	m_Camera.ReadPixelformat(pixelformat, bytesPerLine, pixelformatText);
	if (m_Camera.ReadFramerate(numerator, denominator, width, height, pixelformat) != -2)
	{
		ui.m_edFramerate->setEnabled(true);
		ui.m_edFramerate->setText(QString("%1/%2").arg(numerator).arg(denominator));
	}
	else
		ui.m_edFramerate->setEnabled(false);

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

void v4l2test::UpdateCameraFormat()
{
	int result = -1;
	uint32_t payloadsize = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t pixelformat = 0;
	QString pixelformatText;
	uint32_t bytesPerLine = 0;

	ui.m_liPixelformats->clear();
	ui.m_liFramesizes->clear();

	// Get the payloadsize first to setup the streaming channel
	result = m_Camera.ReadPayloadsize(payloadsize);
	ui.m_edPayloadsize->setText(QString("%1").arg(payloadsize));

	result = m_Camera.ReadFrameSize(width, height);
	ui.m_edWidth->setText(QString("%1").arg(width));
	ui.m_edHeight->setText(QString("%1").arg(height));

	result = m_Camera.ReadPixelformat(pixelformat, bytesPerLine, pixelformatText);
	ui.m_edPixelformat->setText(QString("%1").arg(pixelformat));
	ui.m_edPixelformatText->setText(QString("%1").arg(pixelformatText));

	result = m_Camera.ReadFormats();
}

// Check if IO Read was checked and remove it when not capable
void v4l2test::Check4IOReadAbility()
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

void v4l2test::SetTitleText(QString additionalText)
{
	if (VIEWER_MASTER == m_nViewerNumber)
		setWindowTitle(QString("%1 V%2.%3.%4.%5 - master view  %6").arg(APP_NAME).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH).arg(SVN_REV).arg(additionalText));
	else
		setWindowTitle(QString("%1 V%2.%3.%4.%5 - %6. viewer %7").arg(APP_NAME).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH).arg(SVN_REV).arg(m_nViewerNumber).arg(additionalText));


	/*
	*
	if (VIEWER_MASTER == m_nViewerNumber)
		setWindowTitle(QString("%1 %2 - master view  %3").arg(PROGRAM_NAME, PROGRAM_VERSION, additionalText));
	else
		setWindowTitle(QString("%1 %2 - %3. view  %4").arg(PROGRAM_NAME, PROGRAM_VERSION).arg(m_nViewerNumber).arg(additionalText));
	*/
}


