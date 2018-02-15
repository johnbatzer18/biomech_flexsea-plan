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
#include <commanager.h>
#include <QVector>
#include "rigidDevice.h"
#include "define.h"
#include "w_2dplot.h"

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************

QT_CHARTS_USE_NAMESPACE

#define A2PLOT_VAR_NUM						6

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
	QLineSeries* lineSeries[A2PLOT_VAR_NUM];
	bool forceRecomputeDrawnPoints = false;
	int xMin, xMax, yMin, yMax;
	bool fadePoints = true;

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
		QPen myPen[A2PLOT_VAR_NUM];
		myPen[0].setColor(QColor(0, 255, 64));		//green
		myPen[0].setWidth(2);
		painter->setPen(myPen[0]);
		myPen[1].setColor(QColor(255, 0, 0));		//red
		myPen[1].setWidth(2);
		myPen[2].setColor(QColor(0, 0, 255));		//blue
		myPen[2].setWidth(2);
		myPen[3].setColor(QColor(255, 128, 0));		//orange
		myPen[3].setWidth(2);
		myPen[4].setColor(QColor(255, 255, 0));		//yellow
		myPen[4].setWidth(2);
		myPen[5].setColor(QColor(255, 255, 255));	//white
		myPen[5].setWidth(2);

		//draw data points
		int numLines = dataPoints[0].size()-1;
		for(int y = 0; y < A2PLOT_VAR_NUM; y++)
		{
			painter->setPen(myPen[y]);
			for(int i = 1; i < numLines; i++)
			{
				if(fadePoints == true){painter->setOpacity((i) / (float)numLines);}
				else{painter->setOpacity(1.0);}

				if(dataPoints[y].at(i).toPoint().x() > (dataPoints[y].at(i+1).toPoint().x()))
				{
					//qDebug() << "End of line.";
					painter->setOpacity(0);
					painter->drawLine(chart()->mapToPosition(dataPoints[y].at(i)), chart()->mapToPosition(dataPoints[y].at(i+1)));
				}
				else
				{
					painter->drawLine(chart()->mapToPosition(dataPoints[y].at(i)), chart()->mapToPosition(dataPoints[y].at(i+1)));
				}
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
		while(dataPoints[0].size() > 0 && dataPoints[0].size() > (int)x)
			dataPoints[0].removeFirst();
	}

	void addDataPoints(QPointF p[A2PLOT_VAR_NUM])
	{
		for(int y = 0; y < A2PLOT_VAR_NUM; y++)
		{
			while(dataPoints[y].size() > 0 && dataPoints[y].size() >= (int)maxDataPoints)
				dataPoints[y].removeFirst();

			dataPoints[y].push_back(p[y]);
		}
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

	void clearOverlay() { dataPoints[0].clear(); chart()->update(); this->update(); }

signals:
	void pointsChanged();

private:
	bool firstDraw = true;
	unsigned int maxDataPoints = 20;
	QVector<QPointF> dataPoints[A2PLOT_VAR_NUM];

};

namespace Ui {
class W_AnkleAnglePlot;
}

class W_AnkleAnglePlot : public QWidget, public Counter<W_AnkleAnglePlot>
{
	Q_OBJECT

public:

	explicit W_AnkleAnglePlot(QWidget *parent = 0,
							  QList<FlexseaDevice*> *devListInit = nullptr);
	virtual ~W_AnkleAnglePlot();

public slots:

	void receiveNewData(void);
	void refreshDisplayLog(int index, FlexseaDevice * devPtr);
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

private slots:
	void on_pushButtonOneCycle_clicked();
	void on_checkBoxDisableFading_toggled(bool checked);
	void on_cBoxvar1_currentIndexChanged(int index);
	void on_cBoxvar2_currentIndexChanged(int index);
	void on_cBoxvar3_currentIndexChanged(int index);
	void on_cBoxvar4_currentIndexChanged(int index);
	void on_cBoxvar5_currentIndexChanged(int index);
	void on_cBoxvar6_currentIndexChanged(int index);

private:

	Ui::W_AnkleAnglePlot *ui;
	QChart *chart;
	AnkleAngleChartView *chartView;
	QLineSeries* lineSeries[A2PLOT_VAR_NUM];
	QPointF pts[A2PLOT_VAR_NUM];
	int16_t instantStepEnergy = 0;
	QComboBox **comboVar[A2PLOT_VAR_NUM];

	struct vtp_s vtp[6];
	uint8_t varToPlotFormat[A2PLOT_VAR_NUM];
	uint8_t varIndex[A2PLOT_VAR_NUM];

	QList<FlexseaDevice*> *liveDevList;
	QList<FlexseaDevice*> *currentDevList;
	FlexseaDevice* selectedDevList[A2PLOT_VAR_NUM];

	bool plotFreezed, initFlag;
	bool pointsVisible;

	bool isComPortOpen = true;
	int streamingFreq = 1;
	int rollover = 1;
	bool fadePoints = true;

	//Scaling:
	int32_t scaling[2];

	void setAxesLimits();
	void displayOrNot(void);
	void mapSensorsToPoints(int idx);
	void fakeDataToPoints(int idx);
	void initPtr();
	void updateVarList(uint8_t item);
	void assignVariable(uint8_t item);
	int vtpToInt(uint8_t row);
};

#endif // W_ANKLE_ANGLE_PLOT_H
