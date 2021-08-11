#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

#include <QWidget>
#include <QEvent>
#include <QDialog>
#include "ui_CustomDialog.h"

class CustomDialog : public QDialog
{
    Q_OBJECT
public:
    static void Warning(QWidget *parent, QString title, QString text);
    static void Info(QWidget *parent, QString title, QString text);
    static void Error(QWidget *parent, QString title, QString text);

private slots:
    void OnOkButtonClicked();

private:
    explicit CustomDialog(QString title, QString text, int type, QWidget *parent);
    void closeEvent(QCloseEvent *event);

    QString m_Title;
    QString m_Text;

    enum{
        warning = 0,
        info = 1,
        error = 2
    };

    Ui::CustomDialog ui;
};

#endif // CUSTOMDIALOG_H
