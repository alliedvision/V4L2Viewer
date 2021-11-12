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


#include "V4L2Viewer.h"
#include <QDebug>
#include <iostream>
#include "Version.h"
#include "GitRevision.h"

int main( int argc, char *argv[] )
{
    QApplication a( argc, argv );
    a.setApplicationVersion(QString("%1.%2.%3 (%4)")
        .arg(APP_VERSION_MAJOR)
        .arg(APP_VERSION_MINOR)
        .arg(APP_VERSION_PATCH)
        .arg(GIT_VERSION));

    QCommandLineParser parser;
    QCommandLineOption startOption(QStringList() << "s" << "stream",
            QCoreApplication::translate("main", "Automatically start stream."));
    parser.addOption(startOption);
    QCommandLineOption deviceOption(QStringList() << "d" << "device",
    QCoreApplication::translate("main", "Immediately open camera."),
    QCoreApplication::translate("main", "camera"));
    parser.addOption(deviceOption);
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(a);

    Q_INIT_RESOURCE(V4L2Viewer);
    V4L2Viewer w;
    w.show();
    if(parser.isSet(deviceOption)) {
        QString deviceName = parser.value(deviceOption);
        if(!w.OpenCloseCamera(deviceName)) {
            std::cerr << "Failed to open camera " << deviceName.toStdString() << std::endl;
            return -1;
        }
        if(parser.isSet(startOption)) {
            w.StartStream();
        }
    } else if(parser.isSet(startOption)) {
        std::cerr << "Cannot start streaming without a device" << std::endl;
        return -1;
    }
    return a.exec();
}
