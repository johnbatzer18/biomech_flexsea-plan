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

//****************************************************************************
// Include(s)
//****************************************************************************

//#include "main.h"
#include <unistd.h>
#include <flexsea_system.h>
#include <flexsea_board.h>
#include "w_stepResponse.h"
#include "flexsea_generic.h"
#include "trapez.h"
#include "ui_w_stepResponse.h"
#include "serialdriver.h"
#include <QTimer>
#include <QDebug>
#include <QString>
#include <flexsea_comm.h>
#include <cmd-ActPack.h>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_StepResponse::W_StepResponse(QWidget *parent, W_2DPlot *pplot, W_SlaveComm *pslaveComm) :
	QWidget(parent),
    ui(new Ui::W_StepResponse)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

    this->pplot = pplot;
    this->pslaveComm = pslaveComm;
    initStep();
    init_ctrl_gains();
    initFreq();
}

W_StepResponse::~W_StepResponse()
{
    pslaveComm->stopExperiment();
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

void W_StepResponse::initStep(void)
{
    initTabToggle();
    ui->Toggle->setCurrentIndex(0);

	//Populates Slave list:
	FlexSEA_Generic::populateSlaveComboBox(ui->comboBox_slave, SL_BASE_EX, \
											SL_LEN_EX+SL_LEN_MN);
	//Variables:
	active_slave_index = ui->comboBox_slave->currentIndex();
    active_slave = FlexSEA_Generic::getSlaveID(SL_BASE_EX, active_slave_index);
    ptimerCtrl = new QTimer(this);

    //update slider vals
    ui->hSlider_Ctrl->setValue(step_lastMin);
    ui->hSlider_Ctrl->setMinimum(step_lastMin);
    ui->hSlider_Ctrl->setMaximum(step_lastMax);
    ui->disp_slider->setText("Off");
    ui->control_slider_min->setText(QString::number(step_lastMin));
    ui->control_slider_max->setText(QString::number(step_lastMax));


	//Variable option lists:
    var_list_controllers << "Open" << "Current";
	for(int index = 0; index < var_list_controllers.count(); index++)
	{
		ui->comboBox_ctrl_list->addItem(var_list_controllers.at(index));
	}

    ui->statusController->setText("Active controller: Open");

	//Display control encoder:
    var_list_enc_disp << "Execute's";
	for(int index = 0; index < var_list_enc_disp.count(); index++)
	{
		ui->comboBoxDispSel->addItem(var_list_enc_disp.at(index));
	}
    ui->labelDispEncoder->setText("Execute's");   //Initial

    //Command style & FSM2:
	ui->comboBoxCmdStyle->addItem("ActPack");
	ui->comboBoxCmdStyle->setCurrentIndex(0);
	ui->comboBoxFSM2->addItem("Enabled (default)");
	ui->comboBoxFSM2->addItem("Disabled");
	ui->comboBoxFSM2->setCurrentIndex(0);
	ui->comboBoxFSM2->setEnabled(false);

	initActPack();
}

void W_StepResponse::initTabToggle(void)
{
	//Limit input fields:
	const QValidator *validInt = new QIntValidator(-10000000, 10000000, this);
//	const QValidator *validUint = new QIntValidator(0, 10000000, this);
    ui->step_setp_a->setValidator(validInt);

	//Setpoints:
    ui->step_setp_a->setText("0");
    ui->step_time->setText("0");

	//Toggle:
    step_toggle_state = 0;
}

//Initialize ActPack to be in open loop control mode
void W_StepResponse::initActPack(void)
{
    ActPack.controller = CTRL_OPEN;
	ActPack.setpoint = 0;
	ActPack.setGains = KEEP;
	ActPack.g0 = 0;
	ActPack.g1 = 0;
	ActPack.g2 = 0;
	ActPack.g3 = 0;
	ActPack.system = 0;
    sendActPack();
}

void W_StepResponse::initFreq(void) {
    ui->lineEdit_MinFreq->setText(QString::number(freq_lastMin));
    ui->lineEdit_MaxFreq->setText(QString::number(freq_lastMax));
    ui->slider_Freq->setMinimum(freq_lastMin);
    ui->slider_Freq->setMaximum(freq_lastMax);
    ui->slider_Freq->setValue(freq_lastMin);
    ui->label_Freq->setText(QString::number(ui->slider_Freq->value()));

    //disable freq UI until toggled
    ui->checkBox_Freq->setChecked(false);
    ui->lineEdit_MinFreq->setEnabled(false);
    ui->lineEdit_MaxFreq->setEnabled(false);
    ui->slider_Freq->setEnabled(false);
}

//Send the ActPack command. It will use the ActPack structure values.
void W_StepResponse::sendActPack(void)
{
	uint8_t info[2] = {PORT_USB, PORT_USB};
	uint16_t numb = 0;
	uint8_t offset = 0;

	/*//Debugging only:
	qDebug() << "[ActPack]";
	qDebug() << "Controller: " << ActPack.controller;
	qDebug() << "Setpoint: " << ActPack.setpoint;
	qDebug() << "Set Gains: " << ActPack.setGains;
	qDebug() << "g0: " << ActPack.g0;
	qDebug() << "g1: " << ActPack.g1;
	qDebug() << "g2: " << ActPack.g2;
	qDebug() << "g3: " << ActPack.g3;*/

	//Send command:
	tx_cmd_actpack_rw(TX_N_DEFAULT, offset, ActPack.controller, ActPack.setpoint, \
					  ActPack.setGains, ActPack.g0, ActPack.g1, ActPack.g2, \
					  ActPack.g3, ActPack.system);
	pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
	emit writeCommand(numb, comm_str_usb, WRITE);

	if(ActPack.setGains == CHANGE)
	{
		ActPack.setGains = KEEP;
	}
}

void W_StepResponse::controller_setpoint(int val)
{
//    uint16_t numb = 0;
    uint16_t valid = 0;
//    uint8_t info[2] = {PORT_USB, PORT_USB};

	qDebug() << "Entered controller_setpoint()";

	switch(wanted_controller)
	{
        case 0: //Open
			valid = 1;
			tx_cmd_ctrl_o_w(TX_N_DEFAULT, val);
			qDebug() << "Open: " << val << "mV";
			break;
        case 1: //Current
            valid = 1;
            tx_cmd_ctrl_i_w(TX_N_DEFAULT, val);
            qDebug() << "Current: " << val << "mA";
            break;
		default:
			valid = 0;
			break;
	}

	if(valid)
	{
		ActPack.setpoint = val;

//        if(!ui->comboBoxCmdStyle->currentIndex())
//        {
//            //Common for all gain functions:
//            pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
//            emit writeCommand(numb, comm_str_usb, WRITE);
//        } else {
            sendActPack();
//        }
	}
	else
	{
		qDebug() << "Invalid controller selected.";
	}
}

void W_StepResponse::freq_controller_setpoint(int val) {
    atV = val;
    ui->disp_slider->setText(QString::number(atV));
    ui->label_Freq->setText(QString::number(atFreq));
    ptimerCtrl->stop();
    ptimerCtrl = new QTimer(this);

    //start sweep
    connect(ptimerCtrl, SIGNAL(timeout()), this, SLOT(sinSweep()));
    //2ms update time
    ptimerCtrl->start(5);
}

void W_StepResponse::sinSweep(void) {
    withinSweepTime += ptimerCtrl->interval();

    if (!enabledFreqControl) {
        stopMotor();
    } else if (withinSweepTime >= step_stepDuration*1000 && step_stepDuration != 0) {
        //if we're doing time stepping
        if (step_numberSteps != 0) {
            atV += vStep;
            atFreq += freqStep;
            //should we continue increasing the step or stop?
            if (atV >= step_lastMax && atFreq >= freq_lastMax) {
                stopMotor();
            } else {
                withinSweepTime = 0;
                freq_controller_setpoint(atV);
            }
        //else stop motors
        } else {
            stopMotor();
        }
    } else {
        controller_setpoint((int) (atV*sin(withinSweepTime*atFreq/1000)));
    }
}

void W_StepResponse::stream_ctrl(void)
{
	struct execute_s *ex_ptr;
	FlexSEA_Generic::assignExecutePtr(&ex_ptr, SL_BASE_EX, active_slave_index);

	if(ui->comboBoxDispSel->currentIndex() == 0)    //Execute's
	{
		ui->labelDispEncoder->setText(QString::number(*ex_ptr->enc_ang));
	}
	else
	{
		ui->labelDispEncoder->setText("Invalid.");
	}

}

void W_StepResponse::control_trapeze(void)
{
    //Call 10x to match 1ms & 100Hz timebases:
    for(int i = 0; i < 10; i++)
    {
        trapez_get_pos(trapez_steps);
    }
    step_setpoint_trap = trapez_get_pos(trapez_steps);
}

void W_StepResponse::init_ctrl_gains(void)
{
    int i = 0, j = 0;
    for(i = 0; i < STEP_CONTROLLERS; i++)
    {
        for(j = 0; j < STEP_GAIN_FIELDS; j++)
        {
            //All gains = 0:
            ctrl_gains[i][j] = 0;
        }
    }
}

void W_StepResponse::save_ctrl_gains(int controller, int16_t *gains)
{
    int i = 0;
    for(i = 0; i < STEP_GAIN_FIELDS; i++)
    {
        ctrl_gains[controller][i] = gains[i];
    }
}
//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_StepResponse::setController(uint8_t ctrl)
{
//    uint8_t info[2] = {PORT_USB, PORT_USB};
//    uint16_t numb = 0;

//    if(!ui->comboBoxCmdStyle->currentIndex())
//    {
//        //Prepare and send command:
//        tx_cmd_ctrl_mode_w(TX_N_DEFAULT, ctrl);
//        pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
//        emit writeCommand(numb, comm_str_usb, WRITE);
//    }
//    else
//    {
        ActPack.controller = ctrl;
        sendActPack();
//    }
}

void W_StepResponse::on_pushButton_SetController_clicked()
{
    int16_t ctrl = CTRL_NONE;

    selected_controller = ui->comboBox_ctrl_list->currentIndex();
    wanted_controller = selected_controller;

    switch(wanted_controller)
    {
        case 0: //Open
            ctrl = CTRL_OPEN;
            ui->label_55->setText("Voltage (mV)");
            ui->label_59->setText("Min (mV)");
            ui->label_60->setText("Max (mV)");
            ui->control_slider_min->setText("0");
            ui->control_slider_max->setText("1000");
            ui->step_sliderNumSteps->setText("0");
            ui->step_sliderTime->setText("0");
            update_CtrlMinMax();
            break;
        case 1: //Current
            ctrl = CTRL_CURRENT;
            ui->label_55->setText("Current (mA)");
            ui->label_59->setText("Min (mA)");
            ui->label_60->setText("Max (mA)");
            ui->control_slider_min->setText("0");
            ui->control_slider_max->setText("1000");
            ui->step_sliderNumSteps->setText("0");
            ui->step_sliderTime->setText("0");
            update_CtrlMinMax();
            break;
        default:
            ctrl = CTRL_NONE;
            break;
    }

    setController(ctrl);

    //Notify user:
    QString msg;
    msg = "Active controller: " + var_list_controllers.at(wanted_controller);
    ui->statusController->setText(msg);
}

void W_StepResponse::on_pushButton_UpdateGains_clicked()
{
    int16_t gains[4] = {0,0,0,0};
//    uint16_t numb = 0;
      uint16_t valid = 0;
//    uint8_t info[2] = {PORT_USB, PORT_USB};

    //Save gains in temp variables:
    gains[0] = ui->lineEdit_Gain0->text().toInt();
    gains[1] = ui->lineEdit_Gain1->text().toInt();
    gains[2] = ui->lineEdit_Gain2->text().toInt();
    gains[3] = 0;

    ui->lineEdit_Gain0->clear();
    ui->lineEdit_Gain1->clear();
    ui->lineEdit_Gain2->clear();

    //Send command to hardware:

    //Different controllers have different gain functions:
    selected_controller = ui->comboBox_ctrl_list->currentIndex();
    switch(selected_controller)
    {
        case 0: //Open
            valid = 0;
            break;
        case 1: //Current
            valid = 1;
            save_ctrl_gains(selected_controller, gains);
            tx_cmd_ctrl_i_g_w(TX_N_DEFAULT, gains[0], gains[1], gains[2]);
            break;
        default:
            valid = 0;
            break;
    }

    refreshStatusGain();

    if(valid)
    {
        qDebug() << "Valid controller.";
        ActPack.setGains = CHANGE;
        ActPack.g0 = gains[0];
        ActPack.g1 = gains[1];
        ActPack.g2 = gains[2];
        ActPack.g3 = gains[3];

//        if(!ui->comboBoxCmdStyle->currentIndex())
//        {
//            //Common for all gain functions:
//            pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
//            emit writeCommand(numb, comm_str_usb, WRITE);
//        }
//        else
//        {
            sendActPack();
//        }
    }
    else
    {
        qDebug() << "Invalid controller, no gains set.";
    }
}

void W_StepResponse::refreshStatusGain(void)
{
    QString str;

    str = "<i>Current Control Gains: I = [" + \
            QString::number(ctrl_gains[1][0]) + ", " + \
            QString::number(ctrl_gains[1][1]) + ", " + \
            QString::number(ctrl_gains[1][2]) + "]</i>";

    ui->label_CurrentGains->setText(str);
    qDebug() << str;
}

void W_StepResponse::on_pushButton_setp_a_go_clicked()
{
    if (!ui->pushButton_setp_a_go->text().compare("Run Test")) {
        stopMotor();
        int val = 0;
        step_toggleStepDuration = ui->step_time->text().toFloat();
        val = ui->step_setp_a->text().toInt();
        qDebug() << "Step response set to" << val <<".";

        step_setpoint = ui->step_setp_a->text().toInt();
        trap_posi = ui->labelDispEncoder->text().toInt();
        trap_posf = ui->step_setp_a->text().toInt();
        trap_pos = step_setpoint;

        controller_setpoint(val);
        QTimer::singleShot((int) (step_toggleStepDuration * 1000), this, SLOT(stopMotor()));
        ui->pushButton_setp_a_go->setText("Stop");
    }
    else {
        stopMotor();
    }
}

void W_StepResponse::on_comboBox_slave_currentIndexChanged(int index)
{
	(void)index;	//Unused for now

	qDebug() << "Changed active slave";
	active_slave_index = ui->comboBox_slave->currentIndex();
	active_slave = FlexSEA_Generic::getSlaveID(SL_BASE_EX, active_slave_index);
}

void W_StepResponse::on_pushButton_updateTime_clicked() {

    step_toggleStepDuration = ui->step_time->text().toFloat();
    qDebug() << "Test duration set to" << step_toggleStepDuration <<" s.";
}

void W_StepResponse::on_pushButton_Stop_clicked() {
    ui->control_slider_min->setText("0");
    stopMotor();
}

void W_StepResponse::on_pushButton_Run_clicked() {
    if (enabledFreqControl && step_numberSteps > 0) {
        vStep = (step_lastMax - step_lastMin)/step_numberSteps;
        freqStep = (freq_lastMax - freq_lastMin)/step_numberSteps;
        atV = step_lastMin;
        atFreq = freq_lastMin;
        freq_controller_setpoint(atV);
    } else if (step_numberSteps > 0) {
        ptimerCtrl = new QTimer(this);
        vStep = (step_lastMax - step_lastMin)/step_numberSteps;
        atV = step_lastMin;
        connect(ptimerCtrl, SIGNAL(timeout()), this, SLOT(control_timer()));
        ptimerCtrl->start((int) (step_stepDuration * 1000));
    } else {
        qDebug() << "Number of steps must be greater than 0.";
    }
}

void W_StepResponse::on_pushButton_RunLog_clicked() {
    //slave Rigid must be at index 0 of slave comm window!
    int *commSlaveID;
    int temp = -1;
    commSlaveID = &temp;

    if (comPortStatus == PortOpeningSucceed) {
        pslaveComm->getSlaveId(commSlaveID);
        if (active_slave == *commSlaveID) {
            if (step_numberSteps > 0 && step_lastMax >= step_lastMin) {
                QString testString = QString("Active Controller: ") + QString::number(wanted_controller) + QString(". ")
                        + QString::number(step_numberSteps) + QString(" steps from ")
                        + QString::number(step_lastMin) + QString(" to ") + QString::number(step_lastMax);
                dataLogging = 1;
                //6 for 100Hz log, 7 for 200Hz
                pslaveComm->startExperiment(6, true, false, "o=0,1,2,3;", testString);
                on_pushButton_Run_clicked();
            } else {
                qDebug() << "Specify valid test parameters.";
            }
        } else {
            qDebug() << "Current device must be the same on both Slave Comm window(in first slot) and this window.";
        }
    } else {
        qDebug() << "Com port closed. Open and retry.";
    }
}

void W_StepResponse::control_timer() {
    if (atStep < step_numberSteps) {
        qDebug() << "Input set to" << atV << ".";
        ui->disp_slider->setText(QString::number(atV));
        controller_setpoint(atV);
        atStep += 1;
        atV += vStep;
    } else {
        stopMotor();
    }
}

void W_StepResponse::stopMotor() {
    controller_setpoint(0);
    if (dataLogging) {
        dataLogging = 0;
        pslaveComm->stopExperiment();
    }
    ui->slider_Freq->setValue(freq_lastMin);
    atStep = 0;
    atV = 0;
    vStep = 0;
    withinSweepTime = 0;
    freqStep = 0;
    ptimerCtrl->stop();
    ui->pushButton_setp_a_go->setText("Run Test");
    qDebug() << "Motor stopped!";
}

void W_StepResponse::on_pushButton_UpdateSlider_clicked() {
    update_CtrlMinMax();
}

void W_StepResponse::update_CtrlMinMax()
{
    stopMotor();
    update_FreqMinMax();
    //Get min & max, steps, step duration, and update slider limits:
    int min = ui->control_slider_min->text().toInt();
    int max = ui->control_slider_max->text().toInt();

    //update number of steps
    int stepsInput = ui->step_sliderNumSteps->text().toInt();
    if (stepsInput > 0) {
        step_numberSteps = stepsInput;
    } else {
        step_numberSteps = 0;
    }
    ui->step_sliderNumSteps->setText(QString::number(step_numberSteps));

    //update duration of each step
    float stepsDuration = ui->step_sliderTime->text().toFloat();
    if (stepsDuration > 0) {
        step_stepDuration = stepsDuration;
    } else {
        step_stepDuration = 0;
    }
    ui->step_sliderTime->setText(QString::number(step_stepDuration));

    if(step_lastMin != min || step_lastMax != max)
    {
        //Safety:
        if(max < 0) {
            ui->control_slider_min->setText(QString::number(step_lastMin));
            ui->control_slider_max->setText(QString::number(step_lastMax));
        } else if (max < min) {
            ui->control_slider_min->setText(QString::number(0));
            ui->hSlider_Ctrl->setValue(0);
            ui->hSlider_Ctrl->setMinimum(0);
            ui->hSlider_Ctrl->setMaximum(max);
            step_lastMin = 0;
            step_lastMax = max;
        } else {
            if (ui->hSlider_Ctrl->value() > max) {
                ui->hSlider_Ctrl->setValue(0);
            }
            ui->hSlider_Ctrl->setMinimum(min);
            ui->hSlider_Ctrl->setMaximum(max);
            step_lastMin = min;
            step_lastMax = max;
        }

        //Reset button's color:
        ui->pushButton_UpdateSlider->setStyleSheet("");
    }

}

void W_StepResponse::update_FreqMinMax() {
    if (enabledFreqControl) {
        float min = ui->lineEdit_MinFreq->text().toFloat();
        float max = ui->lineEdit_MaxFreq->text().toFloat();

        if(freq_lastMin != min || freq_lastMax != max)
        {
            //Safety:
            if(min < 0 || max < 0) {
                ui->lineEdit_MinFreq->setText(QString::number(freq_lastMin));
                ui->lineEdit_MaxFreq->setText(QString::number(freq_lastMax));
            } else if (max < min) {
                ui->lineEdit_MinFreq->setText(QString::number(0));
                ui->lineEdit_MaxFreq->setText(QString::number(max));
                ui->slider_Freq->setValue(0);
                ui->slider_Freq->setMinimum(0);
                ui->slider_Freq->setMaximum(max);
                freq_lastMin = 0;
                freq_lastMax = max;
            } else {
                if (ui->slider_Freq->value() > max) {
                    ui->slider_Freq->setValue(0);
                }
                ui->slider_Freq->setMinimum(min);
                ui->slider_Freq->setMaximum(max);
                freq_lastMin = min;
                freq_lastMax = max;
            }
        }
    }
}

void W_StepResponse::on_hSlider_Ctrl_valueChanged(int value)
{
    (void)value;	//Unused for now

    int val = 0;
    val = ui->hSlider_Ctrl->value();
    ui->disp_slider->setText(QString::number(val));
    step_setpoint = val;

    //zero out step duration and numSteps
    step_numberSteps = 0;
    ui->step_sliderNumSteps->setText(QString::number(step_numberSteps));
    step_stepDuration = 0;
    ui->step_sliderTime->setText(QString::number(step_stepDuration));

    //When we move the slider we do not use trapeze, we just "slip" the setpoint
    trap_pos = val;
    trap_posi = val;
    trap_posf = val;

    if (enabledFreqControl) {
        freq_controller_setpoint(val);
    } else {
        controller_setpoint(val);
    }

    //Wait 2ms to avoid sending a million packets when we move the slider
    usleep(2000);
}

void W_StepResponse::on_control_slider_min_textEdited(const QString &arg1)
{
    (void)arg1;
    minMaxTextChanged();
}

void W_StepResponse::on_control_slider_max_textEdited(const QString &arg1)
{
    (void)arg1;
    minMaxTextChanged();
}

void W_StepResponse::minMaxTextChanged(void)
{
    ui->pushButton_UpdateSlider->setStyleSheet("background-color: rgb(255, 255, 0); \
                                               color: rgb(0, 0, 0)");
}

void W_StepResponse::on_control_slider_min_editingFinished()
{
    update_CtrlMinMax();
}

void W_StepResponse::on_control_slider_max_editingFinished()
{
    update_CtrlMinMax();
}

void W_StepResponse::on_checkBox_Freq_toggled() {
    enabledFreqControl = !enabledFreqControl;

    if (enabledFreqControl) {
        ui->lineEdit_MinFreq->setEnabled(true);
        ui->lineEdit_MaxFreq->setEnabled(true);
        ui->slider_Freq->setEnabled(true);
    } else {
        ui->lineEdit_MinFreq->setEnabled(false);
        ui->lineEdit_MaxFreq->setEnabled(false);
        ui->slider_Freq->setEnabled(false);
    }
}

void W_StepResponse::on_slider_Freq_valueChanged() {
    ui->label_Freq->setText(QString::number(ui->slider_Freq->value()));
    atFreq = ui->slider_Freq->value();
}

