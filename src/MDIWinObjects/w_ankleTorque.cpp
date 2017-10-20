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
	[This file] W_AnkleTorque.h: 2D Plot window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-09-12 | jfduval | Added Freeze/Release
	* 2016-12-1x | jfduval | Major refactoring
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_ankleTorque.h"
#include "ui_w_ankleTorque.h"
#include <QApplication>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QDebug>
#include <QElapsedTimer>
#include <QValidator>
#include "flexsea_generic.h"
#include "main.h"

QT_CHARTS_USE_NAMESPACE

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_AnkleTorque::W_AnkleTorque(QWidget *parent, StreamManager* sm) :
	QWidget(parent), streamManager(sm),
	ui(new Ui::W_AnkleTorque)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	//Chart:
	chart = new QChart();
	chart->legend()->hide();

	QLineSeries* lineSeries = new QLineSeries();
	lineSeries->append(0, 0);
	chart->addSeries(lineSeries);
	chart->createDefaultAxes();

	//Colors:
	chart->setTheme(QChart::ChartThemeDark);

	//Chart view:
	chartView = new AnkleTorqueChartView(chart);
	chartView->isActive = false;
	chartView->lineSeries = lineSeries;
	chartView->setMaxDataPoints(150);
	for(int i = 0; i < ATCV_NUMPOINTS; i++){chartView->setPoint(i, 0.0f, 0.0f);}

	connect(chartView,	&AnkleTorqueChartView::pointsChanged,
				this,	&W_AnkleTorque::handlePointChange);

	ui->gridLayout_test->addWidget(chartView, 0,0);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->setBaseSize(600,300);
	chartView->setMinimumSize(500,300);
	chartView->setMaximumSize(4000,2500);
	chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	chart->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	QPixmapCache::setCacheLimit(100000);

	const QValidator *validator = new QDoubleValidator(-1000, 1000, 2, this);
	ui->lineEditXMin->setValidator(validator);
	ui->lineEditXMax->setValidator(validator);
	ui->lineEditYMin->setValidator(validator);
	ui->lineEditYMax->setValidator(validator);

	ui->lineEditXMin->setText(QString::number(chartView->xMin));
	ui->lineEditXMax->setText(QString::number(chartView->xMax));
	ui->lineEditYMin->setText(QString::number(chartView->yMin));
	ui->lineEditYMax->setText(QString::number(chartView->yMax));

	ui->checkBoxOverlay->setChecked(false);
	ui->checkBoxOverlay->setEnabled(false);

	ui->checkBoxLabels->setChecked(true);
	chartView->drawText = true;

	ui->lineEditPersistentPoints->setEnabled(false);
}

W_AnkleTorque::~W_AnkleTorque()
{
	emit windowClosed();
	delete ui;
}

void W_AnkleTorque::handlePointChange()
{
	QPointF p[ATCV_NUMPOINTS];
	int8_t ptArray[ATCV_NUMPOINTS][2];
	chartView->getPoints(p, ATCV_NUMPOINTS);

	for(int i = 0; i < ATCV_NUMPOINTS; i++)
	{
		ptArray[i][0] = (int8_t) p[i].x();
		ptArray[i][1] = (int8_t) p[i].y();
	}

	emit pointsChanged(ptArray);
}

//****************************************************************************
// Public slot(s):
//****************************************************************************

void W_AnkleTorque::torquePointsChanged(int8_t pts0[6][2], int8_t pts1[6][2])
{
	qDebug() << "UserTesting has new points for AnkleTorqueTool... (leg = " << activeLeg << ")";

	int8_t atAngle[6], atTorque[6];
	int idx = activeLeg;
	if(activeLeg == 2){idx = 0;}

	for(int i = 0; i < ATCV_NUMPOINTS; i++)
	{
		if(idx == 0)
		{
			atAngle[i] = pts0[i][0];
			atTorque[i] = pts0[i][1];
		}
		else
		{
			atAngle[i] = pts1[i][0];
			atTorque[i] = pts1[i][1];
		}

		chartView->setPoint(i, atAngle[i], atTorque[i]);
	}

	chartView->isActive = true;
	chartView->update();
	chart->update();
}

void W_AnkleTorque::legs(bool ind, uint8_t LR)
{
	QString txt = "Both legs";
	activeLeg = 2;
	if(ind)
	{
		if(!LR)
		{
			txt = "Right Leg";
			activeLeg = 0;
		}
		else
		{
			txt = "Left Leg";
			activeLeg = 1;
		}
	}
	ui->labelLeg->setText(txt);
}

void W_AnkleTorque::setAxesLimits()
{
	QString xMinText = ui->lineEditXMin->text();
	QString xMaxText = ui->lineEditXMax->text();
	QString yMinText = ui->lineEditYMin->text();
	QString yMaxText = ui->lineEditYMax->text();

	bool success = true;
	int xmin=-1,xmax=0,ymin=-1,ymax=0;
	if(success) xmin = xMinText.toFloat(&success);
	if(success) xmax = xMaxText.toFloat(&success);
	if(success) ymin = yMinText.toFloat(&success);
	if(success) ymax = yMaxText.toFloat(&success);

	if(success)
	{
		chartView->setAxisLimits(xmin, xmax, ymin, ymax);
		chartView->forceRecomputeDrawnPoints = true;
		chartView->update();
		chart->update();
	}
}

void W_AnkleTorque::on_lineEditXMin_returnPressed() {setAxesLimits();}
void W_AnkleTorque::on_lineEditXMax_returnPressed() {setAxesLimits();}
void W_AnkleTorque::on_lineEditYMin_returnPressed() {setAxesLimits();}
void W_AnkleTorque::on_lineEditYMax_returnPressed() {setAxesLimits();}

void W_AnkleTorque::on_lineEditPersistentPoints_returnPressed() {
	QString text = ui->lineEditPersistentPoints->text();
	bool success = true;

	int numPoints = text.toInt(&success);
	if(success && numPoints > 2)
	{
		chartView->setMaxDataPoints(numPoints);
	}
	else
	{
		numPoints = chartView->getMaxDataPoints();
		ui->lineEditPersistentPoints->clear();
		ui->lineEditPersistentPoints->setText(QString::number(numPoints));
	}
	chart->update();
	chartView->update();
}

void W_AnkleTorque::on_checkBoxLabels_toggled(bool checked)
{
	chartView->drawText = checked;
	chartView->update();
	chart->update();
}
