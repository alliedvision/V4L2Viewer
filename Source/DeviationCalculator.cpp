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

DeviationCalculator::DeviationCalculator(QSharedPointer<QByteArray> referenceFrame, std::map<unsigned int, QSharedPointer<MyFrame> > tableRowToFrame) :
    m_referenceFrame(referenceFrame),
    m_tableRowToFrame(tableRowToFrame)
{

}

void DeviationCalculator::run()
{
    if(m_referenceFrame)
    {
        if(m_tableRowToFrame.size() > 0)
        {
            for(std::map<unsigned int, QSharedPointer<MyFrame> >::const_iterator it = m_tableRowToFrame.begin();
                it != m_tableRowToFrame.end();
                ++it)
            {
                unsigned int row = it->first;
                QSharedPointer<MyFrame> frame = it->second;

                QSharedPointer<QByteArray> compareFrame( new QByteArray((char*)frame->GetBuffer(), frame->GetBufferlength()));
                double deviation;

                emit OnCalcDeviationReady_Signal(row, CountUnequalBytes(m_referenceFrame, compareFrame), (it == --m_tableRowToFrame.end()));
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

int DeviationCalculator::CountUnequalBytes(QSharedPointer<QByteArray> reference, QSharedPointer<QByteArray> compareFrame)
{
    int unequalBytes = 0;

    // set deviation to error if the two frames cannot be compared
    if (reference->size() != compareFrame->size())
    {
        unequalBytes = -1;
    }
    // compare frames bytewise
    else
    {
        for (int i = 0; i < reference->size(); ++i)
        {
            if (reference->at(i) != compareFrame->at(i))
            {
                unequalBytes++;
            }
        }
    }

    return unequalBytes;
}
