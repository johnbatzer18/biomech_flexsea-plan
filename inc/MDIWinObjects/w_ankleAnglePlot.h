/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'plan-gui' Graphical User Interface
	Copyright (C) 2016 Dephy, Inc. <http://dephy.com/>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] w_2dplot.h: 2D Plot window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-09-12 | jfduval | Added Freeze/Release
****************************************************************************/

#ifndef W_ANKLE_ANGLE_PLOT_H
#define W_ANKLE_ANGLE_PLOT_H

//****************************************************************************
// Include(s)
//****************************************************************************

//#include <QApplication>
#include <QWidget>
#include "counter.h"
#include <QTimer>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <streammanager.h>
#include <QVector>
#include "rigidDevice.h"
#include "define.h"

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************

QT_CHARTS_USE_NAMESPACE

class AnkleAngleChartView : public QChartView
{
		Q_OBJECT

public:
	explicit AnkleAngleChartView(QChart* parent) : QChartView(parent), activeSetPoint(-1), activeDrag(0) {
		qDebug() << "Updates " << this->updatesEnabled();

		xMin = 0;
		xMax = 1500;
		yMin = 0;
		yMax = 900;

		parent->axisX()->setRange(xMin, xMax);
		parent->axisY()->setRange(yMin, yMax);
		QColor darkGray(100, 100, 100);
		parent->axisX()->setGridLineColor(darkGray);
		parent->axisY()->setGridLineColor(darkGray);
	}
	virtual ~AnkleAngleChartView(){}

	bool isActive = true;
	QLineSeries* lineSeries;
	bool forceRecomputeDrawnPoints = false;
	int xMin, xMax, yMin, yMax;

	virtual void drawForeground(QPainter* painter, const QRectF &rect)
	{
		if(!isActive) return;

		if(firstDraw || forceRecomputeDrawnPoints)
		{
			firstDraw = false;
			forceRecomputeDrawnPoints = false;
		}

		QChartView::drawForeground(painter, rect);

		painter->setBrush(Qt::NoBrush);
		QPen myPen;
		myPen.setColor(QColor(0, 255, 64));
		myPen.setWidth(2);
		painter->setPen(myPen);	//bright green

		//draw data points
		int numLines = dataPoints.size()-1;
		for(int i = 1; i < numLines; i++)
		{
			painter->setOpacity((i) / (float)numLines);
			if(dataPoints.at(i).toPoint().x() > (dataPoints.at(i+1).toPoint().x()))
			{
				//qDebug() << "End of line.";
				painter->setOpacity(0);
				painter->drawLine(chart()->mapToPosition(dataPoints.at(i)), chart()->mapToPosition(dataPoints.at(i+1)));
			}
			else
			{
				painter->drawLine(chart()->mapToPosition(dataPoints.at(i)), chart()->mapToPosition(dataPoints.at(i+1)));
			}
		}
		painter->setOpacity(1);
	}

	int activeSetPoint;
	int activeDrag;

	uint16_t getMaxDataPoints() { return maxDataPoints; }
	void setMaxDataPoints(uint16_t x)
	{
		maxDataPoints = x;
		while(dataPoints.size() > 0 && dataPoints.size() > (int)x)
			dataPoints.removeFirst();
	}
	void addDataPoint(QPointF p)
	{
		while(dataPoints.size() > 0 && dataPoints.size() >= (int)maxDataPoints)
			dataPoints.removeFirst();

		dataPoints.push_back(p);
	}
	void addDataPoint(float angle, float torque)
	{
		while(dataPoints.size() > 0 && dataPoints.size() >= (int)maxDataPoints)
			dataPoints.removeFirst();

		dataPoints.push_back(QPointF(angle, torque));
	}

	void setAxisLimits(float xMin, float xMax, float yMin, float yMax)
	{
		this->xMin = xMin;
		this->xMax = xMax;
		this->yMin = yMin;
		this->yMax = yMax;

		chart()->axisX()->setRange(xMin, xMax);
		chart()->axisY()->setRange(yMin, yMax);
	}

	void clearOverlay() { dataPoints.clear(); chart()->update(); this->update(); }

signals:
	void pointsChanged();

private:
	bool firstDraw = true;
	unsigned int maxDataPoints = 20;
	QVector<QPointF> dataPoints;

};

namespace Ui {
class W_AnkleAnglePlot;
}

class W_AnkleAnglePlot : public QWidget, public Counter<W_AnkleAnglePlot>
{
	Q_OBJECT

public:

	explicit W_AnkleAnglePlot(QWidget *parent = 0, StreamManager* sm = nullptr);
	virtual ~W_AnkleAnglePlot();

public slots:

	void receiveNewData(void);
	void streamingFrequency(int f);
	void resizeEvent(QResizeEvent *event)
	{
		if(chartView) chartView->forceRecomputeDrawnPoints = true;
		QWidget::resizeEvent(event);
	}

	void on_lineEditXMax_returnPressed();
	void on_lineEditYMin_returnPressed();
	void on_lineEditYMax_returnPressed();
	void on_lineEditPersistentPoints_returnPressed();

signals:

	void windowClosed(void);
	void getCurrentDevice(FlexseaDevice** device);

private:

	Ui::W_AnkleAnglePlot *ui;
	QChart *chart;
	AnkleAngleChartView *chartView;

	bool plotFreezed, initFlag;
	bool pointsVisible;

	bool isComPortOpen = true;
	int streamingFreq = 1;
	int rollover = 1;

	//Scaling:
	int32_t scaling[2];

	void setAxesLimits();
	StreamManager* streamManager = nullptr;
};

#endif // W_ANKLE_ANGLE_PLOT_H
