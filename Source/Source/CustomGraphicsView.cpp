#include "CustomGraphicsView.h"
#include <QWheelEvent>
#include <QToolTip>

double CustomGraphicsView::MAX_ZOOM_IN = 16.0;
double CustomGraphicsView::MAX_ZOOM_OUT = 1/8.0;
double CustomGraphicsView::ZOOM_INCREMENT = 2.0;

CustomGraphicsView::CustomGraphicsView(QWidget *parent): QGraphicsView(parent)
  , m_dScaleFactor(1.0)
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
    QPoint point = event->pos();
    if (m_dScaleFactor >= 1)
    {
        //QColor myPixel = image.pixel(static_cast<int>(imageScenePointF.x()), static_cast<int>(imageScenePointF.y()));
        QToolTip::showText(mapToGlobal(point), QString("x:%1, y:%2, r:%3/g:%4/b:%5")
                           .arg(static_cast<int>(point.x())).arg(static_cast<int>(point.y()))
                           .arg(125)
                           .arg(125)
                           .arg(125), this);
    }

//    QPoint imageViewPoint = ui.m_ImageView->mapFrom(this, event->pos());
//    QPointF imageScenePointF = ui.m_ImageView->mapToScene(imageViewPoint);

//    if (m_PixmapItem != nullptr && m_PixmapItem->pixmap().rect().contains(imageScenePointF.toPoint()))
//    {
//        if (m_dScaleFactor >= 1)
//        {
//            QImage image = m_PixmapItem->pixmap().toImage();
//            QColor myPixel = image.pixel(static_cast<int>(imageScenePointF.x()), static_cast<int>(imageScenePointF.y()));
//            QToolTip::showText(ui.m_ImageView->mapToGlobal(imageViewPoint), QString("x:%1, y:%2, r:%3/g:%4/b:%5")
//                               .arg(static_cast<int>(imageScenePointF.x())).arg(static_cast<int>(imageScenePointF.y()))
//                               .arg(myPixel.red())
//                               .arg(myPixel.green())
//                               .arg(myPixel.blue()), this);
//        }
//    }
}

void CustomGraphicsView::SetScaleFactorToDefault()
{
    m_dScaleFactor = 1.0;
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
    m_dScaleFactor *= ZOOM_INCREMENT;

    QTransform transformation;
    transformation.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transformation);
}

void CustomGraphicsView::OnZoomOut()
{
    m_dScaleFactor *= (1 / ZOOM_INCREMENT);

    QTransform transformation;
    transformation.scale(m_dScaleFactor, m_dScaleFactor);
    setTransform(transformation);
}


