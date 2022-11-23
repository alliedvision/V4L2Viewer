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


#include "IntegerEnumerationControl.h"
#include "linux/v4l2-controls.h"
#include <math.h>

IntegerEnumerationControl::IntegerEnumerationControl(int32_t id, int32_t min, int32_t max, int32_t value, QString name, QString unit, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent),
    m_Min(min),
    m_Max(max),
    m_Value(value),
    m_SliderValue(0)
{
    m_ControlInfo = QString(tr("%1 control accepts 32-bit integers. \n Minimum: %2 \n Maximum: %3 \n Unit: %4")
                          .arg(name)
                          .arg(m_Min)
                          .arg(m_Max)
                          .arg(unit));

    m_LineEdit.setText(QString::number(m_Value));
    m_CurrentValue.setText(QString::number(m_Value));
    m_Slider.setOrientation(Qt::Horizontal);

    m_MainLayout.addWidget(&m_NameOfControl, 0, 0);
    m_MainLayout.addWidget(&m_Slider, 0, 1);
    m_MainLayout.addWidget(&m_LineEdit, 0, 2);
    m_LineEdit.setAlignment(Qt::AlignRight);

    auto const len = int(std::ceil(std::log10(max+1)));
    m_LineEdit.setMaxLength(len);

    m_MainLayout.setColumnStretch(0, 2);
    m_MainLayout.setColumnStretch(1, 2);
    m_MainLayout.setColumnStretch(2, 1);

    if (bIsReadOnly)
    {
        setEnabled(false);
        m_ControlInfo += tr("\n Control is READONLY");
    }
    else
    {
        setEnabled(true);
        connect(&m_LineEdit, SIGNAL(editingFinished()), this, SLOT(OnLineEditPressed()));
        connect(&m_Slider, SIGNAL(sliderReleased()), this, SLOT(OnSliderReleased()));
        if (id == V4L2_CID_EXPOSURE)
        {
            connect(&m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderLogValueChanged(int)));
            m_Slider.setMaximum(10000);
            m_Slider.setMinimum(0);
            m_Slider.setValue(GetSliderLogValue(value));
        }
        else
        {
            connect(&m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
            m_Slider.setMaximum(max);
            m_Slider.setMinimum(min);
            m_Slider.setValue(value);
        }
        m_Slider.setValue(value);
    }
}

void IntegerEnumerationControl::UpdateValue(int32_t value)
{
    m_LineEdit.blockSignals(true);
    m_LineEdit.setText(QString::number(value));
    m_CurrentValue.setText(QString::number(value));
    m_LineEdit.blockSignals(false);

    m_Slider.blockSignals(true);
    if (m_id != V4L2_CID_EXPOSURE)
    {
        m_Slider.setValue(value);
    }
    else
    {
        m_Slider.setValue(GetSliderLogValue(value));
    }
    m_Slider.blockSignals(false);
}

void IntegerEnumerationControl::OnLineEditPressed()
{
    int32_t value = m_LineEdit.text().toInt();
    emit PassNewValue(m_id, value);
}

void IntegerEnumerationControl::OnSliderValueChanged(int value)
{
    m_LineEdit.setText(QString::number(value));
    m_SliderValue = static_cast<int32_t>(value);

    emit PassSliderValue(m_id, m_SliderValue);
}

void IntegerEnumerationControl::OnSliderLogValueChanged(int value)
{
    int minSliderValue = m_Slider.minimum();
    int maxSliderValue = m_Slider.maximum();

    double minExposure = log(m_Min);
    double maxExposure = log(m_Max);

    double scale = (maxExposure - minExposure) / (maxSliderValue - minSliderValue);

    double outValue = exp(minExposure + scale*(value - minSliderValue));
    m_SliderValue = static_cast<int32_t>(outValue);
    m_LineEdit.setText(QString::number(m_SliderValue));

    emit PassSliderValue(m_id, m_SliderValue);
}

void IntegerEnumerationControl::OnSliderReleased()
{
    emit PassNewValue(m_id, m_SliderValue);
}

int32_t IntegerEnumerationControl::GetSliderLogValue(int32_t value)
{
    double logExposureMin = log(m_Min);
    double logExposureMax = log(m_Max);
    int32_t minSliderExp = m_Slider.minimum();
    int32_t maxSliderExp = m_Slider.maximum();
    double scale = (logExposureMax - logExposureMin) / (maxSliderExp - minSliderExp);
    double result = minSliderExp + ( log(value) - logExposureMin ) / scale;
    return static_cast<int32_t>(result);
}


















