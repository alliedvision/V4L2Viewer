#ifndef SOFTWARERENDERSYSTEM_H
#define SOFTWARERENDERSYSTEM_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "RenderSystem.h"
#include "SoftwareRenderWidget.h"
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QBoxLayout>
#include <QScrollArea>
#include <memory>
#include <thread>

class SoftwareRenderSystem: public RenderSystem
{
    Q_OBJECT
public:
    SoftwareRenderSystem();
    ~SoftwareRenderSystem();
    void SetScaleFactor(double scale) override;
    void SetFlipX(bool flip);
    void SetFlipY(bool flip);

    QWidget* GetWidget() const override;
    void PassFrame(BufferWrapper const& buffer, std::function<void()> doneCallback) override;
    bool CanRender(uint32_t pixelFormat) const override;

signals:
    void RequestZoom(QPointF center, bool zoomIn);
    void PixelClicked(QPointF pos);

protected slots:
    void ZoomRequestedByWidget(QPointF center, bool zoomIn);
    void ImageClicked(QPointF point);


private:
    QScrollArea *scrollArea;
    QBoxLayout *layout;
    SoftwareRenderWidget *widget;
    bool flipX = false;
    bool flipY = false;

    void ApplyScale();
    void ConversionThreadMain();

    // TODO encapsulate for re-use in hwaccel renderer?
    std::atomic<bool> bufferAvailable;
    std::unique_ptr<std::thread> conversionThread;
    std::atomic_bool  stopConversionThread{false};
    QMutex frameAvailableMutex;
    QWaitCondition newFrameAvailable;
    BufferWrapper nextBuffer;
    std::function<void()> nextDoneCallback;
};

#endif
