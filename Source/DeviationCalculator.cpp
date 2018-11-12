#include "DeviationCalculator.h"

DeviationCalculator::DeviationCalculator(QSharedPointer<QByteArray> referenceFrame, std::map<unsigned int, QSharedPointer<MyFrame> > tableRowToFrame) :
	m_referenceFrame(referenceFrame),
	m_tableRowToFrame(tableRowToFrame)
{

}

void DeviationCalculator::run()
{
	std::map<unsigned int, double> tableRowToDeviation;
	if(m_referenceFrame)
	{
		for(std::map<unsigned int, QSharedPointer<MyFrame> >::const_iterator it = m_tableRowToFrame.begin();
			it != m_tableRowToFrame.end();
			++it)
		{
			unsigned int row = it->first;
			QSharedPointer<MyFrame> frame = it->second;

			QByteArray compareFrame((char*)frame->GetBuffer(), frame->GetBufferlength());

			if (m_referenceFrame->size() != compareFrame.size())
			{
				continue;
			}

			unsigned int unequalBytes = 0;
			for (int i = 0; i < m_referenceFrame->size(); ++i)
			{
				if (m_referenceFrame->at(i) != compareFrame.at(i))
				{
					unequalBytes++;
				}
			}

			double deviation = ((double)unequalBytes)/((double)m_referenceFrame->size());
			tableRowToDeviation.insert(std::make_pair(row, deviation));
		}
	}

	emit OnCalcDeviationReady_Signal(tableRowToDeviation);
}