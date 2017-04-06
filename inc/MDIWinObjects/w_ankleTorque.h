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
#include <vector>
#include <QVector>

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
		xMin = -30;
		xMax = 0;
		yMin = 0;
		yMax = 60;

		int xAxisExtent5Percent = (5 * (xMax - xMin) + 50) / 100;
		int yAxisExtent5Percent = (5 * (yMax - yMin) + 50) / 100;

		parent->axisX()->setRange(xMin - 5, xMax + 5);
		parent->axisY()->setRange(yMin - 5, yMax + 5);
	}
	virtual ~AnkleTorqueChartView(){}

	int radius = 10;
	bool isActive = true;
	QLineSeries* lineSeries;
	bool windowResized = false;

	void recomputeProfileDrawPoints()
	{
		for(int i = 0; i < ATCV_NUMPOINTS; i++)
			drawnPoints[i] = chart()->mapToPosition(points[i]);
	}

	virtual void drawForeground(QPainter* painter, const QRectF &rect)
	{
		if(!isActive) return;

		if(firstDraw || windowResized)
		{
			firstDraw = false;
			windowResized = false;
			recomputeProfileDrawPoints();
		}

		QChartView::drawForeground(painter, rect);

		painter->setBrush(Qt::NoBrush);
		painter->setPen(QPen(QColor(Qt::green)));

		//draw data points
		int numLines = dataPoints.size()-1;
		for(int i = 0; i < numLines; i++)
		{
			painter->setOpacity((i) / (float)numLines);
			painter->drawLine(chart()->mapToPosition(dataPoints.at(i)), chart()->mapToPosition(dataPoints.at(i+1)));
		}
		painter->setOpacity(1);

		//draw bounds
		painter->setPen(QPen(QColor(Qt::red)));
		painter->drawRect(QRectF(
							  chart()->mapToPosition(QPointF(xMin, yMin)),
							  chart()->mapToPosition(QPointF(xMax, yMax))));

		//draw profile points
		painter->setPen(QPen(QColor(Qt::white)));
		QPointF textPoint;
		QString positionLabel = "";
		QPainterPath path;
		QFont font("Arial", 10, 50, false);
		for(int i = 0; i < ATCV_NUMPOINTS; i++)
		{
			//line to the next point
			if(i + 1 < ATCV_NUMPOINTS)
				painter->drawLine(drawnPoints[i], drawnPoints[i+1]);

			//text labeling the index
			textPoint = drawnPoints[i];
			textPoint.setY(textPoint.y() - radius - 2);

			path.addText(textPoint, font, QString::number(i+1));

			//text labeling the position
			textPoint.setY(textPoint.y() + 14 + 2 * radius);
			textPoint.setX(textPoint.x() - radius);
			positionLabel = QString::number(points[i].x()) + ", " + QString::number(points[i].y());

			path.addText(textPoint, font, positionLabel);

			//the circle
			painter->drawEllipse(drawnPoints[i], radius, radius);
		}

		painter->setBrush(QBrush(QColor(Qt::black)));
		painter->setPen(QColor(Qt::white));
		painter->drawPath(path);
	}

	int activeSetPoint;
	int activeDrag;
	virtual void mousePressEvent(QMouseEvent *event)
	{
		if(!(activeSetPoint < 0) || !isActive) return;

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

		static float distThresh = 0.25*radius;

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

		emit pointsChanged();
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


signals:
	void pointsChanged();

private:
	QPointF points[ATCV_NUMPOINTS];
	QPointF drawnPoints[ATCV_NUMPOINTS];
	bool firstDraw = true;
	int xMin, xMax, yMin, yMax;

	unsigned int maxDataPoints = 20;
	QVector<QPointF> dataPoints;

};

namespace Ui {
class W_AnkleTorque;
}

class W_AnkleTorque : public QWidget, public Counter<W_AnkleTorque>
{
	Q_OBJECT

public:

	explicit W_AnkleTorque(QWidget *parent = 0);
	virtual ~W_AnkleTorque();

	static int getCommandCode();

public slots:

	void receiveNewData(void);
	void refresh2DPlot(void);
	void handlePointChange();
	void comStatusChanged(bool open);
	void requestProfileRead();
	void resizeEvent(QResizeEvent *event)
	{
		if(chartView) chartView->windowResized = true;
		QWidget::resizeEvent(event);
	}

signals:

	void windowClosed(void);
	void getSlaveId(int* slaveId);
	void writeCommand(uint16_t numBytes, uint8_t* bytes, uint8_t readWrite);

private:

	Ui::W_AnkleTorque *ui;
	QChart *chart;
	AnkleTorqueChartView *chartView;
	QTimer* timer;

	bool plotFreezed, initFlag;
	bool pointsVisible;

	bool isComPortOpen = true;

	//Scaling:
	int32_t scaling[2];
};

#endif // W_ANKLE_TORQUE_TOOL_H