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
    [This file] W_GainsCommand: FSM Gain Control
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
    * 2018-03-08 | tonyshu@mit.edu | New code
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_gainsCommand.h"
#include "ui_w_gainsCommand.h"
#include "flexsea_cmd_user.h"
#include "flexsea_generic.h"
#include "flexsea_system.h"
#include "flexsea_board.h"
#include "flexsea_user_structs.h"
#include "flexsea.h"
#include "cmd-DLeg.h"
#include "state_machine.h"
#include "state_variables.h"
#include <QDebug>
#include <QTimer>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_GainsCommand::W_GainsCommand(QWidget *parent,
				   FlexseaDevice *currentLog,
				   RigidDevice *deviceLogPtr,
				   DisplayMode mode,
				   QList<RigidDevice> *deviceListPtr) :
                   QWidget(parent), ui(new Ui::W_GainsCommand)
{
	ui->setupUi(this);

	deviceLog  = deviceLogPtr;
	deviceList = deviceListPtr;

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

    initWindow();
}

W_GainsCommand::~W_GainsCommand()
{
	emit windowClosed();
	delete ui;
}

//****************************************************************************
// Public function(s):
//****************************************************************************
void W_GainsCommand::initWindow(void) {
    //set current state null
    QFont font("Arial", 30, QFont::Bold);
    ui->label_currentState->setAutoFillBackground(true);
    ui->label_currentState->setFont(font);
    ui->label_currentState->setText("None");

    //populate slave comboBox

    FlexSEA_Generic::populateSlaveComboBox(ui->comboBox_slave, SL_BASE_EX, \
                                            SL_LEN_EX+SL_LEN_MN);
    //Variables:
    active_slave_index = ui->comboBox_slave->currentIndex();
    active_slave = FlexSEA_Generic::getSlaveID(SL_BASE_EX, active_slave_index);

    //populate drop down selector
    var_list_states << "Early Swing" << "Late Swing" << "Early Stance" \
                    << "Late Stance" << "Late Stance Power";
    for(int index = 0; index < var_list_states.count(); index++)
    {
        ui->comboBox_stateSelect->addItem(var_list_states.at(index));
    }

    //populate view gains window with current gains
    refreshAllVals();

    //create timer for value refresh
    pupdateTimer = new QTimer(this);
    connect(pupdateTimer, SIGNAL(timeout()), this, SLOT(updateSysVals()));
    //ms update time
    pupdateTimer->start(200);

}
//****************************************************************************
// Public slot(s):
//****************************************************************************
void W_GainsCommand::refreshAllVals(void) {

    refreshEswVals();
    refreshLswVals();
    refreshEstVals();
    refreshLstPVals();
    updateState();
}

void W_GainsCommand::on_comboBox_stateSelect_currentIndexChanged(int index) {
    ui->lineEdit_k1->setText(QString::number(stateGains[index]->k1));
    ui->lineEdit_k2->setText(QString::number(stateGains[index]->k2));
    ui->lineEdit_b->setText(QString::number(stateGains[index]->b));
    ui->lineEdit_thetaEq->setText(QString::number(stateGains[index]->thetaDes));
}

void W_GainsCommand::on_pushButton_updateParams_clicked() {
    //update gains on Plan
    int stateIndex = ui->comboBox_stateSelect->currentIndex();

    uint16_t oldk1 = stateGains[stateIndex]->k1;
    uint16_t oldk2 = stateGains[stateIndex]->k2;
    uint16_t oldb = stateGains[stateIndex]->b;
    uint16_t oldThetaDes = stateGains[stateIndex]->thetaDes;

    stateGains[stateIndex]->k1 = ui->lineEdit_k1->text().toFloat();
    stateGains[stateIndex]->k2 = ui->lineEdit_k2->text().toFloat();
    stateGains[stateIndex]->b = ui->lineEdit_b->text().toFloat();
    stateGains[stateIndex]->thetaDes = ui->lineEdit_thetaEq->text().toFloat();

    //setup and send serial
    uint8_t info[2] = {PORT_USB, PORT_USB};
    uint16_t numb = 0;

    tx_cmd_dleg_rw(TX_N_DEFAULT, stateIndex);

    //If Manage side update not successful, refresh will show old values
    stateGains[stateIndex]->k1 = oldk1;
    stateGains[stateIndex]->k2 = oldk2;
    stateGains[stateIndex]->b = oldb;
    stateGains[stateIndex]->thetaDes = oldThetaDes;

    refreshAllVals();

    pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
    emit writeCommand(numb, comm_str_usb, WRITE);
}

void W_GainsCommand::on_comboBox_slave_currentIndexChanged(int index) {
    (void)index;	//Unused for now

    qDebug() << "Changed FSM gains active slave";
    active_slave_index = ui->comboBox_slave->currentIndex();
    active_slave = FlexSEA_Generic::getSlaveID(SL_BASE_EX, active_slave_index);

    refreshAllVals();
}

void W_GainsCommand::on_pushButton_currentScalar_clicked() {
    float oldScalar = currentScalarPlan;
    currentScalarPlan = ui->lineEdit_currentScalar->text().toFloat();

    //set limits
    if (currentScalarPlan > 1) {
        currentScalarPlan = 1;
    } else if (currentScalarPlan < 0) {
        currentScalarPlan = 0;
    }

    uint8_t info[2] = {PORT_USB, PORT_USB};
    uint16_t numb = 0;

    uint8_t stateIndex = 0;
    tx_cmd_dleg_rw(TX_N_DEFAULT, stateIndex);
    pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
    emit writeCommand(numb, comm_str_usb, WRITE);

    // this will update upon reply from manage
    currentScalarPlan = oldScalar;
}

void W_GainsCommand::on_pushButton_findPoles_clicked() {
    fsm1StatePlan = -1; //user MIT DLeg state for findPoles()

    uint8_t info[2] = {PORT_USB, PORT_USB};
    uint16_t numb = 0;

    uint8_t stateIndex = 0;
    tx_cmd_dleg_rw(TX_N_DEFAULT, stateIndex);
    pack(P_AND_S_DEFAULT, active_slave, info, &numb, comm_str_usb);
    emit writeCommand(numb, comm_str_usb, WRITE);
}


//****************************************************************************
// Private function(s):
//****************************************************************************

void W_GainsCommand::refreshEswVals(void) {
    ui->label_eswk1->setText(QString::number(eswGains.k1));
    ui->label_eswk2->setText(QString::number(eswGains.k2));
    ui->label_eswb->setText(QString::number(eswGains.b));
    ui->label_eswThetaEq->setText(QString::number(eswGains.thetaDes));
}

void W_GainsCommand::refreshLswVals(void) {
    ui->label_lswk1->setText(QString::number(lswGains.k1));
    ui->label_lswk2->setText(QString::number(lswGains.k2));
    ui->label_lswb->setText(QString::number(lswGains.b));
    ui->label_lswThetaEq->setText(QString::number(lswGains.thetaDes));
}

void W_GainsCommand::refreshEstVals(void) {
    ui->label_estk1->setText(QString::number(estGains.k1));
    ui->label_estk2->setText(QString::number(estGains.k2));
    ui->label_estb->setText(QString::number(estGains.b));
    ui->label_estThetaEq->setText(QString::number(estGains.thetaDes));
}

void W_GainsCommand::refreshLstPVals(void) {
    ui->label_lstPk1->setText(QString::number(lstPowerGains.k1));
    ui->label_lstPk2->setText(QString::number(lstPowerGains.k2));
    ui->label_lstPb->setText(QString::number(lstPowerGains.b));
    ui->label_lstPThetaEq->setText(QString::number(lstPowerGains.thetaDes));
}

void W_GainsCommand::updateSysVals(void) {
    ui->label_jointAngle->setText(QString::number(act1.jointAngleDegrees, 'f', 2));
    ui->label_jointVelocity->setText(QString::number(act1.jointVelDegrees, 'f', 2));
    ui->label_momentArm->setText(QString::number(act1.linkageMomentArm, 'f', 2));
    ui->label_axialForce->setText(QString::number(act1.axialForce, 'f', 2));
    ui->label_jointTorque->setText(QString::number(act1.jointTorque, 'f', 2));
    ui->label_reflectMotor->setText(QString::number(act1.tauMeas, 'f', 2));
    ui->label_desMotor->setText(QString::number(act1.tauDes, 'f', 0));
    ui->label_desCurrent->setText(QString::number(act1.desiredCurrent, 'f', 0));
    ui->label_currentOpLimit->setText(QString::number(act1.currentOpLimit));

    QString safeMessage;
    ui->label_safetyFlag->setStyleSheet("QLabel { background-color : red; color : white; }");

    switch(act1.safetyFlag) {

        case 0:
            safeMessage = "OK";
            ui->label_safetyFlag->setStyleSheet("QLabel { background-color : green; color : white; }");
        break;

        case 1:
            safeMessage = ">ANGLE!";
        break;

        case 2:
            safeMessage = ">TORQUE!";
        break;

        case 3:
            safeMessage = ">TEMP!";
        break;

        case 4:
            safeMessage = ">TEMP!";
        break;

        default:
            safeMessage = "COMM ERROR";
        break;
    }

    ui->label_safetyFlag->setText(safeMessage);
}

void W_GainsCommand::updateState(void) {
    QPalette pal = palette();

    switch (stateMachine.current_state) {

        qDebug() << QString::number(stateMachine.current_state);
        case STATE_IDLE:

            ui->label_currentState->setText("Idle");
            pal.setColor(QPalette::Background, Qt::darkGray);

            break;

        case STATE_INIT:

            ui->label_currentState->setText("Init");
            pal.setColor(QPalette::Background, Qt::gray);

            break;

        case STATE_EARLY_SWING:

            ui->label_currentState->setText("Early Swing");
            pal.setColor(QPalette::Background, Qt::green);

            break;

        case STATE_LATE_SWING:

            ui->label_currentState->setText("Late Swing");
            pal.setColor(QPalette::Background, Qt::cyan);

            break;

        case STATE_EARLY_STANCE:

            ui->label_currentState->setText("Early Stance");
            pal.setColor(QPalette::Background, Qt::magenta);

            break;

        case STATE_LATE_STANCE:

            ui->label_currentState->setText("Late Stance");
            pal.setColor(QPalette::Background, Qt::yellow);

            break;

        case STATE_LATE_STANCE_POWER :

            ui->label_currentState->setText("Late Stance Power");
            pal.setColor(QPalette::Background, Qt::red);

            break;

        default:
            ui->label_currentState->setText("Undefined");
            pal.setColor(QPalette::Background, Qt::lightGray);

            break;
    }

    ui->label_currentState->setPalette(pal);
    ui->label_currentState->update();
}
//****************************************************************************
// Private slot(s):
//****************************************************************************
