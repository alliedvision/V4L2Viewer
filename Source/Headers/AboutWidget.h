#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QDialog>
#include <QLabel>

class AboutWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AboutWidget(QWidget *parent = nullptr);
    void UpdateStrings();

private:
    QLabel *m_TextLabel;
};

#endif // ABOUTWIDGET_H
