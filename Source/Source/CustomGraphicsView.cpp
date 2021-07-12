/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CustomGraphicsView.cpp

  Description: This class inehrits by QGraphicsView. It is used as a main
               image area. It is done as a seperate class to allow more
               actions on the area to be implemented.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

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


