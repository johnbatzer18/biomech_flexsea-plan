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
	[This file] w_rigid: Rigid View Window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-04-18 | jfduval | New code
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_rigid.h"
#include "ui_w_rigid.h"

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_Rigid::W_Rigid(QWidget *parent,
				   RigidDevice *deviceLogPtr,
				   DisplayMode mode,
				   QList<RigidDevice> *deviceListPtr) :
	QWidget(parent),
	ui(new Ui::W_Rigid)
{
	ui->setupUi(this);

	deviceLog  = deviceLogPtr;
	deviceList = deviceListPtr;

	displayMode = mode;

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	updateDisplayMode(displayMode, nullptr);
}

W_Rigid::~W_Rigid()
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

//Call this function to refresh the display
void W_Rigid::refreshDisplay(void)
{
	int index = ui->comboBox_slave->currentIndex();
	display(&((*deviceList)[index]), 0);
}

void W_Rigid::refreshDisplayLog(int index, FlexseaDevice * devPtr)
{
	if(devPtr->slaveName == deviceLog->slaveName)
	{
		if(deviceLog->riList.isEmpty() == false)
		{
			 display(deviceLog, index);
		}
	}
}

void W_Rigid::updateDisplayMode(DisplayMode mode, FlexseaDevice* devPtr)
{
	(void)devPtr;
	displayMode = mode;
	if(displayMode == DisplayLogData)
	{
		initLog();
	}
	else
	{
		initLive();
	}
}

//****************************************************************************
// Private function(s):
//****************************************************************************

void W_Rigid::initLive(void)
{
	//Populates Slave list:
	ui->comboBox_slave->clear();

	for(int i = 0; i < (*deviceList).length(); i++)
	{
		ui->comboBox_slave->addItem((*deviceList)[i].slaveName);
	}
}

void W_Rigid::initLog(void)
{
	//Populates Slave list:
	ui->comboBox_slave->clear();
	ui->comboBox_slave->addItem(deviceLog->slaveName);
}

void W_Rigid::display(RigidDevice *devicePtr, int index)
{
	struct rigid_s *go = devicePtr->riList[index];
	//Raw values:
	//===========

	ui->disp_accx->setText(QString::number(go->accel.x));
	ui->disp_accy->setText(QString::number(go->accel.y));
	ui->disp_accz->setText(QString::number(go->accel.z));

	ui->disp_gyrox->setText(QString::number(go->gyro.x));
	ui->disp_gyroy->setText(QString::number(go->gyro.y));
	ui->disp_gyroz->setText(QString::number(go->gyro.z));

	ui->disp_magnetox->setText(QString::number(go->magneto.x));
	ui->disp_magnetoy->setText(QString::number(go->magneto.y));
	ui->disp_magnetoz->setText(QString::number(go->magneto.z));

	ui->disp_cs1->setText(QString::number(go->capsense[0]));
	ui->disp_cs2->setText(QString::number(go->capsense[1]));
	ui->disp_cs3->setText(QString::number(go->capsense[2]));
	ui->disp_cs4->setText(QString::number(go->capsense[3]));

	ui->disp_io1->setText(QString::number(go->io[0]));
	ui->disp_io2->setText(QString::number(go->io[1]));

	ui->disp_stat1->setText(QString::number(go->status));

	//Decoded values:
	//===============

	ui->disp_accx_d->setText(QString::number((float)go->decoded.accel.x/1000,'f',2));
	ui->disp_accy_d->setText(QString::number((float)go->decoded.accel.y/1000,'f',2));
	ui->disp_accz_d->setText(QString::number((float)go->decoded.accel.z/1000,'f',2));

	ui->disp_gyrox_d->setText(QString::number(go->decoded.gyro.x, 'i', 0));
	ui->disp_gyroy_d->setText(QString::number(go->decoded.gyro.y, 'i', 0));
	ui->disp_gyroz_d->setText(QString::number(go->decoded.gyro.z, 'i', 0));

	ui->disp_magnetox_d->setText(QString::number(go->decoded.magneto.x, 'i', 0));
	ui->disp_magnetoy_d->setText(QString::number(go->decoded.magneto.y, 'i', 0));
	ui->disp_magnetoz_d->setText(QString::number(go->decoded.magneto.z, 'i', 0));

	//==========
}

//****************************************************************************
// Private slot(s):
//****************************************************************************
