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

#ifndef W_GAINS_COMMAND_H
#define W_GAINS_COMMAND_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QWidget>
#include <QTimer>
#include "counter.h"
#include "rigidDevice.h"
#include "state_machine.h"
#include "state_variables.h"
#include "define.h"

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************

namespace Ui {
class W_GainsCommand;
}

class W_GainsCommand : public QWidget, public Counter<W_GainsCommand>
{
	Q_OBJECT

public:
	//Constructor & Destructor:
    explicit W_GainsCommand(QWidget *parent = 0,
					  FlexseaDevice *currentLog = nullptr,
					  RigidDevice *deviceLogPtr = nullptr,
					  DisplayMode mode = DisplayLiveData,
					  QList<RigidDevice> *deviceListPtr = nullptr);
    ~W_GainsCommand();

	//Function(s):

    void initWindow(void);

    //Structure copied from user-mn-MIT-DLeg-2dof
    typedef struct act_s
    {
        float jointAngle;
        float jointAngleDegrees;
        float jointVel;
        float jointVelDegrees;
        float jointAcc;
        float linkageMomentArm;
        float axialForce;
        float jointTorque;
        int32_t motorVel;		// motor velocity [rad/s]
        int32_t motorAcc;		// motor acceleration [rad/s/s]
        int16_t regTemp;		// regulate temperature
        int16_t motTemp;		// motor temperature
        int32_t motCurr;		// motor current
        int32_t desiredCurrent; // desired current from getMotorCurrent()
        int32_t currentOpLimit; // current throttling limit
        int8_t safetyFlag;		// todo: consider if necessary
    } Act_s;

public slots:
    void refreshAllVals(void);
    void updateSysVals(void);
    void on_comboBox_stateSelect_currentIndexChanged(int index);
    void on_pushButton_updateParams_clicked();
    void on_comboBox_slave_currentIndexChanged(int index);
    void on_pushButton_currentScalar_clicked();
    void on_pushButton_findPoles_clicked();

signals:
	void windowClosed(void);
    void writeCommand(uint8_t numb, uint8_t *tx_data, uint8_t r_w);

private:
	//Variables & Objects:
    Ui::W_GainsCommand *ui;

	DisplayMode displayMode;
    QTimer* pupdateTimer;

	QList<RigidDevice> *deviceList;
    QStringList var_list_states;
    RigidDevice *deviceLog;
    int32_t currentEncVal = 0;
    int active_slave, active_slave_index;

    //Function(s):
    void display(RigidDevice *devicePtr, int index);
    void refreshEswVals(void);
    void refreshLswVals(void);
    void refreshEstVals(void);
    void refreshLstPVals(void);
    void updateState(void);
};

//****************************************************************************
// Definition(s)
//****************************************************************************

#endif // W_GAINS_COMMAND_H
