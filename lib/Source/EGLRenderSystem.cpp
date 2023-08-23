#include "EGLRenderSystem.h"
#include "ImageTransform.h"
#include <QWheelEvent>
#include <QToolTip>



EGLRenderSystem::EGLRenderSystem()
  : container(new QWidget)
  , layout(new QGridLayout)
  , verticalScrollbar(new QScrollBar(Qt::Vertical))
  , horizontalScrollbar(new QScrollBar(Qt::Horizontal))
  , glWidget(new EGLRenderWidget([this] { 
        if(newFrame.exchange(false)) {
            renderFPS.trigger();
        }
    }))
{
    container->setLayout(layout);
    
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    layout->addWidget(glWidget, 0, 0, 1, 1);
    layout->addWidget(verticalScrollbar, 0, 1, 1, 1);
    layout->addWidget(horizontalScrollbar, 1, 0, 1, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 0);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 0);


    connect(this, SIGNAL(EffectiveSizeChanged()), this, SLOT(UpdateScrollbars()), Qt::QueuedConnection);
    connect(glWidget, SIGNAL(resized()), this, SLOT(UpdateScrollbars()));
    connect(horizontalScrollbar, SIGNAL(valueChanged(int)), this, SLOT(ScrollChanged()));
    connect(verticalScrollbar, SIGNAL(valueChanged(int)), this, SLOT(ScrollChanged()));
    connect(glWidget, SIGNAL(requestZoom(QPointF,bool)), this, SLOT(ZoomRequestedByWidget(QPointF,bool)));
    connect(glWidget, SIGNAL(clicked(QPointF)), this, SLOT(ImageClicked(QPointF)));
}

EGLRenderSystem::~EGLRenderSystem() {
}

void EGLRenderSystem::ZoomRequestedByWidget(QPointF center, bool zoomIn) {
    emit RequestZoom(center, zoomIn);
}

void EGLRenderSystem::ImageClicked(QPointF point) {
    emit PixelClicked(point);
}

void EGLRenderSystem::SetScaleFactor(double scale)
{
    scaleFactor = scale;
    glWidget->setScale(scale);
    emit EffectiveSizeChanged();
}

void EGLRenderSystem::SetFlipX(bool flip) {
    flipX = flip;
    glWidget->setFlip(flipX, flipY);
}

void EGLRenderSystem::SetFlipY(bool flip) {
    flipY = flip;
    glWidget->setFlip(flipX, flipY);
}

QWidget* EGLRenderSystem::GetWidget() const {
    return container;
}

void EGLRenderSystem::UpdateScrollbars() {
    int const effectiveWidth = int(scaleFactor * double(curWidth));
    int const effectiveHeight = int(scaleFactor * double(curHeight));
    int const scrollBarWidth = verticalScrollbar->width();
    int const scrollBarHeight = horizontalScrollbar->height();

    if(effectiveHeight <= container->height() && effectiveWidth <= container->width()) {
        horizontalScrollbar->hide();
        verticalScrollbar->hide();
    } else if (effectiveWidth > container->width()) {
        horizontalScrollbar->show();
        verticalScrollbar->setVisible(effectiveHeight + scrollBarHeight > container->height());
    } else if (effectiveHeight > container->height()) {
        verticalScrollbar->show();
        horizontalScrollbar->setVisible(effectiveWidth + scrollBarWidth > container->width());
    }

    if(verticalScrollbar->isVisible()) {
        auto const range = effectiveHeight + (horizontalScrollbar->isVisible() ? scrollBarHeight : 0) - container->height();
        verticalScrollbar->setMinimum(-(range+1) / 2);
        verticalScrollbar->setMaximum(range / 2);
    } else {
        verticalScrollbar->setMinimum(0);
        verticalScrollbar->setMaximum(0);
    }

    if(horizontalScrollbar->isVisible()) {
        auto const range = effectiveWidth + (verticalScrollbar->isVisible() ? scrollBarWidth : 0) - container->width();
        horizontalScrollbar->setMinimum(-(range+1)/2);
        horizontalScrollbar->setMaximum(range/2);
    } else {
        horizontalScrollbar->setMinimum(0);
        horizontalScrollbar->setMaximum(0);
    }

    ScrollChanged();
}

void EGLRenderSystem::PassFrame(BufferWrapper const& buffer, std::function<void()> doneCallback) {
    if(buffer.width != curWidth || buffer.height != curHeight || buffer.pixelFormat != curPixelformat) {
        glWidget->setFormat(buffer.width, buffer.height, buffer.pixelFormat);
        curWidth = buffer.width;
        curHeight = buffer.height;
        curPixelformat = buffer.pixelFormat;
        emit EffectiveSizeChanged();
    }

    glWidget->nextFrame(buffer, doneCallback);
    newFrame = true;
}

void EGLRenderSystem::ScrollChanged() {
    glWidget->setScroll(horizontalScrollbar->value(), verticalScrollbar->value());
}

bool EGLRenderSystem::CanRender(uint32_t pixelFormat) const {
    return EGLRenderWidget::canRender(pixelFormat);
}
