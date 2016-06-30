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
#include <GSMMoteShieldV1AccessProvider.h>
#include "GSMMoteIO.h"

#define __TOUTSHUTDOWN__ 5000
#define __TOUTMODEMCONFIGURATION__ 5000//equivalent to 30000 because of time in interrupt routine.
#define __TOUTAT__ 1000

const char _command_AT[] PROGMEM = "AT";
const char _command_CGREG[] PROGMEM = "AT+CGREG?";


GSMMoteShieldV1AccessProvider::GSMMoteShieldV1AccessProvider(bool debug)
{
	theGSMMoteShieldV1ModemCore.setDebug(debug);

}

void GSMMoteShieldV1AccessProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case MODEMCONFIG:
			ModemConfigurationContinue();
			break;
		case ALIVETEST:
			isModemAliveContinue();
			break;
	}
}

///////////////////////////////////////////////////////CONFIGURATION FUNCTIONS///////////////////////////////////////////////////////////////////

// Begin
// Restart or start the modem
// May be synchronous
GSMMote_NetworkStatus_t GSMMoteShieldV1AccessProvider::begin(char* pin, bool restart, bool synchronous)
{	
	pinMode(__RESETPIN__, OUTPUT);

	#ifdef TTOPEN_V1
	pinMode(__POWERPIN__, OUTPUT);
	digitalWrite(__POWERPIN__, HIGH);
	#endif

	// If asked for modem restart, restart
	if (restart) 
		HWrestart();
	else 
 		HWstart();
  
	theGSMMoteShieldV1ModemCore.gss.begin(9600);
	// Launch modem configuration commands
	ModemConfiguration(pin);
	// If synchronous, wait till ModemConfiguration is over
	if(synchronous)
	{
		// if we shorten this delay, the command fails
		while(ready()==0) 
			delay(1000); 
	}
	return getStatus();
}

//HWrestart.
int GSMMoteShieldV1AccessProvider::HWrestart()
{
	#ifdef TTOPEN_V1
	digitalWrite(__POWERPIN__, HIGH);
	delay(1000);
	#endif
	
	theGSMMoteShieldV1ModemCore.setStatus(IDLE);
	digitalWrite(__RESETPIN__, HIGH);
	delay(12000);
	digitalWrite(__RESETPIN__, LOW);
	delay(1000);
	return 1; //configandwait(pin);
}

//HWrestart.
int GSMMoteShieldV1AccessProvider::HWstart()
{

	theGSMMoteShieldV1ModemCore.setStatus(IDLE);
	digitalWrite(__RESETPIN__, HIGH);
	delay(2000);
	digitalWrite(__RESETPIN__, LOW);
	//delay(1000);

	return 1; //configandwait(pin);
}

//Initial configuration main function.
int GSMMoteShieldV1AccessProvider::ModemConfiguration(char* pin)
{
	theGSMMoteShieldV1ModemCore.setPhoneNumber(pin);
	theGSMMoteShieldV1ModemCore.openCommand(this,MODEMCONFIG);
	theGSMMoteShieldV1ModemCore.setStatus(CONNECTING);
	ModemConfigurationContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Initial configuration continue function.
void GSMMoteShieldV1AccessProvider::ModemConfigurationContinue()
{
	bool resp;

	// 1: Send AT
	// 2: Wait AT OK and SetPin or CGREG
	// 3: Wait Pin OK and CGREG
	// 4: Wait CGREG and Flow SW control or CGREG
	// 5: Wait IFC OK and SMS Text Mode
	// 6: Wait SMS text Mode OK and Calling line identification
	// 7: Wait Calling Line Id OK and Echo off
	// 8: Wait for OK and COLP command for connecting line identification.
	// 9: Wait for OK.
	int ct=theGSMMoteShieldV1ModemCore.getCommandCounter();
	if(ct==1)
	{
		// Launch AT	
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_AT);
	}
	else if(ct==2)
	{
		// Wait for AT - OK.
	   if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	   {
			if(resp)
			{ 
				// OK received
				if(theGSMMoteShieldV1ModemCore.getPhoneNumber() && (theGSMMoteShieldV1ModemCore.getPhoneNumber()[0]!=0)) 
					{
						theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CPIN="), false);
						theGSMMoteShieldV1ModemCore.setCommandCounter(3);
						theGSMMoteShieldV1ModemCore.genericCommand_rqc(theGSMMoteShieldV1ModemCore.getPhoneNumber());
					}
				else 
					{
						//DEBUG	
						//Serial.println("AT+CGREG?");	
						theGSMMoteShieldV1ModemCore.setCommandCounter(4);
						theGSMMoteShieldV1ModemCore.takeMilliseconds();
						theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_CGREG);
					}
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==3)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				theGSMMoteShieldV1ModemCore.setCommandCounter(4);
				theGSMMoteShieldV1ModemCore.takeMilliseconds();
				theGSMMoteShieldV1ModemCore.delayInsideInterrupt(2000);
				theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_CGREG);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
	    }
	}
	else if(ct==4)
	{
		char auxLocate1 [12];
		char auxLocate2 [12];
		prepareAuxLocate(PSTR("+CGREG: 0,1"), auxLocate1);
		prepareAuxLocate(PSTR("+CGREG: 0,5"), auxLocate2);
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp, auxLocate1, auxLocate2))
		{
			if(resp)
			{
				theGSMMoteShieldV1ModemCore.setCommandCounter(5);
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+IFC=1,1"));
			}
			else
			{
				// If not, launch command again
				if(theGSMMoteShieldV1ModemCore.takeMilliseconds() > __TOUTMODEMCONFIGURATION__)
				{
					theGSMMoteShieldV1ModemCore.closeCommand(3);
				}
				else 
				{
					theGSMMoteShieldV1ModemCore.delayInsideInterrupt(2000);
					theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_CGREG);
				}
			}
		}	
	}
	else if(ct==5)
	{
		// 5: Wait IFC OK
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			//Delay for SW flow control being active.
			theGSMMoteShieldV1ModemCore.delayInsideInterrupt(2000);
			// 9: SMS Text Mode
			theGSMMoteShieldV1ModemCore.setCommandCounter(6);
			theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CMGF=1"));
		}
	}
	else if(ct==6)
	{
		// 6: Wait SMS text Mode OK
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			//Calling line identification
			theGSMMoteShieldV1ModemCore.setCommandCounter(7);			
			theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CLIP=1"));
		}
	}
	else if(ct==7)
	{
		// 7: Wait Calling Line Id OK
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			// Echo off
			theGSMMoteShieldV1ModemCore.setCommandCounter(8);			
			theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("ATE0"));
		}
	}
	else if(ct==8)
	{
		// 8: Wait ATEO OK, send COLP
		// In Arduino Mega, attention, take away the COLP step
		// It looks as we can only have 8 steps
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			theGSMMoteShieldV1ModemCore.setCommandCounter(9);
			theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+COLP=1"));
		}
	}
	else if(ct==9)
	{
		// 9: Wait ATCOLP OK
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			if (resp) 
				{
					theGSMMoteShieldV1ModemCore.setStatus(GSM_READY);
					theGSMMoteShieldV1ModemCore.closeCommand(1);
				}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
 	}
}

//Alive Test main function.
int GSMMoteShieldV1AccessProvider::isAccessAlive()
{
	theGSMMoteShieldV1ModemCore.setCommandError(0);
	theGSMMoteShieldV1ModemCore.setCommandCounter(1);
	theGSMMoteShieldV1ModemCore.openCommand(this,ALIVETEST);
	isModemAliveContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Alive Test continue function.
void GSMMoteShieldV1AccessProvider::isModemAliveContinue()
{
bool rsp;
switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_AT);
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
      break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(rsp))
		{
			if (rsp) theGSMMoteShieldV1ModemCore.closeCommand(1);
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
      break;
	}
}

//Shutdown.
bool GSMMoteShieldV1AccessProvider::shutdown()
{
	unsigned long m;
	bool resp;
	char auxLocate [18];
	
	// It makes no sense to have an asynchronous shutdown
	pinMode(__RESETPIN__, OUTPUT);
	digitalWrite(__RESETPIN__, HIGH);
	delay(1500);
	digitalWrite(__RESETPIN__, LOW);
	theGSMMoteShieldV1ModemCore.setStatus(IDLE);
	theGSMMoteShieldV1ModemCore.gss.close();
	
	m=millis();
	prepareAuxLocate(PSTR("POWER DOWN"), auxLocate);
	while((millis()-m) < __TOUTSHUTDOWN__)
	{
		delay(1);
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp, auxLocate))
			return resp;
	}
	return false;
}

//Secure shutdown.
bool GSMMoteShieldV1AccessProvider::secureShutdown()
{
	// It makes no sense to have an asynchronous shutdown
	pinMode(__RESETPIN__, OUTPUT);
	digitalWrite(__RESETPIN__, HIGH);
	delay(900);
	digitalWrite(__RESETPIN__, LOW);
	theGSMMoteShieldV1ModemCore.setStatus(OFF);
	theGSMMoteShieldV1ModemCore.gss.close();

#ifdef TTOPEN_V1
	_delay_ms(12000);
	digitalWrite(__POWERPIN__, LOW);
#endif
	
	return true;
}
