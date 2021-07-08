/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ActiveExposureWidget.h

  Description: This widget class stores widgets for updating active exposure
               control.

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

#ifndef ACTIVEEXPOSUREWIDGET_H
#define ACTIVEEXPOSUREWIDGET_H

#include <QWidget>
#include "ui_ActiveExposureWidget.h"

class ActiveExposureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ActiveExposureWidget(QWidget *parent = nullptr);
    // This function updates invert widget with the given state
    //
    // Parameters:
    // [in] (bool) state - passed new state value
    void SetInvert(bool state);
    // This function updates active state widget with the given value
    //
    // Parameters:
    // [in] (bool) state - passed new state value
    void SetActive(bool state);
    // This function updates active line selector range widget with the given values
    //
    // Parameters:
    // [in] (int32_t) currentValue - new current value
    // [in] (int32_t) min - minimum value to be passed
    // [in] (int32_t) max - maximum value to be passed
    // [in] (int32_t) step - step value to be passed
    void SetLineSelectorRange(int32_t currentValue, int32_t min, int32_t max, int32_t step);
    // This function blocks invert and line selector widgets, when active checkbox is set to true
    //
    // Parameters:
    // [in] (bool) state - passed new state value
    void BlockInvertAndLineSelector(bool state);
    Ui::ActiveExposureWidget ui;

signals:
    // This signal sends new invert state to the Camera class
    //
    // Parameters:
    // [in] (bool) state - new state to be passed
    void SendInvertState(bool state);
    // This signal sends new active line mode state to the Camera class
    //
    // Parameters:
    // [in] (bool) state - new state to be passed
    void SendActiveState(bool state);
    // This signal sends new line selector value to the Camera class
    //
    // Parameters:
    // [in] (int32_t) value - new value to be set
    void SendLineSelectorValue(int32_t value);

private slots:
    // This slot function is called when invert checkBox is clicked
    void OnInvertClicked();
    // This slot function is called when active line mode checkBox is clicked
    void OnActiveClicked();
    // This slot function is called when index on the line selector list widget is changed
    void OnLineSelectorListItemChanged(const QString &currentText);

};

#endif // ACTIVEEXPOSUREWIDGET_H
