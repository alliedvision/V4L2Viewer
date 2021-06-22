#include "ListIntEnumerationControl.h"

ListIntEnumerationControl::ListIntEnumerationControl(int32_t id, QList<int64_t> list, QString name, QWidget *parent):
    IControlEnumerationHolder(id, name, parent)
{
    m_ControlInfo.setText(QString(tr("%1 control is represented as list of integers. \n Values listed below are available for the camera")
                          .arg(name)));
    m_Layout.addWidget(&m_ControlInfo, 0, 0);
    m_Layout.addWidget(&m_ListWidget, 1, 0);
    m_MainWidget.setLayout(&m_Layout);

    for (QList<int64_t>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(QString::number(*it));
    }

    connect(&m_ListWidget, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnListItemChanged(const QString &)));
}

void ListIntEnumerationControl::UpdateValue(QList<int64_t> list)
{
    m_ListWidget.blockSignals(true);
    m_ListWidget.clear();
    for (QList<int64_t>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(QString::number(*it));
    }
    m_ListWidget.blockSignals(false);
}

void ListIntEnumerationControl::OnListItemChanged(const QString &currentText)
{
    int64_t value = static_cast<int64_t>(currentText.toLongLong());
    emit PassNewValue(m_id, value);
}
