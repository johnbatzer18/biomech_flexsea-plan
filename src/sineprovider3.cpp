#include "sineprovider3.h"
#include <cmath>
#include <QTime>
#include <random>

SineProvider3::SineProvider3() : distribution(10.0, 3.0)
{
	angle = 0;
	numFields = SINE_PROVIDER3_NUM_FIELDS;
	internalTimer.setSingleShot(false);
	internalTimer.setInterval(100);
	connect(&internalTimer, &QTimer::timeout, this, &SineProvider3::incrementAngle);
	internalTimer.start();
}

SineProvider3::SineProvider3(int initialAngle, int offset) : distribution(10.0, 3.0)
{
	angle = initialAngle;
	(void) offset;
	numFields = SINE_PROVIDER3_NUM_FIELDS;
	internalTimer.setSingleShot(false);
	internalTimer.setInterval(100);
	connect(&internalTimer, &QTimer::timeout, this, &SineProvider3::incrementAngle);
	internalTimer.start();
}

QuantData SineProvider3::getNewestData()
{
	QuantData result(SINE_PROVIDER3_NUM_FIELDS);

	for(int i = 0; i < SINE_PROVIDER3_NUM_FIELDS; i++)
	{
		float angleInRadians = (float)(angle) * M_PI / 180;
		result[i] = 100 * sin(angleInRadians);
	}

	return result;
}

QList<QuantData> SineProvider3::getXMostRecent(int x)
{
	while(pastValues.size() < x)
	{
		float angleInRadians = (float)(angle) * M_PI / 180;
		QuantData pieceOfData(SINE_PROVIDER3_NUM_FIELDS);
		int sinVal = 100*sin(angleInRadians);

		pieceOfData[0] = sinVal;
		pieceOfData[1] = sinVal+(distribution(generator));
		pieceOfData[2] = sinVal-(distribution(generator));

		pastValues.enqueue(pieceOfData);
	}

	return pastValues;
}

QList<QuantData> SineProvider3::getDataRange(int rangeStart, int rangeEnd)
{
	(void) rangeStart;
	(void) rangeEnd;

	QList<QuantData> result;
	return result;
}

QString SineProvider3::getLabel()
{
	return QStringLiteral("Sine Function 3 vals");
}

void SineProvider3::incrementAngle() {
	angle+=10;

	if(pastValues.size() > 49)
	{
		pastValues.dequeue();
	}

	float angleInRadians = (float)(angle) * M_PI / 180;
	QuantData pieceOfData(SINE_PROVIDER3_NUM_FIELDS);
	int sinVal = 100*sin(angleInRadians);

	pieceOfData[0] = sinVal;
	pieceOfData[1] = sinVal+(distribution(generator)) + 10;
	pieceOfData[2] = sinVal-(distribution(generator)) - 10;

	pastValues.enqueue(pieceOfData);
}
