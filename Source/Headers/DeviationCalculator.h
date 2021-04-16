/*=============================================================================
  Copyright (C) 2021 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        DeviationCalculator.h

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

