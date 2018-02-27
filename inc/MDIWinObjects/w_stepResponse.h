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
	[This file] w_control.cpp: Control Window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	*
****************************************************************************/

#ifndef W_STEP_H
#define W_STEP_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QWidget>
#include "w_2dplot.h"
#include "w_slavecomm.h"
#include "counter.h"
#include "flexsea_generic.h"
#include "cmd-ActPack.h"

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************
#define STEP_CONTROLLERS         2
#define STEP_GAIN_FIELDS         4

namespace Ui {
class W_StepResponse;
}

class W_StepResponse : public QWidget, public Counter<W_StepResponse>
{
	Q_OBJECT

public:
	//Constructor & Destructor:
    explicit W_StepResponse(QWidget *parent = 0, W_2DPlot *pplot = 0, W_SlaveComm *pslaveComm = 0);
    ~W_StepResponse();
	static int getSetp(void);

public slots:

signals:
	void windowClosed(void);
	void writeCommand(uint8_t numb, uint8_t *tx_data, uint8_t r_w);

private slots:
    void on_pushButton_setp_a_go_clicked();
    void on_comboBox_slave_currentIndexChanged(int index);
    void on_pushButton_UpdateGains_clicked();
    void on_pushButton_SetController_clicked();
    void on_pushButton_updateTime_clicked();
    void on_pushButton_UpdateSlider_clicked();
    void on_hSlider_Ctrl_valueChanged(int value);
    void on_pushButton_Stop_clicked();
    void on_pushButton_Run_clicked();
    void on_pushButton_RunLog_clicked();
    void on_control_slider_min_textEdited(const QString &arg1);
    void on_control_slider_max_textEdited(const QString &arg1);
    void on_control_slider_max_editingFinished();
    void on_control_slider_min_editingFinished();
    void stopMotor();
    void control_timer();
    void on_checkBox_Freq_toggled();
    void on_slider_Freq_valueChanged();
    void sinSweep();

private:
	//Variables & Objects:
    Ui::W_StepResponse *ui;
    QWidget *parent;
    W_SlaveComm *pslaveComm;
    W_2DPlot *pplot;
	int active_slave, active_slave_index;
	int wanted_controller = 0, selected_controller = 0, active_controller = 0;
	int trap_pos = 0, trap_posi = 0, trap_posf = 0, trap_spd = 0, trap_acc = 0;

    //vars for step control
    int step_numberSteps = 0;
    float step_stepDuration = 0;
    int vStep = 0;
    int atStep = 0;
    int atV = 0;
    int dataLogging = 0;
    int step_lastMin = 0, step_lastMax = 1000;
    //vars for freq control
    float freq_lastMin = 0, freq_lastMax = 5;
    int enabledFreqControl = 0;
    int withinSweepTime = 0;
    float freqStep = 0;
    float atFreq = 0;

    int step_setpoint = 0, step_setpoint_trap = 0;
    int step_toggle_state = 0;
    int ctrl_gains[STEP_CONTROLLERS][STEP_GAIN_FIELDS];
	int trapez_steps = 0;
    float step_toggleStepDuration = 0;
	uint8_t toggle_output_state = 0;
	QStringList var_list_controllers, var_list_enc_disp;
    QTimer *ptimerCtrl, *timerDisplay;
	uint8_t transferBuf[48];
	static int setp;
    struct ActPack_s ActPack;

    //Function(s):
    void initStep(void);
	void initTabSlider(void);
	void initTabToggle(void);
    void initFreq(void);
	void init_ctrl_gains(void);
	void save_ctrl_gains(int controller, int16_t *gains);
    void refreshStatusGain(void);
	void controller_setpoint(int val);
    void freq_controller_setpoint(int val);
	void initTimers(void);
	void stream_ctrl(void);
	void control_trapeze(void);
    void minMaxTextChanged(void);
    void update_CtrlMinMax(void);
    void update_FreqMinMax(void);
	void initActPack(void);
	void sendActPack(void);
	void setController(uint8_t ctrl);
};

#endif // W_STEP_H
