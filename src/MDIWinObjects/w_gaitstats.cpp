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

//	//Variables:
//	active_slave_index = ui->comboBox_slave->currentIndex();
//	active_slave = FlexSEA_Generic::getSlaveID(SL_BASE_ALL, active_slave_index);

//	//Timer used to refresh the received data:
//	refreshDelayTimer = new QTimer(this);
//	connect(refreshDelayTimer,	&QTimer::timeout,
//			this,				&W_UserRW::refreshDisplay);

/*	QString myText = "Test2", otherStr = "";

	uint8_t arr[4] = {0,1,2,3};
	sprintLine(0, &myText, arr, 4);
	qDebug() << myText;

	ui->te_code->setPlainText(myText);
	*/

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

void W_GaitStats::sprintHeader(QString *headTxt, int columns)
{
	QString headerText = "", dashes = "";
	for(int i = 0; i < columns; i++){defaultArray[i] = i;}
	sprintLine(-1, &headerText, defaultArray, columns);
	headerText.append("\n\n");
	for(int i = 0; i < 0.85*headerText.length(); i++)
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
	else{myTxt.sprintf("[  ] ");}
	for(int i = 0; i < len-1; i++)
	{
		dataPoint.sprintf("%4i   |  ", arr[i]);
		myTxt.append(dataPoint);
	}
	dataPoint.sprintf("%4i", arr[len-1]);
	myTxt.append(dataPoint);

	*txt = myTxt;
}

//Send a Write command:
void W_GaitStats::writeUserData(uint8_t index)
{
	uint8_t info[2] = {PORT_USB, PORT_USB};
	uint16_t numb = 0;

//	//Refresh variable:
//	user_data_1.w[0] = (int16_t)ui->w0->text().toInt();
//	user_data_1.w[1] = (int16_t)ui->w1->text().toInt();
//	user_data_1.w[2] = (int16_t)ui->w2->text().toInt();
//	user_data_1.w[3] = (int16_t)ui->w3->text().toInt();

	//qDebug() << "Write user data" << index << ":" << user_data_1.w[index];

	//Prepare and send command:
	tx_cmd_data_user_w(TX_N_DEFAULT, index);
	pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

//Send a Read command:
void W_GaitStats::readUserData(void)
{
	uint8_t info[2] = {PORT_USB, PORT_USB};
	uint16_t numb = 0;

	//Prepare and send command:
	tx_cmd_data_user_r(TX_N_DEFAULT);
	pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, READ);

	//Display will be refreshed in 75ms:
	refreshDelayTimer->start(75);
}


void W_GaitStats::receiveNewData()
{

}

void W_GaitStats::comStatusChanged(SerialPortStatus status, int nbTries)
{
	(void)nbTries;	// Not use by this slot.

	if(status == PortOpeningSucceed)
		userDataMan->requestMetaData(active_slave);
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_GaitStats::on_pb_ClearRow_clicked()
{
	ui->le_number->clear();
}

void W_GaitStats::on_pb_Refresh_clicked()
{

}
