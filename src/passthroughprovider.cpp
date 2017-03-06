#include "passthroughprovider.h"
#include <QTime>

PassThroughProvider::PassThroughProvider(unsigned int sampleFrequency) {
	numFields = 1;
	data = nullptr;
	dataSignSize = S_16;
	dataBuffer.reserve(1000);
	this->sampleFrequency = sampleFrequency;
	sampleTimer = new QTimer();
	sampleTimer->setInterval(1000 / sampleFrequency);
	sampleTimer->setSingleShot(true);
	connect(sampleTimer, &QTimer::timeout, this, &PassThroughProvider::handleNewDataRecieved);
	sampleTimer->start();
}

QList<QuantData> PassThroughProvider::getXMostRecent(int x)
{
	QList<QuantData> result;
	result.reserve(x);
	int i = 0;
	while(i + dataBuffer.size() < x)
	{
		QuantData nullData(numFields);
		result.append(nullData);
		i++;
	}

	if(dataBuffer.size() < x)
	{
		for(i = 0; i < dataBuffer.size(); i++)
		{
			QuantData actualData(numFields);
			actualData.at(0) = dataBuffer.at(i).value;
			result.append(actualData);
		}
	}
	else
	{
		for(i = dataBuffer.size() - x; i < dataBuffer.size(); i++)
		{
			QuantData actualData(numFields);
			actualData.at(0) = dataBuffer.at(i).value;
			result.append(actualData);
		}
	}

	return result;
}

void PassThroughProvider::handleNewDataRecieved()
{
	sampleTimer->setInterval(1000 / this->sampleFrequency);
	sampleTimer->start();
	if(data)
	{
		while(dataBuffer.size() > 999)
		{
			dataBuffer.dequeue();
		}
		TimeStampedData d;

		// Surely there is a better way to do this, but I don't know what it is
		switch (dataSignSize)
		{
		case U_8:
			d.value = (*((uint8_t*)data));
			break;
		case U_16:
			d.value = (*((uint16_t*)data));
			break;
		case U_32:
			d.value = (*((uint32_t*)data));
			break;
		case S_8:
			d.value = (*((int8_t*)data));
			break;
		case S_16:
			d.value = (*((int16_t*)data));
			break;
		case S_32:
			d.value = (*((int32_t*)data));
			break;
		}

		d.timestamp = QTime::currentTime().msec();
		dataBuffer.enqueue(d);
	}
}
