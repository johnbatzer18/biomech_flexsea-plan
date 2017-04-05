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

#ifndef W_ANKLE_TORQUE_TOOL_H
#define W_ANKLE_TORQUE_TOOL_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QApplication>
#include <QWidget>
#include "counter.h"
#include <QTimer>
#include <QtCharts>
#include <QtCharts/QChartView>
#include "flexsea_generic.h"
#include "flexseaDevice.h"
#include <QtCharts/QXYSeries>
#include "executeDevice.h"
#include "define.h"
#include <QDrag>

//****************************************************************************
// Definition(s)
//****************************************************************************

#define INIT_PLOT_XMIN				0
#define INIT_PLOT_XMAX				200
#define INIT_PLOT_YMIN				-1000
#define INIT_PLOT_YMAX				1000
#define INIT_PLOT_LEN				((INIT_PLOT_XMAX-INIT_PLOT_XMIN)+1)
#define VAR_NUM						6
#define PLOT_BUF_LEN				1000

#define TWO_PI						(2*3.14159)
#define PHASE_INCREMENT				(TWO_PI/75)
#define A_GAIN						1000

//Stats:
#define STATS_FIELDS				4
#define STATS_MIN					0
#define STATS_MAX					1
#define STATS_AVG					2
#define STATS_RMS					3

//Scaling:
#define SCALE_DEFAULT_M				1
#define SCALE_DEFAULT_B				0

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************

QT_CHARTS_USE_NAMESPACE

class AnkleTorqueChartView : public QChartView
{
		Q_OBJECT

#define ATCV_NUMPOINTS 6
#define ATCV_ABS(x) ( (x) > 0 ? (x) : -(x) )

public:
	explicit AnkleTorqueChartView(QChart* parent) : QChartView(parent), activeSetPoint(-1), activeDrag(0) {
		qDebug() << "Updates " << this->updatesEnabled();
		xMin = -45;
		xMax = 5;
		yMin = -500;
		yMax = 200;

		int xAxisExtent5Percent = (5 * (xMax - xMin) + 50) / 100;
		int yAxisExtent5Percent = (5 * (yMax - yMin) + 50) / 100;

		parent->axisX()->setRange(xMin - xAxisExtent5Percent, xMax + xAxisExtent5Percent);
		parent->axisY()->setRange(yMin - yAxisExtent5Percent, yMax + yAxisExtent5Percent);

	}
	virtual ~AnkleTorqueChartView(){}

	int radius = 10;
	virtual void drawForeground(QPainter* painter, const QRectF &rect)
	{
		if(firstDraw)
		{
			firstDraw = false;
			for(int i = 0; i < ATCV_NUMPOINTS; i++)
				drawnPoints[i] = chart()->mapToPosition(points[i]);
		}

		QChartView::drawForeground(painter, rect);

		painter->setPen(QPen(QColor(Qt::red)));
		painter->setBrush(Qt::NoBrush);

		painter->drawRect(QRectF(
							  chart()->mapToPosition(QPointF(xMin, yMin)),
							  chart()->mapToPosition(QPointF(xMax, yMax))));

		painter->setPen(QPen(QColor(Qt::white)));
		for(int i = 0; i < ATCV_NUMPOINTS; i++)
		{
			painter->drawEllipse(drawnPoints[i], radius, radius);
		}
	}

	int activeSetPoint;
	int activeDrag;
	virtual void mousePressEvent(QMouseEvent *event)
	{
		if(!(activeSetPoint < 0)) return;

		QPointF p = event->pos();
		QPointF sp;
		static int distThresh = radius*radius;
		float xdist, ydist, dist;

		for(int i = 0; i < ATCV_NUMPOINTS; i++)
		{
			sp = drawnPoints[i];

			xdist = (p.x() - sp.x());
			ydist = (p.y() - sp.y());

			dist = xdist*xdist + ydist*ydist;

			if( dist < distThresh )
			{
				activeSetPoint = i;
				return;
			}
		}
		activeSetPoint = -1;
	}

	virtual void mouseMoveEvent(QMouseEvent* event)
	{
		if(activeSetPoint < 0) return;

		QPointF p = event->pos();
		QPointF sp = drawnPoints[activeSetPoint];

		float dist = (p - sp).manhattanLength();

		static float distThresh = 0.5*radius;

		if(dist > distThresh)
		{
			drawnPoints[activeSetPoint] = p;
			this->update();
			chart()->update();
		}
	}

	void enforceBounds(QPointF* p)
	{
		if(!p) return;

		if(p->x() > xMax) p->setX(xMax);
		if(p->y() > yMax) p->setY(yMax);

		if(p->x() < xMin) p->setX(xMin);
		if(p->y() < yMin) p->setY(yMin);
	}

	virtual void mouseReleaseEvent(QMouseEvent* event)
	{
		if(activeSetPoint < 0) return;

		drawnPoints[activeSetPoint] = event->pos();
		QPointF* p = &points[activeSetPoint];
		*p = chart()->mapToValue(drawnPoints[activeSetPoint]);

		enforceBounds(p);

		drawnPoints[activeSetPoint] = chart()->mapToPosition(*p);

		this->update();
		chart()->update();
		activeSetPoint = -1;
	}

	void setPoints(QPointF p[ATCV_NUMPOINTS])
	{
		for(int i = 0; i < ATCV_NUMPOINTS; i++)
		{
			points[i] = p[i];
			enforceBounds(&points[i]);
			drawnPoints[i] = chart()->mapToPosition(p[i]);
		}
		this->update();
		chart()->update();
	}

	void setPoint(int i, const QPointF &p)
	{
		if(i >= 0 && i < ATCV_NUMPOINTS)
		{
			points[i] = p;
			enforceBounds(&points[i]);
			drawnPoints[i] = chart()->mapToPosition(points[i]);
			this->update();
			chart()->update();
		}
	}

	void setPoint(int i, float x, float y)
	{
		if(i >= 0 && i < ATCV_NUMPOINTS)
		{
			points[i] = QPointF(x, y);
			enforceBounds(&points[i]);
			drawnPoints[i] = chart()->mapToPosition(points[i]);
			this->update();
			chart()->update();
		}
	}

	void getPoints(QPointF* p, int n)
	{
		for(int i = 0; i < n && i < ATCV_NUMPOINTS; i++)
			p[i] = points[i];
	}

signals:
	void pointsChanged();

private:
	QPointF points[ATCV_NUMPOINTS];
	QPointF drawnPoints[ATCV_NUMPOINTS];
	bool firstDraw = true;
	int xMin, xMax, yMin, yMax;
};

namespace Ui {
class W_AnkleTorque;
}

class W_AnkleTorque : public QWidget, public Counter<W_AnkleTorque>
{
	Q_OBJECT

public:

	//Constructor & Destructor:
	explicit W_AnkleTorque(QWidget *parent = 0);
	~W_AnkleTorque();

public slots:

	void receiveNewData(void);
	void refresh2DPlot(void);
	void updateDisplayMode(DisplayMode mode, FlexseaDevice* devPtr);
	void handlePointChange();

signals:

	void windowClosed(void);
	void getSlaveId(int* slaveId);
	void writeCommand(uint16_t numBytes, uint8_t* bytes, uint8_t readWrite);

private:

	Ui::W_AnkleTorque *ui;
	QChart *chart;
	AnkleTorqueChartView *chartView;
	QLineSeries *qlsChart[VAR_NUM];
	QVector<QPointF> vDataBuffer[VAR_NUM];
	QDateTime *timerRefreshDisplay, *timerRefreshData;
	int plot_xmin, plot_ymin, plot_xmax, plot_ymax, plot_len;
	int globalYmin, globalYmax;

	QStringList var_list_margin;
	bool plotFreezed, initFlag;
	bool pointsVisible;

	uint8_t varToPlotFormat[6];

	uint8_t varIndex[VAR_NUM];
	int64_t stats[VAR_NUM][STATS_FIELDS];
	int32_t myFakeData;
	float dataRate;

	//Scaling:
	int32_t scaling[VAR_NUM][2];
};

#endif // W_ANKLE_TORQUE_TOOL_H
