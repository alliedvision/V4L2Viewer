#include "SoftwareRenderSystem.h"
#include "ImageTransform.h"
#include <QWheelEvent>
#include <QGraphicsPixmapItem>
#include <QToolTip>
#include <QMutexLocker>


static void DoNothing() {}


SoftwareRenderSystem::SoftwareRenderSystem()
  : scrollArea(new QScrollArea)
  , layout(new QBoxLayout(QBoxLayout::LeftToRight))
  , widget(new SoftwareRenderWidget)
  , nextDoneCallback(&DoNothing)
{
    scrollArea->setLayout(layout);
    scrollArea->setStyleSheet("QScrollArea{border:none;}");
    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    conversionThread = std::unique_ptr<QThread>(QThread::create([&] {
       ConversionThreadMain();
    }));
    conversionThread->start();

    connect(widget, SIGNAL(RequestZoom(QPointF,bool)), this, SLOT(ZoomRequestedByWidget(QPointF,bool)));
    connect(widget, SIGNAL(Clicked(QPointF)), this, SLOT(ImageClicked(QPointF)));
}

SoftwareRenderSystem::~SoftwareRenderSystem() {
    conversionThread->requestInterruption();
    newFrameAvailable.wakeAll();
    conversionThread->wait();
}

void SoftwareRenderSystem::ZoomRequestedByWidget(QPointF center, bool zoomIn) {
    emit RequestZoom(center, zoomIn);
}

void SoftwareRenderSystem::ImageClicked(QPointF point) {
    emit PixelClicked(point);
}

void SoftwareRenderSystem::ConversionThreadMain() {
    while(!conversionThread->isInterruptionRequested()) {
        frameAvailableMutex.lock();
        while(!bufferAvailable) { // avoid lost or spurious wakeup
          newFrameAvailable.wait(&frameAvailableMutex);
          if(!bufferAvailable && conversionThread->isInterruptionRequested()) {
            frameAvailableMutex.unlock();
            return;
          }
        }

        BufferWrapper const buffer = nextBuffer;
        auto doneCallback = nextDoneCallback;
        nextDoneCallback = &DoNothing;
        bufferAvailable = false;
        frameAvailableMutex.unlock();

        QImage convertedImage;
        int result = ImageTransform::ConvertFrame(buffer.data, buffer.length,
                                              buffer.width, buffer.height, buffer.pixelFormat,
                                              buffer.payloadSize, buffer.bytesPerLine, convertedImage);

        auto pixmap = QPixmap::fromImage(convertedImage);
        widget->SetPixmap(pixmap);
        renderFPS.trigger();

        doneCallback();
    }
}

void SoftwareRenderSystem::ApplyScale() {
    QTransform transformation;
    transformation.scale(scaleFactor * (flipX ? -1.0 : 1.0),
                         scaleFactor * (flipY ? -1.0 : 1.0));
    widget->setTransform(transformation);
}

void SoftwareRenderSystem::SetScaleFactor(double scale)
{
    scaleFactor = scale;
    ApplyScale();
}

void SoftwareRenderSystem::SetFlipX(bool flip) {
    flipX = flip;
    ApplyScale();
}

void SoftwareRenderSystem::SetFlipY(bool flip) {
    flipY = flip;
    ApplyScale();
}

QWidget* SoftwareRenderSystem::GetWidget() const {
    return scrollArea;
}

void SoftwareRenderSystem::PassFrame(BufferWrapper const& buffer, std::function<void()> doneCallback) {
    QMutexLocker locker(&frameAvailableMutex);
    nextBuffer = buffer;
    // always invoke callback, worker sets it to doNothing if it took ownership of the buffer 
    nextDoneCallback();
    nextDoneCallback = doneCallback;
    bufferAvailable = true;
    newFrameAvailable.wakeAll();
}

bool SoftwareRenderSystem::CanRender(uint32_t pixelFormat) const {
    return ImageTransform::CanConvert(pixelFormat);
}
