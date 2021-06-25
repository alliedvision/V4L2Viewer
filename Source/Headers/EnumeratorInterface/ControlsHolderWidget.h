#ifndef CONTROLSHOLDERWIDGET_H
#define CONTROLSHOLDERWIDGET_H

#include <QDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <EnumeratorInterface/IControlEnumerationHolder.h>
#include "ui_ControlsHolderWidget.h"

class ControlsHolderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlsHolderWidget(QWidget *parent = nullptr);
    void AddElement(IControlEnumerationHolder *controlWidget);
    void RemoveElements();
    bool IsControlAlreadySet(int32_t id);
    IControlEnumerationHolder* GetControlWidget(int32_t id, bool &bIsSuccess);

    Ui::ControlsHolderWidget ui;

private:
    QGridLayout *m_pWidgetLayout;
    QVector<IControlEnumerationHolder*> itemVector;
};


#endif // CONTROLSHOLDERWIDGET_H
