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
#include <GSMMoteShieldV1ServerProvider.h>
#include <GSMMoteShieldV1ModemCore.h>
#include <Arduino.h>

GSMMoteShieldV1ServerProvider::GSMMoteShieldV1ServerProvider()
{
	theGSMMoteMobileServerProvider=this;
};

//Response management.
void GSMMoteShieldV1ServerProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case NONE:
			theGSMMoteShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case CONNECTSERVER:
			connectTCPServerContinue();
			break;	
		/*case GETIP:
			getIPContinue();
			break;*/
	}
}

//Connect Server main function.
int GSMMoteShieldV1ServerProvider::connectTCPServer(int port)
{
	// We forget about LocalIP as it has no real use, the modem does whatever it likes
	theGSMMoteShieldV1ModemCore.setPort(port);			
	theGSMMoteShieldV1ModemCore.openCommand(this,CONNECTSERVER);
	// From this moment on we wait for a call
	connectTCPServerContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Connect Server continue function.
void GSMMoteShieldV1ServerProvider::connectTCPServerContinue()
{

	bool resp;
	// 1: Read Local IP "AT+QILOCIP"
	// 2: Waiting for IP and Set local port "AT+QILPORT"
	// 3: Waiting for QILPOR OK andConfigure as server "AT+QISERVER"
	// 4: Wait for SERVER OK

	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		//"AT+QILOCIP."
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QILOCIP"));
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		//Not IP storing but the command is necessary.
		//if(parseQILOCIP_rsp(local_IP, local_IP_Length, resp))
		// This awful trick saves some RAM bytes
		char aux[3];
		aux[0]='\r';aux[1]='\n';aux[2]=0;
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp, aux))
	    {
			//Response received
			if(resp)
			{
				// Great. Go for the next step
				// AT+QILPORT
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QILPORT=\"TCP\","),false);
				theGSMMoteShieldV1ModemCore.print(	theGSMMoteShieldV1ModemCore.getPort());
				theGSMMoteShieldV1ModemCore.print('\r');
				theGSMMoteShieldV1ModemCore.setCommandCounter(3);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}	
		break;	
	case 3:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			// Response received
			if(resp)
			{
				// OK received
				// Great. Go for the next step
				// AT+QISERVER
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QISERVER"),true);
				theGSMMoteShieldV1ModemCore.setCommandCounter(4);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}	
		break;	
	case 4:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			// Response received
			// OK received, kathapoon, chessespoon
			if (resp) 
			{
				theGSMMoteShieldV1ModemCore.registerUMProvider(this);
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			}
			else 
				theGSMMoteShieldV1ModemCore.closeCommand(3);
		}		
		break;	
	}
}

//QILOCIP parse.
/*bool GSMMoteShieldV1ServerProvider::parseQILOCIP_rsp(char* LocalIP, int LocalIPlength, bool& rsp)
{
	if (!(theGSMMoteShieldV1ModemCore.theBuffer().extractSubstring("\r\n","\r\n", LocalIP, LocalIPlength)))
		rsp = false;
	else 
		rsp = true;
	return true;
}

//Get IP main function.
int GSMMoteShieldV1ServerProvider::getIP(char* LocalIP, int LocalIPlength)
{
	theGSMMoteShieldV1ModemCore.setPhoneNumber(LocalIP);
	theGSMMoteShieldV1ModemCore.setPort(LocalIPlength);
	theGSMMoteShieldV1ModemCore.openCommand(this,GETIP);
	getIPContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

void GSMMoteShieldV1ServerProvider::getIPContinue()
{

	bool resp;
	// 1: Read Local IP "AT+QILOCIP"
	// 2: Waiting for IP.

	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		//AT+QILOCIP
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_MonoQILOCIP);
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(parseQILOCIP_rsp(theGSMMoteShieldV1ModemCore.getPhoneNumber(), theGSMMoteShieldV1ModemCore.getPort(), resp))
	    {
			if (resp) 
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			else 
				theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		break;	
	}
}*/

bool GSMMoteShieldV1ServerProvider::getSocketAsServerModemStatus(int s)
{
	if(theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED)
		return true;
	else 
		return false;
}


//URC recognize.
bool GSMMoteShieldV1ServerProvider::recognizeUnsolicitedEvent(byte oldTail)
{

	int nlength;
	char auxLocate [15];
	
	//REMOTE SOCKET CLOSED.
	prepareAuxLocate(PSTR("CLOSED\r\n"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.gss.cb.locate(auxLocate))
	{
		//To detect remote socket closed for example inside socket data.
		theGSMMoteShieldV1ModemCore.setStatus(GPRS_READY);
	}
	
	
	//REMOTE SOCKET ACCEPTED.
	prepareAuxLocate(PSTR("CONNECT\r\n"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.gss.cb.locate(auxLocate))
	{
		//To detect remote socket closed for example inside socket data.
		theGSMMoteShieldV1ModemCore.theBuffer().chopUntil(auxLocate, true);
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		theGSMMoteShieldV1ModemCore.setStatus(TRANSPARENT_CONNECTED);
		return true;
	}
	
	return false;
}

bool GSMMoteShieldV1ServerProvider::getStatusSocketAsServer(uint8_t socket)
{
	return(theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED);
};

void GSMMoteShieldV1ServerProvider::releaseSocket(int socket)
{
}

int GSMMoteShieldV1ServerProvider::getNewOccupiedSocketAsServer()
{
	return 0;
}
