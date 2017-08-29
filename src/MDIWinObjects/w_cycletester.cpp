/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'plan-gui' Graphical User Interface
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Origin] Based on Jean-Francois Duval's work at the MIT Media Lab
	Biomechatronics research group <http://biomech.media.mit.edu/>
	[Contributors]
*****************************************************************************
	[This file] w_control.cpp: Cycle Tester Window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-05-26 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************


#include "w_cycletester.h"
#include "ui_w_cycletester.h"
#include <QTimer>
#include <QDebug>
#include "cmd-CycleTester.h"
#include <flexsea_system.h>
#include <flexsea_comm.h>
#include "flexsea_board.h"

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_CycleTester::W_CycleTester(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::W_CycleTester)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	init();
}

W_CycleTester::~W_CycleTester()
{
	emit windowClosed();
	delete ui;
}

//****************************************************************************
// Public function(s):
//****************************************************************************

//****************************************************************************
// Public slot(s):
//****************************************************************************

//****************************************************************************
// Private function(s):
//****************************************************************************

void W_CycleTester::init(void)
{
	//Tabs:
	initCtrlTab();
	initProfileTab();

	//Always start with the Ctrl tab:
	ui->tabWidget->setCurrentIndex(0);

	//Timers:
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
	buttonTimer = new QTimer(this);
	connect(buttonTimer, SIGNAL(timeout()), this, SLOT(buttonTimerEvent()));
}

void W_CycleTester::initCtrlTab(void)
{
	//Displays:
	ui->progressBar->setValue(0);
	ui->progressBar->setDisabled(true);
	ui->lcdNumber->display(0);
	ui->lcdNumber->setDisabled(true);
	ui->lcdNumberNV->setDisabled(true);
	ui->progressBarTemp->setValue(15);
	ui->progressBarTemp->setDisabled(true);
	ui->label_temp->setText("-°C");
	ui->label_peakCurrent->setText("-");
	displayStatus(0);

	//Buttons:
	resetPBstate = true;
	ui->pushButtonReset->setText("Reset");
	streamingPBstate = true;
	ui->pushButtonStartStreaming->setText("Start Streaming");
	ui->pushButtonConfirmReset->setDisabled(true);
}

void W_CycleTester::initProfileTab(void)
{
	const QValidator *validT = new QIntValidator(0, 10000, this);
	const QValidator *validI = new QIntValidator(0, 65535, this);

	//Data:

	//Displays:
	ui->label_piu_t0->setText("-");
	ui->label_piu_y0->setText("-");
	ui->label_piu_t1->setText("-");
	ui->label_piu_y1->setText("-");
	ui->label_piu_t2->setText("-");
	ui->label_piu_y2->setText("-");
	ui->label_piu_t3->setText("-");
	ui->label_piu_y3->setText("-");
	ui->label_piu_t4->setText("-");
	ui->label_piu_y4->setText("-");
	ui->label_piu_vCurr->setText("-");
	ui->label_piu_pCurr->setText("-");

	ui->lineEdit_np_t0->setText("");
	ui->lineEdit_np_t0->setValidator(validT);
	ui->label_np_y0->setText("-");
	ui->lineEdit_np_t1->setText("");
	ui->lineEdit_np_t1->setValidator(validT);
	ui->label_np_y1->setText("-");
	ui->lineEdit_np_t2->setText("");
	ui->lineEdit_np_t2->setValidator(validT);
	ui->label_np_y2->setText("-");
	ui->lineEdit_np_t3->setText("");
	ui->lineEdit_np_t3->setValidator(validT);
	ui->label_np_y3->setText("-");
	ui->lineEdit_np_t4->setText("");
	ui->lineEdit_np_t4->setValidator(validT);
	ui->label_np_y4->setText("-");
	ui->lineEdit_np_vCurr->setText("");
	ui->lineEdit_np_vCurr->setValidator(validI);
	ui->lineEdit_np_pCurr->setText("");
	ui->lineEdit_np_pCurr->setValidator(validI);
}

void W_CycleTester::experimentControl(enum expCtrl e)
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};
	uint8_t act1 = e;

	//ToDo, or remove
	switch(e)
	{
		case CT_C_INIT:

			break;
		case CT_C_START:

			break;
		case CT_C_PAUSE:

			break;
		case CT_C_STOP:

			break;
		default:
			break;
	}

	//Prep & send:
	tx_cmd_cycle_tester_w(TX_N_DEFAULT, 0, act1, CT_S_DEFAULT, 0);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

void W_CycleTester::experimentStats(enum expStats e)
{
	switch(e)
	{
		case CT_S_READ:
			qDebug() << "Read.";
			timerEvent();
			break;
		case CT_S_START_STREAMING:
			streamingPBstate = streamingPBstate ? false : true;
			if(streamingPBstate == true)
			{
				//User clicked on Stop:
				qDebug() << "Stop streaming.";
				ui->pushButtonStartStreaming->setText("Start Streaming");
				ui->lcdNumber->setEnabled(false);
				ui->lcdNumberNV->setEnabled(false);
				ui->progressBar->setEnabled(false);
				ui->pushButtonRead->setDisabled(false);
				ui->pushButtonReset->setDisabled(false);

				//Stop timer:
				timer->stop();
			}
			else
			{
				//User clicked on Start:
				qDebug() << "Start streaming.";
				ui->pushButtonStartStreaming->setText("Stop Streaming");
				ui->lcdNumber->setEnabled(true);
				ui->lcdNumberNV->setEnabled(true);
				ui->progressBar->setEnabled(true);
				ui->pushButtonRead->setDisabled(true);
				ui->pushButtonReset->setDisabled(true);

				//Start timer:
				timer->start(TIMER_PERIOD);
			}
			break;
		case CT_S_RESET:
			resetPBstate = resetPBstate ? false : true;
			if(resetPBstate == true)
			{
				//User clicked on Cancel:
				qDebug() << "Cancel.";
				ui->pushButtonReset->setText("Reset");
				ui->pushButtonConfirmReset->setDisabled(true);
				ui->pushButtonConfirmReset->setStyleSheet("");
			}
			else
			{
				//User clicked on Reset:
				qDebug() << "Reset.";
				ui->pushButtonReset->setText("Cancel");
				ui->pushButtonConfirmReset->setEnabled(true);
				ui->pushButtonConfirmReset->setStyleSheet(\
					"background-color: rgb(255, 0, 0); color: rgb(0, 0, 0)");
			}
			break;
		case CT_S_CONFIRM_RESET:
			qDebug() << "Confirm reset.";
			resetStats();

			//Recursive call to reset state:
			experimentStats(CT_S_RESET);
			break;
		default:
			break;
	}
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_CycleTester::on_pushButtonInit_clicked()
{
	experimentControl(CT_C_INIT);
}

void W_CycleTester::on_pushButtonStart_clicked()
{
	experimentControl(CT_C_START);
}

void W_CycleTester::on_pushButtonPause_clicked()
{
	experimentControl(CT_C_PAUSE);
}

void W_CycleTester::on_pushButtonStop_clicked()
{
	experimentControl(CT_C_STOP);
}

void W_CycleTester::on_pushButtonRead_clicked()
{
	experimentStats(CT_S_READ);
}

void W_CycleTester::on_pushButtonStartStreaming_clicked()
{
	experimentStats(CT_S_START_STREAMING);
}

void W_CycleTester::on_pushButtonReset_clicked()
{
	experimentStats(CT_S_RESET);
}

void W_CycleTester::on_pushButtonConfirmReset_clicked()
{
	experimentStats(CT_S_CONFIRM_RESET);
}

void W_CycleTester::timerEvent(void)
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Refresh display with last values:
	ui->progressBar->setValue(ctStats_pct);
	ui->lcdNumber->display((double)cyclesVolatile);
	ui->lcdNumberNV->display((double)cyclesNonVolatile);
	ui->label_peakCurrent->setText(QString::number(peakCurrent));
	ui->label_valleyCurrent->setText(QString::number(valleyCurrent));
	ui->label_mod1->setText(QString::number(ctStats_mod1));
	ui->label_mod2->setText(QString::number(ctStats_mod2));
	displayStatus(ctStats_errorMsg);
	displayTemp(ctStats_temp);
	ui->label_FSM_State->setText(displayFSMstate(ctStats_fsm1State));
	ui->label_cyclePower->setText(QString::number((float)cyclePower/1000));
	ui->label_peakPower->setText(QString::number((float)peakPower/10));
	ui->label_instantPower->setText(QString::number((float)instantPower/10));

	//We only stream when the Controls & Stats tab is selected:
	if(ui->tabWidget->currentIndex() == 0)
	{
		//Prep & send:
		tx_cmd_cycle_tester_r(TX_N_DEFAULT, 0);
		pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
		emit writeCommand(numb, comm_str_usb, WRITE);
	}
}

void W_CycleTester::buttonTimerEvent(void)
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	buttonTimer->stop();
	if(ui->pushButtonStatus->isDown())
	{
		//User held the button down long enough
		qDebug() << "Time to reset the CT errors.";
	}
	else
	{
		qDebug() << "False alert - nothing to do.";
		return;
	}

	//Prep & send:
	tx_cmd_cycle_tester_w(TX_N_DEFAULT, 0, 0, CT_S_DEFAULT, 1);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

void W_CycleTester::resetStats(void)
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Prep & send:
	tx_cmd_cycle_tester_w(TX_N_DEFAULT, 0, 0, CT_S_CONFIRM_RESET, 0);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

void W_CycleTester::displayStatus(uint8_t s)
{
	uint8_t errCnt = 0;
	uint8_t mem = 0, tempW = 0, temp = 0, motion = 0, stuck = 0, cord = 0;
	QString txt = "Ok";
	QString style = "background-color: rgb(128, 128, 128); color: rgb(0, 0, 0)";

	//Decode flags:
	mem = s & CT_ERR_MEM;
	tempW = s & CT_ERR_TEMP_WARN;
	temp = s & CT_ERR_TEMP;
	motion = s & CT_ERR_MOTION;
	stuck = s & CT_ERR_STUCK;
	cord = s & CT_ERR_CORD;

	//Sequential assignement - priority comes from order:
	if(mem)
	{
		txt = "Mem";
		style = "background-color: rgb(255, 140, 0); color: rgb(0, 0, 0)";
		errCnt++;
	}
	if(tempW)
	{
		txt = "Temp";
		style = "background-color: rgb(255, 140, 0); color: rgb(0, 0, 0)";
		errCnt++;
	}
	if(temp)
	{
		txt = "Temp";
		style = "background-color: rgb(255, 0, 0); color: rgb(0, 0, 0)";
		errCnt++;
	}
	if(motion)
	{
		txt = "Motion";
		style = "background-color: rgb(255, 0, 0); color: rgb(0, 0, 0)";
		errCnt++;
	}
	if(stuck)
	{
		txt = "Stuck";
		style = "background-color: rgb(255, 0, 0); color: rgb(0, 0, 0)";
		errCnt++;
	}
	if(cord)
	{
		txt = "Cord";
		style = "background-color: rgb(255, 0, 0); color: rgb(0, 0, 0)";
		errCnt++;
	}

	//More than 1?
	if(errCnt > 1)
	{
		txt = ">1 Err";
		style = "background-color: rgb(255, 0, 0); color: rgb(0, 0, 0)";
	}

	//Apply to indicator:
	ui->pushButtonStatus->setText(txt);
	ui->pushButtonStatus->setStyleSheet(style);
}

//Return a QString version of the enum text
QString W_CycleTester::displayFSMstate(uint8_t s)
{
	QString tmp;

	switch(s)
	{
		case 0:
			tmp = "CT_FSM1_BOOT";
			break;
		case 1:
			tmp = "CT_FSM1_INIT_CTRL0";
			break;
		case 2:
			tmp = "CT_FSM1_INIT_CTRL1";
			break;
		case 3:
			tmp = "CT_FSM1_INIT_ERROR";
			break;
		case 4:
			tmp = "CT_FSM1_INIT_CTRL2";
			break;
		case 5:
			tmp = "CT_FSM1_CYCLE";
			break;
		case 6:
			tmp = "CT_FSM1_PAUSE";
			break;
		case 7:
			tmp = "CT_FSM1_STOP";
			break;
		case 8:
			tmp = "CT_FSM1_ERROR";
			break;
		default:
			tmp = "Unknown state.";
			break;
	}

	return tmp;
}

void W_CycleTester::displayTemp(int8_t t)
{
	int32_t r = 0, g = 0;
	uint8_t diff = 0;
	QString txt = "";
	QString styleStart = "QProgressBar::chunk {background: rgb(";
	QString styleMid = "";
	QString styleEnd = ", 0);}";
	QString style;

	//Limits:
	if(t < TEMP_MIN){t = TEMP_MIN;}
	if(t > TEMP_MAX){t = TEMP_MAX;}

	//Colors:
	if(t <= (TEMP_MIN + TEMP_SPAN/2))
	{
		diff = t - TEMP_MIN;
		r = (0xFF * 2* diff) / TEMP_SPAN;
		g = 0xFF;
	}
	else
	{
		diff = t - TEMP_MIN;
		r = 0xFF;
		g = 512 - ((0xFF * 2 * diff) / TEMP_SPAN);
	}

	if(r > 255){r = 255;}
	if(r < 0){r = 0;}
	if(g > 255){g = 255;}
	if(g < 0){g = 0;}

	//Label:
	txt = QString::number(t) + " °C";
	ui->label_temp->setText(txt);

	//Progress bar:
	ui->progressBarTemp->setValue(t);
	styleMid = QString::number(r) + ',' + QString::number(g);
	style = styleStart + styleMid + styleEnd;
	ui->progressBarTemp->setStyleSheet(style);
}

void W_CycleTester::on_pushButtonStatus_pressed()
{
	qDebug() << "Button pressed...";
	//Start timer:
	buttonTimer->setTimerType(Qt::CoarseTimer);
	buttonTimer->setSingleShot(true);
	buttonTimer->setInterval(1750);
	buttonTimer->start();
}

void W_CycleTester::on_pbRead_clicked()
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Prep & send:
	tx_cmd_cycle_tester_r(TX_N_DEFAULT, 1);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	//emit writeCommand(numb, comm_str_usb, WRITE);
	emit writeCommand(numb, comm_str_usb, READ);

	//Refresh display:
	refreshProfileDisplay();	//ToDo this should be done after a delay...
}

void W_CycleTester::on_pbCopy_clicked()
{
	//Copy data:
	for(int i = 0; i < 5; i++)
	{
		ctNewProfileYT[0][i] = ctProfileInUseYT[0][i];
		ctNewProfileYT[1][i] = ctProfileInUseYT[1][i];
	}
	ctNewProfileCurr[0] = ctProfileInUseCurr[0];
	ctNewProfileCurr[1] = ctProfileInUseCurr[1];

	//Refresh display:
	refreshProfileDisplay();
}

void W_CycleTester::refreshProfileDisplay(void)
{
	ui->label_piu_t0->setText(QString::number(ctProfileInUseYT[0][0]));
	ui->label_piu_y0->setText(QString::number(ctProfileInUseYT[1][0]));
	ui->label_piu_t1->setText(QString::number(ctProfileInUseYT[0][1]));
	ui->label_piu_y1->setText(QString::number(ctProfileInUseYT[1][1]));
	ui->label_piu_t2->setText(QString::number(ctProfileInUseYT[0][2]));
	ui->label_piu_y2->setText(QString::number(ctProfileInUseYT[1][2]));
	ui->label_piu_t3->setText(QString::number(ctProfileInUseYT[0][3]));
	ui->label_piu_y3->setText(QString::number(ctProfileInUseYT[1][3]));
	ui->label_piu_t4->setText(QString::number(ctProfileInUseYT[0][4]));
	ui->label_piu_y4->setText(QString::number(ctProfileInUseYT[1][4]));
	ui->label_piu_vCurr->setText(QString::number(ctProfileInUseCurr[0]));
	ui->label_piu_pCurr->setText(QString::number(ctProfileInUseCurr[1]));

	ui->lineEdit_np_t0->setText(QString::number(ctNewProfileYT[0][0]));
	ui->label_np_y0->setText(QString::number(ctNewProfileYT[1][0]));
	ui->lineEdit_np_t1->setText(QString::number(ctNewProfileYT[0][1]));
	ui->label_np_y1->setText(QString::number(ctNewProfileYT[1][1]));
	ui->lineEdit_np_t2->setText(QString::number(ctNewProfileYT[0][2]));
	ui->label_np_y2->setText(QString::number(ctNewProfileYT[1][2]));
	ui->lineEdit_np_t3->setText(QString::number(ctNewProfileYT[0][3]));
	ui->label_np_y3->setText(QString::number(ctNewProfileYT[1][3]));
	ui->lineEdit_np_t4->setText(QString::number(ctNewProfileYT[0][4]));
	ui->label_np_y4->setText(QString::number(ctNewProfileYT[1][4]));
	ui->lineEdit_np_vCurr->setText(QString::number(ctNewProfileCurr[0]));
	ui->lineEdit_np_pCurr->setText(QString::number(ctNewProfileCurr[1]));
}

void W_CycleTester::on_pbCompute_clicked()
{
	//Get numbers from lineEdits:
	ctNewProfileCurr[0] = ui->lineEdit_np_vCurr->text().toInt();
	ctNewProfileCurr[1] = ui->lineEdit_np_pCurr->text().toInt();
	ctNewProfileYT[0][0] = ui->lineEdit_np_t0->text().toInt();
	ctNewProfileYT[0][1] = ui->lineEdit_np_t1->text().toInt();
	ctNewProfileYT[0][2] = ui->lineEdit_np_t2->text().toInt();
	ctNewProfileYT[0][3] = ui->lineEdit_np_t3->text().toInt();
	ctNewProfileYT[0][4] = ui->lineEdit_np_t4->text().toInt();

	/*
	ctNewProfileYT[1][0] = ctNewProfileCurr[0];
	ctNewProfileYT[1][1] = ctNewProfileCurr[0];
	ctNewProfileYT[1][2] = ctNewProfileCurr[1];
	ctNewProfileYT[1][3] = ctNewProfileCurr[1];
	ctNewProfileYT[1][4] = ctNewProfileCurr[0];
	*/
	ctNewProfileYT[1][0] = ui->label_np_y0->text().toInt();
	ctNewProfileYT[1][1] = ui->label_np_y1->text().toInt();
	ctNewProfileYT[1][2] = ui->label_np_y2->text().toInt();
	ctNewProfileYT[1][3] = ui->label_np_y3->text().toInt();
	ctNewProfileYT[1][4] = ui->label_np_y4->text().toInt();

	//Refresh display:
	refreshProfileDisplay();
}

void W_CycleTester::on_pbWrite_clicked()
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Prep & send:
	tx_cmd_cycle_tester_w(TX_N_DEFAULT, 1, 0, 0, 0);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}
