/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'plan-gui' Graphical User Interface
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>
*****************************************************************************
	[Lead developper] Jean-Francois (JF) Duval, jfduval at dephy dot com.
	[Contributors]
*****************************************************************************
	[This file] w_usertesting.h: User testing Form Window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-10-03 | jfduval | New sub-project
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QtWidgets>
#include <stdint.h>
#include "flexsea_generic.h"
#include "w_usertesting.h"
#include "ui_w_usertesting.h"
#include <flexsea_comm.h>
#include <flexsea_board.h>
#include <QString>
#include <QTextStream>
#include <QColor>
#include "scribblearea.h"
#include <QMessageBox>
#include <QTimer>
#include <QElapsedTimer>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_UserTesting::W_UserTesting(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::W_UserTesting)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	ui->tabWidget->setCurrentIndex(0);
	initTabSubject();
	initTabExperiment();
	initTimers();
}

W_UserTesting::~W_UserTesting()
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

void W_UserTesting::initTabSubject(void)
{
	ui->radioButtonSexM->setChecked(true);
	userID = "U-";
	ui->lineEditNameUID->setText(userID);
	initSigBox();
}

void W_UserTesting::initTabExperiment(void)
{
	ui->radioButtonActWalk->setChecked(true);
	sliderToSpin();

	ui->label_ExpUID->setText(userID);

	//Start/Stop:
	ui->pushButtonExpStart->setEnabled(true);
	ui->pushButtonExpStop->setEnabled(false);
	ongoingExperiment = false;
}

void W_UserTesting::initSigBox(void)
{
	QColor penColor;
	penColor.setNamedColor("black");
	scribbleArea = new ScribbleArea;
	//ui->gridLayout_Sig->addWidget(scribbleArea);
	ui->frame->layout()->addWidget(scribbleArea);
			//addWidget(scribbleArea);
	scribbleArea->setPenColor(penColor);
	scribbleArea->setPenWidth(5);

	//Draw box around the area:
	ui->frame->setFrameShape(QFrame::Box);
	ui->frame->setFrameShadow(QFrame::Plain);
	ui->frame->setLineWidth(2);
	ui->frame->setMidLineWidth(1);
}

void W_UserTesting::initTimers(void)
{
	//Experiment: counts between Start and Stop
	expTime = 0;

	//Display refresh 10Hz:
	dispTimer = new QTimer(this);
	connect(dispTimer, SIGNAL(timeout()), this, SLOT(dispTimerTick()));
	dispTimer->start(0.1);
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_UserTesting::dispTimerTick(void)
{
	if(ongoingExperiment){expTime = expTimer.elapsed() / 1000;}
	ui->lcdNumberSeconds->display(expTime);
}

void W_UserTesting::on_pushButtonApprove_clicked()
{
	//ToDo use real file location?
	QString fn = userID + ".png";
	qDebug() << fn;
	scribbleArea->saveImage(fn, "png");
}

void W_UserTesting::on_pushButtonSigClear_clicked()
{
	scribbleArea->clearImage();
}

void W_UserTesting::on_lineEditNameFirst_editingFinished()
{
	nameEditingFinished(0);
}

void W_UserTesting::on_lineEditNameM_editingFinished()
{
	nameEditingFinished(1);
}

void W_UserTesting::on_lineEditNameLast_editingFinished()
{
	nameEditingFinished(2);

	//<Messing with people:
	QString lastNameEntered = ui->lineEditNameLast->text();
	QString lm = "mooney";
	QString jc = "cechmanek";
	if(!QString::compare(lastNameEntered, lm, Qt::CaseInsensitive))
	{
		QMessageBox::warning(this, tr("Autocorrect"), \
				tr("Misspelled named ‘Mooney’ automatically changed to ‘Money’."));
		ui->lineEditNameLast->setText("Money");
	}

	if(!QString::compare(lastNameEntered, jc, Qt::CaseInsensitive))
	{
		QMessageBox::warning(this, tr("Autocorrect"), \
				tr("Foreign-sounding name 'Cechmanek' automatically changed to the more patriotic ‘Smith’."));
		ui->lineEditNameLast->setText("Smith");
	}
	//Messing with people>
}

void W_UserTesting::on_lineEditNameUID_editingFinished()
{
	nameEditingFinished(3);
}

void W_UserTesting::nameEditingFinished(uint8_t i)
{
	if(i < 3)
	{
		//Generate new UID based on initials. Skips empties.
		QString uid = "U-";
		QChar letter;
		letter = ui->lineEditNameFirst->text().at(0);
		if(letter != 0){uid += letter;}
		letter = ui->lineEditNameM->text().at(0);
		if(letter != 0){uid += letter;}
		letter = ui->lineEditNameLast->text().at(0);
		if(letter != 0){uid += letter;}

		userID = uid;
		ui->lineEditNameUID->setText(uid);
		ui->label_ExpUID->setText(userID);	//Label on Exp tab
	}
	else
	{
		userID = ui->lineEditNameUID->text();
	}
}

void W_UserTesting::on_pushButtonExpStart_clicked()
{
	int refreshRate = 7;	//Index that corresponds to 200Hz
	ui->pushButtonExpStart->setEnabled(false);
	ui->pushButtonExpStop->setEnabled(true);
	ongoingExperiment = true;
	emit startExperiment(refreshRate, true, true, "o=0;", userID);	//ToDo pass better user notes
	expTimer.start();
}

void W_UserTesting::on_pushButtonExpStop_clicked()
{
	ui->pushButtonExpStart->setEnabled(true);
	ui->pushButtonExpStop->setEnabled(false);
	ongoingExperiment = false;
	emit stopExperiment();
}

void W_UserTesting::on_horizontalSliderSpeed_valueChanged(int value)
{
	double realValue = (double)value / 10;
	speed(0, realValue);
}

void W_UserTesting::on_horizontalSliderIncline_valueChanged(int value)
{
	double realValue = (double)value / 10;
	incline(0, realValue);
}

void W_UserTesting::on_doubleSpinBoxSpeed_valueChanged(double arg1)
{
	speed(1, arg1);
}


void W_UserTesting::on_doubleSpinBoxIncline_valueChanged(double arg1)
{
	incline(1, arg1);
}

void W_UserTesting::speed(int index, double val)
{
	if(index == 0)
	{
		//qDebug() << "Speed:" << val;
		//Change came from the slider
		ui->doubleSpinBoxSpeed->setValue(val);
	}
	else
	{
		//Change came from the spinBox
		double rV = val*10;
		ui->horizontalSliderSpeed->setValue((int)rV);
	}
}

void W_UserTesting::incline(int index, double val)
{
	if(index == 0)
	{
		qDebug() << "Incline:" << val;
		//Change came from the slider
		ui->doubleSpinBoxIncline->setValue(val);
	}
	else
	{
		//Change came from the spinBox
		double rV = val*10;
		ui->horizontalSliderIncline->setValue((int)rV);
	}
}

//Call this once at boot to match all the displays
void W_UserTesting::sliderToSpin(void)
{
	double val = (double)ui->horizontalSliderSpeed->value()/10;
	ui->doubleSpinBoxSpeed->setValue(val);
	val = (double)ui->horizontalSliderIncline->value()/10;
	ui->doubleSpinBoxIncline->setValue(val);
}

void W_UserTesting::on_pushButtonClearNotes_clicked()
{
	ui->plainTextEdit->clear();
}
