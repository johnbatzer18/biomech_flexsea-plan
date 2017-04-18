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
	[This file] rigidDevice: Rigid Device Class
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-04-18 | jfduval | New code
	*
****************************************************************************/

//****************************************************************************
// Include(s)
//****************************************************************************

#include "rigidDevice.h"
#include <QDebug>
#include <QTextStream>

//****************************************************************************
// Constructor & Destructor:
//****************************************************************************

RigidDevice::RigidDevice(void): FlexseaDevice()
{
	if(header.length() != headerDecoded.length())
	{
		qDebug() << "Mismatch between header length Rigid!";
	}

	this->dataSource = LogDataFile;
	serializedLength = header.length();
	slaveTypeName = "rigid";
}

RigidDevice::RigidDevice(rigid_s *devicePtr): FlexseaDevice()
{
	if(header.length() != headerDecoded.length())
	{
		qDebug() << "Mismatch between header length Rigid!";
	}

	this->dataSource = LiveDataFile;
	timeStamp.append(TimeStamp());
	riList.append(devicePtr);
	eventFlags.append(0);
	serializedLength = header.length();
	slaveTypeName = "rigid";
}

//****************************************************************************
// Public function(s):
//****************************************************************************

QString RigidDevice::getHeaderStr(void)
{
	return header.join(',');
}

QStringList RigidDevice::header = QStringList()
								<< "Timestamp"
								<< "Timestamp (ms)"
								<< "Event Flags"
								<< "Accel X"
								<< "Accel Y"
								<< "Accel Z"
								<< "Gyro X"
								<< "Gyro Y"
								<< "Gyro Z"
								<< "Magneto X"
								<< "Magneto X"
								<< "Magneto Z"
								<< "IO[0]"
								<< "IO[0]"
								<< "CapSense[1]"
								<< "CapSense[2]"
								<< "CapSense[3]"
								<< "CapSense[4]"
								<< "Status";

QStringList RigidDevice::headerDecoded = QStringList()
								<< "Raw Value Only"
								<< "Raw Value Only"
								<< "Raw Value Only"
								<< "Decoded: mg"
								<< "Decoded: mg"
								<< "Decoded: mg"
								<< "Decoded: deg/s"
								<< "Decoded: deg/s"
								<< "Decoded: deg/s"
								<< "Decoded: uT"
								<< "Decoded: uT"
								<< "Decoded: uT"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only";

QString RigidDevice::getLastSerializedStr(void)
{
	QString str;
	QTextStream(&str) <<	timeStamp.last().date		<< ',' << \
							timeStamp.last().ms			<< ',' << \
							eventFlags.last()			<< ',' << \
							riList.last()->accel.x		<< ',' << \
							riList.last()->accel.y		<< ',' << \
							riList.last()->accel.z		<< ',' << \
							riList.last()->gyro.x		<< ',' << \
							riList.last()->gyro.y		<< ',' << \
							riList.last()->gyro.z		<< ',' << \
							riList.last()->magneto.x	<< ',' << \
							riList.last()->magneto.y	<< ',' << \
							riList.last()->magneto.z	<< ',' << \
							riList.last()->io[0]		<< ',' << \
							riList.last()->io[1]		<< ',' << \
							riList.last()->capsense[0]	<< ',' << \
							riList.last()->capsense[1]	<< ',' << \
							riList.last()->capsense[2]	<< ',' << \
							riList.last()->capsense[3]	<< ',' << \
							riList.last()->status;
	return str;
}

void RigidDevice::appendSerializedStr(QStringList *splitLine)
{
	uint8_t idx = 0;

	//Check if data line contain the number of data expected
	if(splitLine->length() >= serializedLength)
	{
		appendEmptyLine();
		timeStamp.last().date		= (*splitLine)[idx++];
		timeStamp.last().ms			= (*splitLine)[idx++].toInt();
		eventFlags.last()			= (*splitLine)[idx++].toInt();

		riList.last()->accel.x		= (*splitLine)[idx++].toInt();
		riList.last()->accel.y		= (*splitLine)[idx++].toInt();
		riList.last()->accel.z		= (*splitLine)[idx++].toInt();
		riList.last()->gyro.x		= (*splitLine)[idx++].toInt();
		riList.last()->gyro.y		= (*splitLine)[idx++].toInt();
		riList.last()->gyro.z		= (*splitLine)[idx++].toInt();
		riList.last()->magneto.x	= (*splitLine)[idx++].toInt();
		riList.last()->magneto.y	= (*splitLine)[idx++].toInt();
		riList.last()->magneto.z	= (*splitLine)[idx++].toInt();
		riList.last()->io[0]		= (*splitLine)[idx++].toInt();
		riList.last()->io[1]		= (*splitLine)[idx++].toInt();
		riList.last()->capsense[0]	= (*splitLine)[idx++].toInt();
		riList.last()->capsense[1]	= (*splitLine)[idx++].toInt();
		riList.last()->capsense[2]	= (*splitLine)[idx++].toInt();
		riList.last()->capsense[3]	= (*splitLine)[idx++].toInt();
		riList.last()->status		= (*splitLine)[idx++].toInt();
	}
}

struct std_variable RigidDevice::getSerializedVar(int parameter)
{
	return getSerializedVar(parameter, 0);
}

struct std_variable RigidDevice::getSerializedVar(int parameter, int index)
{
	struct std_variable var;

	if(index >= riList.length())
	{
		parameter = INT_MAX;
	}

	//Assign pointer:
	switch(parameter)
	{
		/*Format: (every Case except Unused)
		 * Line 1: data format, raw variable
		 * Line 2: raw variable
		 * Line 3: decoded variable (always int32),
					null if not decoded  */
		case 0: //"TimeStamp"
			var.format = FORMAT_QSTR;
			var.rawGenPtr = &timeStamp[index].date;
			var.decodedPtr = nullptr;
			break;
		case 1: //"TimeStamp (ms)"
			var.format = FORMAT_32S;
			var.rawGenPtr = &timeStamp[index].ms;
			var.decodedPtr = nullptr;
			break;
		case 2: //"Event Flag"
			var.format = FORMAT_32S;
			var.rawGenPtr = &eventFlags[index];
			var.decodedPtr = nullptr;
			break;
		case 3: //"Accel X"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->accel.x;
			var.decodedPtr = &riList[index]->decoded.accel.x;
			break;
		case 4: //"Accel Y"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->accel.y;
			var.decodedPtr = &riList[index]->decoded.accel.y;
			break;
		case 5: //"Accel Z"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->accel.z;
			var.decodedPtr = &riList[index]->decoded.accel.z;
			break;
		case 6: //"Gyro X"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->gyro.x;
			var.decodedPtr = &riList[index]->decoded.gyro.x;
			break;
		case 7: //"Gyro Y"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->gyro.y;
			var.decodedPtr = &riList[index]->decoded.gyro.y;
			break;
		case 8: //"Gyro Z"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->gyro.z;
			var.decodedPtr = &riList[index]->decoded.gyro.z;
			break;
		case 9: //"Magneto X"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->magneto.x;
			var.decodedPtr = &riList[index]->decoded.magneto.x;
			break;
		case 10: //"Magneto Y"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->magneto.y;
			var.decodedPtr = &riList[index]->decoded.magneto.y;
			break;
		case 11: //"Magneto Z"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->magneto.z;
			var.decodedPtr = &riList[index]->decoded.magneto.z;
			break;
		case 12: //"IO 1"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->io[0];
			var.decodedPtr = nullptr;
			break;
		case 13: //"IO 2"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->io[1];
			var.decodedPtr = nullptr;
			break;
		case 14: //"Capsense 1"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->capsense[0];
			var.decodedPtr = nullptr;
			break;
		case 15: //"Capsense 2"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->capsense[1];
			var.decodedPtr = nullptr;
		case 16: //"Capsense 3"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->capsense[2];
			var.decodedPtr = nullptr;
		case 17: //"Capsense 4"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->capsense[3];
			var.decodedPtr = nullptr;
		case 18: //"Status"
			var.format = FORMAT_8U;
			var.rawGenPtr = &riList[index]->status;
			var.decodedPtr = nullptr;
			break;
		default:
			var.format = NULL_PTR;
			var.rawGenPtr = nullptr;
			var.decodedPtr = nullptr;
			break;
	}

	return var;
}

void RigidDevice::clear(void)
{
	FlexseaDevice::clear();
	riList.clear();
	timeStamp.clear();
	eventFlags.clear();
}

void RigidDevice::appendEmptyLine(void)
{
	timeStamp.append(TimeStamp());
	riList.append(new rigid_s());
	eventFlags.append(0);
}

void RigidDevice::decodeLastLine(void)
{
	decode(riList.last());
}

void RigidDevice::decodeAllLine(void)
{
	for(int i = 0; i < riList.size(); ++i)
	{
		decode(riList[i]);
	}
}

QString RigidDevice::getStatusStr(int index)
{
	(void)index;	//Unused

	return QString("No decoding available for this board");
}

void RigidDevice::decode(struct rigid_s *riPtr)
{
	//Accel in mG
	riPtr->decoded.accel.x = (1000*riPtr->accel.x)/8192;
	riPtr->decoded.accel.y = (1000*riPtr->accel.y)/8192;
	riPtr->decoded.accel.z = (1000*riPtr->accel.z)/8192;

	//Gyro in degrees/s
	riPtr->decoded.gyro.x = (100*riPtr->gyro.x)/164;
	riPtr->decoded.gyro.y = (100*riPtr->gyro.y)/164;
	riPtr->decoded.gyro.z = (100*riPtr->gyro.z)/164;

	//Magneto in uT (0.15uT/LSB)
	riPtr->decoded.magneto.x = (15*riPtr->magneto.x)/100;
	riPtr->decoded.magneto.y = (15*riPtr->magneto.y)/100;
	riPtr->decoded.magneto.z = (15*riPtr->magneto.z)/100;
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

