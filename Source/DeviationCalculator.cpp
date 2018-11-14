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

				QByteArray compareFrame((char*)frame->GetBuffer(), frame->GetBufferlength());
				double deviation;

				// set deviation to error if the two frames cannot be compared
				if (m_referenceFrame->size() != compareFrame.size())
				{
					deviation = -1.0;
				}
				// compare frames bytewise
				else
				{
					unsigned int unequalBytes = 0;
					for (int i = 0; i < m_referenceFrame->size(); ++i)
					{
						if (m_referenceFrame->at(i) != compareFrame.at(i))
						{
							unequalBytes++;
						}
					}
					deviation = ((double)unequalBytes)/((double)m_referenceFrame->size());
				}
				 
				emit OnCalcDeviationReady_Signal(row, deviation, (it == --m_tableRowToFrame.end()));
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