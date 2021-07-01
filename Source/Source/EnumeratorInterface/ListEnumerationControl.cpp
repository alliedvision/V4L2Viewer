#include "ListEnumerationControl.h"

ListEnumerationControl::ListEnumerationControl(int32_t id, int32_t value, QList<QString> list, QString name, bool bIsReadOnly, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ListWidget.setMinimumHeight(150);
    m_ControlInfo = QString(tr("%1 control is represented as list of strings. \n Values listed below are available for the camera")
                          .arg(name));

    m_ControlEditWidget.m_pLayout->addWidget(&m_ListWidget);

    for (QList<QString>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(*it);
    }

    if (value >= list.count())
    {
        m_CurrentValue.setText("Unknown Current Value");
    }
    else
    {
        m_CurrentValue.setText(list.at(value));
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

void ListEnumerationControl::UpdateValue(QList<QString> list, int32_t value)
{
    m_ListWidget.blockSignals(true);
    m_ListWidget.clear();
    for (QList<QString>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(*it);
    }

    if (value >= list.count())
    {
        m_CurrentValue.setText("Unknown Current Value");
    }
    else
    {
        m_CurrentValue.setText(list.at(value));
        m_ListWidget.setCurrentRow(value);
    }

    m_ListWidget.blockSignals(false);
}

void ListEnumerationControl::OnListItemChanged(const QString &currentText)
{
   emit PassNewValue(m_id, currentText.toStdString().c_str());
}
