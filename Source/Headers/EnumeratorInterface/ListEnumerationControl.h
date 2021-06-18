#ifndef LISTENUMERATIONCONTROL_H
#define LISTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListEnumerationControl(QList<QString> list, QString name, QWidget *parent);

private slots:
    void OnListItemChanged(const QString &currentText);

signals:
    void PassNewValue(QString value);

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QListWidget m_ListWidget;

};

#endif // LISTENUMERATIONCONTROL_H
