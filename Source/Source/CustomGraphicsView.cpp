#include "CustomGraphicsView.h"
#include <QWheelEvent>
#include <QGraphicsPixmapItem>
#include <QToolTip>
#include <QDebug>

double CustomGraphicsView::MAX_ZOOM_IN = 16.0;
double CustomGraphicsView::MAX_ZOOM_OUT = 1/8.0;
double CustomGraphicsView::ZOOM_INCREMENT = 2.0;

CustomGraphicsView::CustomGraphicsView(QWidget *parent): QGraphicsView(parent)
  , m_dScaleFactor(1.0)
  , m_pPixmapItem(nullptr)
{

}

void CustomGraphicsView::wheelEvent(QWheelEvent *event)
{
    QPointF point = mapToScene(event->pos());
    centerOn(point);
    if (event->delta() > 0)
        OnZoomIn();
    else
        OnZoomOut();
}

void CustomGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (m_pPixmapItem == nullptr)
    {
        return;
    }

    if (scene() == nullptr)
    {
        return;
    }

    QRectF sceneR = sceneRect();
    QPoint mousePos = event->pos();
    QPointF imageScenePointF = mapToScene(mousePos);

    if (imageScenePointF.x() < sceneR.width() && imageScenePointF.y() < sceneR.height() &&
        imageScenePointF.x() >= 0 && imageScenePointF.y() >= 0)
    {
        QImage image = m_pPixmapItem->pixmap().toImage();
        QColor myPixel = image.pixel(static_cast<int>(imageScenePointF.x()), static_cast<int>(imageScenePointF.y()));

        QToolTip::showText(mapToGlobal(mousePos), QString("x:%1, y:%2, r:%3/g:%4/b:%5")
                           .arg(static_cast<int>(imageScenePointF.x())).arg(static_cast<int>(imageScenePointF.y()))
                           .arg(myPixel.red())
                           .arg(myPixel.green())
                           .arg(myPixel.blue()), this);
    }
}

void CustomGraphicsView::SetScaleFactorToDefault()
{
    m_dScaleFactor = 1.0;
    resetMatrix();
}

double CustomGraphicsView::GetScaleFactorValue()
{
    return m_dScaleFactor;
}

void CustomGraphicsView::TransformImageView()
{
    QTransform transformation;
    transformation.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transformation);
}

void CustomGraphicsView::OnZoomIn()
{
    if (m_dScaleFactor < MAX_ZOOM_IN )
    {
        m_dScaleFactor *= ZOOM_INCREMENT;
        QTransform transformation;
        transformation.scale(m_dScaleFactor, m_dScaleFactor);
        setTransform(transformation);
    }
}

void CustomGraphicsView::OnZoomOut()
{
    if (m_dScaleFactor > MAX_ZOOM_OUT )
    {
        m_dScaleFactor *= (1 / ZOOM_INCREMENT);
        QTransform transformation;
        transformation.scale(m_dScaleFactor, m_dScaleFactor);
        setTransform(transformation);
    }
}

void CustomGraphicsView::SetPixmapItem(QGraphicsPixmapItem *pixmapItem)
{
    m_pPixmapItem = pixmapItem;
}


