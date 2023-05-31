#ifndef EGLRENDERSYSTEM_H
#define EGLRENDERSYSTEM_H

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "RenderSystem.h"
#include "EGLRenderWidget.h"
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <atomic>
#include <QScrollBar>
#include <QGridLayout>



class EGLRenderSystem: public RenderSystem
{
    Q_OBJECT
public:
    EGLRenderSystem();
    ~EGLRenderSystem();
    void SetScaleFactor(double scale) override;
    void SetFlipX(bool flip);
    void SetFlipY(bool flip);

    QWidget* GetWidget() const override;
    void PassFrame(BufferWrapper const& buffer, std::function<void()> doneCallback) override;
    bool CanRender(uint32_t pixelFormat) const override;

signals:
    void RequestZoom(QPointF center, bool zoomIn);
    void PixelClicked(QPointF pos);
    void EffectiveSizeChanged();

protected slots:
    void ZoomRequestedByWidget(QPointF center, bool zoomIn);
    void ImageClicked(QPointF point);

private slots:
    void UpdateScrollbars();
    void ScrollChanged();


private:
    QWidget *container;
    QGridLayout *layout;
    QScrollBar *verticalScrollbar;
    QScrollBar *horizontalScrollbar;
    EGLRenderWidget *glWidget;
    bool flipX = false;
    bool flipY = false;
    int curWidth = 0;
    int curHeight = 0;
    int curPixelformat = 0;
    std::atomic<bool> newFrame = false;
};

#endif
