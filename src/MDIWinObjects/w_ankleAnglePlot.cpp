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

W_AnkleAnglePlot::W_AnkleAnglePlot(QWidget *parent,
								   QList<FlexseaDevice*> *devListInit) :
	QWidget(parent),
	ui(new Ui::W_AnkleAnglePlot)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	initFlag = true;

	liveDevList = devListInit;
	currentDevList = liveDevList;

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
	const QValidator *validatorY = new QDoubleValidator(-50000, 50000, 2, this);
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

	initPtr();
	for(int item = 0; item < A2PLOT_VAR_NUM; item++)
	{
		updateVarList(item);
	}

	initFlag = false;
}

W_AnkleAnglePlot::~W_AnkleAnglePlot()
{
	emit windowClosed();
	delete ui;
}

//****************************************************************************
// Public slot(s):
//****************************************************************************

#define TRIG_DISP_LATCH         25
#define GAIT_ENERGY_LOG_PTS     10

void W_AnkleAnglePlot::receiveNewData(void)
{
	static uint16_t idx = 0, lastGstate = 0;
	static int16_t gaitEnergyLog[GAIT_ENERGY_LOG_PTS], gaitEnergyIndex = 0;
	static int16_t dispEnergy = 0;
	uint16_t newGstate = 0;
	uint16_t step = 0;
	int32_t energySum = 0;
	static int lastRollover = 0, triggerPoint = 0, trigCnt = 0, rollCnt = 0;
	int seconds = rollover / 100;
	if(seconds <= 0){seconds = 1;}

	chartView->isActive = true;
	chartView->update();
	chart->update();

	//Protection against 0 div:
	if(rollover <= 0){rollover = 1;}
	if(streamingFreq <= 0){streamingFreq = 1;}

	//Protection against changing rollover that could lock it up:
	if(lastRollover != rollover){idx = 0;}
	lastRollover = rollover;

	step = rollover / ((seconds * streamingFreq) / 10);
	idx += step;
	if(idx > rollover)
	{
		idx = 0;
		rollCnt = TRIG_DISP_LATCH;
	}

	//Gait Energy display - save last 10 points:
	gaitEnergyIndex++;
	gaitEnergyIndex %= GAIT_ENERGY_LOG_PTS;
	gaitEnergyLog[gaitEnergyIndex] = instantStepEnergy;

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

			//Average energy:
			energySum = 0;
			for(int i = 0; i < GAIT_ENERGY_LOG_PTS; i++)
			{
				energySum += gaitEnergyLog[i];
			}
			dispEnergy = energySum / GAIT_ENERGY_LOG_PTS;
			ui->label_Joules->setText(QString::number((float)dispEnergy/10, 'f',1) + "J");
			qDebug() << "Last cycle's energy:" << dispEnergy << "J.";
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

			//Average energy:
			energySum = 0;
			for(int i = 0; i < GAIT_ENERGY_LOG_PTS; i++)
			{
				energySum += gaitEnergyLog[i];
			}
			dispEnergy = energySum / GAIT_ENERGY_LOG_PTS;
			ui->label_Joules->setText(QString::number((float)dispEnergy/10, 'f',1) + "J");
			qDebug() << "Last cycle's energy:" << dispEnergy << "J.";
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
		if((*comboVar[i])->currentIndex() == 0){pts[i] = QPointF(0,0);}
	}
}

//This links rigid1 (or other) values to points
void W_AnkleAnglePlot::mapSensorsToPoints(int idx)
{
	struct rigid_s *ri = &rigid1;

	if(ui->comboBoxLeg->currentIndex() == 1){ri = &rigid2;}

	pts[0] = QPointF(idx, vtpToInt(0));
	pts[1] = QPointF(idx, vtpToInt(1));
	pts[2] = QPointF(idx, vtpToInt(2));
	pts[3] = QPointF(idx, vtpToInt(3));
	pts[4] = QPointF(idx, vtpToInt(4));
	pts[5] = QPointF(idx, vtpToInt(5));

	//Latch step energy:
	instantStepEnergy = ri->ctrl.step_energy;
}

int W_AnkleAnglePlot::vtpToInt(uint8_t row)
{
	int val;

	if(!vtp[row].used) return 0;

	if((vtp[row].rawGenPtr) == nullptr)
	{
		val = 0;
	}
	else
	{
		switch(vtp[row].format)
		{
			case FORMAT_32S:
				val = (*(int32_t*)vtp[row].rawGenPtr);
				break;
			case FORMAT_32U:
				val = (int)(*(uint32_t*)vtp[row].rawGenPtr);
				break;
			case FORMAT_16S:
				val = (int)(*(int16_t*)vtp[row].rawGenPtr);
				break;
			case FORMAT_16U:
				val = (int)(*(uint16_t*)vtp[row].rawGenPtr);
				break;
			case FORMAT_8S:
				val = (int)(*(int8_t*)vtp[row].rawGenPtr);
				break;
			case FORMAT_8U:
				val = (int)(*(uint8_t*)vtp[row].rawGenPtr);
				break;
			default:
				val = 0;
				break;
		}
	}

	return val;

}

//Assigns a pointer to the desired variable. This function is called whenever
//we change Slave or Variable. The runtime plotting function will then use the
//pointer.
void W_AnkleAnglePlot::assignVariable(uint8_t item)
{
	varIndex[item] = (*comboVar[item])->currentIndex();
	struct std_variable varHandle = selectedDevList[item]->getSerializedVar(varIndex[item] + 1);

	if(varIndex[item] == 0)
	{
		vtp[item].used = false;
		vtp[item].format = NULL_PTR;
		vtp[item].rawGenPtr = nullptr;
		vtp[item].decodedPtr = nullptr;
	}
	else
	{
		vtp[item].used = true;
		vtp[item].format = varHandle.format;
		vtp[item].rawGenPtr = varHandle.rawGenPtr;
		vtp[item].decodedPtr = varHandle.decodedPtr;
	}

	//if(allChannelUnused()) drawingTimer->stop();
	//else if(!drawingTimer->isActive()) drawingTimer->start();
}

//This generates fake data and maps it to points
void W_AnkleAnglePlot::fakeDataToPoints(int idx)
{
	static int trig[2] = {0,0};
	static int latchP = 0;
	int lim = 0;
	int p = 0;
	static bool k = false;

	if(ui->comboBoxLeg->currentIndex() == 0){lim = 1000;}
	else{lim = 1500;}

	if(idx > lim && idx < lim+100){trig[0] = 100;}
	else{trig[0] = 0;}
	if(idx > rollover/2) {trig[1] = 110;}
	else{trig[1] = 0;}

	if(idx < rollover/2){p = idx; latchP = idx;}
	else{p = latchP;}
	p = p/33;
	p = pow(p, 2);

	pts[0] = QPointF(idx, idx);
	pts[1] = QPointF(idx, trig[0]);
	pts[2] = QPointF(idx, trig[1]);
	pts[3] = QPointF(idx, p);
	if(!k){pts[4] = QPointF(idx, 50); k = true;}
	else{pts[4] = QPointF(idx, -50); k = false;}
	pts[5] = QPointF(idx, 20);

	//Latch step energy:
	instantStepEnergy = p;
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
	if(success) xmin = xMinText.toInt(&success);
	if(success) xmax = xMaxText.toInt(&success);
	if(success) ymin = yMinText.toInt(&success);
	if(success) ymax = yMaxText.toInt(&success);

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

void W_AnkleAnglePlot::initPtr(void)
{
	comboVar[0] = &ui->cBoxvar1;
	comboVar[1] = &ui->cBoxvar2;
	comboVar[2] = &ui->cBoxvar3;
	comboVar[3] = &ui->cBoxvar4;
	comboVar[4] = &ui->cBoxvar5;
	comboVar[5] = &ui->cBoxvar6;

	for(int i = 0; i < A2PLOT_VAR_NUM; i++)
	{
		//ToDo use dropdown
		if(ui->comboBoxLeg->currentIndex() == 0)
			selectedDevList[i] = (*currentDevList)[13];
		else
			selectedDevList[i] = (*currentDevList)[14];
	}
}

//Each board type has a different variable list.
void W_AnkleAnglePlot::updateVarList(uint8_t item)
{
	//Fill the comboBox:
	(*comboVar[item])->clear();
	(*comboVar[item])->setToolTipDuration(350);

	QStringList headerList(selectedDevList[item]->getHeader());

	(*comboVar[item])->addItem("**Unused**");
	(*comboVar[item])->setItemData(0, "Unused", Qt::ToolTipRole);
	for(int i = 2; i < headerList.length(); i++)
	{
		(*comboVar[item])->addItem(headerList[i]);
	}
}

//Variable comboBoxes:

void W_AnkleAnglePlot::on_cBoxvar1_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	if(initFlag == false)
	{
		//saveCurrentSettings(0);
		assignVariable(0);
	}
}

void W_AnkleAnglePlot::on_cBoxvar2_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	if(initFlag == false)
	{
		//saveCurrentSettings(1);
		assignVariable(1);
	}
}

void W_AnkleAnglePlot::on_cBoxvar3_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	if(initFlag == false)
	{
		//saveCurrentSettings(2);
		assignVariable(2);
	}
}

void W_AnkleAnglePlot::on_cBoxvar4_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	if(initFlag == false)
	{
		//saveCurrentSettings(3);
		assignVariable(3);
	}
}

void W_AnkleAnglePlot::on_cBoxvar5_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	if(initFlag == false)
	{
		//saveCurrentSettings(4);
		assignVariable(4);
	}
}

void W_AnkleAnglePlot::on_cBoxvar6_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	if(initFlag == false)
	{
		//saveCurrentSettings(5);
		assignVariable(5);
	}
}

void W_AnkleAnglePlot::on_comboBoxLeg_currentIndexChanged(int index)
{
	int i = 0, idx = 0;;

	if(initFlag == false)
	{
		if(ui->comboBoxLeg->currentIndex() == 0){idx = 13;}
		else{idx = 14;}

		for(i = 0; i < A2PLOT_VAR_NUM; i++)
		{
			selectedDevList[i] = (*currentDevList)[idx];
			assignVariable(i);
		}

		qDebug() << "Changed leg";
	}
}

void W_AnkleAnglePlot::on_pushButtonPreset0_clicked(){preset(0);}
void W_AnkleAnglePlot::on_pushButtonPreset1_clicked(){preset(1);}
void W_AnkleAnglePlot::on_pushButtonPreset2_clicked(){preset(2);}
void W_AnkleAnglePlot::on_pushButtonPreset3_clicked(){preset(3);}
void W_AnkleAnglePlot::on_pushButtonPreset4_clicked(){preset(4);}

void W_AnkleAnglePlot::preset(uint8_t p)
{
	qDebug() << "Applying preset #" << p;

	for(int item = 0; item < A2PLOT_VAR_NUM; item++)
	{
		(*comboVar[item])->setCurrentIndex(presetVariables[p][item]);
	}
}
