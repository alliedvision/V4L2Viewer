/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CustomGraphicsView.h

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
