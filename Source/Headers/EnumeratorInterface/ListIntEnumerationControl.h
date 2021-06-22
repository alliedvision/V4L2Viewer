#ifndef LISTINTENUMERATIONCONTROL_H
#define LISTINTENUMERATIONCONTROL_H

#include "IControlEnumerationHolder.h"

class ListIntEnumerationControl : public IControlEnumerationHolder
{
    Q_OBJECT
public:
    ListIntEnumerationControl(int32_t id, QList<int64_t> list, QString name, bool bIsReadOnly, QWidget *parent);
    void UpdateValue(QList<int64_t> list);

private slots:
    void OnListItemChanged(const QString &currentText);

signals:
    void PassNewValue(int32_t id, int64_t value);

private:
    QListWidget m_ListWidget;

};

#endif // LISTINTENUMERATIONCONTROL_H
