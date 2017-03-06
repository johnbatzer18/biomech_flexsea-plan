#ifndef SINEPROVIDER3_H
#define SINEPROVIDER3_H

#include <dataprovider.h>
#include <QTimer>
#include <QQueue>

#define SINE_PROVIDER3_NUM_FIELDS 3

class SineProvider3 : public DataProvider
{

public:
	explicit SineProvider3();
	SineProvider3(int initialAngle, int offset = 0);

	virtual ~SineProvider3(){}

	virtual QuantData getNewestData();
	virtual QList<QuantData> getXMostRecent(int x);
	virtual QList<QuantData> getDataRange(int rangeStart, int rangeEnd);
	virtual QString getLabel();

public slots:
	void incrementAngle();

private:
	int angle;
	QTimer internalTimer;
	QQueue<QuantData> pastValues;

	std::default_random_engine generator;
	std::normal_distribution<double> distribution;

};

#endif // SINEPROVIDER3_H
