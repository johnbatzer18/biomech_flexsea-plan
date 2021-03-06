//****************************************************************************
// MIT Media Lab - Biomechatronics
// Jean-Francois (Jeff) Duval
// jfduval@media.mit.edu
// 05/2016
//****************************************************************************
// MainWindow: Qt GUI Main file - tab:Stream 5 (In Control)
//****************************************************************************

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_incontrol.h"
#include "ui_w_incontrol.h"
#include "flexsea_cmd_in_control.h"
#include "flexsea_system.h"

W_InControl::W_InControl(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::W_InControl)
{
	ui->setupUi(this);

	//Populates Slave list:
	FlexSEA_Generic::populateSlaveComboBox(ui->comboBox_slave, SL_BASE_EX, \
											SL_LEN_EX);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	init_tab_stream_in_ctrl();
}

W_InControl::~W_InControl()
{
	emit windowClosed();
	delete ui;
}

int W_InControl::getActiveSlave() const
{
	int active_slave_index = ui->comboBox_slave->currentIndex();
	return FlexSEA_Generic::getSlaveID(SL_BASE_EX, active_slave_index);
}

void W_InControl::init_tab_stream_in_ctrl(void)
{
	ui->inctrl_w0->setText("0");
	ui->inctrl_w1->setText("0");
	ui->inctrl_w2->setText("0");
	ui->inctrl_w3->setText("0");
}

void W_InControl::updateUIData(void)
{
		//Raw values:

		//ui->disp_inctrl_active_controller->setText(var_list_controllers.at(in_control_1.controller));
		ui->disp_inctrl_setp->setText(QString::number(in_control_1.setp));
		ui->disp_inctrl_actual_val->setText(QString::number(in_control_1.actual_val));
		ui->disp_inctrl_error->setText(QString::number(in_control_1.error));
		ui->disp_inctrl_pwm->setText(QString::number(in_control_1.pwm));

		ui->disp_inctrl_output->setText(QString::number(in_control_1.output));
		ui->disp_inctrl_dir->setText(QString::number(in_control_1.mot_dir));

		ui->disp_inctrl_current->setText(QString::number(in_control_1.current));

		ui->disp_inctrl_0->setText(QString::number(in_control_1.r[0]));

		ui->disp_inctrl_1->setText(QString::number(in_control_1.r[1]));
}

void W_InControl::stream_in_ctrl(void)
{
}


void W_InControl::on_pushButton_inctrl_w0_clicked()
{
//    in_control_1.w[0] = ui->inctrl_w0->text().toInt();
//    write_in_control_w(0);
}

void W_InControl::on_pushButton_inctrl_w1_clicked()
{
//    in_control_1.w[1] = ui->inctrl_w1->text().toInt();
//    write_in_control_w(1);
}

void W_InControl::on_pushButton_inctrl_w2_clicked()
{
//    in_control_1.w[2] = ui->inctrl_w2->text().toInt();
//    write_in_control_w(2);
}

void W_InControl::on_pushButton_inctrl_w3_clicked()
{
//    in_control_1.w[3] = ui->inctrl_w3->text().toInt();
//    write_in_control_w(3);
}

void W_InControl::write_in_control_w(uint8_t var)
{
	(void)var;
//    int numb = 0;

//    numb = tx_cmd_in_control(active_slave_1, CMD_WRITE, payload_str, PAYLOAD_BUF_LEN, var);
//    numb = comm_gen_str(payload_str, comm_str_usb, PAYLOAD_BUF_LEN);
//    numb = COMM_STR_BUF_LEN;

//    USBSerialPort_Write(numb, comm_str_usb);        //QSerialPort
}
