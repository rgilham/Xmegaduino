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
#include <GSMMoteMobileClientService.h>
#include <GSMMoteMobileClientProvider.h>
#include <Arduino.h>

// While there is only a shield (ShieldV1) we will include it by default
#include <GSMMoteShieldV1ClientProvider.h>
GSMMoteShieldV1ClientProvider theShieldV1ClientProvider;


#define GSMMoteMOBILECLIENTSERVICE_CLIENT 0x01 // 1: This side is Client. 0: This side is Server
#define GSMMoteMOBILECLIENTSERVICE_WRITING 0x02 // 1: TRUE 0: FALSE
#define GSMMoteMOBILECLIENTSERVICE_SYNCH 0x04 // 1: TRUE, compatible with other clients 0: FALSE

#define __TOUTBEGINWRITE__ 10000


GSMMoteMobileClientService::GSMMoteMobileClientService(bool synch)
{
	flags = GSMMoteMOBILECLIENTSERVICE_CLIENT;
	if(synch)
		flags |= GSMMoteMOBILECLIENTSERVICE_SYNCH;
	mySocket=255;
}

GSMMoteMobileClientService::GSMMoteMobileClientService(int socket, bool synch)
{
	// We are creating a socket on an existing, occupied one.
	flags=0;
	if(synch)
		flags |= GSMMoteMOBILECLIENTSERVICE_SYNCH;
	mySocket=socket;
	theGSMMoteMobileClientProvider->getSocket(socket);
	
}

// Returns 0 if last command is still executing
// 1 if success
// >1 if error 
int GSMMoteMobileClientService::ready()
{	
	return theGSMMoteMobileClientProvider->ready();
}

int GSMMoteMobileClientService::connect(IPAddress add, uint16_t port) 
{
	if(theGSMMoteMobileClientProvider==0)
		return 2;
		
	// TODO: ask for the socket id
	mySocket=theGSMMoteMobileClientProvider->getSocket();

	if(mySocket<0)
		return 2;
	
	int res=theGSMMoteMobileClientProvider->connectTCPClient(add, port, mySocket);
	if(flags & GSMMoteMOBILECLIENTSERVICE_SYNCH)
		res=waitForAnswer();
	
	return res;
};

int GSMMoteMobileClientService::connect(const char *host, uint16_t port)
{

	if(theGSMMoteMobileClientProvider==0)
		return 2;		
	// TODO: ask for the socket id
	mySocket=theGSMMoteMobileClientProvider->getSocket();

	if(mySocket<0)
		return 2;
	
	int res=theGSMMoteMobileClientProvider->connectTCPClient(host, port, mySocket);
	if(flags & GSMMoteMOBILECLIENTSERVICE_SYNCH)
		res=waitForAnswer();
		
	return res;
}

int GSMMoteMobileClientService::waitForAnswer()
{
	unsigned long m;
	m=millis();
	int res;
	
	while(((millis()-m)< __TOUTBEGINWRITE__ )&&(ready()==0)) 
		delay(100);
	
	res=ready();

	// If we get something different from a 1, we are having a problem
	if(res!=1)
		res=0;

	return res;
}

void GSMMoteMobileClientService::beginWrite(bool sync)
{
	flags |= GSMMoteMOBILECLIENTSERVICE_WRITING;
	theGSMMoteMobileClientProvider->beginWriteSocket(flags & GSMMoteMOBILECLIENTSERVICE_CLIENT, mySocket);
	if(sync)
		waitForAnswer();
}

size_t GSMMoteMobileClientService::write(uint8_t c)
{	
	if(!(flags & GSMMoteMOBILECLIENTSERVICE_WRITING))
		beginWrite(true);
	theGSMMoteMobileClientProvider->writeSocket(c);
	return 1;
}

size_t GSMMoteMobileClientService::write(const uint8_t* buf)
{
	if(!(flags & GSMMoteMOBILECLIENTSERVICE_WRITING))
		beginWrite(true);
	theGSMMoteMobileClientProvider->writeSocket((const char*)(buf));
	return strlen((const char*)buf);
}

size_t GSMMoteMobileClientService::write(const uint8_t* buf, size_t sz)
{
	if(!(flags & GSMMoteMOBILECLIENTSERVICE_WRITING))
		beginWrite(true);
	for(int i=0;i<sz;i++)
		theGSMMoteMobileClientProvider->writeSocket(buf[i]);
	return sz;
}

void GSMMoteMobileClientService::endWrite(bool sync)
{
	flags ^= GSMMoteMOBILECLIENTSERVICE_WRITING;
	theGSMMoteMobileClientProvider->endWriteSocket();
	if(sync)
		waitForAnswer();
}

uint8_t GSMMoteMobileClientService::connected()
{
	if(mySocket==255)
		return 0;
	return theGSMMoteMobileClientProvider->getStatusSocketClient(mySocket);	 
}

GSMMoteMobileClientService::operator bool()
{
	return connected()==1;
};

int GSMMoteMobileClientService::available()
{
	int res;

	// Even if not connected, we are looking for available data
	
	if(flags & GSMMoteMOBILECLIENTSERVICE_WRITING)
		endWrite(true);

	res=theGSMMoteMobileClientProvider->availableSocket(flags & GSMMoteMOBILECLIENTSERVICE_CLIENT,mySocket);
	if(flags & GSMMoteMOBILECLIENTSERVICE_SYNCH)
		res=waitForAnswer();

	return res;
}

int GSMMoteMobileClientService::read(uint8_t *buf, size_t size)
{
	int i;
	uint8_t c;
	
	for(i=0;i<size;i++)
	{
		c=read();
		if(c==0)
			break;
		buf[i]=c;
	}
	
	return i;
/* This is the old implementation, testing a simpler one
	int res;
	// If we were writing, just stop doing it.
	if(flags & GSMMoteMOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	res=theGSMMoteMobileClientProvider->readSocket(flags & GSMMoteMOBILECLIENTSERVICE_CLIENT, (char *)(buf), size, mySocket);

	return res;
*/
}

int GSMMoteMobileClientService::read()
{
	if(flags & GSMMoteMOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	int c=theGSMMoteMobileClientProvider->readSocket();
	return c;
}

int GSMMoteMobileClientService::peek()
{
	if(flags & GSMMoteMOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	return theGSMMoteMobileClientProvider->peekSocket(/*mySocket, false*/);
}

void GSMMoteMobileClientService::flush()
{
	if(flags & GSMMoteMOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	theGSMMoteMobileClientProvider->flushSocket(/*mySocket*/);
	if(flags & GSMMoteMOBILECLIENTSERVICE_SYNCH)
		waitForAnswer();

}

void GSMMoteMobileClientService::stop()
{
	if(flags & GSMMoteMOBILECLIENTSERVICE_WRITING)
		endWrite(true);
	theGSMMoteMobileClientProvider->disconnectTCP(flags & GSMMoteMOBILECLIENTSERVICE_CLIENT, mySocket);
	theGSMMoteMobileClientProvider->releaseSocket(mySocket);
	mySocket = 0;
	if(flags & GSMMoteMOBILECLIENTSERVICE_SYNCH)
		waitForAnswer();
}

