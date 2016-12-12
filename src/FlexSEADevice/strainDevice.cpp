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
	[This file] StrainDevice: Strain Device Data Class
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2016-12-10 | sbelanger | Initial GPL-3.0 release
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "strainDevice.h"
#include "flexsea_generic.h"
#include <QDebug>
#include <QTextStream>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

StrainDevice::StrainDevice(enum DataSourceFile dataSourceInit): FlexseaDevice()
{
	this->dataSource = dataSourceInit;
}

//****************************************************************************
// Public function(s):
//****************************************************************************

QString StrainDevice::getHeaderStr(void)
{
	return QString("Timestamp,") + \
					"Timestamp (ms)," + \
					"ch1," + \
					"ch2," + \
					"ch3," + \
					"ch4," + \
					"ch5," + \
					"ch6";
}

QString StrainDevice::getLastLineStr(void)
{
	QString str;
	QTextStream(&str) <<	stList.last().timeStampDate << ',' << \
							stList.last().timeStamp_ms << ',' << \
							stList.last().data.ch[0].strain_filtered << ',' << \
							stList.last().data.ch[1].strain_filtered << ',' << \
							stList.last().data.ch[2].strain_filtered << ',' << \
							stList.last().data.ch[3].strain_filtered << ',' << \
							stList.last().data.ch[4].strain_filtered << ',' << \
							stList.last().data.ch[5].strain_filtered;
	return str;
}

void StrainDevice::clear(void)
{
	FlexseaDevice::clear();
	stList.clear();
}

void StrainDevice::appendEmptyLine(void)
{
	stList.append(StrainStamp());
}

void StrainDevice::decodeLastLine(void)
{
	decode(&stList.last().data);
}

void StrainDevice::decodeAllLine(void)
{
	for(int i = 0; i < stList.size(); ++i)
	{
		decode(&stList[i].data);
	}
}

QString StrainDevice::getStatusStr(void)
{
	return QString("No decoding available for this board");
}

void StrainDevice::decode(struct strain_s *stPtr)
{
	stPtr->decoded.strain[0] = (100*(stPtr->ch[0].strain_filtered-STRAIN_MIDPOINT)/STRAIN_MIDPOINT);
	stPtr->decoded.strain[1] = (100*(stPtr->ch[1].strain_filtered-STRAIN_MIDPOINT)/STRAIN_MIDPOINT);
	stPtr->decoded.strain[2] = (100*(stPtr->ch[2].strain_filtered-STRAIN_MIDPOINT)/STRAIN_MIDPOINT);
	stPtr->decoded.strain[3] = (100*(stPtr->ch[3].strain_filtered-STRAIN_MIDPOINT)/STRAIN_MIDPOINT);
	stPtr->decoded.strain[4] = (100*(stPtr->ch[4].strain_filtered-STRAIN_MIDPOINT)/STRAIN_MIDPOINT);
	stPtr->decoded.strain[5] = (100*(stPtr->ch[5].strain_filtered-STRAIN_MIDPOINT)/STRAIN_MIDPOINT);
}

//****************************************************************************
// Public slot(s):
//****************************************************************************


//****************************************************************************
// Private function(s):
//****************************************************************************


//****************************************************************************
// Private slot(s):
//****************************************************************************

