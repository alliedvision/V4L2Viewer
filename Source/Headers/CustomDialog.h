/* Allied Vision V4L2Viewer - Graphical Video4Linux Viewer Example
   Copyright (C) 2021 Allied Vision Technologies GmbH

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  */

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
