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

	for(int i = 0; i < A2PLOT_VAR_NUM; i++)
	{
		lineSeries[i] = new QLineSeries();
		lineSeries[i]->append(0, 0);
		chart->addSeries(lineSeries[i]);
	}

	chart->createDefaultAxes();

	//Colors:
	chart->setTheme(QChart::ChartThemeDark);

	//Chart view:
	chartView = new AnkleAngleChartView(chart);
	chartView->isActive = false;
	for(int i = 0; i < A2PLOT_VAR_NUM; i++)
	{
		chartView->lineSeries[0] = lineSeries[0];
	}
	chartView->setMaxDataPoints(500);

	ui->gridLayout_test->addWidget(chartView, 0,0);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->setBaseSize(600,300);
	chartView->setMinimumSize(500,300);
	chartView->setMaximumSize(4000,2500);
	chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	chart->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	QPixmapCache::setCacheLimit(100000);

	const QValidator *validatorX = new QDoubleValidator(-2000, 3000, 2, this);
	const QValidator *validatorY = new QDoubleValidator(-9000, 9000, 2, this);
	ui->lineEditXMax->setValidator(validatorX);
	ui->lineEditYMin->setValidator(validatorY);
	ui->lineEditYMax->setValidator(validatorY);

	ui->lineEditXMax->setText(QString::number(chartView->xMax));
	ui->lineEditYMin->setText(QString::number(chartView->yMin));
	ui->lineEditYMax->setText(QString::number(chartView->yMax));
	rollover = chartView->xMax;

	int numPoints = chartView->getMaxDataPoints();
	ui->lineEditPersistentPoints->clear();
	ui->lineEditPersistentPoints->setText(QString::number(numPoints));

	//ComboBox leg:
	ui->comboBoxLeg->addItem("Right Leg");
	ui->comboBoxLeg	->addItem("Left Leg");
	ui->comboBoxLeg->setCurrentIndex(0);

	ui->checkBoxDisableFading->setChecked(false);
	emit on_checkBoxDisableFading_toggled(false);

	//Enable/disable variables:
	ui->checkBoxGS->setChecked(false);
	ui->checkBoxWS->setChecked(false);
	ui->checkBoxP->setChecked(false);
	ui->checkBoxCLHS->setChecked(false);
	//Array of cBoxes:
	cbVar[1] = &ui->checkBoxGS;
	cbVar[2] = &ui->checkBoxWS;
	cbVar[3] = &ui->checkBoxP;
	cbVar[4] = &ui->checkBoxCLHS;
}

W_AnkleAnglePlot::~W_AnkleAnglePlot()
{
	emit windowClosed();
	delete ui;
}

//****************************************************************************
// Public slot(s):
//****************************************************************************

//#define TRIG_ZERO	50
#define TRIG_DISP_LATCH	25

void W_AnkleAnglePlot::receiveNewData(void)
{
	static uint16_t idx = 0, lastGstate = 0;
	uint16_t newGstate = 0;
	uint16_t step = 0;
	static int lastRollover = 0, triggerPoint = 0, trigCnt = 0, rollCnt = 0;
	int seconds = rollover / 1000;
	if(seconds <= 0){seconds = 1;}
	//ToDo: this could be done with floats

	chartView->isActive = true;
	chartView->update();
	chart->update();

	//Protection against 0 div:
	if(rollover <= 0){rollover = 1;}
	if(streamingFreq <= 0){streamingFreq = 1;}

	//Protection against changing rollover that could lock it up:
	if(lastRollover != rollover){idx = 0;}
	lastRollover = rollover;

	step = rollover / (seconds * streamingFreq);
	idx += step;
	if(idx > rollover)
	{
		idx = 0;
		rollCnt = TRIG_DISP_LATCH;
	}

	if(ui->checkBoxFake->isChecked() == false)
	{
		//Normal operation - real data:
		//Fixed trigger: gaitState
		newGstate = rigid1.ctrl.gaitState;
		if(ui->comboBoxLeg->currentIndex() == 1){newGstate = rigid2.ctrl.gaitState;}

		if(lastGstate == 0 &&  newGstate == 1)
		{
			//Triggered
			triggerPoint = idx;
			trigCnt = TRIG_DISP_LATCH;
			idx = 0;
			qDebug() << "trigger latched";
		}
		lastGstate = newGstate;
		mapSensorsToPoints(idx);
	}
	else
	{
		//Fake data - test, demo & dev:
		if(lastGstate == 0 && pts[1].y())
		{
			triggerPoint = idx;
			trigCnt = TRIG_DISP_LATCH;
			idx = 0;
		}
		lastGstate = pts[1].y() ;
		fakeDataToPoints(idx);
	}

	//Display trigger events:
	if(trigCnt > 0)
	{
		trigCnt--;
		ui->label_Trigger->setText("Trig! " + QString::number(triggerPoint));
		ui->label_Trigger->setStyleSheet("background-color: rgb(0, 255, 0); \
									   color: rgb(0, 0, 0)");
	}
	else
	{
		ui->label_Trigger->setText("");
		ui->label_Trigger->setStyleSheet("");
	}

	//Display rollover events:
	if(rollCnt > 0)
	{
		rollCnt--;
		ui->label_Rollover->setText("Rollover!");
		ui->label_Rollover->setStyleSheet("background-color: rgb(255, 0, 0); \
									   color: rgb(0, 0, 0)");
	}
	else
	{
		ui->label_Rollover->setText("");
		ui->label_Rollover->setStyleSheet("");
	}

	displayOrNot();
	chartView->addDataPoints(pts);

	//Debugging:
	QString dbg = "Freq: " + QString::number(streamingFreq) + ", Rollover: " + \
			QString::number(rollover) + ", Index: " + QString::number(idx) + \
			", Step: " + QString::number(step);
	//qDebug() << dbg;
	ui->labelDebug->setText(dbg);
}

void W_AnkleAnglePlot::refreshDisplayLog(int index, FlexseaDevice * devPtr)
{
	//Work in Progress - doesn't work yet
	//qDebug() << "A2Plot refreshDisplayLog()";
	/*
	(void)devPtr;
	logIndex = index;

	saveNewPointsLog(index);
	refresh2DPlot();
	*/

	(void)index;
	(void)devPtr;
	receiveNewData();
}

//Suppress values that we do not want to display:
void W_AnkleAnglePlot::displayOrNot(void)
{
	for(int i = 1; i < A2PLOT_VAR_NUM; i++)
	{
		if((*cbVar[i])->isChecked() == false){pts[i] = QPointF(0,0);}
	}
}

//This links rigid1 (or other) values to points
void W_AnkleAnglePlot::mapSensorsToPoints(int idx)
{
	struct rigid_s *ri = &rigid1;

	if(ui->comboBoxLeg->currentIndex() == 1){ri = &rigid2;}

	pts[0] = QPointF(idx, *ri->ex.joint_ang);
	pts[1] = QPointF(idx, 100*ri->ctrl.gaitState);
	pts[2] = QPointF(idx, 100*ri->ctrl.walkingState);
	pts[3] = QPointF(idx, ri->ctrl.step_energy);
	pts[4] = QPointF(idx, ri->ctrl.contra_hs);
}

//This generates fake data and maps it to points
void W_AnkleAnglePlot::fakeDataToPoints(int idx)
{
	static int trig[2] = {0,0};
	int lim = 0;
	int p = 0;
	static bool k = false;

	if(ui->comboBoxLeg->currentIndex() == 0){lim = 1000;}
	else{lim = 1500;}

	if(idx > lim && idx < lim+100){trig[0] = 100;}
	else{trig[0] = 0;}
	if(idx > rollover/2) {trig[1] = 110;}
	else{trig[1] = 0;}

	if(idx < rollover/2){p = idx;}
	else{p = 0;}
	p = p/33;
	p = pow(p, 2);

	pts[0] = QPointF(idx, idx);
	pts[1] = QPointF(idx, trig[0]);
	pts[2] = QPointF(idx, trig[1]);
	pts[3] = QPointF(idx, p);
	if(!k){pts[4] = QPointF(idx, 50); k = true;}
	else{pts[4] = QPointF(idx, 0); k = false;}
}

void W_AnkleAnglePlot::streamingFrequency(int f)
{
	streamingFreq = f;
}

void W_AnkleAnglePlot::setAxesLimits()
{
	QString xMinText = "0";
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
		rollover = xmax;
		chartView->setAxisLimits(xmin, xmax, ymin, ymax);
		chartView->forceRecomputeDrawnPoints = true;
		chartView->update();
		chart->update();
	}
}

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

void W_AnkleAnglePlot::on_pushButtonOneCycle_clicked()
{
	int pts = (rollover * streamingFreq) / 1000;
	QString str = QString::number(pts);
	ui->lineEditPersistentPoints->setText(str);
	emit on_lineEditPersistentPoints_returnPressed();
}

void W_AnkleAnglePlot::on_checkBoxDisableFading_toggled(bool checked)
{
	fadePoints = !checked;
	chartView->fadePoints = fadePoints;
}
