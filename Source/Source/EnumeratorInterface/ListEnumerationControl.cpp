#include "ListEnumerationControl.h"

ListEnumerationControl::ListEnumerationControl(QList<QString> list, QString name, QWidget *parent):
    IControlEnumerationHolder(name, parent)
{
    m_ControlInfo.setText("Control info here");
    m_Layout.addWidget(&m_ControlInfo, 0, 0);
    m_Layout.addWidget(&m_ListWidget, 1, 0);
    m_MainWidget.setLayout(&m_Layout);

    for (QList<QString>::iterator it = list.begin(); it<list.end(); ++it)
    {
        m_ListWidget.addItem(*it);
    }

    connect(&m_ListWidget, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnListItemChanged(const QString &)));
}

void ListEnumerationControl::OnListItemChanged(const QString &currentText)
{
    emit PassNewValue(QString(currentText));
}
