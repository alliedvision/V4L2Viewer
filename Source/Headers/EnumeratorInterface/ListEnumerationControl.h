#ifndef LISTENUMERATIONCONTROL_H
#define LISTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListEnumerationControl(int32_t id, int32_t value, QList<QString> list, QString name, bool bIsReadOnly, QWidget *parent);
    void UpdateValue(QList<QString> list, int32_t value);

private slots:
    void OnListItemChanged(const QString &currentText);

signals:
    void PassNewValue(int32_t id, const char *value);

private:
    QListWidget m_ListWidget;
};

#endif // LISTENUMERATIONCONTROL_H
