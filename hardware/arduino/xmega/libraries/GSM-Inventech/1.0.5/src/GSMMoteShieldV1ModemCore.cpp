/*
This file is part of the GSMMote communications library for Arduino
-- Multi-transport communications platform
-- Fully asynchronous
-- Includes code for the Arduino-Telefonica GSM/GPRS Shield V1
-- Voice calls
-- SMS
-- TCP/IP connections
-- HTTP basic clients

This library has been developed by Telefónica Digital - PDI -
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

#include <Arduino.h>
#include <GSMMoteShieldV1ModemCore.h>

GSMMoteShieldV1ModemCore theGSMMoteShieldV1ModemCore;

char* __ok__="OK";

GSMMoteShieldV1ModemCore::GSMMoteShieldV1ModemCore() : gss()
{
	//gss.registerMgr(this);
	_dataInBufferFrom=0;
	_dataInBufferTo=0;
	commandError=1;
	commandCounter=0;
	ongoingCommand=NONE;
	takeMilliseconds();
	
	for(int i=0;i<UMPROVIDERS;i++)
		UMProvider[i]=0;
}

void GSMMoteShieldV1ModemCore::registerUMProvider(GSMMoteShieldV1BaseProvider* provider)
{
	for(int i=0;i<UMPROVIDERS;i++)
	{
		if(UMProvider[i]==0)
		{
			UMProvider[i]=provider;
			break;
		}

	}

}

void GSMMoteShieldV1ModemCore::unRegisterUMProvider(GSMMoteShieldV1BaseProvider* provider)
{
	for(int i=0;i<UMPROVIDERS;i++)
	{
		if(UMProvider[i]==provider)
		{
			UMProvider[i]=0;
			break;
		}
	}
}


//Response parse.
bool GSMMoteShieldV1ModemCore::genericParse_rsp(bool& rsp, char* string, char* string2)
{
	if((string==0) && (string2==0))
		string=__ok__;
	
	rsp=theBuffer().locate(string);
	
	if((!rsp)&&(string2!=0))
		rsp=theBuffer().locate(string2);
		
	return true;
}

void GSMMoteShieldV1ModemCore::closeCommand(int code)
{
	// If we were configuring the modem,
	// and there's been an error
	// we don't know exactly where we are
	if((code!=1)&&(theGSMMoteShieldV1ModemCore.getOngoingCommand()==MODEMCONFIG))
		theGSMMoteShieldV1ModemCore.setStatus(ERROR);

	setCommandError(code);
	ongoingCommand=NONE;
	activeProvider=0;
	commandCounter=1;
}

//Generic command (stored in flash).
void GSMMoteShieldV1ModemCore::genericCommand_rq(PGM_P str, bool addCR)
{
	theBuffer().flush();
	writePGM(str, addCR);
}

//Generic command (const string).
void GSMMoteShieldV1ModemCore::genericCommand_rqc(const char* str, bool addCR)
{
	theBuffer().flush();
	print(str);
	if(addCR)
		print("\r");
}

// If we are not debugging, lets manage data in interrupt time
// but if we are not, just take note.
void GSMMoteShieldV1ModemCore::manageMsg(byte from, byte to)
{
	if(_debug)
	{
		_dataInBufferFrom=from;
		_dataInBufferTo=to;
	}
	else
	{
		manageMsgNow(from, to);
	}
}

void GSMMoteShieldV1ModemCore::manageReceivedData()
{
	if(_debug)
	{
/*		Serial.print(theBuffer().getHead());
		Serial.print(" ");
		Serial.println(theBuffer().getTail());*/
		if(_dataInBufferFrom != _dataInBufferTo)
		{
			theBuffer().debugBuffer();
			manageMsgNow(_dataInBufferFrom, _dataInBufferTo);
			_dataInBufferFrom=0;
			_dataInBufferTo=0;
		}
	}
	else
	{
		// Just debugging the non debugging
//		Serial.println();
//		Serial.print("Com:");
//		Serial.print(ongoingCommand);
//		Serial.print("   Step:");
//		Serial.print(commandCounter);
	}
}

//Select between URC or response.
void GSMMoteShieldV1ModemCore::manageMsgNow(byte from, byte to)
{
	bool recognized=false;
	
	for(int i=0;(i<UMPROVIDERS)&&(!recognized);i++)
	{
		if(UMProvider[i])
			recognized=UMProvider[i]->recognizeUnsolicitedEvent(from);
	}
	if((!recognized)&&(activeProvider))
		activeProvider->manageResponse(from, to);
}


void GSMMoteShieldV1ModemCore::openCommand(GSMMoteShieldV1BaseProvider* provider, GSMMote_commandType_e c)
{	
	activeProvider=provider;
	commandError=0;
	commandCounter=1;
	ongoingCommand=c;
	_dataInBufferFrom=0;
	_dataInBufferTo=0;

};

size_t GSMMoteShieldV1ModemCore::writePGM(PGM_P str, bool CR)
{
	int i=0;
	char c;
	
	do
	{	
		c=pgm_read_byte_near(str + i); 
		if(c!=0)
			write(c);
		i++;
	} while (c!=0);
	if(CR)
		print("\r");
		
	return 1;
}

size_t GSMMoteShieldV1ModemCore::write(uint8_t c)
{
	if(_debug)
		GSMMoteCircularBuffer::printCharDebug(c);
	return gss.write(c);
}

unsigned long GSMMoteShieldV1ModemCore::takeMilliseconds()
{
	unsigned long now=millis();
	unsigned long delta;
	delta=now-milliseconds;
	milliseconds=now;
	return delta;
}

void GSMMoteShieldV1ModemCore::delayInsideInterrupt(unsigned long milliseconds)
{
	//for (unsigned long k=0;k<milliseconds;k++)
		//theGSMMoteShieldV1ModemCore.gss.tunedDelay(1000);
}
