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
	[This file] W_AnkleTorque.h: 2D Plot window
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-09-09 | jfduval | Initial GPL-3.0 release
	* 2016-09-12 | jfduval | Added Freeze/Release
	* 2016-12-1x | jfduval | Major refactoring
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "w_ankleTorque.h"
#include "ui_w_ankleTorque.h"
#include <QApplication>
#include <QtCharts>
#include <QtCharts/QChartView>
#include <QtCharts/QSplineSeries>
#include <QDebug>
#include <QElapsedTimer>
#include "flexsea_generic.h"
#include "main.h"

QT_CHARTS_USE_NAMESPACE

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

W_AnkleTorque::W_AnkleTorque(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::W_AnkleTorque)
{
	ui->setupUi(this);

	setWindowTitle(this->getDescription());
	setWindowIcon(QIcon(":icons/d_logo_small.png"));

	//Chart:
	chart = new QChart();
	chart->legend()->hide();

	for(int i = 0; i < 2; i++)
	{
		qlsChart[i] = new QLineSeries();
		qlsChart[i]->append(0, 0);
		chart->addSeries(qlsChart[i]);
	}

	chart->createDefaultAxes();
	chart->axisX()->setRange(-70, 30);
	chart->axisY()->setRange(-1000, 1000);

	//Colors:
	chart->setTheme(QChart::ChartThemeDark);

	//Chart view:
	chartView = new AnkleTorqueChartView(chart);
	for(int i = 0; i < ATCV_NUMPOINTS; i++)
	{
		chartView->setPoint(i, (float)(-60 + i*15), 0.0f);
	}

	ui->gridLayout_test->addWidget(chartView, 0,0);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->setBaseSize(600,300);
	chartView->setMinimumSize(500,300);
	chartView->setMaximumSize(4000,2500);
	chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	chart->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	QPixmapCache::setCacheLimit(100000);


}

W_AnkleTorque::~W_AnkleTorque()
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

void W_AnkleTorque::receiveNewData(void)
{

}

void W_AnkleTorque::refresh2DPlot(void)
{

}

void W_AnkleTorque::updateDisplayMode(DisplayMode mode, FlexseaDevice* devPtr)
{

}
