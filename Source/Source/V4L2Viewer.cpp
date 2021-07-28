/*=============================================================================
Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

Redistribution of this file, in original or modified form, without
prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

File:        V4L2Viewer.cpp

Description: This class is a main class of the application. It describes how
             the main window looks like and also performs all actions
             which takes place in the GUI.

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
#include "IntegerEnumerationControl.h"
#include "Integer64EnumerationControl.h"
#include "BooleanEnumerationControl.h"
#include "ButtonEnumerationControl.h"
#include "ListEnumerationControl.h"
#include "ListIntEnumerationControl.h"
#include "CustomGraphicsView.h"

#include <QtCore>
#include <QtGlobal>
#include <QStringList>
#include <QFontDatabase>

#include <ctime>
#include <limits>
#include <sstream>

#define NUM_COLORS 3
#define BIT_DEPTH 8

#define MANUF_NAME_AV       "Allied Vision"
#define APP_NAME            "Video4Linux2 Viewer"
#define APP_VERSION_MAJOR   1
#define APP_VERSION_MINOR   0
#define APP_VERSION_PATCH   0
#ifndef SCM_REVISION
#define SCM_REVISION        0
#endif

// CCI registers
#define CCI_GCPRM_16R                               0x0010
#define CCI_BCRM_16R                                0x0014

#define EXPOSURE_MAX_CHANGE 1800000000


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

V4L2Viewer::V4L2Viewer(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , m_BLOCKING_MODE(true)
    , m_MMAP_BUFFER(IO_METHOD_MMAP) // use mmap by default
    , m_VIDIOC_TRY_FMT(true) // use VIDIOC_TRY_FMT by default
    , m_ShowFrames(true)
    , m_bIsExposureAbsUsed(false)
    , m_nDroppedFrames(0)
    , m_nStreamNumber(0)
    , m_bIsOpen(false)
    , m_bIsStreaming(false)
    , m_sliderGainValue(0)
    , m_sliderBlackLevelValue(0)
    , m_sliderGammaValue(0)
    , m_sliderExposureValue(0)
    , m_bIsCropAvailable(false)
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

    // Connect GUI events with event handlers
    connect(ui.m_OpenCloseButton,             SIGNAL(clicked()),         this, SLOT(OnOpenCloseButtonClicked()));
    connect(ui.m_StartButton,                 SIGNAL(clicked()),         this, SLOT(OnStartButtonClicked()));
    connect(ui.m_StopButton,                  SIGNAL(clicked()),         this, SLOT(OnStopButtonClicked()));
    connect(ui.m_ZoomFitButton,               SIGNAL(clicked()),         this, SLOT(OnZoomFitButtonClicked()));
    connect(ui.m_ZoomInButton,                SIGNAL(clicked()),         this, SLOT(OnZoomInButtonClicked()));
    connect(ui.m_ZoomOutButton,               SIGNAL(clicked()),         this, SLOT(OnZoomOutButtonClicked()));
    connect(ui.m_SaveImageButton,             SIGNAL(clicked()),         this, SLOT(OnSaveImageClicked()));
    connect(ui.m_ExposureActiveButton,        SIGNAL(clicked()),         this, SLOT(OnExposureActiveClicked()));

    SetTitleText();

    // Start Camera
    connect(&m_Camera, SIGNAL(OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameReady_Signal(const QImage &, const unsigned long long &)),                                       this, SLOT(OnFrameReady(const QImage &, const unsigned long long &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameID_Signal(const unsigned long long &)),                                                          this, SLOT(OnFrameID(const unsigned long long &)));
    connect(&m_Camera, SIGNAL(OnCameraPixelFormat_Signal(const QString &)),                                                                 this, SLOT(OnCameraPixelFormat(const QString &)));

    connect(&m_Camera, SIGNAL(PassAutoExposureValue(int32_t)), this, SLOT(OnUpdateAutoExposure(int32_t)));
    connect(&m_Camera, SIGNAL(PassAutoGainValue(int32_t)), this, SLOT(OnUpdateAutoGain(int32_t)));

    connect(&m_Camera, SIGNAL(SendIntDataToEnumerationWidget(int32_t, int32_t, int32_t, int32_t, QString, QString, bool)),      this, SLOT(PassIntDataToEnumerationWidget(int32_t, int32_t, int32_t, int32_t, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SentInt64DataToEnumerationWidget(int32_t, int64_t, int64_t, int64_t, QString, QString, bool)),    this, SLOT(PassIntDataToEnumerationWidget(int32_t, int64_t, int64_t, int64_t, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendBoolDataToEnumerationWidget(int32_t, bool, QString, QString, bool)),                          this, SLOT(PassBoolDataToEnumerationWidget(int32_t, bool, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendButtonDataToEnumerationWidget(int32_t, QString, QString, bool)),                              this, SLOT(PassButtonDataToEnumerationWidget(int32_t, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendListDataToEnumerationWidget(int32_t, int32_t, QList<QString>, QString, QString, bool)),       this, SLOT(PassListDataToEnumerationWidget(int32_t, int32_t, QList<QString>, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendListIntDataToEnumerationWidget(int32_t, int32_t, QList<int64_t>, QString, QString, bool)),    this, SLOT(PassListDataToEnumerationWidget(int32_t, int32_t, QList<int64_t>, QString, QString, bool)));

    connect(&m_Camera, SIGNAL(SendSignalToUpdateWidgets()), this, SLOT(OnReadAllValues()));

    // Setup blocking mode radio buttons
    m_BlockingModeRadioButtonGroup = new QButtonGroup(this);
    m_BlockingModeRadioButtonGroup->setExclusive(true);

    connect(ui.m_sliderExposure,    SIGNAL(valueChanged(int)),  this, SLOT(OnSliderExposureValueChange(int)));
    connect(ui.m_sliderGain,        SIGNAL(valueChanged(int)),  this, SLOT(OnSliderGainValueChange(int)));
    connect(ui.m_sliderBlackLevel,  SIGNAL(valueChanged(int)),  this, SLOT(OnSliderBlackLevelValueChange(int)));
    connect(ui.m_sliderGamma,       SIGNAL(valueChanged(int)),  this, SLOT(OnSliderGammaValueChange(int)));
    connect(ui.m_sliderExposure,    SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));
    connect(ui.m_sliderGain,        SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));
    connect(ui.m_sliderBlackLevel,  SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));
    connect(ui.m_sliderGamma,       SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));

    connect(ui.m_TitleUseMMAP, SIGNAL(triggered()), this, SLOT(OnUseMMAP()));
    connect(ui.m_TitleUseUSERPTR, SIGNAL(triggered()), this, SLOT(OnUseUSERPTR()));

    connect(ui.m_DisplayImagesCheckBox, SIGNAL(clicked()), this, SLOT(OnShowFrames()));

    connect(ui.m_TitleLogtofile, SIGNAL(triggered()), this, SLOT(OnLogToFile()));
    connect(ui.m_TitleLangEnglish, SIGNAL(triggered()), this, SLOT(OnLanguageChange()));
    connect(ui.m_TitleLangGerman, SIGNAL(triggered()), this, SLOT(OnLanguageChange()));
    connect(ui.m_TitleSavePNG, SIGNAL(triggered()), this, SLOT(OnSavePNG()));
    connect(ui.m_TitleSaveRAW, SIGNAL(triggered()), this, SLOT(OnSaveRAW()));

    connect(ui.m_camerasListCheckBox, SIGNAL(clicked()), this, SLOT(OnCameraListButtonClicked()));
    connect(ui.m_Splitter1, SIGNAL(splitterMoved(int, int)), this, SLOT(OnMenuSplitterMoved(int, int)));

    connect(ui.m_ImageView, SIGNAL(UpdateZoomLabel()), this, SLOT(OnUpdateZoomLabel()));

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

    connect(ui.m_chkContWhiteBalance, SIGNAL(clicked()), this, SLOT(OnContinousWhiteBalance()));
    connect(ui.m_edFrameRate, SIGNAL(returnPressed()), this, SLOT(OnFrameRate()));
    connect(ui.m_edCropXOffset, SIGNAL(returnPressed()), this, SLOT(OnCropXOffset()));
    connect(ui.m_edCropYOffset, SIGNAL(returnPressed()), this, SLOT(OnCropYOffset()));
    connect(ui.m_edCropWidth, SIGNAL(returnPressed()), this, SLOT(OnCropWidth()));
    connect(ui.m_edCropHeight, SIGNAL(returnPressed()), this, SLOT(OnCropHeight()));

    m_pActiveExposureWidget = new ActiveExposureWidget();
    connect(m_pActiveExposureWidget, SIGNAL(SendInvertState(bool)), this, SLOT(PassInvertState(bool)));
    connect(m_pActiveExposureWidget, SIGNAL(SendActiveState(bool)), this, SLOT(PassActiveState(bool)));
    connect(m_pActiveExposureWidget, SIGNAL(SendLineSelectorValue(int32_t)), this, SLOT(PassLineSelectorValue(int32_t)));

    connect(ui.m_AllControlsButton, SIGNAL(clicked()), this, SLOT(ShowHideEnumerationControlWidget()));

    // Set the splitter stretch factors
    ui.m_Splitter1->setStretchFactor(0, 45);
    ui.m_Splitter1->setStretchFactor(1, 55);
    ui.m_Splitter2->setStretchFactor(0, 75);
    ui.m_Splitter2->setStretchFactor(1, 25);

    // Set the viewer scene
    m_pScene = QSharedPointer<QGraphicsScene>(new QGraphicsScene());

    m_PixmapItem = new QGraphicsPixmapItem();

    ui.m_ImageView->setScene(m_pScene.data());
    ui.m_ImageView->SetPixmapItem(m_PixmapItem);

    m_pScene->addItem(m_PixmapItem);

    ///////////////////// Number of frames /////////////////////
    // add the number of used frames option to the menu
    m_NumberOfUsedFramesLineEdit = new QLineEdit(this);
    m_NumberOfUsedFramesLineEdit->setText("5");
    m_NumberOfUsedFramesLineEdit->setValidator(new QIntValidator(1, 500000, this));

    // prepare the layout
    QHBoxLayout *layoutNum = new QHBoxLayout;
    QLabel *labelNum = new QLabel(tr("Number of used Frames:"));
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

    QHBoxLayout *layoutFixedFrameRate = new QHBoxLayout;
    QLabel *labelFixedFrameRate = new QLabel(tr("Fixed frame rate:") ,this);
    QWidget *widgetFixedFrameRate = new QWidget(this);
    m_NumberOfFixedFrameRateWidgetAction = new QWidgetAction(this);
    m_NumberOfFixedFrameRate = new QLineEdit("60", this);
    m_NumberOfFixedFrameRate->setValidator(new QIntValidator(5, 100000, this));
    layoutFixedFrameRate->addWidget(labelFixedFrameRate);
    layoutFixedFrameRate->addWidget(m_NumberOfFixedFrameRate);
    widgetFixedFrameRate->setLayout(layoutFixedFrameRate);
    widgetFixedFrameRate->setStyleSheet("QWidget{background:transparent; color:white;} QWidget::disabled{color:rgb(79,79,79);}");
    m_NumberOfFixedFrameRateWidgetAction->setDefaultWidget(widgetFixedFrameRate);

    // add about widget to the menu bar
    m_pAboutWidget = new AboutWidget(this);
    m_pAboutWidget->SetVersion(QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH));
    QWidgetAction *aboutWidgetAction = new QWidgetAction(this);
    aboutWidgetAction->setDefaultWidget(m_pAboutWidget);
    ui.m_MenuAbout->addAction(aboutWidgetAction);

    ui.menuBar->setNativeMenuBar(false);

    QMainWindow::showMaximized();

    UpdateViewerLayout();
    UpdateZoomButtons();


    // set check boxes state for mmap according to variable m_MMAP_BUFFER

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

    ui.m_camerasListCheckBox->setChecked(true);
    m_pEnumerationControlWidget = new ControlsHolderWidget();

    ui.m_ImageControlFrame->setEnabled(false);
    ui.m_FlipHorizontalCheckBox->setEnabled(false);
    ui.m_FlipVerticalCheckBox->setEnabled(false);
    ui.m_DisplayImagesCheckBox->setEnabled(false);
    ui.m_SaveImageButton->setEnabled(false);

    connect(ui.m_allFeaturesDockWidget, SIGNAL(topLevelChanged(bool)), this, SLOT(OnDockWidgetPositionChanged(bool)));
    ui.m_allFeaturesDockWidget->setWidget(m_pEnumerationControlWidget);
    ui.m_allFeaturesDockWidget->hide();
    ui.m_allFeaturesDockWidget->setStyleSheet("QDockWidget {"
                                                "titlebar-close-icon: url(:/V4L2Viewer/Cross128.png);"
                                                "titlebar-normal-icon: url(:/V4L2Viewer/resize4.png);}");

    Logger::LogEx(QString("V4L2Viewer git commit = %1").arg(GIT_VERSION).toStdString().c_str());
}

V4L2Viewer::~V4L2Viewer()
{
    m_Camera.DeviceDiscoveryStop();

    // if we are streaming stop streaming
    if ( true == m_bIsOpen )
        OnOpenCloseButtonClicked();

    delete m_pActiveExposureWidget;
}

// The event handler to close the program
void V4L2Viewer::OnMenuCloseTriggered()
{
    close();
}

void V4L2Viewer::OnUseMMAP()
{
    m_MMAP_BUFFER = IO_METHOD_MMAP;

    ui.m_TitleUseMMAP->setChecked(true);
    ui.m_TitleUseUSERPTR->setChecked(false);
}

void V4L2Viewer::OnUseUSERPTR()
{
    m_MMAP_BUFFER = IO_METHOD_USERPTR;

    ui.m_TitleUseMMAP->setChecked(false);
    ui.m_TitleUseUSERPTR->setChecked(true);
}

void V4L2Viewer::OnShowFrames()
{
    m_ShowFrames = !m_ShowFrames;
    m_Camera.SwitchFrameTransfer2GUI(m_ShowFrames);
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

void V4L2Viewer::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui.retranslateUi(this);
        m_pAboutWidget->UpdateStrings();
        SetTitleText();

        if (false == m_bIsOpen)
        {
            ui.m_OpenCloseButton->setText(QString(tr("Open Camera")));
        }
        else
        {
            ui.m_OpenCloseButton->setText(QString(tr("Close Camera")));
        }
    }
    else
    {
        QMainWindow::changeEvent(event);
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
                ui.m_ImageControlFrame->setEnabled(true);
                GetImageInformation();
                SetTitleText();
                ui.m_CamerasListBox->removeItemWidget(ui.m_CamerasListBox->item(nRow));
                CameraListCustomItem *newItem = new CameraListCustomItem(devName, this);
                QString devInfo = GetDeviceInfo();
                newItem->SetCameraInformation(devInfo);
                ui.m_CamerasListBox->item(nRow)->setSizeHint(newItem->sizeHint());
                ui.m_CamerasListBox->setItemWidget(ui.m_CamerasListBox->item(nRow), newItem);
                ui.m_Splitter1->setSizes(QList<int>{0,1});
                ui.m_camerasListCheckBox->setChecked(false);
                ui.m_ImageView->SetZoomAllowed(true);
                ui.m_FlipHorizontalCheckBox->setEnabled(true);
                ui.m_FlipVerticalCheckBox->setEnabled(true);
                ui.m_DisplayImagesCheckBox->setEnabled(true);
                ui.m_SaveImageButton->setEnabled(true);
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

            ui.m_ImageView->SetScaleFactorToDefault();
            ui.m_ImageView->SetZoomAllowed(false);

            ui.m_FlipHorizontalCheckBox->setEnabled(false);
            ui.m_FlipVerticalCheckBox->setEnabled(false);
            ui.m_DisplayImagesCheckBox->setEnabled(false);
            ui.m_SaveImageButton->setEnabled(false);

            err = CloseCamera(m_cameras[nRow]);
            if (0 == err)
            {
                ui.m_CamerasListBox->removeItemWidget(ui.m_CamerasListBox->item(nRow));
                CameraListCustomItem *newItem = new CameraListCustomItem(devName, this);
                ui.m_CamerasListBox->item(nRow)->setSizeHint(newItem->sizeHint());
                ui.m_CamerasListBox->setItemWidget(ui.m_CamerasListBox->item(nRow), newItem);
            }

            m_pEnumerationControlWidget->RemoveElements();
            ui.m_allFeaturesDockWidget->hide();

            SetTitleText();

            ui.m_ImageControlFrame->setEnabled(false);
        }

        if (false == m_bIsOpen)
        {
            ui.m_OpenCloseButton->setText(QString(tr("Open Camera")));
        }
        else
        {
            ui.m_OpenCloseButton->setText(QString(tr("Close Camera")));
        }

        UpdateViewerLayout();
    }

    ui.m_OpenCloseButton->setEnabled( 0 <= m_cameras.size() || m_bIsOpen );

    ui.m_TitleUseMMAP->setEnabled( !m_bIsOpen );
    ui.m_TitleUseUSERPTR->setEnabled( !m_bIsOpen );
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
    result = m_Camera.ReadFrameSize(width, height);
    result = m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);

    if (result == 0)
        StartStreaming(pixelFormat, payloadSize, width, height, bytesPerLine);
}

void V4L2Viewer::OnCameraPixelFormat(const QString& pixelFormat)
{
    ui.m_pixelFormats->blockSignals(true);
    ui.m_pixelFormats->addItem(pixelFormat);
    ui.m_pixelFormats->blockSignals(false);
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

void V4L2Viewer::OnUpdateAutoExposure(int32_t value)
{
    ui.m_edExposure->setText(QString::number(value));
}

void V4L2Viewer::OnUpdateAutoGain(int32_t value)
{
    ui.m_edGain->setText(QString::number(value));
}

void V4L2Viewer::OnSliderExposureValueChange(int value)
{
    int minSliderVal = ui.m_sliderExposure->minimum();
    int maxSliderVal = ui.m_sliderExposure->maximum();

    double minExp = log(m_MinimumExposure);
    double maxExp = log(m_MaximumExposure);

    double scale = (maxExp - minExp) / (maxSliderVal-minSliderVal);
    double outValue = exp(minExp + scale*(value-minSliderVal));

    if (!m_bIsExposureAbsUsed)
    {
        int32_t sliderExposureValue = static_cast<int32_t>(outValue*100000);
        ui.m_edExposure->setText(QString("%1").arg(sliderExposureValue));
        m_Camera.SetExposure(sliderExposureValue);
    }
    else
    {
        int32_t sliderExposureValue = static_cast<int32_t>(outValue);
        int64_t sliderExposureValue64 = static_cast<int64_t>(outValue*100000);
        ui.m_edExposure->setText(QString("%1").arg(sliderExposureValue64));
        m_Camera.SetExposureAbs(sliderExposureValue);
    }
}

void V4L2Viewer::OnSliderGainValueChange(int value)
{
    ui.m_edGain->setText(QString::number(value));
    m_sliderGainValue = static_cast<int32_t>(value);
    m_Camera.SetGain(m_sliderGainValue);
}

void V4L2Viewer::OnSliderGammaValueChange(int value)
{
    ui.m_edGamma->setText(QString::number(value));
    m_sliderGammaValue = static_cast<int32_t>(value);
    m_Camera.SetGamma(m_sliderGammaValue);
}

void V4L2Viewer::OnSliderBlackLevelValueChange(int value)
{
    ui.m_edBlackLevel->setText(QString::number(value));
    m_sliderBlackLevelValue = static_cast<int32_t>(value);
    m_Camera.SetBrightness(m_sliderBlackLevelValue);
}

void V4L2Viewer::OnSlidersReleased()
{
    GetImageInformation();
}

void V4L2Viewer::OnCameraListButtonClicked()
{
    if (!ui.m_camerasListCheckBox->isChecked())
    {
        ui.m_Splitter1->setSizes(QList<int>{0,1});
    }
    else
    {
        ui.m_Splitter1->setSizes(QList<int>{250,1});
    }
}

void V4L2Viewer::OnMenuSplitterMoved(int pos, int index)
{
    if (pos == 0)
    {
        ui.m_camerasListCheckBox->setChecked(false);
    }
    else
    {
        ui.m_camerasListCheckBox->setChecked(true);
    }
}

void V4L2Viewer::ShowHideEnumerationControlWidget()
{
    if (ui.m_allFeaturesDockWidget->isHidden())
    {
        ui.m_allFeaturesDockWidget->setGeometry(this->pos().x()+this->width()/2 - 300, this->pos().y()+this->height()/2 - 250, 600, 500);
        ui.m_allFeaturesDockWidget->show();
    }
    else
    {
        ui.m_allFeaturesDockWidget->hide();
    }
}

void V4L2Viewer::PassIntDataToEnumerationWidget(int32_t id, int32_t min, int32_t max, int32_t value, QString name, QString unit, bool bIsReadOnly)
{
    if (!m_pEnumerationControlWidget->IsControlAlreadySet(id))
    {
        IControlEnumerationHolder *ptr = new IntegerEnumerationControl(id, min, max, value, name, unit, bIsReadOnly, this);
        connect(dynamic_cast<IntegerEnumerationControl*>(ptr), SIGNAL(PassNewValue(int32_t, int32_t)), &m_Camera, SLOT(SetEnumerationControlValue(int32_t, int32_t)));
        connect(dynamic_cast<IntegerEnumerationControl*>(ptr), SIGNAL(PassSliderValue(int32_t, int32_t)), &m_Camera, SLOT(SetSliderEnumerationControlValue(int32_t, int32_t)));
        m_pEnumerationControlWidget->AddElement(ptr);
    }
    else
    {
        bool bIsSucced;
        IControlEnumerationHolder *obj = m_pEnumerationControlWidget->GetControlWidget(id, bIsSucced);
        if (bIsSucced)
        {
            dynamic_cast<IntegerEnumerationControl*>(obj)->UpdateValue(value);
        }
    }
}

void V4L2Viewer::PassIntDataToEnumerationWidget(int32_t id, int64_t min, int64_t max, int64_t value, QString name, QString unit, bool bIsReadOnly)
{
    if (!m_pEnumerationControlWidget->IsControlAlreadySet(id))
    {
        IControlEnumerationHolder *ptr = new Integer64EnumerationControl(id, min, max, value, name, unit, bIsReadOnly, this);
        connect(dynamic_cast<Integer64EnumerationControl*>(ptr), SIGNAL(PassNewValue(int32_t, int64_t)), &m_Camera, SLOT(SetEnumerationControlValue(int32_t, int64_t)));
        connect(dynamic_cast<Integer64EnumerationControl*>(ptr), SIGNAL(PassSliderValue(int32_t, int64_t)), &m_Camera, SLOT(SetSliderEnumerationControlValue(int32_t, int64_t)));
        m_pEnumerationControlWidget->AddElement(ptr);
    }
    else
    {
        bool bIsSucced;
        IControlEnumerationHolder *obj = m_pEnumerationControlWidget->GetControlWidget(id, bIsSucced);
        if (bIsSucced)
        {
            dynamic_cast<Integer64EnumerationControl*>(obj)->UpdateValue(value);
        }
    }
}

void V4L2Viewer::PassBoolDataToEnumerationWidget(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly)
{
    if (!m_pEnumerationControlWidget->IsControlAlreadySet(id))
    {
        IControlEnumerationHolder *ptr = new BooleanEnumerationControl(id, value, name, unit, bIsReadOnly, this);
        connect(dynamic_cast<BooleanEnumerationControl*>(ptr), SIGNAL(PassNewValue(int32_t, bool)), &m_Camera, SLOT(SetEnumerationControlValue(int32_t, bool)));
        m_pEnumerationControlWidget->AddElement(ptr);
    }
    else
    {
        bool bIsSucced;
        IControlEnumerationHolder *obj = m_pEnumerationControlWidget->GetControlWidget(id, bIsSucced);
        if (bIsSucced)
        {
            dynamic_cast<BooleanEnumerationControl*>(obj)->UpdateValue(value);
        }
    }
}

void V4L2Viewer::PassButtonDataToEnumerationWidget(int32_t id, QString name, QString unit, bool bIsReadOnly)
{
    if (!m_pEnumerationControlWidget->IsControlAlreadySet(id))
    {
        IControlEnumerationHolder *ptr = new ButtonEnumerationControl(id, name, unit, bIsReadOnly, this);
        connect(dynamic_cast<ButtonEnumerationControl*>(ptr), SIGNAL(PassActionPerform(int32_t)), &m_Camera, SLOT(SetEnumerationControlValue(int32_t)));
        m_pEnumerationControlWidget->AddElement(ptr);
    }
}

void V4L2Viewer::PassListDataToEnumerationWidget(int32_t id, int32_t value, QList<QString> list, QString name, QString unit, bool bIsReadOnly)
{
    if (!m_pEnumerationControlWidget->IsControlAlreadySet(id))
    {
        IControlEnumerationHolder *ptr = new ListEnumerationControl(id, value, list, name, unit, bIsReadOnly, this);
        connect(dynamic_cast<ListEnumerationControl*>(ptr), SIGNAL(PassNewValue(int32_t, const char *)), &m_Camera, SLOT(SetEnumerationControlValueList(int32_t, const char*)));
        m_pEnumerationControlWidget->AddElement(ptr);
    }
    else
    {
        bool bIsSucced;
        IControlEnumerationHolder *obj = m_pEnumerationControlWidget->GetControlWidget(id, bIsSucced);
        if (bIsSucced)
        {
            dynamic_cast<ListEnumerationControl*>(obj)->UpdateValue(list, value);
        }
    }
}

void V4L2Viewer::PassListDataToEnumerationWidget(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly)
{
    if (!m_pEnumerationControlWidget->IsControlAlreadySet(id))
    {
        IControlEnumerationHolder *ptr = new ListIntEnumerationControl(id, value, list, name, unit, bIsReadOnly, this);
        connect(dynamic_cast<ListIntEnumerationControl*>(ptr), SIGNAL(PassNewValue(int32_t, int64_t)), &m_Camera, SLOT(SetEnumerationControlValueIntList(int32_t, int64_t)));
        m_pEnumerationControlWidget->AddElement(ptr);
    }
    else
    {
        bool bIsSucced;
        IControlEnumerationHolder *obj = m_pEnumerationControlWidget->GetControlWidget(id, bIsSucced);
        if (bIsSucced)
        {
            dynamic_cast<ListIntEnumerationControl*>(obj)->UpdateValue(list, value);
        }
    }
}

void V4L2Viewer::OnExposureActiveClicked()
{
    QPoint point(this->width()/2 - m_pActiveExposureWidget->width()/2, this->height()/2 - m_pActiveExposureWidget->height()/2);
    QPoint glob = mapToGlobal(point);
    m_pActiveExposureWidget->setGeometry(glob.x(), glob.y(), m_pActiveExposureWidget->width(), m_pActiveExposureWidget->height());
    m_pActiveExposureWidget->show();
}

void V4L2Viewer::PassInvertState(bool state)
{
    if (m_Camera.SetExposureActiveInvert(state) >= 0)
    {
        GetImageInformation();
    }
}

void V4L2Viewer::PassActiveState(bool state)
{
    if (m_Camera.SetExposureActiveLineMode(state) >= 0)
    {
        GetImageInformation();
    }
}

void V4L2Viewer::PassLineSelectorValue(int32_t value)
{
    if (m_Camera.SetExposureActiveLineSelector(value) >= 0)
    {
        GetImageInformation();
    }
}

void V4L2Viewer::OnUpdateZoomLabel()
{
    UpdateZoomButtons();
}

void V4L2Viewer::OnDockWidgetPositionChanged(bool topLevel)
{
    if (topLevel)
    {
        ui.m_allFeaturesDockWidget->setGeometry(this->pos().x()+this->width()/2 - 300, this->pos().y()+this->height()/2 - 250, 600, 500);
    }
}

void V4L2Viewer::StartStreaming(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine)
{
    int err = 0;

    // disable the start button to show that the start acquisition is in process
    ui.m_StartButton->setEnabled(false);

    ui.m_labelPixelFormats->setEnabled(false);
    ui.m_pixelFormats->setEnabled(false);

    ui.m_labelFrameRate->setEnabled(false);
    ui.m_edFrameRate->setEnabled(false);

    if (m_bIsCropAvailable)
    {
        ui.m_cropWidget->setEnabled(false);
    }

    if (m_bIsFrameIntervalAvailable)
    {
        ui.m_labelFrameRate->setEnabled(false);
        ui.m_edFrameRate->setEnabled(false);
    }

    QApplication::processEvents();

    m_nDroppedFrames = 0;

    // start streaming

    if (m_Camera.CreateUserBuffer(m_NumberOfUsedFramesLineEdit->text().toLong(), payloadSize) == 0)
    {
        m_Camera.QueueAllUserBuffer();
        m_Camera.StartStreaming();
        err = m_Camera.StartStreamChannel(pixelFormat,
                                          payloadSize,
                                          width,
                                          height,
                                          bytesPerLine,
                                          NULL,
                                          ui.m_TitleLogtofile->isChecked());

        if (0 == err)
        {
            m_bIsStreaming = true;
        }

        UpdateViewerLayout();

        m_FramesReceivedTimer.start(1000);
        m_Camera.QueueAllUserBuffer();
    }
}

// The event handler for stopping acquisition
void V4L2Viewer::OnStopButtonClicked()
{
    // disable the stop button to show that the stop acquisition is in process
    ui.m_StopButton->setEnabled(false);

    ui.m_pixelFormats->setEnabled(true);
    ui.m_labelPixelFormats->setEnabled(true);

    ui.m_labelFrameRate->setEnabled(true);
    ui.m_edFrameRate->setEnabled(true);

    if(m_bIsCropAvailable)
    {
        ui.m_cropWidget->setEnabled(true);
    }

    if (m_bIsFrameIntervalAvailable)
    {
        ui.m_labelFrameRate->setEnabled(true);
        ui.m_edFrameRate->setEnabled(true);
    }

    m_Camera.StopStreamChannel();
    m_Camera.StopStreaming();

    QApplication::processEvents();

    m_bIsStreaming = false;
    UpdateViewerLayout();

    m_FramesReceivedTimer.stop();

    m_Camera.DeleteUserBuffer();
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
        int size = image.byteCount();
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

// This event handler is triggered through a Qt signal posted by the camera observer
void V4L2Viewer::OnCameraListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
    bool bUpdateList = false;
    qDebug() << "Plugged Action: " << reason;

    // We only react on new cameras being found and known cameras being unplugged
    if (UpdateTriggerPluggedIn == reason)
    {
        qDebug() << "Plugged In Trigger";
        bUpdateList = true;

        std::string manuName;
        std::string devName = deviceName.toLatin1().data();
    }
    else if (UpdateTriggerPluggedOut == reason)
    {
        qDebug() << "Plugged Out Trigger";
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

    strCameraName = "Camera";

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
}

// Update the viewer range
void V4L2Viewer::UpdateViewerLayout()
{
    m_NumberOfUsedFramesWidgetAction->setEnabled(!m_bIsOpen);

    if (!m_bIsOpen)
    {
        QPixmap pix(":/V4L2Viewer/icon_camera_256.png");
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
            if (static_cast<int>(i) == currentIndex)
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

// The event handler to resize the image to fit to window
void V4L2Viewer::OnZoomFitButtonClicked()
{
    if (ui.m_ZoomFitButton->isChecked())
    {
        ui.m_ImageView->SetZoomAllowed(false);
        ui.m_ImageView->fitInView(m_pScene->sceneRect(), Qt::KeepAspectRatio);
    }
    else
    {
        ui.m_ImageView->SetZoomAllowed(true);
        ui.m_ImageView->TransformImageView();
    }

    UpdateZoomButtons();
}

// The event handler for resize the image
void V4L2Viewer::OnZoomInButtonClicked()
{
    ui.m_ImageView->OnZoomIn();
    UpdateZoomButtons();
}

// The event handler for resize the image
void V4L2Viewer::OnZoomOutButtonClicked()
{
    ui.m_ImageView->OnZoomOut();
    UpdateZoomButtons();
}

// Update the zoom buttons
void V4L2Viewer::UpdateZoomButtons()
{
    ui.m_ZoomFitButton->setEnabled(m_bIsOpen);

    if (ui.m_ImageView->GetScaleFactorValue() >= CustomGraphicsView::MAX_ZOOM_IN)
    {
        ui.m_ZoomInButton->setEnabled(false);
    }
    else
    {
        ui.m_ZoomInButton->setEnabled(true && m_bIsOpen && !ui.m_ZoomFitButton->isChecked());
    }

    if (ui.m_ImageView->GetScaleFactorValue() <= CustomGraphicsView::MAX_ZOOM_OUT)
    {
        ui.m_ZoomOutButton->setEnabled(false);
    }
    else
    {
        ui.m_ZoomOutButton->setEnabled(true && m_bIsOpen && !ui.m_ZoomFitButton->isChecked());
    }

    ui.m_ZoomLabel->setText(QString("%1%").arg(ui.m_ImageView->GetScaleFactorValue() * 100));
    ui.m_ZoomLabel->setEnabled(m_bIsOpen && m_bIsStreaming && !ui.m_ZoomFitButton->isChecked());
}

// Open/Close the camera
int V4L2Viewer::OpenAndSetupCamera(const uint32_t cardNumber, const QString &deviceName)
{
    int err = 0;

    std::string devName = deviceName.toStdString();
    err = m_Camera.OpenDevice(devName, m_BLOCKING_MODE, m_MMAP_BUFFER, m_VIDIOC_TRY_FMT);

    if (err != 0)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("Camera can't be opened, because is in use by another application \n or was disconnected!") );
    }

    return err;
}

int V4L2Viewer::CloseCamera(const uint32_t cardNumber)
{
    int err = 0;

    err = m_Camera.CloseDevice();

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
        m_Camera.ReadPayloadSize(payloadSize);
    }

    m_Camera.ReadFrameSize(width, height);
    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));
}

void V4L2Viewer::OnGain()
{
    int32_t nVal = int64_2_int32(ui.m_edGain->text().toLongLong());

    if (m_Camera.SetGain(nVal) < 0)
    {
        int32_t tmp = 0;
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Gain!") );
        m_Camera.ReadGain(tmp);
        ui.m_edGain->setText(QString("%1").arg(tmp));
        UpdateSlidersPositions(ui.m_sliderGain, tmp);
    }
    else
    {
        GetImageInformation();
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
    int64_t nVal = static_cast<int64_t>(ui.m_edExposure->text().toLongLong());

    if (nVal >= EXPOSURE_MAX_CHANGE)
    {
        int32_t nValAbs = static_cast<int32_t>(nVal/100000);
        if (m_Camera.SetExposureAbs(nValAbs) < 0)
        {
            QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE ExposureAbs!") );
            GetImageInformation();
        }
        else
        {
            GetImageInformation();
        }
    }
    else
    {
        int32_t nVal32 = int64_2_int32(nVal);
        if (m_Camera.SetExposure(nVal32) < 0)
        {
            QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE Exposure!") );
            GetImageInformation();
        }
        else
        {
            GetImageInformation();
        }
    }
}

void V4L2Viewer::OnExposureAbs()
{
    int32_t nVal = int64_2_int32(ui.m_edExposureAbs->text().toLongLong());

    if (m_Camera.SetExposureAbs(nVal) < 0)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE ExposureAbs!") );
        //GetImageInformation();
    }
    else
    {
        //GetImageInformation();
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
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SET pixelFormat!") );
    }
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
        UpdateSlidersPositions(ui.m_sliderGamma, tmp);
    }
    else
    {
        GetImageInformation();
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
        UpdateSlidersPositions(ui.m_sliderBlackLevel, tmp);
    }
    else
    {
        GetImageInformation();
    }
}

void V4L2Viewer::OnContinousWhiteBalance()
{
    m_Camera.SetContinousWhiteBalance(ui.m_chkContWhiteBalance->isChecked());
}

void V4L2Viewer::OnFrameRate()
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pixelFormat = 0;
    uint32_t bytesPerLine = 0;
    QString pixelFormatText;
    QString frameRate = ui.m_edFrameRate->text();
    uint32_t numerator = 0;
    uint32_t denominator = 0;

    bool bIsConverted = false;
    uint32_t value = frameRate.toInt(&bIsConverted);
    if (!bIsConverted)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("Only Framerate [Hz] value is acepted!") );
        return;
    }
    numerator = 1;
    denominator = value;

    m_Camera.ReadFrameSize(width, height);
    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);

    if (m_Camera.SetFrameRate(numerator, denominator) < 0)
    {
        QMessageBox::warning( this, tr("Video4Linux"), tr("FAILED TO SAVE frame rate!") );
        m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat);
        denominator /= 1000;
        ui.m_edFrameRate->setText(QString("%1").arg(denominator));
    }
    else
    {
        GetImageInformation();
    }
}

void V4L2Viewer::OnCropXOffset()
{
    uint32_t xOffset;
    uint32_t yOffset;
    uint32_t width;
    uint32_t height;

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        xOffset = ui.m_edCropXOffset->text().toInt();
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
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

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        yOffset = ui.m_edCropYOffset->text().toInt();
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
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

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        width = ui.m_edCropWidth->text().toInt();
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
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
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        height = ui.m_edCropHeight->text().toInt();
        if (m_Camera.SetCrop(xOffset, yOffset, width, height) == 0)
        {
            // readback to show it was set correct
            if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
            {
                ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
                ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
                ui.m_edCropWidth->setText(QString("%1").arg(width));
                ui.m_edCropHeight->setText(QString("%1").arg(height));
            }
        }
    }

    OnReadAllValues();
}

void V4L2Viewer::OnReadAllValues()
{
    GetImageInformation();
}

/////////////////////// Tools /////////////////////////////////////

void V4L2Viewer::GetImageInformation()
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t xOffset = 0;
    uint32_t yOffset = 0;
    int32_t gain = 0;
    int32_t min = 0;
    int32_t max = 0;
    int32_t minExp = 0;
    int32_t maxExp = 0;
    int64_t minExpAbs = 0;
    int64_t maxExpAbs = 0;
    int32_t step = 0;
    int32_t value = 0;
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

    if (ui.m_chkContWhiteBalance->isEnabled())
    {
        bool bIsAutoOn;
        if (m_Camera.ReadAutoWhiteBalance(bIsAutoOn) == 0)
        {
            ui.m_chkContWhiteBalance->setChecked(bIsAutoOn);
        }
    }

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

    if (m_Camera.ReadMinMaxGain(min, max) != -2)
    {
        ui.m_sliderGain->setEnabled(true);
        ui.m_sliderGain->setMinimum(min);
        ui.m_sliderGain->setMaximum(max);
        UpdateSlidersPositions(ui.m_sliderGain, gain);
    }
    else
    {
        ui.m_sliderGain->setEnabled(false);
    }

    if (m_Camera.ReadAutoGain(autogain) != -2)
    {
        ui.m_labelGainAuto->setEnabled(true);
        ui.m_chkAutoGain->setEnabled(true);
        ui.m_chkAutoGain->setChecked(autogain);
    }
    else
    {
        ui.m_labelGainAuto->setEnabled(false);
        ui.m_chkAutoGain->setEnabled(false);
    }

    if (m_Camera.ReadExposure(exposure) != -2)
    {
        ui.m_edExposure->setEnabled(true);
        ui.m_labelExposure->setEnabled(true);

        qDebug() << "Exposure read: " << exposure;
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

        qDebug() << "Exposure abs read: " << exposureAbs;
    }
    else
    {
        ui.m_edExposureAbs->setEnabled(false);
        ui.m_labelExposureAbs->setEnabled(false);
    }

    int64_t exp64 = static_cast<int64_t>(exposureAbs)*100000;
    qDebug() << "Exposure abs multiplicated read: " << exp64;
    if (exp64 >= EXPOSURE_MAX_CHANGE)
    {
        ui.m_edExposure->setText(QString("%1").arg(exp64));
        //m_bIsExposureAbsUsed = true;
    }
    else
    {
        ui.m_edExposure->setText(QString("%1").arg(exposure));
        //m_bIsExposureAbsUsed = false;
    }

    if (m_Camera.ReadMinMaxExposure(minExp, maxExp) != -2 &&
        m_Camera.ReadMinMaxExposureAbs(minExpAbs, maxExpAbs) != -2)
    {
            ui.m_sliderExposure->blockSignals(true);
            ui.m_sliderExposure->setEnabled(true);
            ui.m_sliderExposure->setMinimum(minExpAbs);
            ui.m_sliderExposure->setMaximum(maxExpAbs);
            ui.m_sliderExposure->blockSignals(false);

            m_MinimumExposureAbs = minExpAbs;
            m_MaximumExposureAbs = maxExpAbs;

            m_MinimumExposure = minExp;
            m_MaximumExposure = maxExp;

//            int32_t result = GetSliderValueFromLog(exposure);
//            UpdateSlidersPositions(ui.m_sliderExposure, result);
    }
    else
    {
        ui.m_sliderExposure->setEnabled(false);
    }

    if (m_Camera.ReadAutoExposure(autoexposure) != -2)
    {
        ui.m_labelExposureAuto->setEnabled(true);
        ui.m_chkAutoExposure->setEnabled(true);
        ui.m_chkAutoExposure->setChecked(autoexposure);
    }
    else
    {
        ui.m_labelExposureAuto->setEnabled(false);
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

    min = 0;
    max = 0;
    if (m_Camera.ReadMinMaxGamma(min, max) != -2)
    {
        ui.m_sliderGamma->setEnabled(true);
        ui.m_sliderGamma->setMinimum(min);
        ui.m_sliderGamma->setMaximum(max);
        UpdateSlidersPositions(ui.m_sliderGamma, nSVal);
    }
    else
    {
        ui.m_sliderGamma->setEnabled(false);
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

    min = 0;
    max = 0;
    if (m_Camera.ReadMinMaxBrightness(min, max) != -2)
    {
        ui.m_sliderBlackLevel->setEnabled(true);
        ui.m_sliderBlackLevel->setMinimum(min);
        ui.m_sliderBlackLevel->setMaximum(max);
        UpdateSlidersPositions(ui.m_sliderBlackLevel, nSVal);
    }
    else
    {
        ui.m_sliderBlackLevel->setEnabled(false);
    }

    m_Camera.ReadFrameSize(width, height);
    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);

    if (!m_bIsStreaming)
    {
        if (m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat) != -2)
        {
            m_bIsFrameIntervalAvailable = true;
            ui.m_edFrameRate->setEnabled(true);
            ui.m_labelFrameRate->setEnabled(true);
            denominator /= 1000;
            ui.m_edFrameRate->setText(QString("%1").arg(denominator));
        }
        else
        {
            m_bIsFrameIntervalAvailable = false;
            ui.m_edFrameRate->setEnabled(false);
            ui.m_labelFrameRate->setEnabled(false);
        }


        if (m_Camera.ReadCrop(xOffset, yOffset, width, height) != -2)
        {
            ui.m_cropWidget->setEnabled(true);
            ui.m_edCropXOffset->setText(QString("%1").arg(xOffset));
            ui.m_edCropYOffset->setText(QString("%1").arg(yOffset));
            ui.m_edCropWidth->setText(QString("%1").arg(width));
            ui.m_edCropHeight->setText(QString("%1").arg(height));
            m_bIsCropAvailable = true;
        }
        else
        {
            ui.m_cropWidget->setEnabled(false);
        }
    }

    bool bIsActive = false;
    bool bIsInverted = false;

    if (m_Camera.ReadExposureActiveLineMode(bIsActive) < 0)
    {
        ui.m_labelExposureActive->setEnabled(false);
        m_pActiveExposureWidget->setEnabled(false);
        ui.m_ExposureActiveButton->setEnabled(false);
    }
    else
    {
        m_pActiveExposureWidget->BlockInvertAndLineSelector(bIsActive);
        m_pActiveExposureWidget->SetActive(bIsActive);
    }

    if (m_Camera.ReadExposureActiveLineSelector(value, min, max, step) < 0)
    {
        m_pActiveExposureWidget->setEnabled(false);
        ui.m_ExposureActiveButton->setEnabled(false);
        ui.m_labelExposureActive->setEnabled(false);
    }
    else
    {
        m_pActiveExposureWidget->SetLineSelectorRange(value, min, max, step);
    }

    if (m_Camera.ReadExposureActiveInvert(bIsInverted) < 0)
    {
        m_pActiveExposureWidget->setEnabled(false);
        ui.m_ExposureActiveButton->setEnabled(false);
        ui.m_labelExposureActive->setEnabled(false);
    }
    else
    {
        m_pActiveExposureWidget->SetInvert(bIsInverted);
    }

    m_Camera.EnumAllControlNewStyle();
}

void V4L2Viewer::UpdateCameraFormat()
{
    uint32_t payloadSize = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pixelFormat = 0;
    QString pixelFormatText;
    uint32_t bytesPerLine = 0;

    ui.m_pixelFormats->blockSignals(true);
    ui.m_pixelFormats->clear();
    ui.m_pixelFormats->blockSignals(false);

    m_Camera.ReadPayloadSize(payloadSize);

    m_Camera.ReadFrameSize(width, height);
    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));

    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    m_Camera.ReadFormats();
    UpdateCurrentPixelFormatOnList(QString::fromStdString(m_Camera.ConvertPixelFormat2String(pixelFormat)));
}

void V4L2Viewer::UpdateCurrentPixelFormatOnList(QString pixelFormat)
{
    ui.m_pixelFormats->blockSignals(true);
    for (int i=0; i<ui.m_pixelFormats->count(); ++i)
    {
        if (ui.m_pixelFormats->itemText(i) == pixelFormat)
        {
            ui.m_pixelFormats->setCurrentIndex(i);
        }
    }
    ui.m_pixelFormats->blockSignals(false);
}

QString V4L2Viewer::GetDeviceInfo()
{
    std::string tmp;

    QString firmware = QString(tr("Camera FW Version = %1")).arg(QString::fromStdString(m_Camera.getAvtDeviceFirmwareVersion()));
    QString devTemp = QString(tr("Camera Device Temperature = %1C")).arg(QString::fromStdString(m_Camera.getAvtDeviceTemperature()));
    QString devSerial = QString(tr("Camera Serial Number = %1")).arg(QString::fromStdString(m_Camera.getAvtDeviceSerialNumber()));

    m_Camera.GetCameraDriverName(tmp);
    QString driverName = QString(tr("Driver name = %1")).arg(tmp.c_str());
    m_Camera.GetCameraBusInfo(tmp);
    QString busInfo = QString(tr("Bus info = %1")).arg(tmp.c_str());
    m_Camera.GetCameraDriverVersion(tmp);
    QString driverVer = QString(tr("Driver version = %1")).arg(tmp.c_str());
    m_Camera.GetCameraCapabilities(tmp);
    QString capabilities = QString(tr("Capabilities = %1")).arg(tmp.c_str());
    return QString(firmware + "<br>" + devTemp + "<br>" + devSerial + "<br>" + driverName + "<br>" + busInfo + "<br>" + driverVer + "<br>" + capabilities);
}

void V4L2Viewer::UpdateSlidersPositions(QSlider *slider, int32_t value)
{
    slider->blockSignals(true);
    slider->setValue(value);
    slider->blockSignals(false);
}

int32_t V4L2Viewer::GetSliderValueFromLog(int32_t value)
{
    double logExpMin = log(m_MinimumExposureAbs);
    double logExpMax = log(m_MaximumExposureAbs);
    int32_t minimumSliderExposure = ui.m_sliderExposure->minimum();
    int32_t maximumSliderExposure = ui.m_sliderExposure->maximum();
    double scale = (logExpMax - logExpMin) / (maximumSliderExposure - minimumSliderExposure);
    double result = minimumSliderExposure + ( log(value) - logExpMin ) / scale;
    return static_cast<int32_t>(result);
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
    }
}

void V4L2Viewer::SetTitleText()
{
    setWindowTitle(QString(tr("%1 V%2.%3.%4")).arg(APP_NAME).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH));
}

