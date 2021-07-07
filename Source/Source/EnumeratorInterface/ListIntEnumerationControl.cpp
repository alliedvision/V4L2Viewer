/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ListIntEnumerationControl.cpp

  Description: This class inherits by IControlEnumerationHolder. It contains
               all necessary data for int list controls which are enumerated.

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

#include "ListIntEnumerationControl.h"

ListIntEnumerationControl::ListIntEnumerationControl(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ListWidget.setMinimumHeight(150);
    m_ControlInfo = QString(tr("%1 control is represented as list of integers. \n Unit: %2")
                          .arg(name)
                          .arg(unit));
    m_ControlEditWidget.m_pLayout->addWidget(&m_ListWidget);

    for (QList<int64_t>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(QString::number(*it));
    }

    if (value >= list.count())
    {
        m_CurrentValue.setText("Unknown Current Value");
    }
    else
    {
        m_CurrentValue.setText(QString::number(list.at(value)));
        m_ListWidget.setCurrentRow(value);
    }

    if (bIsReadOnly)
    {
        setEnabled(false);
        m_ControlInfo += tr("\n Control is READONLY");
    }
    else
    {
        setEnabled(true);
        connect(&m_ListWidget, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnListItemChanged(const QString &)));
    }
}

void ListIntEnumerationControl::UpdateValue(QList<int64_t> list, int32_t value)
{
    m_ListWidget.blockSignals(true);
    m_ListWidget.clear();
    for (QList<int64_t>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(QString::number(*it));
    }

    if (value >= list.count())
    {
        m_CurrentValue.setText("Unknown Current Value");
    }
    else
    {
        m_CurrentValue.setText(QString::number(list.at(value)));
        m_ListWidget.setCurrentRow(value);
    }

    m_ListWidget.blockSignals(false);
}

void ListIntEnumerationControl::OnListItemChanged(const QString &currentText)
{
    int64_t value = static_cast<int64_t>(currentText.toLongLong());
    emit PassNewValue(m_id, value);
}
