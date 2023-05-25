#ifndef RENDERSYSTEM_H
#define RENDERSYSTEM_H

#include <QWidget>
#include <functional>
#include "BufferWrapper.h"
#include "FPSCalculator.h"

class RenderSystem: public QObject
{
    Q_OBJECT
public:
    RenderSystem();
    virtual void SetFlipX(bool flip) = 0;
    virtual void SetFlipY(bool flip) = 0;
    virtual void SetScaleFactor(double scaleFactor) = 0;
    virtual QWidget* GetWidget() const = 0;
    virtual void PassFrame(BufferWrapper const& buffer, std::function<void()> doneCallback) = 0;
    double GetRenderedFPS();

signals:
    void zoomRequested(QPointF center, bool zoomIn);

protected:
    double scaleFactor = 1.0;
    FPSCalculator renderFPS;

};
#endif
