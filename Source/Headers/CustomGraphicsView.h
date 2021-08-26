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


#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H

#include <QGraphicsView>

class CustomGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    CustomGraphicsView(QWidget *parent);
    // This function restore scale factor on the image area
    void SetScaleFactorToDefault();
    // This function returns current value of the scale factor
    //
    // Returns:
    // (double) - scale factor
    double GetScaleFactorValue();
    // This function Transforms image view to fit the area boundaries
    void TransformImageView();
    // This function performs zoom in on the image view
    void OnZoomIn();
    // This function performs zoom out on the image view
    void OnZoomOut();
    // This function sets pointer to the pixmapItem to have control over
    // pixels' colors
    //
    // Parameters:
    // [in] (QGraphicsPixmapItem *) pixmapItem - pointer to be passed
    void SetPixmapItem(QGraphicsPixmapItem *pixmapItem);
    // This function sets whether zoom is allowed or not. It is mostly used
    // when camera is being opened or closed
    //
    // Parameters:
    // [in] (bool) state - new state value to be passed
    void SetZoomAllowed(bool state);

    // This static attribute contains maximum zoom in value
    static double MAX_ZOOM_IN;
    // This static attribute contains maximum zoom out value
    static double MAX_ZOOM_OUT;
    // This static attribute contains increment step
    static double ZOOM_INCREMENT;

signals:
    // This signal sends information about updating label which shows
    // zoom in percentages
    void UpdateZoomLabel();

private:
    // This function is overrided, and it performs wheel event in order
    // to perform only zooming, not moving with scroll bars
    //
    // Parameters:
    // [in] (QWheelEvent *) event - event to be passed
    virtual void wheelEvent(QWheelEvent *event) override;
    // This function is overrided, and it performs press event
    // in order to show tooltip with pixel data
    //
    // Parameters:
    // [in] (QMouseEvent *) event - event to be passed
    virtual void mousePressEvent(QMouseEvent *event) override;

    double m_dScaleFactor;
    QGraphicsPixmapItem *m_pPixmapItem;
    bool m_bIsZoomAllowed;
};

#endif // CUSTOMGRAPHICSVIEW_H
