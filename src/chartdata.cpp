#include "chartdata.h"


ChartData::ChartData(void)
{
	initialize();
}

void ChartData::initialize(void)
{
	setAxis(0,1000,-500,500);
	plotCount=50;
	primarySub1=yLen/30;
	primarySub2=primarySub1*2;
	for(int i=0;i<PLOT_BUF_LENgl;i++)
	{
		graphXArray[i]=xMin+(i*xLen)/(plotCount-1);
	}

	uniqueColors = QList<uint8_t> ({ 0xFF, 0xB3, 0x00,
	0x80, 0x3E, 0x75,
	0xFF, 0x68, 0x00,
	0xA6, 0xBD, 0xD7,
	0xC1, 0x00, 0x20,
	0xCE, 0xA2, 0x62,
	0x81, 0x70, 0x66,
	0x00, 0x7D, 0x34,
	0xF6, 0x76, 0x8E,
	0x00, 0x53, 0x8A,
	0xFF, 0x7A, 0x5C,
	0x53, 0x37, 0x7A,
	0xFF, 0x8E, 0x00,
	0xB3, 0x28, 0x51,
	0xF4, 0xC8, 0x00,
	0x7F, 0x18, 0x0D,
	0x93, 0xAA, 0x00,
	0x59, 0x33, 0x15,
	0xF1, 0x3A, 0x13,
	0x23, 0x2C, 0x16 });

	numUniqueColors = uniqueColors.size() / 3;
	if(numUniqueColors * 3 != uniqueColors.size()) qDebug("CustomChartView/ChartData color list 'uniqueColors' is malformed.");
	rowOfCurrentColor = 0;
}

QColor ChartData::getCurrentDrawColor()const
{
	int rIndex = rowOfCurrentColor*3;
	return QColor(uniqueColors.at(rIndex), uniqueColors.at(rIndex+1), uniqueColors.at(rIndex+2));
}

void ChartData::setAxis(int xMin, int xMax, int yMin, int yMax)
{
	this->xMin=xMin;
	this->xMax=xMax;
	xLen=xMax-xMin;
	this->yMin=yMin;
	this->yMax=yMax;
	yLen=yMax-yMin;
}

void ChartData::trackCursor(double positionRate,double &trackX,double &trackY, int streamIndex, int offset)
{
	trackX=positionRate*xLen+xMin;
	int leftIndex=getClosestPlotIndex(trackX);
	int rightIndex=leftIndex+1;
	for(int i=0;i<PLOT_SERIES;i++)
	{
		double k=(trackX-graphXArray[leftIndex])/(graphXArray[rightIndex]-graphXArray[leftIndex]);

		int yLeft = streams.at(streamIndex).at(leftIndex).at(offset);
		int yRight = streams.at(streamIndex).at(rightIndex).at(offset);

		trackY = k * (yRight - yLeft) + yLeft;
		//trackY[i]=k*(graphYArray[rightIndex][i]-graphYArray[leftIndex][i])+graphYArray[leftIndex][i];
	}
}

int ChartData::getClosestPlotIndex(double trackX)
{
	return binarySearchClosestPlotIndex(0,plotCount-1,trackX);
}

int ChartData::binarySearchClosestPlotIndex(int first,int last,double trackX)
{
	if(first==last)return first-1;
	int middle=(first+last)/2;
	double middlePosition=(double)(graphXArray[middle]-xMin);
	if(trackX<=middlePosition)return binarySearchClosestPlotIndex(first,middle,trackX);
	return binarySearchClosestPlotIndex(middle+1,last,trackX);
}

QPoint ChartData::translate(int x,int y,int width,int height)
{
	int translatedX=x*width/xLen;
	int translatedY=(yMax-y)*height/yLen;
	return QPoint(translatedX,translatedY);
}

void ChartData::rebuildLegendColorList()
{
	legendColors.clear();
	//These two lists should really be cached
	//need a list of labels
	//need a parallel list of their colors
	int dataProviderIndex = 0, colorIndex = 0, rIndex;
	int numProviders = dataProviders.size();

	while(dataProviderIndex < numProviders)
	{
		int numFields, offsets;
		numFields = dataProviders.at(dataProviderIndex)->getNumFields();
		offsets = (numFields % 2) + (numFields / 2);
		colorIndex += offsets - 1;
		rIndex = 3*colorIndex;

		legendColors.append(QColor(uniqueColors.at(rIndex), uniqueColors.at(rIndex+1), uniqueColors.at(rIndex+2)));

		colorIndex++;
		dataProviderIndex++;
	}
}
