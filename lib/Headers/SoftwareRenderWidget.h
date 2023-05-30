#ifndef SOFTWARERENDERWIDGET_H
#define SOFTWARERENDERWIDGET_H

#include <QPixmap>

class SoftwareRenderWidget: public QGraphicsView {
  Q_OBJECT
private:
  QGraphicsScene *m_Scene;
  QGraphicsPixmapItem m_PixmapItem;
  QPixmap *m_CurrentPixmap = nullptr;

signals:
  void RequestZoom(QPointF center, bool zoomIn);
  void Clicked(QPointF point);
  void SetPixmapSignal(QPixmap pixmap);

private slots:
  void OnSetPixmap(QPixmap pixmap);

public:
  SoftwareRenderWidget(QWidget *parent = nullptr);
  ~SoftwareRenderWidget();

  void SetPixmap(QPixmap pixmap);

  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
};

#endif
