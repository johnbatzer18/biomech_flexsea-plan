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
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include "w_event.h"
#include <QButtonGroup>

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

	initTabs();
	initTabSubject();
	initTabExperiment();
	initTabTweaks();
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

void W_UserTesting::logFileName(QString fn, QString fnp)
{
	logFn = fn;
	logFnP = fnp;
}

void W_UserTesting::extFlags(int index)
{
	qDebug() << "Received from Event" << index;
	if(ui->tabWidget->currentIndex() != 0)
	{
		//We do not send flags before the session has started
		flags(index, true);
	}
}

//****************************************************************************
// Private function(s):
//****************************************************************************

void W_UserTesting::initTabs(void)
{
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(0, true);
	ui->tabWidget->setTabEnabled(1, false);

	//No Tweaks yet:
	ui->tabWidget->setTabEnabled(2, false);
}

void W_UserTesting::initTabSubject(void)
{
	ui->radioButtonSexM->setChecked(true);
	userID = "U-";
	ui->lineEditNameUID->setText(userID);
	initSigBox();

	ui->pushButtonApprove->setStyleSheet("background-color: rgb(0, 255, 0); \
											color: rgb(0, 0, 0)");
}

void W_UserTesting::initTabExperiment(void)
{
	ui->radioButtonActWalk->setChecked(true);
	sliderToSpin();

	ui->label_ExpUID->setText(userID);

	ui->radioButtonDataF->setChecked(true);
	ui->radioButtonDUT_L->setChecked(true);

	//Start/Stop:
	ui->pushButtonExpStart->setEnabled(true);
	ui->pushButtonExpStop->setEnabled(false);
	ongoingExperiment = false;

	ui->pushButtonEndSession->setStyleSheet("background-color: rgb(255, 0, 0); \
											color: rgb(0, 0, 0)");

	currentSpeed = ui->doubleSpinBoxSpeed->value();
	currentIncline = ui->doubleSpinBoxIncline->value();

	//Button group - Activity:
	qbgActivity = new QButtonGroup(this);
	qbgActivity->addButton(ui->radioButtonActWalk);
	qbgActivity->addButton(ui->radioButtonActRun);
	qbgActivity->addButton(ui->radioButtonActOther);
	connect(qbgActivity, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(activityButton(QAbstractButton*)));

	//Button groups - DUT:
	qbgDUT = new QButtonGroup(this);
	qbgDUT->addButton(ui->radioButtonDUT_L);
	qbgDUT->addButton(ui->radioButtonDUT_R);
	qbgDUT->addButton(ui->radioButtonDUT_D);
	connect(qbgDUT, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(dutButton(QAbstractButton*)));

	//Button groups - Data:
	qbgData = new QButtonGroup(this);
	qbgData->addButton(ui->radioButtonDataF);
	qbgData->addButton(ui->radioButtonDataA);
	connect(qbgData, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(dataButton(QAbstractButton*)));
}

void W_UserTesting::initTabTweaks(void)
{

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
	dispTimer->start(100);
}

void W_UserTesting::createNewFile(void)
{
	QString dirName = "UserTesting";
	textFile = new QFile(this);
	textStream = new QTextStream;

	//Create directory if needed
	if(!QDir().exists(dirName)){QDir().mkdir(dirName);}

	QString path = QDir::currentPath();
	QString pathExp = path + '/' +dirName + '/';
	QString filename = pathExp + "Exp_" + getTimestamp() + '_' + userID + ".txt";
	//Save path for other functions
	utPath = pathExp;

	//Now we open it:
	textFile->setFileName(filename);

	//Check if the file was successfully opened
	if(textFile->open(QIODevice::ReadWrite) == false)
	{
		qDebug() << "Couldn't RW open file " << filename;
		return;
	}

	qDebug() << "Opened:" << filename;
	textStream->setDevice(textFile);

	//textFile->close();	//Done by other function
}

void W_UserTesting::saveSignature(void)
{
	QString fn = utPath + "Sig_" + userID + ".png";
	qDebug() << fn;
	scribbleArea->saveImage(fn, "png");
}

//Read all the input widgets and save data
void W_UserTesting::latchSubjectInfo(void)
{
	saveSignature();

	name[0] = ui->lineEditNameFirst->text();
	name[1] = ui->lineEditNameM->text();
	name[2] = ui->lineEditNameLast->text();

	sex = "Male";
	if(ui->radioButtonSexF->isChecked()){sex = "Female";}
	if(ui->radioButtonSexO->isChecked()){sex = "Other";}

	DOB = "" + QString::number(ui->spinBoxDOB_YYYY->value()) + '-' \
		  + QString::number(ui->spinBoxDOB_MM->value()) + '-' \
		  + QString::number(ui->spinBoxDOB_DD->value());
	height[0] = ui->spinBoxHeight_ft->value();
	height[1] = ui->spinBoxHeightIn->value();
	weight = ui->spinBoxWeight->value();
	//QString sigFileName;
}

//bool start = true for start, false for stop
void W_UserTesting::recordTimestampStartStop(bool start, int len)
{
	QString prefix, suffix = "", wtf;
	if(start)
	{
		prefix = " Start";
	}
	else
	{
		prefix = " Stop";
		suffix = " (Length: " + QString::number(len) + "s)";
	}

	//Write to file:
	wtf = getTimestamp() + prefix + suffix;
	*textStream << wtf << endl;

	if(start)
	{
		//Filenames used:
		*textStream << "#Log file: " << logFn << endl;
		*textStream << "#Log file (full path): " << logFnP << endl;
		//All parameters:
		getAllInputs();
	}
}

QString W_UserTesting::getTimestamp(void)
{
	return (QDate::currentDate().toString("yyyy-MM-dd_") +
			QTime::currentTime().toString("HH'h'mm'm'ss's'"));
}

void W_UserTesting::writeSubjectInfo(void)
{
	*textStream << "UID: " << userID << endl;
	*textStream << "First name: " << name[0] << endl;
	*textStream << "Middle name: " << name[1] << endl;
	*textStream << "Middle name: " << name[2] << endl;
	*textStream << "Sex: " << sex << endl;
	*textStream << "DOB (YYYY-MM-DD): " << DOB << endl;
	*textStream << "Height: " << height[0] << "'" << height[1] << '"' << endl;
	*textStream << "Weight (lbs): " << weight << endl;
	*textStream << "---" << endl;
}

void W_UserTesting::writeSPeedIncline(double spd, double inc)
{
	*textStream << getTimestamp() << " Speed = " << QString::number(spd) <<\
				", Incline = " << QString::number(inc) << endl;
}

void W_UserTesting::writeNotes(void)
{
	*textStream << "---" << endl << "User notes begin >>>" << endl;
	*textStream << ui->plainTextEdit->toPlainText() << endl << "<<< End of user notes." << endl;
}

void W_UserTesting::closeTextFile(void)
{
	textFile->close();
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_UserTesting::dispTimerTick(void)
{
	static uint8_t spdDiv = 0;

	if(ongoingExperiment){expTime = expTimer.elapsed() / 1000;}
	ui->lcdNumberSeconds->display(expTime);

	//We use this slow refresh to filter out the speed and incline logs:
	static double lastSpeed = currentSpeed;
	static double lastIncline = currentIncline;
	spdDiv++;
	spdDiv %= 10;
	if(!spdDiv)
	{
		if(currentSpeed != lastSpeed || currentIncline != lastIncline)
		{
			lastSpeed = currentSpeed;
			lastIncline = currentIncline;
			writeSPeedIncline(currentSpeed, currentIncline);
		}
	}
}

//This button is now named Start Experimental Session
void W_UserTesting::on_pushButtonApprove_clicked()
{
	//Move to the next tab, lock this one
	ui->tabWidget->setCurrentIndex(1);
	ui->tabWidget->setTabEnabled(0, false);
	ui->tabWidget->setTabEnabled(1, true);

	createNewFile();
	latchSubjectInfo();
	writeSubjectInfo();
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
	}
	else
	{
		userID = ui->lineEditNameUID->text();
	}

	ui->label_ExpUID->setText(userID);	//Label on Exp tab
}

void W_UserTesting::on_pushButtonExpStart_clicked()
{
	int refreshRate = 7;	//Index that corresponds to 200Hz
	ui->pushButtonExpStart->setEnabled(false);
	ui->pushButtonExpStop->setEnabled(true);
	ongoingExperiment = true;
	QString offs = "";
	if(ui->radioButtonDataA->isChecked()){offs = "o=0,1,2,3;";}
	else{offs = "o=0;";}
	emit startExperiment(refreshRate, true, true, offs, userID);	//ToDo pass better user notes
	expTimer.start();
	recordTimestampStartStop(true, 0);
}

void W_UserTesting::on_pushButtonExpStop_clicked()
{
	ui->pushButtonExpStart->setEnabled(true);
	ui->pushButtonExpStop->setEnabled(false);
	ongoingExperiment = false;
	emit stopExperiment();
	recordTimestampStartStop(false, expTime);
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

	currentSpeed = val;
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

	currentIncline = val;
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

//End of a session
void W_UserTesting::on_pushButtonEndSession_clicked()
{
	//Move to the Subject tab, lock this one
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(0, true);
	ui->tabWidget->setTabEnabled(1, false);

	writeNotes();
	closeTextFile();
}

void W_UserTesting::on_pushButtonFlagA_clicked()
{
	flags(0, false);
}

void W_UserTesting::on_pushButtonFlagB_clicked()
{
	flags(1, false);
}

void W_UserTesting::on_pushButtonFlagC_clicked()
{
	flags(2, false);
}

void W_UserTesting::on_pushButtonFlagD_clicked()
{
	flags(3, false);
}

void W_UserTesting::flags(int index, bool external)
{
	QString wtf = "", txt = "";
	if(index == 0){txt = ui->lineEditFlagA->text();}

	wtf = getTimestamp() + " Flag " + QString::number(index) + " " + txt;
	*textStream << wtf << endl;

	if(external == false){emit userFlags(index);}
}

void W_UserTesting::activityButton(QAbstractButton* myb)
{
	QString actPrefixText = " Activity = ", actText = "Walk";
	if(myb == ui->radioButtonActRun){actText = "Run";}
	if(myb == ui->radioButtonActOther){actText = "Other";}
	//qDebug() << (actPrefixText + actText);
	*textStream << getTimestamp() << (actPrefixText + actText) << endl;
}

void W_UserTesting::dutButton(QAbstractButton* myb)
{
	QString dutPrefixText = " DUT = ", dutText = "Left";
	if(myb == ui->radioButtonDUT_R){dutText = "Right";}
	if(myb == ui->radioButtonDUT_D){dutText = "Dual";}

	*textStream << getTimestamp() << (dutPrefixText + dutText) << endl;
}

void W_UserTesting::dataButton(QAbstractButton* myb)
{
	QString dataPrefixText = " Data = ", dataText = "Fast";
	if(myb == ui->radioButtonDataF){dataText = "All";}

	*textStream << getTimestamp() << (dataPrefixText + dataText) << endl;
}

//Call this when it starts so we know the starting condition
void W_UserTesting::getAllInputs(void)
{
	QString tmstp = getTimestamp();

	//Activity:
	QString actPre = "Activity = ", act = "Walk";
	if(ui->radioButtonActRun->isChecked()){act = "Run";}
	if(ui->radioButtonActOther->isChecked()){act = "Other";}

	//Data:
	QString dataPre = "Data = ", data = "Fast";
	if(ui->radioButtonDataA->isChecked()){data = "All";}

	//DUT:
	QString dutPre = "DUT = ", dut = "Left";
	if(ui->radioButtonDUT_R->isChecked()){dut = "Right";}
	if(ui->radioButtonDUT_D->isChecked()){dut = "Dual";}

	//Speed:
	QString spdPre = "Speed = ", spd = "";
	spd = QString::number((double)ui->horizontalSliderSpeed->value()/10);

	//Incline
	QString incPre = "Incline ", inc = "";
	inc = QString::number((double)ui->horizontalSliderIncline->value()/10);

	//Write to file:
	*textStream << tmstp << " Starting conditions: " << (actPre + act) << ", " << \
					(dataPre + data) << ", " << (dutPre + dut) << ", " << \
					(spdPre + spd) << ", " << (incPre + inc) << endl;
}
