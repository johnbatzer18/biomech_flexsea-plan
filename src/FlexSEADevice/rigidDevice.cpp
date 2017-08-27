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
	riList.last()->ex.enc_ang = &enc_ang;
	riList.last()->ex.enc_ang_vel = &enc_vel;
	riList.last()->ex.joint_ang = &joint_ang;
	riList.last()->ex.joint_ang_vel = &joint_ang_vel;
	riList.last()->ex.joint_ang_from_mot = &joint_ang_from_mot;
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

			delete readyToDelete->ex.joint_ang;
			readyToDelete->ex.joint_ang = nullptr;

			delete readyToDelete->ex.joint_ang_vel;
			readyToDelete->ex.joint_ang_vel = nullptr;

			delete readyToDelete->ex.joint_ang_from_mot;
			readyToDelete->ex.joint_ang_from_mot = nullptr;

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
								<< "Motor Angle"
								<< "Motor Velocity"
								<< "Motor Accel"
								<< "Joint Angle"
								<< "Joint Velocity"
								<< "Motor Current"
								<< "Strain"
								<< "PWM"
								<< "Analog[0]"
								<< "Analog[1]"
								<< "Analog[2]"
								<< "Analog[3]"
								<< "VB"
								<< "VG"
								<< "5V"
								<< "Battery current"
								<< "Temperature";

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
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "Raw value only"
								<< "mV"
								<< "mV"
								<< "mV"
								<< "mA"
								<< "Celsius";

QString RigidDevice::getLastSerializedStr(void)
{
	QString str;
	QTextStream(&str) <<	timeStamp.last().date				<< ',' << \
							timeStamp.last().ms					<< ',' << \
							eventFlags.last()					<< ',' << \

							riList.last()->mn.accel.x			<< ',' << \
							riList.last()->mn.accel.y			<< ',' << \
							riList.last()->mn.accel.z			<< ',' << \
							riList.last()->mn.gyro.x			<< ',' << \
							riList.last()->mn.gyro.y			<< ',' << \
							riList.last()->mn.gyro.z			<< ',' << \

							*(riList.last()->ex.enc_ang)		<< ',' << \
							*(riList.last()->ex.enc_ang_vel)	<< ',' << \
							riList.last()->ex.mot_acc			<< ',' << \
							*(riList.last()->ex.joint_ang)		<< ',' << \
							*(riList.last()->ex.joint_ang_vel)	<< ',' << \
							riList.last()->ex.mot_current		<< ',' << \
							riList.last()->ex.strain			<< ',' << \
							riList.last()->ex.ctrl.pwm			<< ',' << \
							riList.last()->mn.analog[0]			<< ',' << \
							riList.last()->mn.analog[1]			<< ',' << \
							riList.last()->mn.analog[2]			<< ',' << \
							riList.last()->mn.analog[3]			<< ',' << \
							riList.last()->re.vb				<< ',' << \
							riList.last()->re.vg				<< ',' << \
							riList.last()->re.v5				<< ',' << \
							riList.last()->re.current			<< ',' << \
							riList.last()->re.temp;
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

		*(riList.last()->ex.enc_ang)	= (*splitLine)[idx++].toInt();
		*(riList.last()->ex.enc_ang_vel)= (*splitLine)[idx++].toInt();
		riList.last()->ex.mot_acc		= (*splitLine)[idx++].toInt();
		*(riList.last()->ex.joint_ang)	= (*splitLine)[idx++].toInt();
		*(riList.last()->ex.joint_ang_vel)	= (*splitLine)[idx++].toInt();
		riList.last()->ex.mot_current		= (*splitLine)[idx++].toInt();
		riList.last()->ex.strain		= (*splitLine)[idx++].toInt();
		riList.last()->ex.ctrl.pwm		= (*splitLine)[idx++].toInt();
		riList.last()->mn.analog[0]		= (*splitLine)[idx++].toInt();
		riList.last()->mn.analog[1]		= (*splitLine)[idx++].toInt();
		riList.last()->mn.analog[2]		= (*splitLine)[idx++].toInt();
		riList.last()->mn.analog[3]		= (*splitLine)[idx++].toInt();

		riList.last()->re.vb			= (*splitLine)[idx++].toInt();
		riList.last()->re.vg			= (*splitLine)[idx++].toInt();
		riList.last()->re.v5			= (*splitLine)[idx++].toInt();
		riList.last()->re.current		= (*splitLine)[idx++].toInt();
		riList.last()->re.temp			= (*splitLine)[idx++].toInt();
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
		case 9: //"Motor Angle"
			var.format = FORMAT_32S;
			var.rawGenPtr = riList[index]->ex.enc_ang;
			var.decodedPtr = nullptr;
			break;
		case 10: //"Motor Velocity"
			var.format = FORMAT_32S;
			var.rawGenPtr = riList[index]->ex.enc_ang_vel;
			var.decodedPtr = nullptr;
			break;
		case 11: //"Motor Acceleration"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->ex.mot_acc;
			var.decodedPtr = nullptr;
			break;
		case 12: //"Joint Angle"
			var.format = FORMAT_32S;
			var.rawGenPtr = riList[index]->ex.joint_ang;
			var.decodedPtr = nullptr;
			break;
		case 13: //"Joint Velocity"
			var.format = FORMAT_32S;
			var.rawGenPtr = riList[index]->ex.joint_ang_vel;
			var.decodedPtr = nullptr;
			break;
		case 14: //"Motor current"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->ex.mot_current;
			var.decodedPtr = nullptr;
			break;
		case 15: //"Strain"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->ex.strain;
			var.decodedPtr = nullptr;
			break;
		case 16: //"PWM"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->ex.ctrl.pwm;
			var.decodedPtr = nullptr;
			break;
		case 17: //"Analog 0"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->mn.analog[0];
			var.decodedPtr = nullptr;
			break;
		case 18: //"Analog 1"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->mn.analog[1];
			var.decodedPtr = nullptr;
			break;
		case 19: //"Analog 2"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->mn.analog[2];
			var.decodedPtr = nullptr;
			break;
		case 20: //"Analog 3"
			var.format = FORMAT_32S;
			var.rawGenPtr = &riList[index]->mn.analog[3];
			var.decodedPtr = nullptr;
			break;
		case 21: //"+VB"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->re.vb;
			var.decodedPtr = nullptr;
			break;
		case 22: //"+VG"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->re.vg;
			var.decodedPtr = nullptr;
			break;
		case 23: //"+5V"
			var.format = FORMAT_16U;
			var.rawGenPtr = &riList[index]->re.v5;
			var.decodedPtr = nullptr;
			break;
		case 24: //"Battery Current"
			var.format = FORMAT_16S;
			var.rawGenPtr = &riList[index]->re.current;
			var.decodedPtr = nullptr;
			break;
		case 25: //"Temperature"
			var.format = FORMAT_8S;
			var.rawGenPtr = &riList[index]->re.temp;
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
	emptyStruct->ex.joint_ang = new int16_t();
	emptyStruct->ex.joint_ang_vel = new int16_t();
	emptyStruct->ex.joint_ang_from_mot = new int16_t();
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

