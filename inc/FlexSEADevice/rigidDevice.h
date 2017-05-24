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

#ifndef RIGIDDEVICE_H
#define RIGIDDEVICE_H

//****************************************************************************
// Include(s)
//****************************************************************************

#include <QList>
#include <QString>
#include <flexsea_global_structs.h>
#include <flexsea_user_structs.h>
#include "flexseaDevice.h"

//****************************************************************************
// Definition(s)
//****************************************************************************

//****************************************************************************
// Namespace & Class
//****************************************************************************

namespace Ui
{
	class RigidDevice;
}

class RigidDevice : public FlexseaDevice
{
public:
	explicit RigidDevice(void);
	explicit RigidDevice(rigid_s *devicePtr);
	virtual ~RigidDevice();

	// Interface implementation
	QString getHeaderStr(void);
	QStringList getHeaderList(void) {return header;}
	QStringList getHeaderDecList(void) {return headerDecoded;}
	QString getLastSerializedStr(void);
	struct std_variable getSerializedVar(int parameter);
	struct std_variable getSerializedVar(int parameter, int index);
	void appendSerializedStr(QStringList *splitLine);
	void decodeLastLine(void);
	void decodeAllLine(void);
	int length(void) {return riList.length();}
	void clear(void);
	void appendEmptyLine(void);
	QString getStatusStr(int index);

	QList<struct rigid_s *> riList;
	QList<bool> ownershipList;
	static void decode(struct rigid_s *riPtr);

private:
	static QStringList header;
	static QStringList headerDecoded;
	int32_t enc_ang;
	int32_t enc_vel;
};

//****************************************************************************
// Definition(s)
//****************************************************************************

#endif // RIGIDDEVICE_H
