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
#include <GSMMoteMobileServerService.h>
#include <GSMMoteMobileServerProvider.h>
#include <GSMMoteMobileClientProvider.h>


#define __TOUTSERVER__ 10000
#define BUFFERSIZETWEET 100

#define GSMMoteMOBILESERVERSERVICE_SYNCH 0x01 // 1: TRUE, compatible with other clients 0: FALSE

// While there is only a shield (ShieldV1) we will include it by default
#include <GSMMoteShieldV1ServerProvider.h>
GSMMoteShieldV1ServerProvider theShieldV1ServerProvider;


GSMMoteMobileServerService::GSMMoteMobileServerService(uint8_t port, bool synch)
{
	mySocket=0;
	_port=port;
	flags = 0;
	
	// If synchronous
	if(synch)
		flags |= GSMMoteMOBILESERVERSERVICE_SYNCH;
}

// Returns 0 if last command is still executing
// 1 if success
// >1 if error 
int GSMMoteMobileServerService::ready()
{	
	return theGSMMoteMobileServerProvider->ready();
}

void GSMMoteMobileServerService::begin()
{	
	if(theGSMMoteMobileServerProvider==0)
		return;
	theGSMMoteMobileServerProvider->connectTCPServer(_port);
	
	if(flags & GSMMoteMOBILESERVERSERVICE_SYNCH)
		waitForAnswer();
}

GSMMoteMobileClientService GSMMoteMobileServerService::available(bool synch)
{	
	int newSocket;
	// In case we are debugging, we'll need to force a look at the buffer
	ready();
	
	newSocket=theGSMMoteMobileServerProvider->getNewOccupiedSocketAsServer();
	
	// Instatiate new client. If we are synch, the client is synchronous/blocking
	GSMMoteMobileClientService client((uint8_t)(newSocket), (flags & GSMMoteMOBILESERVERSERVICE_SYNCH));

	return client;
}

size_t GSMMoteMobileServerService::write(uint8_t c)
{
// Adapt to the new, lean implementation	
//	theGSMMoteMobileServerProvider->writeSocket(c);
	return 1;
}

void GSMMoteMobileServerService::beginWrite()
{
// Adapt to the new, lean implementation
//	theGSMMoteMobileServerProvider->beginWriteSocket(local1Remote0, mySocket);
}

size_t GSMMoteMobileServerService::write(const uint8_t* buf)
{
// Adapt to the new, lean implementation
//	theGSMMoteMobileServerProvider->writeSocket((const char*)(buf));
	return strlen((const char*)buf);
}

size_t GSMMoteMobileServerService::write(const uint8_t* buf, size_t sz)
{
// Adapt to the new, lean implementation
//	theGSMMoteMobileServerProvider->writeSocket((const char*)(buf));
}

void GSMMoteMobileServerService::endWrite()
{
// Adapt to the new, lean implementation
//	theGSMMoteMobileServerProvider->endWriteSocket();
}

void GSMMoteMobileServerService::stop()
{
	
	// Review, should be the server?
	theGSMMoteMobileClientProvider->disconnectTCP(local1Remote0, mySocket);
	if(flags & GSMMoteMOBILESERVERSERVICE_SYNCH)
		waitForAnswer();
	theGSMMoteMobileClientProvider->releaseSocket(mySocket);
	mySocket = -1;
}


/*int GSMMoteMobileServerService::getIP(char* LocalIP, int LocalIPlength)
{
	return theGSMMoteMobileServerProvider->getIP(LocalIP, LocalIPlength);
}*/

int GSMMoteMobileServerService::waitForAnswer()
{
	unsigned long m;
	m=millis();
	int res;
	
	while(((millis()-m)< __TOUTSERVER__ )&&(ready()==0)) 
		delay(10);
	
	res=ready();

	// If we get something different from a 1, we are having a problem
	if(res!=1)
		res=0;

	return res;
}


