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

#ifndef W_CYCLETESTER_H
#define W_CYCLETESTER_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QWidget>
#include "counter.h"
#include "flexsea_generic.h"
#include "cmd-CycleTester.h"
#include <streammanager.h>

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************

namespace Ui {
class W_CycleTester;
}

class W_CycleTester : public QWidget, public Counter<W_CycleTester>
{
	Q_OBJECT

public:
	//Constructor & Destructor:
	explicit W_CycleTester(	QWidget *parent = 0, \
							QList<FlexseaDevice*> *rigidDevListInit = nullptr, \
							StreamManager* sm = nullptr);
	StreamManager* streamManager;
	~W_CycleTester();

public slots:

signals:
	void windowClosed(void);
	void writeCommand(uint8_t numb, uint8_t *tx_data, uint8_t r_w);

private slots:

	void on_pushButtonInit_clicked();
	void on_pushButtonStart_clicked();
	void on_pushButtonStop_clicked();
	void on_pushButtonRead_clicked();
	void on_pushButtonStartStreaming_clicked();
	void on_pushButtonReset_clicked();
	void on_pushButtonConfirmReset_clicked();
	void timerEvent(void);
	void buttonTimerEvent(void);
	void on_pushButtonPause_clicked();
	void on_pushButtonStatus_pressed();
	void on_pbRead_clicked();
	void on_pbCopy_clicked();
	void on_pbCompute_clicked();
	void on_pbWrite_clicked();
	void on_pushButtonStartAutoStreaming_clicked();
	void on_pbPresetW1_clicked();
	void on_pbPresetW2_clicked();
	void on_pbPresetR1_clicked();
	void on_pushButtonStatus_clicked();
	void on_pushButtonReboot_clicked();

private:
	//Variables & Objects:
	Ui::W_CycleTester *ui;
	QTimer *timer, *buttonTimer;
	bool resetPBstate, streamingPBstate, autoStreamingPBstate;

	//Function(s):
	void init(void);
	void initCtrlTab(void);
	void initStatsTab(void);
	void initProfileTab(void);
	void experimentControl(enum expCtrl e);
	void experimentStats(enum expStats e);
	void resetStats(void);
	void displayStatus(uint8_t s);
	QString displayFSMstate(uint8_t s, QString *shortText);
	void displayTemp(int8_t t);
	void refreshProfileDisplay(void);
	void presets(uint8_t i);

	QList<FlexseaDevice*> *rigidDevList;
	QList<FlexseaDevice*> rigidTargetList;

	uint16_t timePresets[3][5] = {{0,400,650,725,1100}, \
								{0,300,450,525,900},\
								{0,250,350,400,600}};
};

#define TIMER_PERIOD	100		//10Hz

#define TEMP_MIN		15
#define TEMP_MAX		80
#define TEMP_SPAN		(TEMP_MAX-TEMP_MIN)

#endif // W_CYCLETESTER_H
