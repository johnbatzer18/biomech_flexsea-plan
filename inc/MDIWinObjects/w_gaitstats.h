/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'plan-gui' Graphical User Interface
	Copyright (C) 2018 Dephy, Inc. <http://dephy.com/>

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
	[This file] w_gaitstats.h: Gaits Statistics
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2018-02-10 | sbelanger | New code, initial release
	*
****************************************************************************/

#ifndef W_GAITSTATS_H
#define W_GAITSTATS_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QWidget>
#include "counter.h"
#include "flexsea_generic.h"
#include "serialdriver.h"
#include <dynamicuserdatamanager.h>
#include <QListWidgetItem>

//****************************************************************************
// Namespace & Class Definition:
//****************************************************************************

namespace Ui {
class W_GaitStats;
}

class W_GaitStats : public QWidget, public Counter<W_GaitStats>
{
	Q_OBJECT

public:
	//Constructor & Destructor:
	explicit W_GaitStats(QWidget *parent = 0, DynamicUserDataManager* userDataManager = nullptr);
	~W_GaitStats();

	void setUserCustomRowHidden(int row, bool shouldHide);

public slots:
	void receiveNewData();
	void comStatusChanged(SerialPortStatus status,int nbTries);

signals:
	void windowClosed(void);
	void writeCommand(uint8_t numb, uint8_t *tx_data, uint8_t r_w);

private slots:
	void on_pb_ClearRow_clicked();
	void on_pb_Refresh_clicked();

private:
	//Variables & Objects:
	Ui::W_GaitStats *ui;
	int active_slave, active_slave_index;
	QTimer *refreshDelayTimer;
	DynamicUserDataManager* userDataMan;

	//Function(s):
	void init(void);
	void writeUserData(uint8_t index);
	void readUserData(void);
};

//****************************************************************************
// Definition(s)
//****************************************************************************

#endif // W_GAITSTATS_H
