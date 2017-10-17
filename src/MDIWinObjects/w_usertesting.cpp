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
#include <flexsea_system.h>
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
#include "cmd-UTT.h"


//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_UserTesting::W_UserTesting(QWidget *parent,
							QString appPath) :
	QWidget(parent),
	ui(new Ui::W_UserTesting)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	uiSetup = true;

	ui->tabWidget->setStyleSheet(":disabled {color: black}");

	init_utt();

	initTabs();
	initTabSubject();
	initTabExperiment();
	initTabTweaks();
	initTimers();

	mwAppPath = appPath;
	tweakHasChanged = false;
	uiSetup = false;
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
	ui->tabWidget->setTabEnabled(2, false);

	ongoingSession = false;
	pbSession(true);
}

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

	ui->radioButtonDataF->setChecked(true);
	ui->radioButtonDUT_L->setChecked(true);

	//Start/Stop:
	ui->pushButtonExpStart->setEnabled(true);
	ui->pushButtonExpStop->setEnabled(false);
	ui->pushButtonExpStart->setStyleSheet("background-color: rgb(0, 255, 0); \
												color: rgb(0, 0, 0)");
	ui->pushButtonExpStop->setStyleSheet("background-color: rgb(255, 0, 0); \
											  color: rgb(0, 0, 0)");

	ongoingExperiment = false;

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
	ui->radioButtonDUT_D->setChecked(true);
	connect(qbgDUT, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(dutButton(QAbstractButton*)));

	//Button groups - Data:
	qbgData = new QButtonGroup(this);
	qbgData->addButton(ui->radioButtonDataF);
	qbgData->addButton(ui->radioButtonDataA);
	ui->radioButtonDataA->setChecked(true);
	connect(qbgData, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(dataButton(QAbstractButton*)));
}

void W_UserTesting::initTabTweaks(void)
{
	//Always start in Linked Legs mode
	ui->checkBoxIndependant->setChecked(false);
	independantLegs(false);

	//Controller list:
	planUTT.leg[UTT_RIGHT].ctrl = 0;
	ui->comboBoxTweaksControllerR->addItem("Unknown");
	ui->comboBoxTweaksControllerR->addItem("No Motor");
	ui->comboBoxTweaksControllerR->addItem("Shadow");
	ui->comboBoxTweaksControllerR->addItem("Spring Only");
	ui->comboBoxTweaksControllerR->addItem("Power");
	planUTT.leg[UTT_LEFT].ctrl = 0;
	ui->comboBoxTweaksControllerL->addItem("Unknown");
	ui->comboBoxTweaksControllerL->addItem("No Motor");
	ui->comboBoxTweaksControllerL->addItem("Shadow");
	ui->comboBoxTweaksControllerL->addItem("Spring Only");
	ui->comboBoxTweaksControllerL->addItem("Power");

	//Controller Option list:
	planUTT.leg[UTT_RIGHT].ctrlOption = 0;
	ui->comboBoxTweaksControllerOptionsR->addItem("Unknown/None");
	ui->comboBoxTweaksControllerOptionsR->addItem("Level 0");
	ui->comboBoxTweaksControllerOptionsR->addItem("Level 1");
	ui->comboBoxTweaksControllerOptionsR->addItem("Level 2");
	planUTT.leg[UTT_LEFT].ctrlOption = 0;
	ui->comboBoxTweaksControllerOptionsL->addItem("Unknown/None");
	ui->comboBoxTweaksControllerOptionsL->addItem("Level 0");
	ui->comboBoxTweaksControllerOptionsL->addItem("Level 1");
	ui->comboBoxTweaksControllerOptionsL->addItem("Level 2");

	//Auto & Write:
	automaticMode = false;
	ui->checkBoxTweaksAutomatic->setChecked(automaticMode);
	ui->pushButtonTweaksWrite->setEnabled(true);
	ui->checkBoxTweaksAutomatic->setEnabled(false);	//ToDo enable once programmed

	//Dials & inputs:
	planUTT.leg[UTT_RIGHT].amplitude = 0;
	//ui->dialAmplitudeR->setValue(planUTT.leg[UTT_RIGHT].amplitude);
	//ui->spinBoxTweaksAmpR->setValue(ui->dialAmplitudeR->value());
	planUTT.leg[UTT_RIGHT].timing = 0;
	//ui->dialTimingR->setValue(planUTT.leg[UTT_RIGHT].timing);
	//ui->spinBoxTweaksTimR->setValue(ui->dialTimingR->value());
	planUTT.leg[UTT_LEFT].amplitude = 0;
	//ui->dialAmplitudeL->setValue(planUTT.leg[UTT_LEFT].amplitude);
	//ui->spinBoxTweaksAmpL->setValue(ui->dialAmplitudeL->value());
	planUTT.leg[UTT_LEFT].timing = 0;
	//ui->dialTimingL->setValue(planUTT.leg[UTT_LEFT].timing);
	//ui->spinBoxTweaksTimL->setValue(ui->dialTimingL->value());

	//Power buttons:
	ui->pushButtonPowerOffR->setStyleSheet("background-color: rgb(255, 0, 0); \
											color: rgb(0, 0, 0)");
	ui->pushButtonPowerOnR->setStyleSheet("background-color: rgb(0, 255, 0); \
											color: rgb(0, 0, 0)");
	ui->pushButtonPowerOffL->setStyleSheet("background-color: rgb(255, 0, 0); \
										  color: rgb(0, 0, 0)");
	ui->pushButtonPowerOnL->setStyleSheet("background-color: rgb(0, 255, 0); \
										  color: rgb(0, 0, 0)");

	planUTT.leg[UTT_RIGHT].powerOn = 0;
	planUTT.leg[UTT_LEFT].powerOn = 0;
	setTweaksUI(UTT_RIGHT);
	setTweaksUI(UTT_LEFT);
}

void W_UserTesting::independantLegs(bool i)
{
	if(i == false)
	{
		//Right is in charge, left tracks (dependant mode)
		ui->tabWidgetTweaksLR->setCurrentIndex(0);
		ui->tabWidgetTweaksLR->setTabEnabled(0, true);
		ui->tabWidgetTweaksLR->setTabEnabled(1, false);
		ui->pushButtonLtoR->setEnabled(false);
		ui->pushButtonRtoL->setEnabled(false);
		wtf("Leg tweaks are Dependant (linked)");
	}
	else
	{
		ui->tabWidgetTweaksLR->setTabEnabled(0, true);
		ui->tabWidgetTweaksLR->setTabEnabled(1, true);
		ui->pushButtonLtoR->setEnabled(true);
		ui->pushButtonRtoL->setEnabled(true);
		wtf("Leg tweaks are Independant");
	}
}

//Set all the Tweaks widgets according to planUTT.leg[]
void W_UserTesting::setTweaksUI(uint8_t leg)
{
	if(leg == UTT_RIGHT)
	{
		ui->comboBoxTweaksControllerR->setCurrentIndex(planUTT.leg[leg].ctrl);
		ui->comboBoxTweaksControllerOptionsR->setCurrentIndex(planUTT.leg[leg].ctrlOption);
		ui->comboBoxTweaksControllerOptionsR->setEnabled(false);
		ui->dialAmplitudeR->setValue(planUTT.leg[leg].amplitude);
		ui->spinBoxTweaksAmpR->setValue(planUTT.leg[leg].amplitude);
		ui->dialTimingR->setValue(planUTT.leg[leg].timing);
		ui->spinBoxTweaksTimR->setValue(planUTT.leg[leg].timing);
	}
	else
	{
		ui->comboBoxTweaksControllerL->setCurrentIndex(planUTT.leg[leg].ctrl);
		ui->comboBoxTweaksControllerOptionsL->setCurrentIndex(planUTT.leg[leg].ctrlOption);
		ui->comboBoxTweaksControllerOptionsL->setEnabled(false);
		ui->dialAmplitudeL->setValue(planUTT.leg[leg].amplitude);
		ui->spinBoxTweaksAmpL->setValue(planUTT.leg[leg].amplitude);
		ui->dialTimingL->setValue(planUTT.leg[leg].timing);
		ui->spinBoxTweaksTimL->setValue(planUTT.leg[leg].timing);
	}
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

//Experimental session button: color & text
void W_UserTesting::pbSession(bool ss)
{
	if(ss == true)
	{
		ui->pushButtonExpSession->setStyleSheet("background-color: rgb(0, 255, 0); \
											color: rgb(0, 0, 0)");
		ui->pushButtonExpSession->setText("Start Experimental Session");
	}
	else
	{
		ui->pushButtonExpSession->setStyleSheet("background-color: rgb(255, 0, 0); \
											color: rgb(0, 0, 0)");
		ui->pushButtonExpSession->setText("Stop Experimental Session");
	}
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

	QString path = mwAppPath;
	QString pathExp = path + '/' +dirName + '/';
	QDir().mkpath(pathExp);	//Create the directory if it didn't already exist
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
void W_UserTesting::recordTimestampStartStop(bool start, int len, QString un)
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
		*textStream << "#Log file user notes: " << un << endl;
		//All parameters:
		getAllInputs();
	}
}

QString W_UserTesting::getTimestamp(void)
{
	return (QDate::currentDate().toString("yyyy-MM-dd_") +
			QTime::currentTime().toString("HH'h'mm'm'ss's'"));
}

//Write 'txt' To File. Timestamp added automatically.
void W_UserTesting::wtf(QString txt)
{
	if(!uiSetup && tfOpen){*textStream << getTimestamp() << " " << txt << endl;}
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

void W_UserTesting::writeSpeedIncline(double spd, double inc)
{
	wtf("Speed = " + QString::number(spd) + ", Incline = " + QString::number(inc));
}

void W_UserTesting::writeAmplitudeTiming(uint8_t leg, int amp, int tim)
{
	QString l = " (Right or Dual)";
	if(leg == UTT_LEFT){l = " (Left)";}
	wtf("Amplitude = " + QString::number(amp) + "%, Timing = " + QString::number(tim) + "%" + l);
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

//Start of a session
void W_UserTesting::startOfSession()
{
	//Move to the next tab, lock this one
	ui->tabWidget->setCurrentIndex(1);
	ui->tabWidget->setTabEnabled(0, false);
	ui->tabWidget->setTabEnabled(1, true);
	ui->tabWidget->setTabEnabled(2, true);

	createNewFile();
	latchSubjectInfo();
	writeSubjectInfo();
	tfOpen = true;	//Allows wtf() to write data
}

//End of a session
void W_UserTesting::endOfSession()
{
	//Move to the Subject tab, lock this one
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setTabEnabled(0, true);
	ui->tabWidget->setTabEnabled(1, false);
	ui->tabWidget->setTabEnabled(2, false);

	writeNotes();
	closeTextFile();
	tfOpen = false;	//Prevents wtf() to write data
}

void W_UserTesting::writeUTT(void)
{
	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Prep & send:
	tx_cmd_utt_w(TX_N_DEFAULT, 0, &planUTT);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_UserTesting::dispTimerTick(void)
{
	if(ongoingExperiment){expTime = expTimer.elapsed() / 1000;}
	ui->lcdNumberSeconds->display(expTime);

	//We use this slow refresh to filter out the speed and incline logs:
	static uint8_t spdDiv = 0;
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
			writeSpeedIncline(currentSpeed, currentIncline);
		}
	}

	//Same filtering for the Tweaks tab:
	static uint8_t twDiv = 0;
	static uint8_t lastAmplitude[2] = {planUTT.leg[UTT_RIGHT].amplitude, planUTT.leg[UTT_LEFT].amplitude};
	static int8_t lastTiming[2] = {planUTT.leg[UTT_RIGHT].timing, planUTT.leg[UTT_RIGHT].timing};
	twDiv++;
	twDiv %= 10;
	if(!twDiv)
	{
		for(int leg = 0; leg < 2; leg++)
		{
			if(planUTT.leg[leg].amplitude != lastAmplitude[leg] || planUTT.leg[leg].timing != lastTiming[leg])
			{
				lastAmplitude[leg] = planUTT.leg[leg].amplitude;
				lastTiming[leg] = planUTT.leg[leg].timing;
				writeAmplitudeTiming(leg, planUTT.leg[leg].amplitude, planUTT.leg[leg].timing);
				tweakHasChanged = true;
			}
		}
	}

	//Read button:
	if(readDisplayLag > 0)
	{
		readDisplayLag--;
		if(!readDisplayLag)
		{
			//We have waited long enough
			planUTT = utt;
			qDebug() << "Refreshing display based on read data.";
			setTweaksUI(UTT_RIGHT);
			setTweaksUI(UTT_LEFT);
			//Send points to Ankle Torque Tool
			//ToDo remove - faking points as a 1st test:
			utt.leg[0].torquePoints[0][0] = -20;
			utt.leg[0].torquePoints[0][1] = 10;
			utt.leg[0].torquePoints[1][0] = -10;
			utt.leg[0].torquePoints[1][1] = 20;
			emit torquePointsChanged();
		}
	}

	//Color-code Write to Device:
	if(tweakHasChanged == true)
	{
		ui->pushButtonTweaksWrite->setStyleSheet("background-color: rgb(255, 255, 0); \
												color: rgb(0, 0, 0)");
	}
	else
	{
		ui->pushButtonTweaksWrite->setStyleSheet("");
	}
}

void W_UserTesting::on_pushButtonExpSession_clicked()
{
	if(ongoingSession == false)
	{
		pbSession(false);
		startOfSession();
		ongoingSession = true;

		//Clear the timer - cleaner start:
		expTime = 0;
	}
	else
	{
		//End any ongoing experiment
		if(ongoingExperiment)
		{
			emit on_pushButtonExpStop_clicked();
		}

		pbSession(true);
		endOfSession();
		ongoingSession = false;
	}
}

//****************************************************************************
// Private slot(s) - Tab User:
//****************************************************************************

void W_UserTesting::on_pushButtonSigClear_clicked(){scribbleArea->clearImage();}

void W_UserTesting::on_lineEditNameFirst_editingFinished(){nameEditingFinished(0);}
void W_UserTesting::on_lineEditNameM_editingFinished(){nameEditingFinished(1);}
void W_UserTesting::on_lineEditNameLast_editingFinished(){nameEditingFinished(2);}
void W_UserTesting::on_lineEditNameUID_editingFinished(){nameEditingFinished(3);}

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

//****************************************************************************
// Private slot(s) - Tab Experiment:
//****************************************************************************

void W_UserTesting::on_pushButtonExpStart_clicked()
{
	int refreshRate = 7;	//Index that corresponds to 200Hz
	ui->pushButtonExpStart->setEnabled(false);
	ui->pushButtonExpStop->setEnabled(true);
	ongoingExperiment = true;
	QString offs = "";
	if(ui->radioButtonDataA->isChecked()){offs = "o=0,1,2,3;";}
	else{offs = "o=0;";}

	qint64 numericalTimestamp = QDateTime::currentMSecsSinceEpoch();
	QString logTxt = userID + "-" + QString::number(numericalTimestamp);
	emit startExperiment(refreshRate, true, true, offs, logTxt);
	expTimer.start();
	recordTimestampStartStop(true, 0, logTxt);
}

void W_UserTesting::on_pushButtonExpStop_clicked()
{
	ui->pushButtonExpStart->setEnabled(true);
	ui->pushButtonExpStop->setEnabled(false);
	ongoingExperiment = false;
	emit stopExperiment();
	recordTimestampStartStop(false, expTime, "");
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

void W_UserTesting::on_doubleSpinBoxSpeed_valueChanged(double arg1){speed(1, arg1);}
void W_UserTesting::on_doubleSpinBoxIncline_valueChanged(double arg1){incline(1, arg1);}

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

void W_UserTesting::on_pushButtonClearNotes_clicked(){	ui->plainTextEdit->clear();}

void W_UserTesting::on_pushButtonFlagA_clicked(){flags(0, false);}
void W_UserTesting::on_pushButtonFlagB_clicked(){flags(1, false);}
void W_UserTesting::on_pushButtonFlagC_clicked(){flags(2, false);}
void W_UserTesting::on_pushButtonFlagD_clicked(){flags(3, false);}

void W_UserTesting::flags(int index, bool external)
{
	static bool butStat[4] = {false, false, false, false};
	QString wtf = "", txt = "";
	if(index == 0){txt = ui->lineEditFlagA->text();}

	if(external && butStat[index])
	{
		butStat[index] = false;
		return;
	}

	wtf = getTimestamp() + " Flag " + QString::number(1 << index) + " " + txt;
	*textStream << wtf << endl;

	if(external == false)
	{
		//qDebug() << "Internal flag";
		butStat[index] = true;
		emit userFlags(index);
	}
}

void W_UserTesting::activityButton(QAbstractButton* myb)
{
	QString actPrefixText = "Activity = ", actText = "Walk";
	if(myb == ui->radioButtonActRun){actText = "Run";}
	if(myb == ui->radioButtonActOther){actText = "Other";}
	//qDebug() << (actPrefixText + actText);
	wtf(actPrefixText + actText);
}

void W_UserTesting::dutButton(QAbstractButton* myb)
{
	QString dutPrefixText = "DUT = ", dutText = "Left";
	if(myb == ui->radioButtonDUT_R){dutText = "Right";}
	if(myb == ui->radioButtonDUT_D){dutText = "Dual";}

	wtf(dutPrefixText + dutText);
}

void W_UserTesting::dataButton(QAbstractButton* myb)
{
	QString dataPrefixText = "Data = ", dataText = "Fast";
	if(myb == ui->radioButtonDataF){dataText = "All";}

	wtf(dataPrefixText + dataText);
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
	QString incPre = "Incline = ", inc = "";
	inc = QString::number((double)ui->horizontalSliderIncline->value()/10);

	//Independant?
	QString depPre = "Leg tweaks are ", dep = "Dependant (linked)";
	if(ui->checkBoxIndependant->isChecked()){dep = "Independant";}

	//Write to file:
	*textStream << tmstp << " Starting conditions: " << (actPre + act) << ", " << \
					(dataPre + data) << ", " << (dutPre + dut) << ", " << \
					(spdPre + spd) << ", " << (incPre + inc) << ", " << \
					(depPre + dep) << endl;
}

//****************************************************************************
// Private slot(s) - Tab Tweaks:
//****************************************************************************

void W_UserTesting::on_comboBoxTweaksControllerR_currentIndexChanged(int index)
{
	//"Power" has sub-options:
	bool enableOptions = false;
	if(index == 4){enableOptions = true;}
	else
	{
		ui->comboBoxTweaksControllerOptionsR->setCurrentIndex(0);
	}
	ui->comboBoxTweaksControllerOptionsR->setEnabled(enableOptions);

	tweaksController(UTT_RIGHT, 0, index);
}

void W_UserTesting::on_comboBoxTweaksControllerOptionsR_currentIndexChanged(int index)
{
	tweaksController(UTT_RIGHT, 1, index);
}

void W_UserTesting::on_comboBoxTweaksControllerL_currentIndexChanged(int index)
{
	//"Power" has sub-options:
	bool enableOptions = false;
	if(index == 4){enableOptions = true;}
	else
	{
		ui->comboBoxTweaksControllerOptionsL->setCurrentIndex(0);
	}
	ui->comboBoxTweaksControllerOptionsL->setEnabled(enableOptions);

	tweaksController(UTT_LEFT, 0, index);
}

void W_UserTesting::on_comboBoxTweaksControllerOptionsL_currentIndexChanged(int index)
{
	tweaksController(UTT_LEFT, 1, index);
}

void W_UserTesting::tweaksController(uint8_t leg, int source, int index)
{
	QString legTxt = "";

	if(uiSetup){return;}

	if(leg == UTT_RIGHT)
	{
		legTxt = " (Right or Dual)";
		if(source == 0)
		{
			//Main controller:
			planUTT.leg[leg].ctrl = index;
			if(index == 4)
			{
				planUTT.leg[leg].ctrlOption = ui->comboBoxTweaksControllerOptionsR->currentIndex();
			}
			else
			{
				planUTT.leg[leg].ctrlOption = 0;
			}
		}
		else
		{
			//Sub:
			planUTT.leg[leg].ctrl = ui->comboBoxTweaksControllerR->currentIndex();
			planUTT.leg[leg].ctrlOption = index;
		}
	}
	else
	{
		legTxt = " (Left)";
		if(source == 0)
		{
			//Main controller:
			planUTT.leg[leg].ctrl = index;
			if(index == 4)
			{
				planUTT.leg[leg].ctrlOption = ui->comboBoxTweaksControllerOptionsL->currentIndex();
			}
			else
			{
				planUTT.leg[leg].ctrlOption = 0;
			}
		}
		else
		{
			//Sub:
			planUTT.leg[leg].ctrl = ui->comboBoxTweaksControllerL->currentIndex();
			planUTT.leg[leg].ctrlOption = index;
		}
	}

	//qDebug() << "Controller:" << planUTT.leg[leg].ctrl << " Option:" << planUTT.leg[leg].ctrlOption;
	wtf("Controller: " + QString::number(planUTT.leg[leg].ctrl) + " (" + QString::number(planUTT.leg[leg].ctrlOption) + ")" + legTxt);
	tweakHasChanged = true;
}

void W_UserTesting::on_dialAmplitudeR_valueChanged(int value){tweaksAmplitude(UTT_RIGHT, 0, value);}
void W_UserTesting::on_spinBoxTweaksAmpR_valueChanged(int arg1){tweaksAmplitude(UTT_RIGHT, 1, arg1);}
void W_UserTesting::on_dialAmplitudeL_valueChanged(int value){tweaksAmplitude(UTT_LEFT, 0, value);}
void W_UserTesting::on_spinBoxTweaksAmpL_valueChanged(int arg1){tweaksAmplitude(UTT_LEFT, 1, arg1);}

void W_UserTesting::tweaksAmplitude(uint8_t leg, int source, int val)
{
	//Protect against recursions:
	static bool internal[2] = {false, false};
	if(internal[leg] == true)
	{
		internal[leg] = false;
		return;
	}

	if(leg == UTT_RIGHT)
	{
		if(source == 0)
		{
			//Change came from the dial:
			internal[leg] = true;
			ui->spinBoxTweaksAmpR->setValue(val);
		}
		else
		{
			//Change came from the spinbox:
			internal[leg] = true;
			ui->dialAmplitudeR->setValue(val);
		}
	}
	else
	{
		if(source == 0)
		{
			//Change came from the dial:
			internal[leg] = true;
			ui->spinBoxTweaksAmpL->setValue(val);
		}
		else
		{
			//Change came from the spinbox:
			internal[leg] = true;
			ui->dialAmplitudeL->setValue(val);
		}
	}

	planUTT.leg[leg].amplitude = val;
	//qDebug() << "Amplitude:" << planUTT.leg[leg].amplitude;
}

void W_UserTesting::on_dialTimingR_valueChanged(int value){tweaksTiming(UTT_RIGHT, 0, value);}
void W_UserTesting::on_spinBoxTweaksTimR_valueChanged(int arg1){tweaksTiming(UTT_RIGHT, 1, arg1);}

void W_UserTesting::on_dialTimingL_valueChanged(int value){tweaksTiming(UTT_LEFT, 0, value);}
void W_UserTesting::on_spinBoxTweaksTimL_valueChanged(int arg1){tweaksTiming(UTT_LEFT, 1, arg1);}

void W_UserTesting::tweaksTiming(uint8_t leg, int source, int val)
{
	//Protect against recursions:
	static bool internal[2] = {false, false};
	if(internal[leg] == true)
	{
		internal[leg] = false;
		return;
	}

	if(leg == UTT_RIGHT)
	{
		if(source == 0)
		{
			//Change came from the dial:
			internal[leg] = true;
			ui->spinBoxTweaksTimR->setValue(val);
		}
		else
		{
			//Change came from the spinbox:
			internal[leg] = true;
			ui->dialTimingR->setValue(val);
		}
	}
	else
	{
		if(source == 0)
		{
			//Change came from the dial:
			internal[leg] = true;
			ui->spinBoxTweaksTimL->setValue(val);
		}
		else
		{
			//Change came from the spinbox:
			internal[leg] = true;
			ui->dialTimingL->setValue(val);
		}
	}

	planUTT.leg[leg].timing = val;
	//qDebug() << "Timing:" << planUTT.leg[leg].timing;
}

void W_UserTesting::on_checkBoxTweaksAutomatic_stateChanged(int arg1)
{
	QString txt = "";

	if(arg1 == 0)
	{
		//Unchecked: normal (non-automatic) mode
		ui->pushButtonTweaksWrite->setEnabled(true);
		txt = "Normal mode (unchecked)";
	}
	else
	{
		ui->pushButtonTweaksWrite->setEnabled(false);
		txt = "Automatic mode (checked)";
	}

	automaticMode = (bool)arg1;
	wtf("Automatic checkbox clicked: " + txt);
	//qDebug() << "Automatic mode:" << automaticMode;
}

void W_UserTesting::on_pushButtonTweaksRead_clicked()
{
	wtf("Read From Device was clicked");

	uint16_t numb = 0;
	uint8_t info[2] = {PORT_USB, PORT_USB};

	//Prep & send:
	tx_cmd_utt_r(TX_N_DEFAULT, 0);
	pack(P_AND_S_DEFAULT, FLEXSEA_MANAGE_1, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, READ);

	readDisplayLag = 5;
}

void W_UserTesting::on_pushButtonTweaksWrite_clicked()
{
	QString extraTxt = "";
	if(ui->checkBoxIndependant->checkState() == false)
	{
		extraTxt = " (Left was copied from Right)";
		copyLegToLeg(true, true);
		setTweaksUI(UTT_LEFT);
	}

	wtf("Write to Device was clicked" + extraTxt);
	writeUTT();
	tweakHasChanged = false;
}

void W_UserTesting::on_pushButtonPowerOffR_clicked()
{
	planUTT.leg[UTT_RIGHT].powerOn = 0;
	wtf("Power Off was clicked (Right or Dual)");
	writeUTT();
}

void W_UserTesting::on_pushButtonPowerOnR_clicked()
{
	planUTT.leg[UTT_RIGHT].powerOn = 1;
	wtf("Power On was clicked (Right or Dual)");
	writeUTT();
}

void W_UserTesting::on_pushButtonPowerOnL_clicked()
{
	planUTT.leg[UTT_LEFT].powerOn = 1;
	wtf("Power On was clicked (Left)");
	writeUTT();
}

void W_UserTesting::on_pushButtonPowerOffL_clicked()
{
	planUTT.leg[UTT_LEFT].powerOn = 0;
	wtf("Power Off was clicked (Left)");
	writeUTT();
}

void W_UserTesting::on_checkBoxIndependant_stateChanged(int arg1)
{
	bool state = false;
	if(arg1){state = true;}
	independantLegs(state);
}

void W_UserTesting::on_pushButtonRtoL_clicked(){copyLegToLeg(true, false);}
void W_UserTesting::on_pushButtonLtoR_clicked(){copyLegToLeg(true, false);}

void W_UserTesting::copyLegToLeg(bool RtL, bool silent)
{
	uint8_t src = UTT_LEFT, dst = UTT_RIGHT;
	QString txt = "Copied tweaks (Left to Right)";

	if(RtL == true)
	{
		src = UTT_RIGHT;
		dst = UTT_LEFT;
		txt = "Copied tweaks (Right to Left)";
	}

	planUTT.leg[dst].ctrl = planUTT.leg[src].ctrl;
	planUTT.leg[dst].ctrlOption = planUTT.leg[src].ctrlOption;
	planUTT.leg[dst].amplitude = planUTT.leg[src].amplitude;
	planUTT.leg[dst].timing = planUTT.leg[src].timing;
	planUTT.leg[dst].powerOn = planUTT.leg[src].powerOn;

	if(!silent)
	{
		setTweaksUI(UTT_RIGHT);
		setTweaksUI(UTT_LEFT);
		wtf(txt);
	}
}
