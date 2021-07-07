/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IntegerEnumerationControl.h

  Description: This class inherits by IControlEnumerationHolder. It contains
               all necessary data for int controls which are enumerated.

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

#ifndef INTEGERENUMERATIONCONTROL_H
#define INTEGERENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class IntegerEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    IntegerEnumerationControl(int32_t id, int32_t min, int32_t max, int32_t value, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    // This function updates value in current widget when it's changed somewhere else
    //
    // Parameters:
    // [in] (int32_t) value - new value for the widget
    void UpdateValue(int32_t value);

signals:
    // This signal passes new value from the line edit to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id value to be passed
    // [in] (int32_t) value - value to be passed
    void PassNewValue(int32_t id, int32_t value);
    // This signal passes new value from the slider to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id value to be passed
    // [in] (int32_t) value - value to be passed
    void PassSliderValue(int32_t id, int32_t value);

private slots:
    // This slot function is called after entering new value to the line edit
    void OnLineEditPressed();
    // This function is called after slider position has changed
    //
    // Parameters:
    // [in] (int) value - new value comes from the slider
    void OnSliderValueChanged(int value);
    // This function is called after slider position has changed, this function is only usable for the exposure,
    // cause it's value is calculated logarithmically
    //
    // Parameters:
    // [in] (int) value - new value comes from the slider
    void OnSliderLogValueChanged(int value);
    // This function is called after slider was released, then the all of the controls should be updated
    void OnSliderReleased();

private:
    // This function is used internally inside this class, it calculates log values and returns proper position for the slider
    //
    // Parameters:
    // [in] (int32_t) value - value to be calculated
    //
    // Returns:
    // (int32_t) - calculated value which is position of the slider
    int32_t GetSliderLogValue(int32_t value);

    // One of the main elements of this widget, it allows to change value
    QSlider m_Slider;
    // One of the main elements of this widget, it allows to change value
    QLineEdit m_LineEdit;
    int32_t m_Min;
    int32_t m_Max;
    int32_t m_Value;
    int32_t m_SliderValue;
};


#endif // INTEGERENUMERATIONCONTROL_H
