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
	[This file] w_config.h: Configuration Window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_config.h"
#include "ui_w_config.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QTimer>
#include <QThread>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_Config::W_Config(QWidget *parent) :
	QWidget(parent),
	serialDriver(nullptr),
	ui(new Ui::W_Config)
{

	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	//Init code:
	dataSourceState = None;
	initCom();

	comPortRefreshTimer = new QTimer(this);
	connect(comPortRefreshTimer, SIGNAL(timeout()), this, SLOT(getComList()));
	comPortRefreshTimer->start(REFRESH_PERIOD); //ms
	getComList();	//Call now to avoid lag when a new window is opened.
}

W_Config::~W_Config()
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

void W_Config::setComProgress(int val)
{
	ui->comProgressBar->setValue(val);
}

//****************************************************************************
// Private function(s):
//****************************************************************************

void W_Config::initCom(void)
{
	//Bluetooth:
	ui->pbBTmode->setEnabled(false);
	btDataMode = true;
	ui->pbBTmode->setText("Set mode: Cmd");
	disableBluetoothCommandButtons();

	//No manual entry, 0% progress, etc.:
	ui->comProgressBar->setValue(0);
	ui->openComButton->setDisabled(false);
	ui->closeComButton->setDisabled(true);
	ui->pbLoadLogFile->setDisabled(false);
	ui->pbCloseLogFile->setDisabled(true);
}

//This gets called by a timer (currently every 750ms)
/*Note: the list is always ordered by port number. If you connect to COM2 and
 * then plug COM1, it will display COM1. That's confusing for the users.*/
void W_Config::getComList(void)
{
	static int lastComPortCounts = 0;
	int ComPortCounts = 0;

	//Available ports?
	QList<QSerialPortInfo> comPortInfo = QSerialPortInfo::availablePorts();
	ComPortCounts = comPortInfo.length();

	//Did it change?
	if(ComPortCounts != lastComPortCounts)
	{
		//Yes.
		qDebug() << "COM Port list changed.";

		ui->comPortComboBox->clear();

		//No port?
		if(ComPortCounts == 0)
		{
			//Empty, add the No Port option
			ui->comPortComboBox->addItem("No Port");
		}
		else
		{
			//Rewrite the list:
			for(const QSerialPortInfo &info : comPortInfo)
			{
				ui->comPortComboBox->addItem(info.portName());
			}
		}
	}
	lastComPortCounts = ComPortCounts;
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_Config::on_openComButton_clicked()
{
	bool success = false;

	//Stop port refresh
	comPortRefreshTimer->stop();
	//Emit signal:
	emit openCom(ui->comPortComboBox->currentText(), 25, 100000, &success);

	//Connection is successful.
	if(success)
	{
		dataSourceState = LiveCOM;
		emit updateDataSourceStatus(dataSourceState, nullptr);

		ui->openComButton->setDisabled(true);
		ui->closeComButton->setDisabled(false);
		ui->comPortComboBox->setDisabled(true);

		ui->pbLoadLogFile->setDisabled(true);
		//ui->pushButtonBTCon->setDisabled(true);

		//Enable Bluetooth button:
		ui->pbBTmode->setEnabled(true);
	}
	else
	{
		comPortRefreshTimer->start(REFRESH_PERIOD);
	}
}

void W_Config::on_closeComButton_clicked()
{
	//Emit signal:
	emit closeCom();

	//Enable Open COM button:
	ui->openComButton->setDisabled(false);
	ui->closeComButton->setDisabled(true);
	ui->comProgressBar->setValue(0);
	ui->comPortComboBox->setDisabled(false);

	ui->pbLoadLogFile->setDisabled(false);
	//ui->pushButtonBTCon->setDisabled(false);

	dataSourceState = None;
	emit updateDataSourceStatus(dataSourceState, nullptr);

	// Avoid refresh lag
	getComList();
	// Restart the auto-Refresh
	comPortRefreshTimer->start(REFRESH_PERIOD);

	//Disable Bluetooth button:
	ui->pbBTmode->setEnabled(false);
}

void W_Config::on_pbLoadLogFile_clicked()
{
	bool isOpen;
	FlexseaDevice *devPtr;
	emit openReadingFile(&isOpen, &devPtr);

	if(isOpen)
	{
		ui->pbLoadLogFile->setDisabled(true);
		ui->pbCloseLogFile->setDisabled(false);
		ui->openComButton->setDisabled(true);
		//ui->pushButtonBTCon->setDisabled(true);
		dataSourceState = FromLogFile;
		emit updateDataSourceStatus(dataSourceState, devPtr);
		emit createLogKeypad(dataSourceState, devPtr);
	}
}

void W_Config::on_pbCloseLogFile_clicked()
{
	emit closeReadingFile();
	ui->pbLoadLogFile->setDisabled(false);
	ui->pbCloseLogFile->setDisabled(true);
	ui->openComButton->setDisabled(false);
	//ui->pushButtonBTCon->setDisabled(false);
	dataSourceState = None;
	emit updateDataSourceStatus(dataSourceState, nullptr);
}

void W_Config::on_pbBTmode_clicked()
{
	uint8_t config[4] = {0,0,0,0};

	if(btDataMode == false)
	{
		btDataMode = true;
		ui->pbBTmode->setText("Set mode: Cmd");
		config[0] = '-';
		config[1] = '-';
		config[2] = '-';
		config[3] = '\n';
		//writeCommand(4, config, 0);
		serialDriver->write(4, config);
		//We are now in Data mode:
		disableBluetoothCommandButtons();
	}
	else
	{
		btDataMode = false;
		ui->pbBTmode->setText("Set mode: Data");
		config[0] = '$';
		config[1] = '$';
		config[2] = '$';
		//writeCommand(3,config, 0);
		serialDriver->write(3, config);
		//We are now in CMD mode:
		enableBluetoothCommandButtons();
	}
}

void W_Config::on_pbBTdefault_clicked()
{
	uint32_t delay = 2000;
	//Settings:
	//Baudrate = 230k
	//Name = FlexSEA-ADDR
	//TX Power = max
	//Inquiry window = 0012
	//Page Window = 0012
	//CfgTimer = 10s

	/*
	uint8_t config[6][20] = {{"S-,FlexSEA\\0"}, \
							 {"SU,230K\n\0"}, \
							 {"SY,0100\n"}, \
							 {"SI,0012\n"}, \
							 {"SJ,0012\n"}};
							 */

	//uint8_t config1[13] = {"S-,FlexSEA\0"};

	uint8_t config2[15] = "SY,0100";
	config2[7] = 13;
	//serialDriver->clear();
	serialDriver->write(8, config2);
	qDebug() << "SY,0100";
	serialDriver->flush();
	serialDriver->clear();
	QThread::msleep(delay);

	uint8_t config4[15] = "SJ,0012";
	config4[7] = 13;
	serialDriver->write(8, config4);
	qDebug() << "SJ,0012";
	serialDriver->flush();
	QThread::msleep(delay);

	uint8_t config[15] = "SU,115K";
	config[7] = 13;
	serialDriver->write(8, config);
	qDebug() << "SU,115K";
	serialDriver->flush();
	serialDriver->clear();
	QThread::msleep(delay);

	uint8_t config3[15] = "SI,0012";
	config3[7] = 13;
	serialDriver->write(8, config3);
	qDebug() << "SI,0012";
	serialDriver->flush();
	serialDriver->clear();
	QThread::msleep(delay);
}

void W_Config::on_pbBTfactory_clicked()
{
	uint8_t config[6] = "SF,1\n";
	//writeCommand(5,config, 0);
	serialDriver->write(5, config);
	serialDriver->flush();
}

void W_Config::on_pbBTreset_clicked()
{
	uint8_t config[5] = "R,1\n";
	//writeCommand(4,config, 0);
	serialDriver->write(4, config);
	serialDriver->flush();
}

void W_Config::enableBluetoothCommandButtons(void)
{
	ui->pbBTfactory->setEnabled(true);
	ui->pbBTdefault->setEnabled(true);
	ui->pbBTreset->setEnabled(true);
}

void W_Config::disableBluetoothCommandButtons(void)
{
	ui->pbBTfactory->setEnabled(false);
	ui->pbBTdefault->setEnabled(false);
	ui->pbBTreset->setEnabled(false);
}
