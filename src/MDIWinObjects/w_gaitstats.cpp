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
	[This file] w_gaitstats.cpp: Gaits Statistics
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2018-02-10 | sbelanger | New code, initial release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <flexsea_system.h>
#include <flexsea_buffers.h>
#include <flexsea_comm.h>
#include "w_gaitstats.h"
#include "ui_w_gaitstats.h"
#include "flexsea_generic.h"
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <flexsea_board.h>
#include <cmd-GaitStats.h>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_GaitStats::W_GaitStats(QWidget *parent, DynamicUserDataManager* userDataManager) :
	QWidget(parent),
	ui(new Ui::W_GaitStats),
	userDataMan(userDataManager)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	init();
}

W_GaitStats::~W_GaitStats()
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

void W_GaitStats::init(void)
{
	ui->le_number->setValidator(new QIntValidator(0, 100, this));

//	//Populates Slave list:
//	FlexSEA_Generic::populateSlaveComboBox(ui->comboBox_slave, SL_BASE_ALL, \
//											SL_LEN_ALL);
//	ui->comboBox_slave->setCurrentIndex(0);	//Execute 1 by default

	//Variables:
	active_slave_index = 0;	//ui->comboBox_slave->currentIndex();
	active_slave = FlexSEA_Generic::getSlaveID(SL_BASE_MN, active_slave_index);

	//Timer used to refresh the received data:
	refreshDelayTimer = new QTimer(this);
	connect(refreshDelayTimer,	&QTimer::timeout,
			this,				&W_GaitStats::refreshDisplay);

	//Timer used to read after we clear a row:
	readAfterClearTimer = new QTimer(this);
	connect(readAfterClearTimer,	&QTimer::timeout,
			this,				&W_GaitStats::readFromSlave);

	//Auto refresh:
	autoRefreshTimer = new QTimer(this);
	connect(autoRefreshTimer,	&QTimer::timeout,
			this,				&W_GaitStats::autoRefresh);
	autoRefreshTimer->start(1000);

	ui->checkBoxAutoRefresh->setChecked(false);

	//Text style:
	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	font.setPointSize(10);
	ui->te_code->setFont(font);

	//Populate the text box with zeros:
	defaultText(3, 10);
}

void W_GaitStats::defaultText(int rows, int columns)
{
	QString fullText = "", row = "", head = "";
	sprintHeader(&head, columns);
	fullText.append(head);

	memset(defaultArray, 0, MAX_COLUMNS);

	for(int i = 0; i < rows; i++)
	{
		sprintLine(i, &row, defaultArray, columns);
		fullText.append(row);
		fullText.append("\n\n");
	}

	ui->te_code->setPlainText(fullText);
}

void W_GaitStats::realText(int rows, int columns)
{
	QString fullText = "", row = "", head = "";
	sprintHeader(&head, columns);
	fullText.append(head);

	for(int i = 0; i < rows; i++)
	{
		sprintLine(i, &row, gaitStats.n[i], columns);
		fullText.append(row);
		fullText.append("\n\n");
	}

	ui->te_code->setPlainText(fullText);
}

void W_GaitStats::sprintHeader(QString *headTxt, int columns)
{
	QString headerText = "", dashes = "";
	for(int i = 0; i < columns; i++){defaultArray[i] = i;}
	sprintLine(-1, &headerText, defaultArray, columns);
	headerText.append("\n");
	for(int i = 0; i < headerText.length(); i++)
	{
		dashes.append('-');
	}

	headerText.append(dashes);
	headerText.append("\n\n");

	*headTxt = headerText;
}

void W_GaitStats::sprintLine(int8_t num, QString *txt, uint8_t *arr, uint8_t len)
{
	QString myTxt = "", dataPoint = "";
	if(num >= 0){myTxt.sprintf("[%i] ", num);}
	else{myTxt.sprintf("[ ] ");}

	for(int i = 0; i < len-1; i++)
	{
		dataPoint.sprintf("%4i |", arr[i]);
		myTxt.append(dataPoint);
	}
	dataPoint.sprintf("%4i", arr[len-1]);
	myTxt.append(dataPoint);

	*txt = myTxt;
}

//Send a Read command:
void W_GaitStats::readFromSlave(void)
{
	uint8_t info[2] = {PORT_USB, PORT_USB};
	uint16_t numb = 0;
	uint8_t offset= 0;

	readAfterClearTimer->stop();

	//Prepare and send command:
	tx_cmd_gait_stats_r(TX_N_DEFAULT, offset);
	pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, READ);

	//Display will be refreshed in 75ms:
	refreshDelayTimer->start(75);
}

//Send a Read command:
void W_GaitStats::writeToSlave(int rowToClear)
{
	uint8_t info[2] = {PORT_USB, PORT_USB};
	uint16_t numb = 0;
	uint8_t offset = 0;
	uint8_t cmd = 0;

	cmd = (uint8_t)(rowToClear | RESET_ROW_MASK);

	//Prepare and send command:
	tx_cmd_gait_stats_w(TX_N_DEFAULT, offset, cmd);
	pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, READ);

	readAfterClearTimer->start(50);
}

void W_GaitStats::receiveNewData()
{

}

void W_GaitStats::autoRefresh(void)
{
	if(ui->checkBoxAutoRefresh->isChecked())
	{
		readFromSlave();
		qDebug() << "Auto-refreshed!";
	}
}

void W_GaitStats::comStatusChanged(SerialPortStatus status, int nbTries)
{
	(void)nbTries;	// Not use by this slot.

	if(status == PortOpeningSucceed)
	{
		userDataMan->requestMetaData(active_slave);
		comOpen = true;
	}
	else if(status == PortClosed)
	{
		comOpen = false;
	}
}

//Refreshes the text box values (display only):
void W_GaitStats::refreshDisplay(void)
{
	refreshDelayTimer->stop();
	qDebug() << "Refresh display.";
	realText(GS_ROWS, GS_COLUMNS);
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_GaitStats::on_pb_ClearRow_clicked()
{
	int rowToClear = 0;
	if(ui->le_number->text().length() != 0)
	{
		rowToClear = ui->le_number->text().toInt();
		qDebug() << "Row to clear:" << rowToClear;
		ui->le_number->clear();

		writeToSlave(rowToClear);
	}
	else
	{
		qDebug() << "No row specified; nothing to do.";
	}
}

void W_GaitStats::on_pb_Refresh_clicked()
{
	readFromSlave();
}
