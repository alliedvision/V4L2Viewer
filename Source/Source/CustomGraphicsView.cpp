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


#include "CustomGraphicsView.h"
#include <QWheelEvent>
#include <QGraphicsPixmapItem>
#include <QToolTip>

double CustomGraphicsView::MAX_ZOOM_IN = 16.0;
double CustomGraphicsView::MAX_ZOOM_OUT = 1/8.0;
double CustomGraphicsView::ZOOM_INCREMENT = 2.0;

CustomGraphicsView::CustomGraphicsView(QWidget *parent): QGraphicsView(parent)
  , m_dScaleFactor(1.0)
  , m_pPixmapItem(nullptr)
  , m_bIsZoomAllowed(false)
{

}

void CustomGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (m_bIsZoomAllowed)
    {
        QPointF point = mapToScene(event->pos());
        centerOn(point);
        if (event->delta() > 0)
            OnZoomIn();
        else
            OnZoomOut();
    }
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
    if (m_dScaleFactor < MAX_ZOOM_IN)
    {
        m_dScaleFactor *= ZOOM_INCREMENT;
        QTransform transformation;
        transformation.scale(m_dScaleFactor, m_dScaleFactor);
        setTransform(transformation);
        emit UpdateZoomLabel();
    }
}

void CustomGraphicsView::OnZoomOut()
{
    if (m_dScaleFactor > MAX_ZOOM_OUT)
    {
        m_dScaleFactor *= (1 / ZOOM_INCREMENT);
        QTransform transformation;
        transformation.scale(m_dScaleFactor, m_dScaleFactor);
        setTransform(transformation);
        emit UpdateZoomLabel();
    }
}

void CustomGraphicsView::SetPixmapItem(QGraphicsPixmapItem *pixmapItem)
{
    m_pPixmapItem = pixmapItem;
}

void CustomGraphicsView::SetZoomAllowed(bool state)
{
    m_bIsZoomAllowed = state;
}


