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
#include "flexsea_generic.h"
#include "main.h"

#include "flexsea.h"
#include "flexsea_board.h"
#include "flexsea_system.h"
#include "flexsea_cmd_angle_torque_profile.h"
#include <QValidator>

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

	/*
	timer = new QTimer();
	timer->setInterval(50);
	timer->setSingleShot(false);
	connect(timer, &QTimer::timeout, this, &W_AnkleTorque::requestProfileRead);
	timer->start();
	requestProfileRead();
	*/

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

/*
void W_AnkleTorque::comStatusChanged(bool open)
{
	isComPortOpen = open;
	if(open)
		requestProfileRead();
}
*/

//void W_AnkleTorque::requestProfileRead()
//{
	/*
	int slaveId = -1;
	emit getSlaveId(&slaveId);
	if(slaveId < 0 || !isComPortOpen) return;

	uint8_t info = PORT_USB;
	uint16_t numb = 0;

	tx_cmd_ankleTorqueProfile_r(TX_N_DEFAULT, 1);
	pack(P_AND_S_DEFAULT, slaveId, &info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, READ);
	*/
//}

W_AnkleTorque::~W_AnkleTorque()
{
	emit windowClosed();
	delete ui;
}

int W_AnkleTorque::getCommandCode() { return CMD_ANGLE_TORQUE_PROFILE; }

void W_AnkleTorque::handlePointChange()
{
	QPointF p[ATCV_NUMPOINTS];
	int8_t ptArray[ATCV_NUMPOINTS][2];
	chartView->getPoints(p, ATCV_NUMPOINTS);
	for(int i = 0; i < ATCV_NUMPOINTS; i++)
	{
		atProfile_torques[i] = p[i].y()*10;
		atProfile_angles[i] = p[i].x()*10;

		ptArray[i][0] = (int8_t) p[i].x();
		ptArray[i][1] = (int8_t) p[i].y();
	}

	emit pointsChanged(ptArray);

	int slaveId = -1;
	//emit getSlaveId(&slaveId);
	if(slaveId < 0) return;

	//uint8_t info = PORT_USB;
	//uint16_t numb = 0;

	chartView->isActive = false;
	timer->start();

	//tx_cmd_ankleTorqueProfile_rw(TX_N_DEFAULT);
	//pack(P_AND_S_DEFAULT, slaveId, &info, &numb, comm_str_usb);
	//emit writeCommand(numb, comm_str_usb, WRITE);
}

//****************************************************************************
// Public slot(s):
//****************************************************************************

void W_AnkleTorque::receiveNewData(void)
{
	if(atProfile_newProfileFlag)
	{
		atProfile_newProfileFlag = 0;
		for(int i = 0; i < ATCV_NUMPOINTS; i++)
			chartView->setPoint(i, atProfile_angles[i] / 10.0f, atProfile_torques[i] / 10.0f);

		chartView->isActive = true;
		timer->stop();
	}
	if(atProfile_newDataFlag)
	{
		atProfile_newDataFlag = 0;
		chartView->addDataPoint(angleBuf[indexOfLastBuffered] / 10.0f, torqueBuf[indexOfLastBuffered] / 10.0f);
	}
	chartView->update();
	chart->update();
}

void W_AnkleTorque::torquePointsChanged(void)
{
	qDebug() << "UserTesting has new points for AnkleTorqueTool...";

	for(int i = 0; i < ATCV_NUMPOINTS; i++)
	{
		atProfile_angles[i] = utt.leg[0].torquePoints[i][0];	//ToDo left vs right for all of these
		atProfile_torques[i] = utt.leg[0].torquePoints[i][1];
		qDebug() << "Point = " + QString::number(utt.leg[0].torquePoints[i][0]) + "," + QString::number(utt.leg[0].torquePoints[i][1]);

		chartView->setPoint(i, atProfile_angles[i] , atProfile_torques[i] );
	}

	chartView->isActive = true;
	chartView->update();
	chart->update();
}

void W_AnkleTorque::legs(bool ind, uint8_t LR)
{
	QString txt = "Both legs";
	if(ind)
	{
		if(!LR){txt = "Right Leg";}
		else{txt = "Left Leg";}
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

/*
void W_AnkleTorque::on_streamButton_pressed()
{
	static bool isStreaming = false;
	const int streamFreq = 33;
	if(!streamManager) return;

	int slaveId = -1;
	FlexseaDevice* device = nullptr;
	emit getSlaveId(&slaveId);
	emit getCurrentDevice(&device);

	if(slaveId < 0 || !device) return;

	if(isStreaming)
	{
		streamManager->stopStreaming(getCommandCode(), slaveId, streamFreq);
		chartView->clearOverlay();
	}
	else
	{
		streamManager->startAutoStreaming(getCommandCode(), slaveId, streamFreq, false, device, 0, 0);	//ToDo this has to be the index!!! not 0,0
	}

	QString btnText = "";
	btnText.append(isStreaming ? "Start" : "Stop");
	btnText.append(" Streaming Overlay");

	ui->streamButton->setText(btnText);
	isStreaming = !isStreaming;
}
*/

void W_AnkleTorque::on_checkBoxLabels_toggled(bool checked)
{
	chartView->drawText = checked;
	chartView->update();
	chart->update();
}
