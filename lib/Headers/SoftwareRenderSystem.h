#ifndef SOFTWARERENDERSYSTEM_H
#define SOFTWARERENDERSYSTEM_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "RenderSystem.h"
#include "SoftwareRenderWidget.h"
#include <QMutex>
#include <QThread>
#include <QWaitCondition>


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

signals:
    void RequestZoom(QPointF center, bool zoomIn);
    void PixelClicked(QPointF pos);

protected slots:
    void ZoomRequestedByWidget(QPointF center, bool zoomIn);
    void ImageClicked(QPointF point);


private:
    SoftwareRenderWidget *widget;
    bool flipX = false;
    bool flipY = false;

    void ApplyScale();
    void ConversionThreadMain();

    // TODO encapsulate for re-use in hwaccel renderer?
    std::atomic<bool> bufferAvailable;
    std::unique_ptr<QThread> conversionThread;
    QMutex frameAvailableMutex;
    QWaitCondition newFrameAvailable;
    BufferWrapper nextBuffer;
    std::function<void()> nextDoneCallback;
};

#endif
