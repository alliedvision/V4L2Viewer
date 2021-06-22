#ifndef LISTENUMERATIONCONTROL_H
#define LISTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListEnumerationControl(int32_t id, QList<QString> list, QString name, QWidget *parent);
    void UpdateValue(QList<QString> list);

private slots:
    void OnListItemChanged(const QString &currentText);

signals:
    void PassNewValue(int32_t id, const char *value);

private:
    QGridLayout m_Layout;
    QLabel m_ControlInfo;
    QListWidget m_ListWidget;
};

#endif // LISTENUMERATIONCONTROL_H
