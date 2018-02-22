/****************************************************************************
	[Project] FlexSEA: Flexible & Scalable Electronics Architecture
	[Sub-project] 'flexsea-comm' Communication stack
	Copyright (C) 2017 Dephy, Inc. <http://dephy.com/>

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
	[This file] flexsea_interface: simple in & out functions
*****************************************************************************
	[Change log] (Convention: YYYY-MM-DD | author | comment)
	* 2017-09-11 | jfduval | Initial release
****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//****************************************************************************
// Include(s)
//****************************************************************************

#include "flexsea.h"
#include "flexsea_comm.h"
#include "flexsea_payload.h"
#include "flexsea_circular_buffer.h"

//****************************************************************************
// Variable(s)
//****************************************************************************

uint8_t npFlag = 0, ppFlag = 0;
uint8_t noWatch = 0;

//****************************************************************************
// Private Function Prototype(s)
//****************************************************************************

//****************************************************************************
// Public Function(s)
//****************************************************************************

//This function replaces flexsea_receive_from_X() and parseXCommands()
//ToDo: add support for 1) RS-485 transceivers reception, 2) SPI error handling
void receiveFlexSEAPacket(Port p, uint8_t *newPacketFlag,  \
							uint8_t *parsedPacketFlag, uint8_t *watch)
{
	uint8_t parseResult = 0;

	//This replaces flexsea_receive_from_X():
	commPeriph[p].rx.unpackedPacketsAvailable = tryParseRx(&commPeriph[p], &packet[p][INBOUND]);
	(*newPacketFlag) = commPeriph[p].rx.unpackedPacketsAvailable;

	//And this replaces parseXCommands():
	if(commPeriph[p].rx.unpackedPacketsAvailable > 0)
	{
		commPeriph[p].rx.unpackedPacketsAvailable = 0;
		parseResult = payload_parse_str(&packet[p][INBOUND]);
		(*parsedPacketFlag) += (parseResult == PARSE_SUCCESSFUL) ? 1 : 0;
		(*watch) = 0; //Valid packets restart the watch count
	}
}

//Host program can use this to feed bytes into a reception buffer
//When autoParse is > 0 we parse the new data
uint8_t receiveFlexSEABytes(uint8_t *d, uint8_t len, uint8_t autoParse)
{
	circ_buff_write(commPeriph[PORT_USB].rx.circularBuff, d, len);
	commPeriph[PORT_USB].rx.bytesReadyFlag++;

	//Parse if needed:
	if(autoParse){receiveFlexSEAPacket(PORT_USB, &npFlag, &ppFlag, &noWatch);}

	return ppFlag;
}

//****************************************************************************
// Private Function(s):
//****************************************************************************

#ifdef __cplusplus
}
#endif
