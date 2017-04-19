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
	ownershipList.append(false); //we assume we don't own this device ptr, and whoever passed it to us is responsible for clean up
	eventFlags.append(0);

	serializedLength = header.length();
	slaveTypeName = "rigid";
}

RigidDevice::~RigidDevice()
{
	if(ownershipList.size() != riList.size())
	{
		qDebug() << "Rigid Device class cleaning up: execute list size doesn't match list of ownership info size.";
		qDebug() << "Not sure whether it is safe to delete these device records.";
		return;
	}

	while(ownershipList.size())
	{
		bool shouldDelete = ownershipList.takeLast();
		rigid_s* readyToDelete = riList.takeLast();
		if(shouldDelete)
		{
			delete readyToDelete->ex.enc_ang;
			readyToDelete->ex.enc_ang = nullptr;

			delete readyToDelete->ex.enc_ang_vel;
			readyToDelete->ex.enc_ang_vel = nullptr;

			delete readyToDelete;
		}
	}
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
							riList.last()->mn.accel.x		<< ',' << \
							riList.last()->mn.accel.y		<< ',' << \
							riList.last()->mn.accel.z		<< ',' << \
							riList.last()->mn.gyro.x		<< ',' << \
							riList.last()->mn.gyro.y		<< ',' << \
							riList.last()->mn.gyro.z		<< ',' << \
							riList.last()->mn.magneto.x	<< ',' << \
							riList.last()->mn.magneto.y	<< ',' << \
							riList.last()->mn.magneto.z	<< ',' << \
							riList.last()->mn.status;
	return str;
}

void RigidDevice::appendSerializedStr(QStringList *splitLine)
{
	uint8_t idx = 0;

	//Check if data line contain the number of data expected
	if(splitLine->length() >= serializedLength)
	{
		appendEmptyLine();
		timeStamp.last().date			= (*splitLine)[idx++];
		timeStamp.last().ms				= (*splitLine)[idx++].toInt();
		eventFlags.last()				= (*splitLine)[idx++].toInt();

		riList.last()->mn.accel.x		= (*splitLine)[idx++].toInt();
		riList.last()->mn.accel.y		= (*splitLine)[idx++].toInt();
		riList.last()->mn.accel.z		= (*splitLine)[idx++].toInt();
		riList.last()->mn.gyro.x		= (*splitLine)[idx++].toInt();
		riList.last()->mn.gyro.y		= (*splitLine)[idx++].toInt();
		riList.last()->mn.gyro.z		= (*splitLine)[idx++].toInt();
		riList.last()->mn.magneto.x		= (*splitLine)[idx++].toInt();
		riList.last()->mn.magneto.y		= (*splitLine)[idx++].toInt();
		riList.last()->mn.magneto.z		= (*splitLine)[idx++].toInt();
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
			var.rawGenPtr = &riList[index]->mn.accel.x;
			var.decodedPtr = &riList[index]->mn.decoded.accel.x;
			break;
		case 4: //"Accel Y"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.accel.y;
			var.decodedPtr = &riList[index]->mn.decoded.accel.y;
			break;
		case 5: //"Accel Z"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.accel.z;
			var.decodedPtr = &riList[index]->mn.decoded.accel.z;
			break;
		case 6: //"Gyro X"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.gyro.x;
			var.decodedPtr = &riList[index]->mn.decoded.gyro.x;
			break;
		case 7: //"Gyro Y"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.gyro.y;
			var.decodedPtr = &riList[index]->mn.decoded.gyro.y;
			break;
		case 8: //"Gyro Z"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.gyro.z;
			var.decodedPtr = &riList[index]->mn.decoded.gyro.z;
			break;
		case 9: //"Magneto X"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.magneto.x;
			var.decodedPtr = &riList[index]->mn.decoded.magneto.x;
			break;
		case 10: //"Magneto Y"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.magneto.y;
			var.decodedPtr = &riList[index]->mn.decoded.magneto.y;
			break;
		case 11: //"Magneto Z"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->mn.magneto.z;
			var.decodedPtr = &riList[index]->mn.decoded.magneto.z;
			break;
		case 12: //"Status"
			var.format = FORMAT_8U;
			var.rawGenPtr = &riList[index]->mn.status;
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
	ownershipList.clear();
	timeStamp.clear();
	eventFlags.clear();
}

void RigidDevice::appendEmptyLine(void)
{
	timeStamp.append(TimeStamp());

	rigid_s *emptyStruct = new rigid_s();
	emptyStruct->ex.enc_ang = new int32_t();
	emptyStruct->ex.enc_ang_vel = new int32_t();
	riList.append(emptyStruct);
	ownershipList.append(true); // we own this struct, so we must delete it in destructor
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
	riPtr->mn.decoded.accel.x = (1000*riPtr->mn.accel.x)/8192;
	riPtr->mn.decoded.accel.y = (1000*riPtr->mn.accel.y)/8192;
	riPtr->mn.decoded.accel.z = (1000*riPtr->mn.accel.z)/8192;

	//Gyro in degrees/s
	riPtr->mn.decoded.gyro.x = (100*riPtr->mn.gyro.x)/164;
	riPtr->mn.decoded.gyro.y = (100*riPtr->mn.gyro.y)/164;
	riPtr->mn.decoded.gyro.z = (100*riPtr->mn.gyro.z)/164;

	//Magneto in uT (0.15uT/LSB)
	riPtr->mn.decoded.magneto.x = (15*riPtr->mn.magneto.x)/100;
	riPtr->mn.decoded.magneto.y = (15*riPtr->mn.magneto.y)/100;
	riPtr->mn.decoded.magneto.z = (15*riPtr->mn.magneto.z)/100;
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

