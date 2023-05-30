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
                  "  border:none;"
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
