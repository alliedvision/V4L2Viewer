#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>

class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    CustomGraphicsView(QWidget *parent);
    void SetScaleFactorToDefault();
    double GetScaleFactorValue();
    void TransformImageView();
    void OnZoomIn();
    void OnZoomOut();
    void SetPixmapItem(QGraphicsPixmapItem *pixmapItem);
    void SetZoomAllowed(bool state);

    static double MAX_ZOOM_IN;
    static double MAX_ZOOM_OUT;
    static double ZOOM_INCREMENT;

signals:
    void UpdateZoomLabel();

private:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    //// Our Qt image to display
    double m_dScaleFactor;
    QGraphicsPixmapItem *m_pPixmapItem;
    bool m_bIsZoomAllowed;
};

#endif // CUSTOMGRAPHICSVIEW_H
