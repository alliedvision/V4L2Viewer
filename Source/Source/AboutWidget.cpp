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


#include "AboutWidget.h"
#include "GitRevision.h"
#include <QGridLayout>
#include <QPixmap>
#include <QScrollArea>

AboutWidget::AboutWidget(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);
    ui.setupUi(this);
}

void AboutWidget::UpdateStrings()
{
    QString text = ui.m_versionLabel->text();
    ui.m_versionLabel->setText(text);
}

void AboutWidget::SetVersion(QString version)
{
    ui.m_versionLabel->setText(tr("<br>Version: %1 <br>").arg(version) +
                               tr("Git commit: %1 <br>").arg(GIT_VERSION));
}


