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


#ifndef INTEGER64ENUMERATIONCONTROL_H
#define INTEGER64ENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class Integer64EnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    Integer64EnumerationControl(int32_t id, int64_t min, int64_t max, int64_t value, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    // This function updates value in current widget when it's changed somewhere else
    //
    // Parameters:
    // [in] (int64_t) value - new value for the widget
    void UpdateValue(int64_t value);

signals:
    // This signal passes new value from the line edit to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id value to be passed
    // [in] (int64_t) value - value to be passed
    void PassNewValue(int32_t id, int64_t value);
    // This signal passes new value from the slider to the Camera class
    //
    // Parameters:
    // [in] (int32_t) id - id value to be passed
    // [in] (int64_t) value - value to be passed
    void PassSliderValue(int32_t id, int64_t value);

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
    // [in] (int64_t) value - value to be calculated
    //
    // Returns:
    // (int32_t) - calculated value which is position of the slider
    int32_t GetSliderLogValue(int64_t value);

    // This function is used internally inside this class, it converts a 64bit control value to a 32bit value for the slider.
    //
    // Parameters:
    // [in] (int64_t) value - value to be calculated
    //
    // Returns:
    // (int32_t) - calculated value which is position of the slider
    int32_t ConvertToSliderValue(int64_t value) const;

    // This function is used internally inside this class, it converts a 32bit slider value to 64bit control value.
    //
    // Parameters:
    // [in] (int32_t) value - value of the slider
    //
    // Returns:
    // (int64_t) - value for the control
    int64_t ConvertFromSliderValue(int32_t value) const;

    // This function is used internally inside this class, it returns the conversion factor that is used to
    // convert a 64bit control value to a 32bit slider value and back
    //
    // Returns:
    // (int32_t) - conversion factor between control and slider value
    int64_t GetConversionFactor() const;

    // One of the main elements of this widget, it allows to change value
    QSlider m_Slider;
    // One of the main elements of this widget, it allows to change value
    QLineEdit m_LineEdit;

    int64_t m_Min;
    int64_t m_Max;
    int64_t m_Value;
    int64_t m_SliderValue;
};

#endif // INTEGER64ENUMERATIONCONTROL_H
