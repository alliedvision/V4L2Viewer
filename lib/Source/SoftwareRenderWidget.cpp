/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 -2025 Allied Vision Technologies GmbH

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

#include "SoftwareRenderSystem.h"
#include <QWheelEvent>

SoftwareRenderWidget::SoftwareRenderWidget(QWidget *parent)
  : QGraphicsView(parent) 
  , m_Scene(new QGraphicsScene)
  {
    setScene(m_Scene);
    m_Scene->addItem(&m_PixmapItem);
    connect(this, SIGNAL(SetPixmapSignal(QPixmap)), this, SLOT(OnSetPixmap(QPixmap)));
    setStyleSheet("QGraphicsView {"
                  "  background-color: rgb(19,20,21);"
                  "  border:20px;"
                  "}");
  }

SoftwareRenderWidget::~SoftwareRenderWidget() {
    delete m_CurrentPixmap;
}

void SoftwareRenderWidget::OnSetPixmap(QPixmap pixmap) {
    m_PixmapItem.setPixmap(pixmap);
    m_Scene->setSceneRect(0, 0, pixmap.width(), pixmap.height());
    show();
}

void SoftwareRenderWidget::SetPixmap(QPixmap pixmap) {
  emit SetPixmapSignal(pixmap);
}

void SoftwareRenderWidget::wheelEvent(QWheelEvent *event)
{
    auto const point = mapToScene(
        #if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
          event->position().toPoint()
        #else
          event->pos()
        #endif
    );
    emit RequestZoom(point, event->angleDelta().y() > 0);
}

void SoftwareRenderWidget::mousePressEvent(QMouseEvent *event)
{
    if (scene() == nullptr)
    {
        return;
    }

    QRectF sceneR = sceneRect();
    QPoint mousePos = event->pos();
    QPointF imageScenePointF = mapToScene(mousePos);

    emit Clicked(imageScenePointF);
}
