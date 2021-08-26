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


#include "DeviationCalculator.h"

DeviationCalculator::DeviationCalculator(QSharedPointer<QByteArray> pReferenceFrame, std::map<unsigned int, QSharedPointer<MyFrame> > rowToFrameTable)
    : m_pReferenceFrame(pReferenceFrame)
    , m_RowToFrameTable(rowToFrameTable)
{

}

void DeviationCalculator::run()
{
    if(m_pReferenceFrame)
    {
        if(m_RowToFrameTable.size() > 0)
        {
            for(std::map<unsigned int, QSharedPointer<MyFrame> >::const_iterator it = m_RowToFrameTable.begin();
                it != m_RowToFrameTable.end();
                ++it)
            {
                unsigned int row = it->first;
                QSharedPointer<MyFrame> pFrame = it->second;

                QSharedPointer<QByteArray> pCompareFrame( new QByteArray((char*)pFrame->GetBuffer(), pFrame->GetBufferlength()));

                emit OnCalcDeviationReady_Signal(row, CountUnequalBytes(m_pReferenceFrame, pCompareFrame), (it == --m_RowToFrameTable.end()));
            }
        }
        // return error if map is empty
        else
        {
            emit OnCalcDeviationReady_Signal(0, -1.0, true);
        }
    }
    // return error if reference image is null
    else
    {
        emit OnCalcDeviationReady_Signal(0, -1.0, true);
    }
}

int DeviationCalculator::CountUnequalBytes(QSharedPointer<QByteArray> pReferenceFrame, QSharedPointer<QByteArray> pCompareFrame)
{
    int unequalBytes = 0;

    // set deviation to error if the two frames cannot be compared
    if (pReferenceFrame->size() != pCompareFrame->size())
    {
        unequalBytes = -1;
    }
    // compare frames bytewise
    else
    {
        for (int i = 0; i < pReferenceFrame->size(); ++i)
        {
            if (pReferenceFrame->at(i) != pCompareFrame->at(i))
            {
                unequalBytes++;
            }
        }
    }

    return unequalBytes;
}
