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


#ifndef DEVIATIONCALCULATOR_H
#define DEVIATIONCALCULATOR_H

#include "MyFrame.h"

#include <QObject>
#include <QSharedPointer>
#include <QThread>

#include <map>

class DeviationCalculator : public QThread
{
    Q_OBJECT

private:
    QSharedPointer<QByteArray> m_pReferenceFrame;
    std::map<unsigned int, QSharedPointer<MyFrame> > m_RowToFrameTable;

public:
    DeviationCalculator(QSharedPointer<QByteArray> pReferenceFrame, std::map<unsigned int, QSharedPointer<MyFrame> > rowToFrameTable);

    // return -1 if bytearrays have different size, else return number of unequal bytes
    static int CountUnequalBytes(QSharedPointer<QByteArray> pReferenceFrame, QSharedPointer<QByteArray> pCompareFrames);

protected:
    void run();

signals:
    void OnCalcDeviationReady_Signal(unsigned int tableRow, int unequalBytes, bool done);
};

#endif // DEVIATIONCALCULATOR_H

