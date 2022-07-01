/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */


#include <V4L2Helper.h>
#include "Logger.h"
#include "V4L2Viewer.h"
#include "SelectSubDeviceDialog.h"
#include "CameraListCustomItem.h"
#include "IntegerEnumerationControl.h"
#include "Integer64EnumerationControl.h"
#include "BooleanEnumerationControl.h"
#include "ButtonEnumerationControl.h"
#include "ListEnumerationControl.h"
#include "ListIntEnumerationControl.h"
#include "CustomGraphicsView.h"
#include "CustomDialog.h"
#include "GitRevision.h"

#include <QtCore>
#include <QtGlobal>
#include <QStringList>
#include <QFontDatabase>
#include <QTextStream>

#include <ctime>
#include <limits>
#include <sstream>

#define NUM_COLORS 3
#define BIT_DEPTH 8

#define MANUF_NAME_AV       "Allied Vision"
#define APP_NAME            "Allied Vision V4L2 Viewer"
#define APP_VERSION_MAJOR   1
#define APP_VERSION_MINOR   1
#define APP_VERSION_PATCH   0
#ifndef SCM_REVISION
#define SCM_REVISION        0
#endif

// CCI registers
#define CCI_GCPRM_16R                               0x0010
#define CCI_BCRM_16R                                0x0014

#define EXPOSURE_MAX_VALUE 2147483647


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
    , m_BUFFER_TYPE(IO_METHOD_USERPTR) // use userptr by default
    , m_NUMBER_OF_USED_FRAMES(5)
    , m_VIDIOC_TRY_FMT(true) // use VIDIOC_TRY_FMT by default
    , m_ShowFrames(true)
    , m_nStreamNumber(0)
    , m_bIsOpen(false)
    , m_bIsStreaming(false)
    , m_sliderGainValue(0)
    , m_sliderBrightnessValue(0)
    , m_sliderGammaValue(0)
    , m_sliderExposureValue(0)
    , m_bIsCropAvailable(false)
    , m_SavedFramesCounter(0)
    , m_LastImageSaveFormat(".png")
    , m_DefaultDenominator(24)
    , m_bIsImageFitByFirstImage(false)
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

    connect(ui.m_FlipHorizontalCheckBox,      SIGNAL(stateChanged(int)), this, SLOT(OnFlipHorizontal(int)));
    connect(ui.m_FlipVerticalCheckBox,        SIGNAL(stateChanged(int)), this, SLOT(OnFlipVertical(int)));

    SetTitleText();

    // Start Camera
    connect(&m_Camera, SIGNAL(OnCameraListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnCameraListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));
    connect(&m_Camera, SIGNAL(OnSubDeviceListChanged_Signal(const int &, unsigned int, unsigned long long, const QString &, const QString &)), this, SLOT(OnSubDeviceListChanged(const int &, unsigned int, unsigned long long, const QString &, const QString &)));
    connect(&m_Camera, SIGNAL(OnCameraFrameReady_Signal(const QImage &, const unsigned long long &)),                                       this, SLOT(OnFrameReady(const QImage &, const unsigned long long &)), Qt::QueuedConnection);
    connect(&m_Camera, SIGNAL(OnCameraFrameID_Signal(const unsigned long long &)),                                                          this, SLOT(OnFrameID(const unsigned long long &)));
    connect(&m_Camera, SIGNAL(OnCameraPixelFormat_Signal(const QString &)),                                                                 this, SLOT(OnCameraPixelFormat(const QString &)));

    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int64_t>("int64_t");

    connect(&m_Camera, SIGNAL(PassAutoExposureValue(int64_t)), this, SLOT(OnUpdateAutoExposure(int64_t)), Qt::QueuedConnection);
    connect(&m_Camera, SIGNAL(PassAutoGainValue(int32_t)), this, SLOT(OnUpdateAutoGain(int32_t)), Qt::QueuedConnection);

    connect(&m_Camera, SIGNAL(SendIntDataToEnumerationWidget(int32_t, int32_t, int32_t, int32_t, QString, QString, bool)),      this, SLOT(PassIntDataToEnumerationWidget(int32_t, int32_t, int32_t, int32_t, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SentInt64DataToEnumerationWidget(int32_t, int64_t, int64_t, int64_t, QString, QString, bool)),    this, SLOT(PassIntDataToEnumerationWidget(int32_t, int64_t, int64_t, int64_t, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendBoolDataToEnumerationWidget(int32_t, bool, QString, QString, bool)),                          this, SLOT(PassBoolDataToEnumerationWidget(int32_t, bool, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendButtonDataToEnumerationWidget(int32_t, QString, QString, bool)),                              this, SLOT(PassButtonDataToEnumerationWidget(int32_t, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendListDataToEnumerationWidget(int32_t, int32_t, QList<QString>, QString, QString, bool)),       this, SLOT(PassListDataToEnumerationWidget(int32_t, int32_t, QList<QString>, QString, QString, bool)));
    connect(&m_Camera, SIGNAL(SendListIntDataToEnumerationWidget(int32_t, int32_t, QList<int64_t>, QString, QString, bool)),    this, SLOT(PassListDataToEnumerationWidget(int32_t, int32_t, QList<int64_t>, QString, QString, bool)));
	connect(&m_Camera,SIGNAL(SendControlStateChange(int32_t, bool)),this,SLOT(PassControlStateChange(int32_t, bool)));

    connect(&m_Camera, SIGNAL(SendSignalToUpdateWidgets()), this, SLOT(OnReadAllValues()));

    // Setup blocking mode radio buttons
    m_BlockingModeRadioButtonGroup = new QButtonGroup(this);
    m_BlockingModeRadioButtonGroup->setExclusive(true);

    connect(ui.m_sliderExposure,    SIGNAL(valueChanged(int)),  this, SLOT(OnSliderExposureValueChange(int)));
    connect(ui.m_sliderGain,        SIGNAL(valueChanged(int)),  this, SLOT(OnSliderGainValueChange(int)));
    connect(ui.m_sliderBrightness,  SIGNAL(valueChanged(int)),  this, SLOT(OnSliderBrightnessValueChange(int)));
    connect(ui.m_sliderGamma,       SIGNAL(valueChanged(int)),  this, SLOT(OnSliderGammaValueChange(int)));
    connect(ui.m_sliderExposure,    SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));
    connect(ui.m_sliderGain,        SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));
    connect(ui.m_sliderBrightness,  SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));
    connect(ui.m_sliderGamma,       SIGNAL(sliderReleased()),   this, SLOT(OnSlidersReleased()));

    connect(ui.m_DisplayImagesCheckBox, SIGNAL(clicked()), this, SLOT(OnShowFrames()));

    connect(ui.m_TitleLogtofile, SIGNAL(triggered()), this, SLOT(OnLogToFile()));
    connect(ui.m_TitleLangEnglish, SIGNAL(triggered()), this, SLOT(OnLanguageChange()));
    connect(ui.m_TitleLangGerman, SIGNAL(triggered()), this, SLOT(OnLanguageChange()));

    connect(ui.m_camerasListCheckBox, SIGNAL(clicked()), this, SLOT(OnCameraListButtonClicked()));
    connect(ui.m_Splitter1, SIGNAL(splitterMoved(int, int)), this, SLOT(OnMenuSplitterMoved(int, int)));

    connect(ui.m_ImageView, SIGNAL(UpdateZoomLabel()), this, SLOT(OnUpdateZoomLabel()));

    m_Camera.DeviceDiscoveryStart();
    m_Camera.SubDeviceDiscoveryStart();
    connect(ui.m_CamerasListBox, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(OnListBoxCamerasItemDoubleClicked(QListWidgetItem *)));

    // Connect the handler to show the frames per second
    connect(&m_FramesReceivedTimer, SIGNAL(timeout()), this, SLOT(OnUpdateFramesReceived()));

    // register meta type for QT signal/slot mechanism
    qRegisterMetaType<QSharedPointer<MyFrame> >("QSharedPointer<MyFrame>");

    // connect the buttons for Image m_ControlRequestTimer
    connect(ui.m_edWidth, SIGNAL(editingFinished()), this, SLOT(OnWidth()));
    connect(ui.m_edHeight, SIGNAL(editingFinished()), this, SLOT(OnHeight()));
    connect(ui.m_edGain, SIGNAL(editingFinished()), this, SLOT(OnGain()));
    connect(ui.m_chkAutoGain, SIGNAL(clicked()), this, SLOT(OnAutoGain()));
    connect(ui.m_edExposure, SIGNAL(editingFinished()), this, SLOT(OnExposure()));
    connect(ui.m_chkAutoExposure, SIGNAL(clicked()), this, SLOT(OnAutoExposure()));
    connect(ui.m_pixelFormats, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnPixelFormatChanged(const QString &)));
	connect(ui.m_frameSizes, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFrameSizeIndexChanged(int)));
    connect(ui.m_edGamma, SIGNAL(editingFinished()), this, SLOT(OnGamma()));
    connect(ui.m_edBrightness, SIGNAL(editingFinished()), this, SLOT(OnBrightness()));

    connect(ui.m_chkContWhiteBalance, SIGNAL(clicked()), this, SLOT(OnContinousWhiteBalance()));
    connect(ui.m_edFrameRate, SIGNAL(editingFinished()), this, SLOT(OnFrameRate()));
    connect(ui.m_edCropXOffset, SIGNAL(editingFinished()), this, SLOT(OnCropXOffset()));
    connect(ui.m_edCropYOffset, SIGNAL(editingFinished()), this, SLOT(OnCropYOffset()));
    connect(ui.m_edCropWidth, SIGNAL(editingFinished()), this, SLOT(OnCropWidth()));
    connect(ui.m_edCropHeight, SIGNAL(editingFinished()), this, SLOT(OnCropHeight()));

    connect(ui.m_AllControlsButton, SIGNAL(clicked()), this, SLOT(ShowHideEnumerationControlWidget()));

    connect(ui.m_chkFrameRateAuto, SIGNAL(clicked()), this, SLOT(OnCheckFrameRateAutoClicked()));

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

    // add about widget to the menu bar
    m_pAboutWidget = new AboutWidget(this);
    m_pAboutWidget->SetVersion(QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH));
    QWidgetAction *aboutWidgetAction = new QWidgetAction(this);
    aboutWidgetAction->setDefaultWidget(m_pAboutWidget);
    ui.m_MenuAbout->addAction(aboutWidgetAction);

    ui.menuBar->setNativeMenuBar(false);

    QMainWindow::showMaximized();

    UpdateViewerLayout();

    ui.m_camerasListCheckBox->setChecked(true);
    m_pEnumerationControlWidget = new ControlsHolderWidget();


    ui.m_ImageControlFrame->setEnabled(false);
    ui.m_FlipHorizontalCheckBox->setEnabled(false);
    ui.m_FlipVerticalCheckBox->setEnabled(false);
    ui.m_DisplayImagesCheckBox->setEnabled(false);
    ui.m_SaveImageButton->setEnabled(false);

    connect(ui.m_allFeaturesDockWidget, SIGNAL(topLevelChanged(bool)), this, SLOT(OnDockWidgetPositionChanged(bool)));
    connect(ui.m_allFeaturesDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(OnDockWidgetVisibilityChanged(bool)));

    ui.m_allFeaturesDockWidget->setWidget(m_pEnumerationControlWidget);
    ui.m_allFeaturesDockWidget->hide();
    ui.m_allFeaturesDockWidget->setStyleSheet("QDockWidget {"
                                                "titlebar-close-icon: url(:/V4L2Viewer/Cross128.png);"
                                                "titlebar-normal-icon: url(:/V4L2Viewer/resize4.png);}");

    LOG_EX(QString("V4L2Viewer git commit = %1").arg(GIT_VERSION).toStdString().c_str());

    ui.m_MenuLang->menuAction()->setEnabled(false);
    ui.m_MenuLang->menuAction()->setVisible(false);

    SetDefaultLabels();
}

void V4L2Viewer::SetDefaultLabels()
{
    ui.m_labelGain->setText(QApplication::translate("V4L2ViewerClass", "Gain [1/100dB]:", Q_NULLPTR));
    ui.m_labelGainAuto->setText(QApplication::translate("V4L2ViewerClass", "Gain Auto", Q_NULLPTR));
    ui.m_labelExposureAuto->setText(QApplication::translate("V4L2ViewerClass", "Exposure Auto", Q_NULLPTR));
    ui.m_labelGamma->setText(QApplication::translate("V4L2ViewerClass", "Gamma:", Q_NULLPTR));
    ui.m_FlipHorizontalCheckBox->setText(QApplication::translate("V4L2ViewerClass", "Flip X", Q_NULLPTR));
    ui.m_FlipVerticalCheckBox->setText(QApplication::translate("V4L2ViewerClass", "Flip Y", Q_NULLPTR));
    ui.m_labelBrightness->setText(QApplication::translate("V4L2ViewerClass", "Brightness:", Q_NULLPTR));
    ui.m_labelWhiteBalanceAuto->setText(QApplication::translate("V4L2ViewerClass", "White Balance Auto", Q_NULLPTR));
    ui.m_labelFrameRateAuto->setText(QApplication::translate("V4L2ViewerClass", "Framerate Auto", Q_NULLPTR));
    ui.m_labelFrameRate->setText(QApplication::translate("V4L2ViewerClass", "Framerate [Hz]:", Q_NULLPTR));
    ui.m_CropLabel->setText(QApplication::translate("V4L2ViewerClass", "Crop", Q_NULLPTR));
    ui.m_labelPixelFormats->setText(QApplication::translate("V4L2ViewerClass", "Pixel Format:", Q_NULLPTR));
    ui.m_labelWidth->setText(QApplication::translate("V4L2ViewerClass", "Width:", Q_NULLPTR));
    ui.m_labelHeight->setText(QApplication::translate("V4L2ViewerClass", "Height:", Q_NULLPTR));
    ui.m_labelExposure->setText(QApplication::translate("V4L2ViewerClass", "Exposure [ns]:", Q_NULLPTR));
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

    QVector<QString> selectedSubDeviceList;

    if(false == m_bIsOpen && !m_SubDevices.empty())
    {
        LOG_EX("V4L2Viewer::OnOpenCloseButtonClicked: user will select sub-devices from list of size %d", m_SubDevices.size());

        SelectSubDeviceDialog selectSubDeviceDialog(m_SubDevices, selectedSubDeviceList, this);
        selectSubDeviceDialog.exec();

        LOG_EX("V4L2Viewer::OnOpenCloseButtonClicked: user selected %d sub-devices", selectedSubDeviceList.size());
    }

    if (-1 < nRow)
    {
        QString devName = dynamic_cast<CameraListCustomItem*>(ui.m_CamerasListBox->itemWidget(ui.m_CamerasListBox->item(nRow)))->GetCameraName();
        QString deviceName = devName.right(devName.length() - devName.indexOf(':') - 2);

        deviceName = deviceName.left(deviceName.indexOf('(') - 1);

        if (false == m_bIsOpen)
        {
            // Start
            err = OpenAndSetupCamera(m_cameras[nRow], deviceName, selectedSubDeviceList);
            // Set up Qt image
            if (0 == err)
            {
                ui.m_ImageControlFrame->setEnabled(true);
                GetImageInformation(true);
                // Set auto framerate to on always when camera is opened
                ui.m_chkFrameRateAuto->setChecked(true);
                ui.m_edFrameRate->setText(QString::number(DEFAULT_FRAME_RATE));
                OnCheckFrameRateAutoClicked();
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

            ui.m_ZoomFitButton->setChecked(false);
            m_bIsImageFitByFirstImage = false;

            ui.m_FrameIdLabel->setText("FrameID: -");
            ui.m_FramesPerSecondLabel->setText("- fps");
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
        UpdateZoomButtons();
    }

    ui.m_OpenCloseButton->setEnabled( 0 <= m_cameras.size() || m_bIsOpen );
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

    LOG_EX("V4L2Viewer::OnStartButtonClicked width=%d,height=%d", width, height);

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
        CustomDialog::Warning( this, tr("Video4Linux"), tr("Language has been already set!") );
    }
}

void V4L2Viewer::OnUpdateAutoExposure(int64_t value)
{
    LOG_EX("V4L2Viewer::OnUpdateAutoExposure: updating displayed exposure text to %d", value);
    ui.m_edExposure->setText(QString::number(value));
    int64_t result = GetSliderValueFromLog(value);
    UpdateSlidersPositions(ui.m_sliderExposure, result);
}

void V4L2Viewer::OnUpdateAutoGain(int32_t value)
{
    ui.m_edGain->setText(QString::number(value));
    UpdateSlidersPositions(ui.m_sliderGain, value);
}

void V4L2Viewer::OnSliderExposureValueChange(int value)
{
    int minSliderVal = ui.m_sliderExposure->minimum();
    int maxSliderVal = ui.m_sliderExposure->maximum();

    double minExp = log(m_MinimumExposure);
    double maxExp = log(m_MaximumExposure);

    double scale = (maxExp - minExp) / (maxSliderVal-minSliderVal);
    double outValue = exp(minExp + scale*(value-minSliderVal));

    int64_t sliderExposureValue64 = static_cast<int64_t>(outValue);

    ui.m_edExposure->setText(QString("%1").arg(sliderExposureValue64));
    m_Camera.SetExposure(sliderExposureValue64);
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

void V4L2Viewer::OnSliderBrightnessValueChange(int value)
{
    ui.m_edBrightness->setText(QString::number(value));
    m_sliderBrightnessValue = static_cast<int32_t>(value);
    m_Camera.SetBrightness(m_sliderBrightnessValue);
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

void V4L2Viewer::PassControlStateChange(int32_t id, bool enabled)
{
	bool bIsSucced;
	IControlEnumerationHolder *obj = m_pEnumerationControlWidget->GetControlWidget(id, bIsSucced);
	if (bIsSucced)
	{
		obj->setEnabled(enabled);
	}

	switch (id) {
		case V4L2_CID_BRIGHTNESS:
			ui.m_edBrightness->setEnabled(enabled);
			ui.m_sliderBrightness->setEnabled(enabled);
			ui.m_labelBrightness->setEnabled(enabled);
			break;
		case V4L2_CID_GAMMA:
			ui.m_edGamma->setEnabled(enabled);
			ui.m_sliderGamma->setEnabled(enabled);
			ui.m_labelGamma->setEnabled(enabled);
			break;
		case V4L2_CID_GAIN:
			ui.m_edGain->setEnabled(enabled);
			ui.m_sliderGain->setEnabled(enabled);
			ui.m_labelGain->setEnabled(enabled);
			break;
		case V4L2_CID_EXPOSURE:
			ui.m_edExposure->setEnabled(enabled);
			ui.m_sliderExposure->setEnabled(enabled);
			ui.m_labelExposure->setEnabled(enabled);
			break;
		case V4L2_CID_AUTOGAIN:
			ui.m_chkAutoGain->setEnabled(enabled);
			ui.m_labelGainAuto->setEnabled(enabled);
			break;
		case V4L2_CID_EXPOSURE_AUTO:
			ui.m_chkAutoExposure->setEnabled(enabled);
			ui.m_labelExposureAuto->setEnabled(enabled);
			break;
		default:
			break;
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

void V4L2Viewer::OnDockWidgetVisibilityChanged(bool visible)
{
    if (visible)
    {
        ui.m_AllControlsButton->setText(tr("Close"));
    }
    else
    {
        ui.m_AllControlsButton->setText(tr("Open"));
    }
}

void V4L2Viewer::OnCheckFrameRateAutoClicked()
{
    if (m_bIsStreaming)
    {
        return;
    }
    uint32_t numerator = 0;
    uint32_t denominator = ui.m_edFrameRate->text().toInt();

    if (ui.m_chkFrameRateAuto->isChecked())
    {
        numerator = 0;
        ui.m_labelFrameRate->setEnabled(false);
        ui.m_edFrameRate->setEnabled(false);
        m_Camera.SetFrameRate(numerator, denominator);
    }
    else
    {
        numerator = 1;
        ui.m_labelFrameRate->setEnabled(true);
        ui.m_edFrameRate->setEnabled(true);
        m_Camera.SetFrameRate(numerator, denominator);
    }
}

void V4L2Viewer::OnFlipHorizontal(int state)
{
    Q_UNUSED(state)
    if (!m_bIsStreaming)
    {
        QImage image = m_PixmapItem->pixmap().toImage();
        image = image.mirrored(true, false);
        m_PixmapItem->setPixmap(QPixmap::fromImage(image));
        m_PixmapItem->update();
    }
}

void V4L2Viewer::OnFlipVertical(int state)
{
    Q_UNUSED(state)
    if (!m_bIsStreaming)
    {
        QImage image = m_PixmapItem->pixmap().toImage();
        image = image.mirrored(false, true);
        m_PixmapItem->setPixmap(QPixmap::fromImage(image));
        m_PixmapItem->update();
    }
}

void V4L2Viewer::StartStreaming(uint32_t pixelFormat, uint32_t payloadSize, uint32_t width, uint32_t height, uint32_t bytesPerLine)
{
    int err = 0;

    // disable the start button to show that the start acquisition is in process
    ui.m_StartButton->setEnabled(false);

    ui.m_labelPixelFormats->setEnabled(false);
	ui.m_labelFrameSizes->setEnabled(false);
    ui.m_pixelFormats->setEnabled(false);
	ui.m_frameSizes->setEnabled(false);

    ui.m_labelFrameRate->setEnabled(false);
    ui.m_edFrameRate->setEnabled(false);

    ui.m_labelFrameRateAuto->setEnabled(false);
    ui.m_chkFrameRateAuto->setEnabled(false);

    if (m_bIsCropAvailable)
    {
        ui.m_cropWidget->setEnabled(false);
    }

    QApplication::processEvents();

    LOG_EX("V4L2Viewer::StartStreaming pixelFormat=%d,payloadSize=%d,width=%d,height=%d,bytesPerLine=%d", pixelFormat, payloadSize, width, height, bytesPerLine);

    // start streaming

    if (m_Camera.CreateUserBuffer(m_NUMBER_OF_USED_FRAMES, payloadSize) == 0)
    {
        LOG_EX("V4L2Viewer::StartStreaming streaming will be started");
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
    }
}

// The event handler for stopping acquisition
void V4L2Viewer::OnStopButtonClicked()
{
    // disable the stop button to show that the stop acquisition is in process
    ui.m_StopButton->setEnabled(false);

    ui.m_pixelFormats->setEnabled(true);
	ui.m_frameSizes->setEnabled(true);
    ui.m_labelPixelFormats->setEnabled(true);
	ui.m_labelFrameSizes->setEnabled(true);

    ui.m_labelFrameRateAuto->setEnabled(true);
    ui.m_chkFrameRateAuto->setEnabled(true);

    if (!ui.m_chkFrameRateAuto->isChecked())
    {
        ui.m_labelFrameRate->setEnabled(true);
        ui.m_edFrameRate->setEnabled(true);
    }

    if(m_bIsCropAvailable)
    {
        ui.m_cropWidget->setEnabled(true);
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
    QString filename = "/Frame_"+QString::number(m_SavedFramesCounter)+m_LastImageSaveFormat;
    QString fullPath = QFileDialog::getSaveFileName(this, tr("Save file"), QDir::homePath()+filename, "*.png *.raw");

    if (fullPath.contains(".png"))
    {
        m_LastImageSaveFormat = ".png";
        QPixmap pixmap = m_PixmapItem->pixmap();
        QImage image = pixmap.toImage();
        image.save(fullPath,"png");
        m_SavedFramesCounter++;
    }
    else if(fullPath.contains(".raw"))
    {
        m_LastImageSaveFormat = ".raw";
        QPixmap pixmap = m_PixmapItem->pixmap();
        QImage image = pixmap.toImage();
        int size = image.byteCount();
        QByteArray data(reinterpret_cast<const char*>(image.bits()), size);
        QFile file(fullPath);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
        m_SavedFramesCounter++;
    }
}

// The event handler to show the processed frame
void V4L2Viewer::OnFrameReady(const QImage &image, const unsigned long long &frameId)
{
    if (m_ShowFrames && m_bIsStreaming)
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
            if (!m_bIsImageFitByFirstImage)
            {
                ui.m_ZoomFitButton->setChecked(true);
                OnZoomFitButtonClicked();
                m_bIsImageFitByFirstImage = true;
            }
        }
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

    // We only react on new cameras being found and known cameras being unplugged
    if (UpdateTriggerPluggedIn == reason)
    {
        bUpdateList = true;

        std::string manuName;
        std::string devName = deviceName.toLatin1().data();
    }
    else if (UpdateTriggerPluggedOut == reason)
    {
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
}

// This event handler is triggered through a Qt signal posted by the camera observer
void V4L2Viewer::OnSubDeviceListChanged(const int &reason, unsigned int cardNumber, unsigned long long deviceID, const QString &deviceName, const QString &info)
{
    // We only react on new sub-device being found and known sub-devices being unplugged
    if (UpdateTriggerPluggedIn == reason)
    {
        bool alreadyStored = false;
        for(QVector<QString>::iterator itSubDevices = m_SubDevices.begin(); itSubDevices != m_SubDevices.end(); ++itSubDevices)
        {
            if(*itSubDevices == deviceName)
            {
                alreadyStored = true;
                break;
            }
        }
        if(!alreadyStored)
        {
            m_SubDevices.push_back(deviceName);
        }
    }
    else if(UpdateTriggerPluggedOut == reason)
    {
        for(QVector<QString>::iterator itSubDevices = m_SubDevices.begin(); itSubDevices != m_SubDevices.end(); ++itSubDevices)
        {
            if(*itSubDevices == deviceName)
            {
                m_SubDevices.erase(itSubDevices);
                break;
            }
        }
    }
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
}

// Update the viewer range
void V4L2Viewer::UpdateViewerLayout()
{
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
}

// The event handler to resize the image to fit to window
void V4L2Viewer::OnZoomFitButtonClicked()
{
    ui.m_ImageView->SetScaleFactorToDefault();
    ui.m_ImageView->fitInView(m_pScene->sceneRect(), Qt::KeepAspectRatio);
    double scaleFitToView = ui.m_ImageView->transform().m11();
    ui.m_ZoomLabel->setText(QString("%1%").arg(scaleFitToView * 100, 1, 'f',1));
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
    ui.m_ZoomLabel->setEnabled(m_bIsOpen);

    if (ui.m_ImageView->GetScaleFactorValue() >= CustomGraphicsView::MAX_ZOOM_IN)
    {
        ui.m_ZoomInButton->setEnabled(false);
    }
    else
    {
        ui.m_ZoomInButton->setEnabled(m_bIsOpen);
    }

    if (ui.m_ImageView->GetScaleFactorValue() <= CustomGraphicsView::MAX_ZOOM_OUT)
    {
        ui.m_ZoomOutButton->setEnabled(false);
    }
    else
    {
        ui.m_ZoomOutButton->setEnabled(m_bIsOpen);
    }
    ui.m_ZoomLabel->setText(QString("%1%").arg(ui.m_ImageView->GetScaleFactorValue() * 100));
}

// Open/Close the camera
int V4L2Viewer::OpenAndSetupCamera(const uint32_t cardNumber, const QString &deviceName, const QVector<QString>& subDevices)
{
    int err = 0;

    std::string devName = deviceName.toStdString();
	QString cameraName;
    QVector<QString> subDevs = subDevices;
    err = m_Camera.OpenDevice(devName, subDevs, m_BLOCKING_MODE, m_BUFFER_TYPE, m_VIDIOC_TRY_FMT);

    if (err != 0)
    {
        CustomDialog::Error( this, tr("Video4Linux"), tr("The camera cannot be opened because it is in use by another application or it has been disconnected!"));
    }

    return err;
}

int V4L2Viewer::CloseCamera(const uint32_t cardNumber)
{
    int err = 0;

    SetDefaultLabels();

    err = m_Camera.CloseDevice();

    return err;
}

// The event handler to show the frames received
void V4L2Viewer::OnUpdateFramesReceived()
{
    auto const fpsReceived = m_Camera.GetReceivedFPS();
    auto const fpsRendered = m_Camera.GetRenderedFPS();
    ui.m_FramesPerSecondLabel->setText(QString::asprintf("%.2f received/ %.2f rendered", fpsReceived, fpsRendered));
}

void V4L2Viewer::OnWidth()
{
    OnHeight();
}

void V4L2Viewer::OnHeight()
{
    uint32_t width = 0;
    uint32_t height = 0;

    LOG_EX("V4L2Viewer::OnHeight: setting frame size to width=%d, height=%d", ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt());

    if (m_Camera.SetFrameSize(ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt()) < 0)
    {
        CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SAVE frame size!") );
    }
    else
    {
        uint32_t payloadSize = 0;
        m_Camera.ReadPayloadSize(payloadSize);
    }

    m_Camera.ReadFrameSize(width, height);

    ui.m_edWidth->setText(QString("%1").arg(width));
    ui.m_edHeight->setText(QString("%1").arg(height));

    LOG_EX("V4L2Viewer::OnHeight: read frame size back from camera as width=%d, height=%d", ui.m_edWidth->text().toInt(), ui.m_edHeight->text().toInt());
}

void V4L2Viewer::OnGain()
{
    int64_t nVal = ui.m_edGain->text().toLongLong();

    if (m_Camera.SetGain(nVal) < 0)
    {
        int64_t tmp = 0;
        CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SAVE Gain!") );
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
    ui.m_sliderGain->setEnabled(!ui.m_chkAutoGain->isChecked());

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

    if (m_Camera.SetExposure(nVal) < 0)
    {
	CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SAVE Exposure!") );
	GetImageInformation();
    }
    else
    {
	GetImageInformation();
    }
}

void V4L2Viewer::OnAutoExposure()
{
    bool autoexposure = false;

    m_Camera.SetAutoExposure(ui.m_chkAutoExposure->isChecked());
    ui.m_sliderExposure->setEnabled(!ui.m_chkAutoExposure->isChecked());

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
        CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SET pixelFormat!") );
    }

	QList<QString> framesizes = m_Camera.GetFrameSizes(result);

	ui.m_frameSizes->blockSignals(true);
	ui.m_frameSizes->clear();
	ui.m_frameSizes->addItems(framesizes);
	ui.m_frameSizes->setCurrentIndex(m_Camera.GetFrameSizeIndex());
	ui.m_frameSizes->blockSignals(false);

    GetImageInformation();
}

void V4L2Viewer::OnGamma()
{
    int32_t nVal = int64_2_int32(ui.m_edGamma->text().toLongLong());

    if (m_Camera.SetGamma(nVal) < 0)
    {
        int32_t tmp = 0;
        CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SAVE gamma!") );
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
    int32_t nVal = int64_2_int32(ui.m_edBrightness->text().toLongLong());

    if (m_Camera.SetBrightness(nVal) < 0)
    {
        int32_t tmp = 0;
        CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SAVE brightness!") );
        m_Camera.ReadBrightness(tmp);
        ui.m_edBrightness->setText(QString("%1").arg(tmp));
        UpdateSlidersPositions(ui.m_sliderBrightness, tmp);
    }
    else
    {
        GetImageInformation();
    }
}

void V4L2Viewer::OnContinousWhiteBalance()
{
    m_Camera.SetAutoWhiteBalance(ui.m_chkContWhiteBalance->isChecked());
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
        CustomDialog::Warning( this, tr("Video4Linux"), tr("Only Framerate [Hz] value is acepted!") );
        return;
    }
    numerator = 1;
    denominator = value;

    m_Camera.ReadFrameSize(width, height);
    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);

    if (m_Camera.SetFrameRate(numerator, denominator) < 0)
    {
        CustomDialog::Error( this, tr("Video4Linux"), tr("FAILED TO SAVE frame rate!") );
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
    int32_t xOffset;
    int32_t yOffset;
    uint32_t width;
    uint32_t height;

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        m_bIsImageFitByFirstImage = false;
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
    int32_t xOffset;
    int32_t yOffset;
    uint32_t width;
    uint32_t height;

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        m_bIsImageFitByFirstImage = false;
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
    int32_t xOffset;
    int32_t yOffset;
    uint32_t width;
    uint32_t height;

    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        m_bIsImageFitByFirstImage = false;
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
    int32_t xOffset;
    int32_t yOffset;
    uint32_t width;
    uint32_t height;
    if (m_Camera.ReadCrop(xOffset, yOffset, width, height) == 0)
    {
        m_bIsImageFitByFirstImage = false;
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

void V4L2Viewer::GetImageInformation(const bool isCalledFromOnOpen)
{
    uint32_t width = 0;
    uint32_t height = 0;
    int32_t xOffset = 0;
    int32_t yOffset = 0;
    int64_t gain = 0;
    int32_t min = 0;
    int32_t max = 0;
    int64_t minExp = 0;
    int64_t maxExp = 0;
    int32_t step = 0;
    int32_t value = 0;
    bool autogain = false;
    int64_t exposure = 0;
    bool autoexposure = false;
    int32_t nSVal;
    uint32_t numerator = 0;
    uint32_t denominator = 0;
    uint32_t pixelFormat = 0;
    QString pixelFormatText;
    uint32_t bytesPerLine = 0;

    m_Camera.EnumAllControlNewStyle();
    m_Camera.PrepareFrameRate();
    m_Camera.PrepareCrop();

    if(isCalledFromOnOpen)
    {
        SetDefaultLabels();
        if(m_Camera.UsesSubdevices()) {
            std::string deviceChar = "";
            deviceChar = m_Camera.GetGainDeviceChar();
            ui.m_labelGain->setText(ui.m_labelGain->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetAutoGainDeviceChar();
            ui.m_labelGainAuto->setText(ui.m_labelGainAuto->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetExposureAutoDeviceChar();
            ui.m_labelExposureAuto->setText(ui.m_labelExposureAuto->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetGammaDeviceChar();
            ui.m_labelGamma->setText(ui.m_labelGamma->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetReverseXDeviceChar();
            ui.m_FlipHorizontalCheckBox->setText(ui.m_FlipHorizontalCheckBox->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetReverseYDeviceChar();
            ui.m_FlipVerticalCheckBox->setText(ui.m_FlipVerticalCheckBox->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetBrightnessDeviceChar();
            ui.m_labelBrightness->setText(ui.m_labelBrightness->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetAutoWhiteBalanceDeviceChar();
            ui.m_labelWhiteBalanceAuto->setText(ui.m_labelWhiteBalanceAuto->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetFrameRateDeviceChar();
            ui.m_labelFrameRateAuto->setText(ui.m_labelFrameRateAuto->text() + " (" + QString(deviceChar.c_str()) + ")");
            ui.m_labelFrameRate->setText(ui.m_labelFrameRate->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetCropDeviceChar();
            ui.m_CropLabel->setText(ui.m_CropLabel->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetPixelFormatDeviceChar();
            ui.m_labelPixelFormats->setText(ui.m_labelPixelFormats->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetWidthDeviceChar();
            ui.m_labelWidth->setText(ui.m_labelWidth->text() + " (" + QString(deviceChar.c_str()) + ")");
            deviceChar = m_Camera.GetHeightDeviceChar();
            ui.m_labelHeight->setText(ui.m_labelHeight->text() + " (" + QString(deviceChar.c_str()) + ")");
        }
    }

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

    ui.m_sliderGain->blockSignals(true);
    if (int64_t min64, max64; m_Camera.ReadMinMaxGain(min64, max64) != -2)
    {
        ui.m_sliderGain->setEnabled(true);
        ui.m_sliderGain->setMinimum(min64);
        ui.m_sliderGain->setMaximum(max64);
        UpdateSlidersPositions(ui.m_sliderGain, gain);
    }
    else
    {
        ui.m_sliderGain->setEnabled(false);
    }
    ui.m_sliderGain->blockSignals(false);

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
        LOG_EX("V4L2Viewer::GetImageInformation: exposure controllable, exposure: %d", exposure);
        ui.m_edExposure->setEnabled(true);
        ui.m_labelExposure->setEnabled(true);
    }
    else
    {
        LOG_EX("V4L2Viewer::GetImageInformation: exposure not controllable");
        ui.m_edExposure->setEnabled(false);
        ui.m_labelExposure->setEnabled(false);
    }

    ui.m_edExposure->setText(QString("%1").arg(exposure));
    ui.m_sliderExposure->blockSignals(true);
    if (m_Camera.ReadMinMaxExposure(minExp, maxExp) != -2)
    {
        if(isCalledFromOnOpen)
        {
            if(m_Camera.UsesSubdevices()) {

                std::string deviceChar = m_Camera.GetExposureDeviceChar();
                ui.m_labelExposure->setText(ui.m_labelExposure->text() + " (" + QString(deviceChar.c_str()) + ")");
            }
        }
        LOG_EX("V4L2Viewer::GetImageInformation: setting text to exposure");
        ui.m_edExposure->setText(QString("%1").arg(exposure));
        m_MinimumExposure = minExp;
        m_MaximumExposure = maxExp;

        int64_t result = GetSliderValueFromLog(exposure);
        UpdateSlidersPositions(ui.m_sliderExposure, result);

    }

    ui.m_sliderExposure->blockSignals(false);

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
    if (m_Camera.ReadBrightness(nSVal) != -2)
    {
        ui.m_edBrightness->setEnabled(true);
        ui.m_labelBrightness->setEnabled(true);
        ui.m_edBrightness->setText(QString("%1").arg(nSVal));
    }
    else
    {
        ui.m_edBrightness->setEnabled(false);
        ui.m_labelBrightness->setEnabled(false);
    }

    min = 0;
    max = 0;
    ui.m_sliderBrightness->blockSignals(true);
    if (m_Camera.ReadMinMaxBrightness(min, max) != -2)
    {
        ui.m_sliderBrightness->setEnabled(true);
        ui.m_sliderBrightness->setMinimum(min);
        ui.m_sliderBrightness->setMaximum(max);
        UpdateSlidersPositions(ui.m_sliderBrightness, nSVal);
    }
    else
    {
        ui.m_sliderBrightness->setEnabled(false);
    }
    ui.m_sliderBrightness->blockSignals(false);

    if(m_Camera.ReadFrameSize(width, height) == 0)
    {
        LOG_EX("V4L2Viewer::GetImageInformation: width=%d, height=%d", width, height);
        ui.m_edWidth->setEnabled(true);
        ui.m_edHeight->setEnabled(true);
        ui.m_edWidth->setText(QString("%1").arg(width));
        ui.m_edHeight->setText(QString("%1").arg(height));
    }

    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);

    if (!m_bIsStreaming)
    {
        if (m_Camera.ReadFrameRate(numerator, denominator, width, height, pixelFormat) != -2)
        {
            ui.m_labelFrameRateAuto->setEnabled(true);
            ui.m_chkFrameRateAuto->setEnabled(true);

            if (denominator == 0)
            {
                ui.m_chkFrameRateAuto->setChecked(true);
                ui.m_edFrameRate->setEnabled(false);
                ui.m_labelFrameRate->setEnabled(false);
            }
            else
            {
                ui.m_chkFrameRateAuto->setChecked(false);
                ui.m_edFrameRate->setEnabled(true);
                ui.m_labelFrameRate->setEnabled(true);
                denominator /= 1000;
                ui.m_edFrameRate->setText(QString("%1").arg(denominator));
            }
        }
        else
        {
            ui.m_edFrameRate->setEnabled(false);
            ui.m_labelFrameRate->setEnabled(false);
            ui.m_labelFrameRateAuto->setEnabled(false);
            ui.m_chkFrameRateAuto->setEnabled(false);
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

    nSVal = 0;
    if (m_Camera.ReadGamma(nSVal) != -2)
    {
        ui.m_edGamma->setEnabled(true);
        ui.m_labelGamma->setEnabled(true);
        ui.m_edGamma->setText(QString("%1").arg(nSVal));
        UpdateSlidersPositions(ui.m_sliderGamma, nSVal);
    }
    else
    {
        ui.m_edGamma->setEnabled(false);
        ui.m_labelGamma->setEnabled(false);
    }

    ui.m_sliderGamma->blockSignals(true);
    if (m_Camera.ReadMinMaxGamma(minExp, maxExp) != -2)
    {
        ui.m_sliderGamma->setEnabled(true);
        ui.m_sliderGamma->setMinimum(minExp);
        ui.m_sliderGamma->setMaximum(maxExp);
    }
    else
    {
        ui.m_sliderGamma->setEnabled(false);
    }
    ui.m_sliderGamma->blockSignals(false);

    ui.m_sliderExposure->setDisabled(autoexposure && ui.m_sliderExposure->isEnabled());
    ui.m_sliderGain->setDisabled(autogain && ui.m_sliderGain->isEnabled());
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

    if(m_Camera.ReadFrameSize(width, height) == 0)
    {
        LOG_EX("V4L2Viewer::UpdateCameraFormat: width=%d, height=%d", width, height);
        ui.m_edWidth->setEnabled(true);
        ui.m_edHeight->setEnabled(true);
        ui.m_edWidth->setText(QString("%1").arg(width));
        ui.m_edHeight->setText(QString("%1").arg(height));
    }

    m_Camera.ReadPixelFormat(pixelFormat, bytesPerLine, pixelFormatText);
    m_Camera.ReadFormats();
    UpdateCurrentPixelFormatOnList(QString::fromStdString(v4l2helper::ConvertPixelFormat2String(pixelFormat)));
}

void V4L2Viewer::UpdateCurrentPixelFormatOnList(QString pixelFormat)
{
    ui.m_pixelFormats->blockSignals(true);
    for (int i=0; i<ui.m_pixelFormats->count(); ++i)
    {
        if (ui.m_pixelFormats->itemText(i) == pixelFormat)
        {
			std::string tmp = pixelFormat.toStdString();
			char *s = (char*)tmp.c_str();
			uint32_t result = 0;

			if (tmp.size() == 4)
			{
				result += *s++;
				result += *s++ << 8;
				result += *s++ << 16;
				result += *s++ << 24;
			}

			QList<QString> framesizes = m_Camera.GetFrameSizes(result);

			ui.m_frameSizes->blockSignals(true);
			ui.m_frameSizes->clear();
			ui.m_frameSizes->addItems(framesizes);
			ui.m_frameSizes->setCurrentIndex(m_Camera.GetFrameSizeIndex());
			ui.m_frameSizes->blockSignals(false);

            ui.m_pixelFormats->setCurrentIndex(i);
        }
    }
    ui.m_pixelFormats->blockSignals(false);
}

QString V4L2Viewer::GetDeviceInfo()
{
    std::string tmp;

    QString firmware = QString(tr("Camera FW Version = %1")).arg(QString::fromStdString(m_Camera.getAvtDeviceFirmwareVersion()));
    QString devSerial = QString(tr("Camera Serial Number = %1")).arg(QString::fromStdString(m_Camera.getAvtDeviceSerialNumber()));

    m_Camera.GetCameraDriverName(tmp);
    QString driverName = QString(tr("Driver name = %1")).arg(tmp.c_str());
    m_Camera.GetCameraBusInfo(tmp);
    QString busInfo = QString(tr("Bus info = %1")).arg(tmp.c_str());
    m_Camera.GetCameraDriverVersion(tmp);
    QString driverVer = QString(tr("Driver version = %1")).arg(tmp.c_str());
    m_Camera.GetCameraCapabilities(tmp);
    QString capabilities = QString(tr("Capabilities = %1")).arg(tmp.c_str());
    return QString(firmware + "<br>" + devSerial + "<br>" + driverName + "<br>" + busInfo + "<br>" + driverVer + "<br>" + capabilities + "<br>");
}

void V4L2Viewer::UpdateSlidersPositions(QSlider *slider, int32_t value)
{
    slider->blockSignals(true);
    slider->setValue(value);
    slider->blockSignals(false);
}

int64_t V4L2Viewer::GetSliderValueFromLog(int64_t value)
{
    double logExpMin = log(m_MinimumExposure);
    double logExpMax = log(m_MaximumExposure);
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

        m_Camera.OpenDevice(deviceName, m_SubDevices, m_BLOCKING_MODE, m_BUFFER_TYPE, m_VIDIOC_TRY_FMT);
        m_Camera.GetCameraReadCapability(ioRead);
        m_Camera.CloseDevice();
    }
}

void V4L2Viewer::SetTitleText()
{
    setWindowTitle(QString(tr("%1 V%2.%3.%4")).arg(APP_NAME).arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_PATCH));
}

void V4L2Viewer::OnFrameSizeIndexChanged(int index)
{
	m_Camera.SetFrameSizeByIndex(index);

	GetImageInformation();
}
