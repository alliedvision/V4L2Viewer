#ifndef LISTINTENUMERATIONCONTROL_H
#define LISTINTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListIntEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListIntEnumerationControl(QList<int64_t> list, QString name, QWidget *parent);

private slots:
    void OnListItemChanged(const QString &currentText);

signals:
    void PassNewValue(int64_t value);

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QListWidget m_ListWidget;

};

#endif // LISTINTENUMERATIONCONTROL_H
