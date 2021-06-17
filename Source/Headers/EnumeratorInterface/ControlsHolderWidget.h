#ifndef CONTROLSHOLDERWIDGET_H
#define CONTROLSHOLDERWIDGET_H

#include <QDialog>
#include <QScrollArea>
#include <QGridLayout>
#include <EnumeratorInterface/IControlEnumerationHolder.h>

class ControlsHolderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlsHolderWidget(QWidget *parent = nullptr);
    void AddElement(IControlEnumerationHolder *controlWidget);

private:
    QWidget     *m_pScrollAreaWidget;
    QScrollArea *m_pScrollArea;

    QGridLayout *m_pScrollAreaGrid;
    QGridLayout *m_pMainLayout;
};

#endif // CONTROLSHOLDERWIDGET_H
