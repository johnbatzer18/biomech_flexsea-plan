#ifndef CHARTDATA_H
#define CHARTDATA_H

#include <QPoint>
#include <QList>
#include <QColor>
#include <dataprovider.h>
#include <quantdata.h>

#define INIT_PLOT_XMIN				0
#define INIT_PLOT_XMAX				200
#define INIT_PLOT_YMINgl			-10000
#define INIT_PLOT_YMAXgl			10000
#define PLOT_BUF_LENgl				1024
#define PLOT_SERIES					7
#define SERIES_PRIMARY				0
#define SERIES_PRIMARY_SUB1_L		1
#define SERIES_PRIMARY_SUB1_H		2
#define SERIES_PRIMARY_SUB2_L		3
#define SERIES_PRIMARY_SUB2_H		4
#define SERIES_SECONDARY			5
#define SERIES_TERTIARY				6

class ChartData
{
public:
	int xMin,xMax,xLen,yMin,yMax,yLen;
	int plotCount;
	int primarySub1,primarySub2;
	int graphXArray[PLOT_BUF_LENgl];
	int graphYArray[PLOT_BUF_LENgl][PLOT_SERIES];

	QList<DataProvider*> dataProviders;
	QList<QList<QuantData>> streams;
	void rebuildLegendColorList();
	QColor getCurrentDrawColor()const;
	void setCurrentDrawColorIndex(int index) { rowOfCurrentColor = index%numUniqueColors; }
	void moveToNextDrawColor() {
		rowOfCurrentColor++;
		rowOfCurrentColor %= numUniqueColors;
	}
	int getNumDrawColors() const { return numUniqueColors; }

	ChartData(void);

	//initialze chart data
	void initialize(void);

	//set ploting area
	void setAxis(int xMin=INIT_PLOT_XMIN, int xMax=INIT_PLOT_XMAX, int yMin=INIT_PLOT_YMINgl, int yMax=INIT_PLOT_YMAXgl);

	QString labelAt(int index) {
		if(index >= 0 && index < dataProviders.size())
			return dataProviders.at(index)->getLabel();
		return QString();
	}
	QColor legendColorAt(int index) {
		if(index >= 0 && index < legendColors.size())
			return legendColors.at(index);
		return QColor();
	}

	/*
	 * track Y Axis for cursor
	 * parameter positionRate (0~1) : pass position of Y Axis as 0~1 value
	 * parameter track : retrieve x value of cursor
	 * parameter track[] : retrieve y values for cursor
	*/
	void trackCursor(double positionRate,double &trackX,double &trackY, int streamIndex, int offset);

	//return index of closest plot to trackX value
	int getClosestPlotIndex(double trackX);

	//find closest index by using binary search
	int binarySearchClosestPlotIndex(int first,int last,double trackX);

	//convert plot data in order to draw on painter
	QPoint translate(int x,int y,int width,int height);

private:
	QList<uint8_t> uniqueColors;
	int numUniqueColors;
	int rowOfCurrentColor;
	QList<QColor> legendColors;
};

#endif // CHARTDATA_H
