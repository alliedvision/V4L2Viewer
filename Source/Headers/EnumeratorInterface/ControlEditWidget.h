#ifndef CONTROLEDITWIDGET_H
#define CONTROLEDITWIDGET_H

#include <QWidget>
#include <QLayout>

class ControlEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ControlEditWidget(QWidget *parent = nullptr);
    QGridLayout *m_pLayout;
};

#endif // CONTROLEDITWIDGET_H
