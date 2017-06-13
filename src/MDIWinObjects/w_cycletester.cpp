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
	//Displays:
	ui->progressBar->setValue(0);
	ui->progressBar->setDisabled(true);
	ui->lcdNumber->display(0);
	ui->lcdNumber->setDisabled(true);
	ui->lcdNumberNV->setDisabled(true);
	displayStatus(0);

	//Buttons:
	resetPBstate = true;
	ui->pushButtonReset->setText("Reset");
	streamingPBstate = true;
	ui->pushButtonStartStreaming->setText("Start Streaming");
	ui->pushButtonConfirmReset->setDisabled(true);

	//Timer:
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
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
	tx_cmd_cycle_tester_w(TX_N_DEFAULT, 0, act1, CT_S_DEFAULT);
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
	displayStatus(ctStats_errorMsg);

	//Prep & send:
	tx_cmd_cycle_tester_r(TX_N_DEFAULT, 0);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

void W_CycleTester::resetStats(void)
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Prep & send:
	tx_cmd_cycle_tester_w(TX_N_DEFAULT, 0, 0, CT_S_CONFIRM_RESET);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

void W_CycleTester::displayStatus(uint8_t s)
{
	uint8_t errCnt = 0;
	uint8_t mem = 0, tempW = 0, temp = 0, motion = 0;
	QString txt = "Ok";
	QString style = "background-color: rgb(128, 128, 128); color: rgb(0, 0, 0)";

	//Decode flags:
	mem = s & CT_ERR_MEM;
	tempW = s & CT_ERR_TEMP_WARN;
	temp = s & CT_ERR_TEMP;
	motion = s & CT_ERR_MOTION;

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
