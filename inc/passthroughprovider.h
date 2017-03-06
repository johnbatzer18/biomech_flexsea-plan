#ifndef PASSTHROUGHPROVIDER_H
#define PASSTHROUGHPROVIDER_H

#include <dataprovider.h>
#include <QQueue>
#include <QTimer>

class PassThroughProvider : public DataProvider
{
public:
	enum DATA_SIGNSIZE { U_8, S_8, U_16, S_16, U_32, S_32};

	explicit PassThroughProvider(unsigned int sampleFrequency = 33);
	virtual ~PassThroughProvider() {
		data = nullptr;
		if(sampleTimer) delete sampleTimer;
		sampleTimer = nullptr;
	}

	void setDataPointer(void* d) { data = d; }
	void setLabel(QString s) { label = s; }

	virtual QuantData getNewestData(){}
	virtual QList<QuantData> getXMostRecent(int x);
	virtual QList<QuantData> getDataRange(int rangeStart, int rangeEnd){}
	virtual QString getLabel() { return label; }

	DATA_SIGNSIZE dataSignSize;
	int sampleFrequency;

private:
	void* data;

	QQueue<TimeStampedData> dataBuffer;
	QString label;
	QTimer* sampleTimer;

private slots:
	void handleNewDataRecieved();
};

#endif // PASSTHROUGHPROVIDER_H
