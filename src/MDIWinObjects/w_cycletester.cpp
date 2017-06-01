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

	//Buttons:
	resetPBstate = true;
	ui->pushButtonReset->setText("Reset");
	streamingPBstate = true;
	ui->pushButtonStartStreaming->setText("Start Streaming");
	ui->pushButtonConfirmReset->setDisabled(true);
}

void W_CycleTester::experimentControl(enum expCtrl e)
{
	switch(e)
	{
		case INIT:
			qDebug() << "Init.";
			break;
		case START:
			qDebug() << "Start.";
			break;
		case STOP:
			qDebug() << "Stop.";
			break;
		default:
			break;
	}
}

void W_CycleTester::experimentStats(enum expStats e)
{
	switch(e)
	{
		case READ:
			qDebug() << "Read.";
			break;
		case START_STREAMING:
			streamingPBstate = streamingPBstate ? false : true;
			if(streamingPBstate == true)
			{
				//User clicked on Stop:
				qDebug() << "Stop streaming.";
				ui->pushButtonStartStreaming->setText("Start Streaming");
				ui->lcdNumber->setEnabled(false);
				ui->progressBar->setEnabled(false);
				ui->pushButtonRead->setDisabled(false);
				ui->pushButtonReset->setDisabled(false);
			}
			else
			{
				//User clicked on Start:
				qDebug() << "Start streaming.";
				ui->pushButtonStartStreaming->setText("Stop Streaming");
				ui->lcdNumber->setEnabled(true);
				ui->progressBar->setEnabled(true);
				ui->pushButtonRead->setDisabled(true);
				ui->pushButtonReset->setDisabled(true);
			}
			break;
		case RESET:
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
		case CONFIRM_RESET:
			qDebug() << "Confirm reset.";

			//Recursive call to reset state:
			experimentStats(RESET);
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
	experimentControl(INIT);
}

void W_CycleTester::on_pushButtonStart_clicked()
{
	experimentControl(START);
}

void W_CycleTester::on_pushButtonStop_clicked()
{
	experimentControl(STOP);
}

void W_CycleTester::on_pushButtonRead_clicked()
{
	experimentStats(READ);
}

void W_CycleTester::on_pushButtonStartStreaming_clicked()
{
	experimentStats(START_STREAMING);
}

void W_CycleTester::on_pushButtonReset_clicked()
{
	experimentStats(RESET);
}

void W_CycleTester::on_pushButtonConfirmReset_clicked()
{
	experimentStats(CONFIRM_RESET);
}
