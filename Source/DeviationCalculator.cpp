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
