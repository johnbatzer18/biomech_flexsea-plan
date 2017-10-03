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

	initSigBox();
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

//****************************************************************************
// Private function(s):
//****************************************************************************

void W_UserTesting::initSigBox(void)
{
	QColor penColor;
	penColor.setNamedColor("black");
	scribbleArea = new ScribbleArea;
	ui->gridLayout_Sig->addWidget(scribbleArea);
	scribbleArea->setPenColor(penColor);
	scribbleArea->setPenWidth(5);
}

//****************************************************************************
// Private slot(s):
//****************************************************************************

void W_UserTesting::on_pushButtonApprove_clicked()
{
	//ToDo use real file location?
	QString fn = userID + ".png";
	qDebug() << fn;
	scribbleArea->saveImage(fn, "png");
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
}
