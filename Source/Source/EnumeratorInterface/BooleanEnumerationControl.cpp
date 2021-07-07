/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BooleanEnumerationControl.cpp

  Description: This class inherits by IControlEnumerationHolder. It contains
               all necessary data for boolean controls which are enumerated.

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

#include "BooleanEnumerationControl.h"

BooleanEnumerationControl::BooleanEnumerationControl(int32_t id, bool value, QString name, QString unit, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo = QString(tr("%1 control accepts boolean values. \n Unit: %2")
                          .arg(name)
                          .arg(unit));
    m_CheckBox.setText("On/Off");
    m_CheckBox.setChecked(value);
    m_ControlEditWidget.m_pLayout->addWidget(&m_CheckBox);
    m_CurrentValue.setText(value ? "True" : "False");
    if (bIsReadOnly)
    {
        setEnabled(false);
        m_ControlInfo += tr("\n Control is READONLY");
    }
    else
    {
        setEnabled(true);
        connect(&m_CheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnStateChanged(int)));
    }
}

void BooleanEnumerationControl::UpdateValue(bool val)
{
    m_CheckBox.blockSignals(true);
    m_CheckBox.setChecked(val);
    m_CurrentValue.setText(val ? "True" : "False");
    m_CheckBox.blockSignals(false);
}

void BooleanEnumerationControl::OnStateChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        emit PassNewValue(m_id, false);
    }
    else
    {
        emit PassNewValue(m_id, true);
    }
}
