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
