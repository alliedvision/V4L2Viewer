#ifndef CUSTOMOPTIONWIDGET_H
#define CUSTOMOPTIONWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>

class CustomOptionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomOptionWidget(QString text, QWidget *parent = nullptr);
    QLineEdit *GetLineEditWidget();

private:
    QLabel *m_pTextLabel;
    QLineEdit *m_pLineEdit;
    QGridLayout *m_pGridLayout;
};

#endif // CUSTOMOPTIONWIDGET_H
