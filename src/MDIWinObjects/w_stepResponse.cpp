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
#include <QTimer>
#include <QDebug>
#include <flexsea_comm.h>
#include <cmd-ActPack.h>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_StepResponse::W_StepResponse(QWidget *parent) :
	QWidget(parent),
    ui(new Ui::W_StepResponse)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

    initStep();
}

W_StepResponse::~W_StepResponse()
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

    //update slider vals
    ui->hSlider_Ctrl->setValue(step_lastMin);
    ui->hSlider_Ctrl->setMinimum(step_lastMin);
    ui->hSlider_Ctrl->setMaximum(step_lastMax);
    ui->disp_slider->setText("Off");
    ui->control_slider_min->setText(QString::number(step_lastMin));
    ui->control_slider_max->setText(QString::number(step_lastMax));


	//Variable option lists:
    var_list_controllers << "Open";
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

void W_StepResponse::save_ctrl_gains(int controller, int16_t *gains)
{
	int i = 0;
	for(i = 0; i < GAIN_FIELDS; i++)
	{
        step_gains[controller][i] = gains[i];
	}
}

void W_StepResponse::controller_setpoint(int val)
{
//	uint16_t numb = 0;
    uint16_t valid = 0;
//	uint8_t info[2] = {PORT_USB, PORT_USB};

	qDebug() << "Entered controller_setpoint()";

	switch(wanted_controller)
	{
        case 0: //Open
			valid = 1;
			tx_cmd_ctrl_o_w(TX_N_DEFAULT, val);
			qDebug() << "Open: " << val << "mV";
			break;
		default:
			valid = 0;
			break;
	}

	if(valid)
	{
		ActPack.setpoint = val;
        sendActPack();
	}
	else
	{
		qDebug() << "Invalid controller selected.";
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
//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_StepResponse::setController(uint8_t ctrl)
{
    uint8_t info[2] = {PORT_USB, PORT_USB};
    uint16_t numb = 0;

    if(!ui->comboBoxCmdStyle->currentIndex())
    {
        //Prepare and send command:
        tx_cmd_ctrl_mode_w(TX_N_DEFAULT, ctrl);
        pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
        emit writeCommand(numb, comm_str_usb, WRITE);
    }
    else
    {
        ActPack.controller = ctrl;
        sendActPack();
    }
}

void W_StepResponse::on_pushButton_setp_a_go_clicked()
{
    if (!ui->pushButton_setp_a_go->text().compare("Go")) {
        int val = 0;
        val = ui->step_setp_a->text().toInt();
        qDebug() << "Supply voltage set to" << val <<" mV.";

        step_setpoint = ui->step_setp_a->text().toInt();
        trap_posi = ui->labelDispEncoder->text().toInt();
        trap_posf = ui->step_setp_a->text().toInt();
        trap_pos = step_setpoint;

        controller_setpoint(val);
        QTimer::singleShot((int) step_toggleStepDuration * 1000, this, SLOT(stopMotor()));
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
    ui->hSlider_Ctrl->setValue(0);
    ui->hSlider_Ctrl->setMinimum(0);
    step_lastMin = 0;
    controller_setpoint(0);
    qDebug() << "Motor stopped!";
}

void W_StepResponse::stopMotor() {
    controller_setpoint(0);
    ui->pushButton_setp_a_go->setText("Go");
    qDebug() << "Motor stopped!";
}

void W_StepResponse::on_pushButton_UpdateSlider_clicked() {
    update_CtrlMinMax();
}

void W_StepResponse::update_CtrlMinMax()
{
    //Get min & max, steps, step duration, and update slider limits:
    int min = ui->control_slider_min->text().toInt();
    int max = ui->control_slider_max->text().toInt();

    int stepsInput = ui->step_sliderNumSteps->text().toInt();
    if (stepsInput > 0) {
        step_numberSteps = stepsInput;
    } else {
        step_numberSteps = 0;
    }
    ui->step_sliderNumSteps->setText(QString::number(step_numberSteps));

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
        if(min < 0 || max < 0) {
            ui->control_slider_min->setText(QString::number(step_lastMin));
            ui->control_slider_max->setText(QString::number(step_lastMax));
        } else if (max < min) {
            ui->control_slider_min->setText(QString::number(0));
            ui->hSlider_Ctrl->setValue(0);
            ui->hSlider_Ctrl->setMinimum(0);
            ui->hSlider_Ctrl->setMaximum(max);
            step_lastMin = min;
            step_lastMax = max;
        } else {
            if (ui->hSlider_Ctrl->value() > max) {
                ui->hSlider_Ctrl->setValue(min);
            }
            ui->hSlider_Ctrl->setMinimum(min);
            ui->hSlider_Ctrl->setMaximum(max);
            step_lastMax = max;
        }

        //Reset button's color:
        ui->pushButton_UpdateSlider->setStyleSheet("");
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
    controller_setpoint(val);

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


