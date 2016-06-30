/*
This file is part of the GSM3 communications library for Arduino
-- Multi-transport communications platform
-- Fully asynchronous
-- Includes code for the Arduino-Telefonica GSM/GPRS Shield V1
-- Voice calls
-- SMS
-- TCP/IP connections
-- HTTP basic clients

This library has been developed by Telef�nica Digital - PDI -
- Physical Internet Lab, as part as its collaboration with
Arduino and the Open Hardware Community. 

September-December 2012

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

The latest version of this library can always be found at
https://github.com/BlueVia/Official-Arduino
*/
#include "GSMMoteHardSerial.h"
#include "GSMMoteIO.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "pins_arduino.h"
#include <HardwareSerial.h>
#include <Arduino.h>

#define __XON__ 0x11
#define __XOFF__ 0x13

#define _GSMSOFTSERIALFLAGS_ESCAPED_ 0x01
#define _GSMSOFTSERIALFLAGS_SENTXOFF_ 0x02




GSMMoteHardSerial* GSMMoteHardSerial::_activeObject=0;



GSMMoteHardSerial::GSMMoteHardSerial():
		cb(this)
{
	//comStatus=0;
	//waitingAnswer=false;
}

int GSMMoteHardSerial::begin(long speed)
{

  _activeObject=this;
  Serial6.begin(speed);
  return 1;

}

void GSMMoteHardSerial::close()
 {
	_activeObject=0;
 }

size_t GSMMoteHardSerial::write(uint8_t c)
{
	// Characters to be escaped under XON/XOFF control with Quectel
	if(c==0x11)
	{
		this->finalWrite(0x77);
		return this->finalWrite(0xEE);
	}

	if(c==0x13)
	{
		this->finalWrite(0x77);
		return this->finalWrite(0xEC);
	}

	if(c==0x77)
	{
		this->finalWrite(0x77);
		return this->finalWrite(0x88);
	}
	
	return this->finalWrite(c);
}

size_t GSMMoteHardSerial::finalWrite(uint8_t c)
{
	return Serial6.write(c);
}


bool GSMMoteHardSerial::keepThisChar(uint8_t* c)
{
	return true;
}

void GSMMoteHardSerial::recv(void)
{

}

void GSMMoteHardSerial::spaceAvailable()
{
	// If there is spaceAvailable in the buffer, lets send a XON
	finalWrite((byte)__XON__);
}

void GSMMoteHardSerial::handle_serialEvent(void)
{
	if (_activeObject)
		_activeObject->recv();
}

void serialEvent6(void)
{
	Serial.print("serialEvent6");
	GSMMoteHardSerial::handle_serialEvent();
}


