/*
This file is part of the GSMMote communications library for Arduino
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
#include <GSMMoteSMSService.h>
#include <GSMMoteMobileNetworkProvider.h>
#include <Arduino.h>

// While there is only a shield (ShieldV1) we will include it by default
#include <GSMMoteShieldV1SMSProvider.h>
GSMMoteShieldV1SMSProvider theShieldV1SMSProvider;

#define GSMMoteSMSSERVICE_SYNCH 0x01 // 1: synchronous 0: asynchronous
#define __TOUT__ 10000


GSMMoteSMSService::GSMMoteSMSService(bool synch)
{
	if(synch)
		flags |= GSMMoteSMSSERVICE_SYNCH;
}

// Returns 0 if last command is still executing
// 1 if success
// >1 if error 
int GSMMoteSMSService::ready()
{	
	return theGSMMoteSMSProvider->ready();
}

int GSMMoteSMSService::beginSMS(const char *number)
{	
	return waitForAnswerIfNeeded(theGSMMoteSMSProvider->beginSMS(number));
};

int GSMMoteSMSService::endSMS()
{
	return waitForAnswerIfNeeded(theGSMMoteSMSProvider->endSMS());
};

size_t GSMMoteSMSService::write(uint8_t c)
{
	theGSMMoteSMSProvider->writeSMS(c);
	return 1;
}

void GSMMoteSMSService::flush()
{		
	theGSMMoteSMSProvider->flushSMS();
	waitForAnswerIfNeeded(1);
};

int GSMMoteSMSService::available()
{
	return waitForAnswerIfNeeded(theGSMMoteSMSProvider->availableSMS());
};

int GSMMoteSMSService::remoteNumber(char* number, int nlength)
{
	return theGSMMoteSMSProvider->remoteSMSNumber(number, nlength);

}

int GSMMoteSMSService::read()
{
	return theGSMMoteSMSProvider->readSMS();
};
int GSMMoteSMSService::peek()
{		
	return theGSMMoteSMSProvider->peekSMS();
};

int GSMMoteSMSService::waitForAnswerIfNeeded(int returnvalue)
{
	// If synchronous
	if(flags & GSMMoteSMSSERVICE_SYNCH )
	{
		unsigned long m;
		m=millis();
		// Wait for __TOUT__
		while(((millis()-m)< __TOUT__ )&&(ready()==0)) 
			delay(100);
		// If everything was OK, return 1
		// else (timeout or error codes) return 0;
		if(ready()==1)
			return 1;
		else
			return 0;
	}
	// If not synchronous just kick ahead the coming result
	return ready();
}





