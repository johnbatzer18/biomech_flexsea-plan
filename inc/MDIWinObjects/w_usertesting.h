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

#ifndef W_USERTESTING_H
#define W_USERTESTING_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QWidget>
#include "counter.h"
#include "flexsea_generic.h"
#include "scribblearea.h"
#include <QElapsedTimer>
#include <QFileDialog>
#include <QTextStream>
#include <QButtonGroup>
#include "flexsea_user_structs.h"

//class ScribbleArea;

namespace Ui {
class W_UserTesting;
}

class W_UserTesting : public QWidget, public Counter<W_UserTesting>
{
	Q_OBJECT

public:
	explicit W_UserTesting(QWidget *parent = 0, QString appPath = "");
	~W_UserTesting();

public slots:
	void logFileName(QString fn, QString fnp);
	void extFlags(int index);

signals:
	void windowClosed(void);
	void startExperiment(int r, bool log, bool autoSample, QString offs, QString uNotes);
	void stopExperiment(void);
	void userFlags(int index);
	void writeCommand(uint8_t numb, uint8_t *tx_data, uint8_t r_w);

private slots:
	void on_pushButtonSigClear_clicked();
	void on_lineEditNameFirst_editingFinished();
	void on_lineEditNameM_editingFinished();
	void on_lineEditNameLast_editingFinished();
	void on_lineEditNameUID_editingFinished();
	void on_pushButtonExpStart_clicked();
	void on_pushButtonExpStop_clicked();
	void on_horizontalSliderSpeed_valueChanged(int value);
	void on_horizontalSliderIncline_valueChanged(int value);
	void on_doubleSpinBoxSpeed_valueChanged(double arg1);
	void on_doubleSpinBoxIncline_valueChanged(double arg1);
	void on_pushButtonClearNotes_clicked();
	void dispTimerTick();
	void on_pushButtonFlagA_clicked();
	void on_pushButtonFlagB_clicked();
	void on_pushButtonFlagC_clicked();
	void on_pushButtonFlagD_clicked();
	void activityButton(QAbstractButton*);
	void dutButton(QAbstractButton *myb);
	void dataButton(QAbstractButton *myb);
	void on_comboBoxTweaksController_currentIndexChanged(int index);
	void on_comboBoxTweaksControllerOptions_currentIndexChanged(int index);
	void on_dialAmplitude_valueChanged(int value);
	void on_spinBoxTweaksAmp_valueChanged(int arg1);
	void on_dialTiming_valueChanged(int value);
	void on_spinBoxTweaksTim_valueChanged(int arg1);
	void on_checkBoxTweaksAutomatic_stateChanged(int arg1);
	void on_pushButtonTweaksRead_clicked();
	void on_pushButtonTweaksWrite_clicked();
	void on_pushButtonPowerOff_clicked();
	void on_pushButtonPowerOn_clicked();
	void on_pushButtonExpSession_clicked();

private:
	Ui::W_UserTesting *ui;
	QString mwAppPath;
	bool uiSetup;
	ScribbleArea *scribbleArea;
	QString userID;
	QElapsedTimer expTimer;
	QTimer *dispTimer;
	int expTime;
	bool ongoingSession, ongoingExperiment;
	QFile *textFile;
	QTextStream *textStream;
	QString utPath;
	QButtonGroup *qbgActivity, *qbgDUT, *qbgData;
	double currentSpeed, currentIncline;

	//Data to be written to file:
	QString name[3];
	QString sex;
	QString DOB;
	int height[2];
	int weight;
	QString sigFileName;
	QString logFn, logFnP;

	//Tweaks:
	bool automaticMode;
	struct dual_utt_s planUTT;
	uint8_t readDisplayLag;
	bool tweakHasChanged;

	void initSigBox(void);
	void nameEditingFinished(uint8_t i);
	void speed(int index, double val);
	void incline(int index, double val);
	void sliderToSpin(void);
	void initTabSubject();
	void initTabExperiment();
	void initTimers();
	void initTabs();
	void createNewFile();
	void writeSubjectInfo();
	void closeTextFile();
	void latchSubjectInfo();
	void recordTimestampStartStop(bool start, int len, QString un);
	QString getTimestamp();
	void saveSignature();
	void writeNotes();
	void flags(int index, bool external);
	void initTabTweaks();
	void getAllInputs();
	void writeSpeedIncline(double spd, double inc);
	void writeAmplitudeTiming(int amp, int tim);
	void tweaksAmplitude(int source, int val);
	void tweaksTiming(int source, int val);
	void tweaksController(int source, int index);
	void wtf(QString txt);
	void pbSession(bool ss);
	void startOfSession();
	void endOfSession();
	void writeUTT();
	void setTweaksUI();
};

//****************************************************************************
// Definition(s)
//****************************************************************************

#endif // W_USERTESTING_H
