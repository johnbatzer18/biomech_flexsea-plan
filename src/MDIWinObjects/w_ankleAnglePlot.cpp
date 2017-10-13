/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'plan-gui' Graphical User Interface
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] JFD, with a lot of cut & paste from D.W.'s work
	[Origin]
	[Contributors]
*****************************************************************************
	[This file] w_ankleAnglePlot.h: 2D Plot window with persistence
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-10-13 | jfduval | Copied from the hacked version of AnkleTorque
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_ankleAnglePlot.h"
#include "ui_w_ankleAnglePlot.h"
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

W_AnkleAnglePlot::W_AnkleAnglePlot(QWidget *parent, StreamManager* sm) :
	QWidget(parent), streamManager(sm),
	ui(new Ui::W_AnkleAnglePlot)
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
	chartView = new AnkleAngleChartView(chart);
	chartView->isActive = false;
	chartView->lineSeries = lineSeries;
	chartView->setMaxDataPoints(500);

	ui->gridLayout_test->addWidget(chartView, 0,0);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->setBaseSize(600,300);
	chartView->setMinimumSize(500,300);
	chartView->setMaximumSize(4000,2500);
	chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	chart->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	QPixmapCache::setCacheLimit(100000);

	timer = new QTimer();
	timer->setInterval(50);
	timer->setSingleShot(false);
	connect(timer, &QTimer::timeout, this, &W_AnkleAnglePlot::requestProfileRead);
	timer->start();
	requestProfileRead();

	const QValidator *validator = new QDoubleValidator(-1000, 1000, 2, this);
	ui->lineEditXMin->setValidator(validator);
	ui->lineEditXMax->setValidator(validator);
	ui->lineEditYMin->setValidator(validator);
	ui->lineEditYMax->setValidator(validator);

	ui->lineEditXMin->setText(QString::number(chartView->xMin));
	ui->lineEditXMax->setText(QString::number(chartView->xMax));
	ui->lineEditYMin->setText(QString::number(chartView->yMin));
	ui->lineEditYMax->setText(QString::number(chartView->yMax));

	int numPoints = chartView->getMaxDataPoints();
	ui->lineEditPersistentPoints->clear();
	ui->lineEditPersistentPoints->setText(QString::number(numPoints));
}

void W_AnkleAnglePlot::requestProfileRead()
{
	//We skip the TX/RX and simply set this flag:
	atProfile_newProfileFlag = 1;
}

W_AnkleAnglePlot::~W_AnkleAnglePlot()
{
	emit windowClosed();
	delete ui;
}

//****************************************************************************
// Public slot(s):
//****************************************************************************

void W_AnkleAnglePlot::receiveNewData(void)
{
	static uint16_t idx = 0, lastGstate = 0;

	if(atProfile_newProfileFlag)
	{
		atProfile_newProfileFlag = 0;

		chartView->isActive = true;
		timer->stop();
	}
	if(atProfile_newDataFlag)
	{
		atProfile_newDataFlag = 0;
	}
	chartView->update();
	chart->update();

	idx += 10;
	idx %= 1200;
	if(lastGstate == 0 && rigid1.ctrl.gaitState == 1)
	{
		idx = 0;
	}
	lastGstate = rigid1.ctrl.gaitState;
	chartView->addDataPoint(idx, *rigid1.ex.joint_ang);
}

void W_AnkleAnglePlot::setAxesLimits()
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

void W_AnkleAnglePlot::on_lineEditXMin_returnPressed() {setAxesLimits();}
void W_AnkleAnglePlot::on_lineEditXMax_returnPressed() {setAxesLimits();}
void W_AnkleAnglePlot::on_lineEditYMin_returnPressed() {setAxesLimits();}
void W_AnkleAnglePlot::on_lineEditYMax_returnPressed() {setAxesLimits();}

void W_AnkleAnglePlot::on_lineEditPersistentPoints_returnPressed() {
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
