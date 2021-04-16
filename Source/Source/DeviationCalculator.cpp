/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        DeviationCalculator.cpp

  Description:

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

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
