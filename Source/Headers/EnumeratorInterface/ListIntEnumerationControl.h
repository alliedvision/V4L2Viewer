#ifndef LISTINTENUMERATIONCONTROL_H
#define LISTINTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListIntEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListIntEnumerationControl(int32_t id, int32_t value, QList<int64_t> list, QString name, QString unit, bool bIsReadOnly, QWidget *parent);
    void UpdateValue(QList<int64_t> list, int32_t value);

private slots:
    void OnListItemChanged(const QString &currentText);

signals:
    void PassNewValue(int32_t id, int64_t value);

private:
    QListWidget m_ListWidget;

};

#endif // LISTINTENUMERATIONCONTROL_H
