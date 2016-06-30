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
#include <GSMMoteShieldV1ClientProvider.h>
#include <GSMMoteShieldV1ModemCore.h>

GSMMoteShieldV1ClientProvider::GSMMoteShieldV1ClientProvider()
{
	theGSMMoteMobileClientProvider=this;
};

//Response management.
void GSMMoteShieldV1ClientProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case NONE:
			theGSMMoteShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case CONNECTTCPCLIENT:
			connectTCPClientContinue();
			break;
		case FLUSHSOCKET:
			flushSocketContinue();
			break;	
	}
}

//Connect TCP main function.
int GSMMoteShieldV1ClientProvider::connectTCPClient(const char* server, int port, int id_socket)
{
	theGSMMoteShieldV1ModemCore.setPort(port);		
	idSocket = id_socket;
	
	theGSMMoteShieldV1ModemCore.setPhoneNumber((char*)server);
	theGSMMoteShieldV1ModemCore.openCommand(this,CONNECTTCPCLIENT);
	theGSMMoteShieldV1ModemCore.registerUMProvider(this);
	connectTCPClientContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();	
}

int GSMMoteShieldV1ClientProvider::connectTCPClient(IPAddress add, int port, int id_socket)
{
	remoteIP=add;
	theGSMMoteShieldV1ModemCore.setPhoneNumber(0);
	return connectTCPClient(0, port, id_socket);
}

//Connect TCP continue function.
void GSMMoteShieldV1ClientProvider::connectTCPClientContinue()
{
	bool resp;
	// 0: Dot or DNS notation activation
	// 1: Disable SW flow control 
	// 2: Waiting for IFC OK
	// 3: Start-up TCP connection "AT+QIOPEN"
	// 4: Wait for connection OK
	// 5: Wait for CONNECT

	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIDNSIP="), false);
		if ((theGSMMoteShieldV1ModemCore.getPhoneNumber()!=0)&&
			((*(theGSMMoteShieldV1ModemCore.getPhoneNumber())<'0')||((*(theGSMMoteShieldV1ModemCore.getPhoneNumber())>'9'))))
		{
			theGSMMoteShieldV1ModemCore.print('1');
			theGSMMoteShieldV1ModemCore.print('\r');
		}
		else 
		{
			theGSMMoteShieldV1ModemCore.print('0');
			theGSMMoteShieldV1ModemCore.print('\r');
		}
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			//Response received
			if(resp)
			{				
				// AT+QIOPEN
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIOPEN="),false);
				theGSMMoteShieldV1ModemCore.print("\"TCP\",\"");
				if(theGSMMoteShieldV1ModemCore.getPhoneNumber()!=0)
				{
					theGSMMoteShieldV1ModemCore.print(theGSMMoteShieldV1ModemCore.getPhoneNumber());
				}
				else
				{
					remoteIP.printTo(theGSMMoteShieldV1ModemCore);
				}
				theGSMMoteShieldV1ModemCore.print('"');
				theGSMMoteShieldV1ModemCore.print(',');
				theGSMMoteShieldV1ModemCore.print(theGSMMoteShieldV1ModemCore.getPort());
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
				// OK Received
				// Great. Go for the next step
				theGSMMoteShieldV1ModemCore.setCommandCounter(4);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}	
		break;
	case 4:
		char auxLocate [12];
		prepareAuxLocate(PSTR("CONNECT\r\n"), auxLocate);
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp,auxLocate))
	    {
			// Response received
			if(resp)
			{
				// Received CONNECT OK
				// Great. We're done
				theGSMMoteShieldV1ModemCore.setStatus(TRANSPARENT_CONNECTED);
				theGSMMoteShieldV1ModemCore.theBuffer().chopUntil(auxLocate, true);
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			}
			else 
				theGSMMoteShieldV1ModemCore.closeCommand(3);
		}		
		break;
		
	}
}

//Disconnect TCP main function.
int GSMMoteShieldV1ClientProvider::disconnectTCP(bool client1Server0, int id_socket)
{		
	// id Socket does not really mean anything, in this case we have
	// only one socket running
	theGSMMoteShieldV1ModemCore.openCommand(this,DISCONNECTTCP);
	
	// If we are not closed, launch the command
//[ZZ]	if(theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED)
//	{
		delay(1000);
		theGSMMoteShieldV1ModemCore.print("+++");
		delay(1000);
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QICLOSE"));
		theGSMMoteShieldV1ModemCore.setStatus(GPRS_READY);
//	}
	// Looks like it runs everytime, so we simply flush to death and go on
	do
	{
		// Empty the local buffer, and tell the modem to XON
		// If meanwhile we receive a DISCONNECT we should detect it as URC.
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		// Give some time for the buffer to refill
		delay(100);
		theGSMMoteShieldV1ModemCore.closeCommand(1);
	}while(theGSMMoteShieldV1ModemCore.theBuffer().storedBytes()>0);

	theGSMMoteShieldV1ModemCore.unRegisterUMProvider(this);
	return theGSMMoteShieldV1ModemCore.getCommandError();
}


//Write socket first chain main function.
void GSMMoteShieldV1ClientProvider::beginWriteSocket(bool client1Server0, int id_socket)
{
}


//Write socket next chain function.
void GSMMoteShieldV1ClientProvider::writeSocket(const char* buf)
{
	if(theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED)
		theGSMMoteShieldV1ModemCore.print(buf);
}

//Write socket character function.
void GSMMoteShieldV1ClientProvider::writeSocket(uint8_t c)
{
	if(theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED)
		theGSMMoteShieldV1ModemCore.print((char)c);
}

//Write socket last chain main function.
void GSMMoteShieldV1ClientProvider::endWriteSocket()
{		
}


//Available socket main function.
int GSMMoteShieldV1ClientProvider::availableSocket(bool client1Server0, int id_socket)
{
		
	if(!(theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED))
		theGSMMoteShieldV1ModemCore.closeCommand(4);
		
	if(theGSMMoteShieldV1ModemCore.theBuffer().storedBytes())
		theGSMMoteShieldV1ModemCore.closeCommand(1);
	else
		theGSMMoteShieldV1ModemCore.closeCommand(4);
		
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

int GSMMoteShieldV1ClientProvider::readSocket()
{
	char charSocket;
		
	if(theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==0)
	{
		return 0;
	}
	
	charSocket = theGSMMoteShieldV1ModemCore.theBuffer().read(); 
	
	if(theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==100)
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();

	return charSocket;

}

//Read socket main function.
int GSMMoteShieldV1ClientProvider::peekSocket()
{
	return theGSMMoteShieldV1ModemCore.theBuffer().peek(0); 
}


//Flush SMS main function.
void GSMMoteShieldV1ClientProvider::flushSocket()
{
	theGSMMoteShieldV1ModemCore.openCommand(this,FLUSHSOCKET);

	flushSocketContinue();
}

//Send SMS continue function.
void GSMMoteShieldV1ClientProvider::flushSocketContinue()
{
	// If we have incomed data
	if(theGSMMoteShieldV1ModemCore.theBuffer().storedBytes()>0)
	{
		// Empty the local buffer, and tell the modem to XON
		// If meanwhile we receive a DISCONNECT we should detect it as URC.
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
	}
	else 
	{
		//We're done		
		theGSMMoteShieldV1ModemCore.closeCommand(1);
	}
}

// URC recognize.
// Yes, we recognize "closes" in client mode
bool GSMMoteShieldV1ClientProvider::recognizeUnsolicitedEvent(byte oldTail)
{
	char auxLocate [12];
	prepareAuxLocate(PSTR("CLOSED"), auxLocate);

	if((theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED) & theGSMMoteShieldV1ModemCore.theBuffer().chopUntil(auxLocate, false, false))
	{
		theGSMMoteShieldV1ModemCore.setStatus(GPRS_READY);
		theGSMMoteShieldV1ModemCore.unRegisterUMProvider(this);
		return true;
	}
		
	return false;
}

int GSMMoteShieldV1ClientProvider::getSocket(int socket)
{
	return 0;
}

void GSMMoteShieldV1ClientProvider::releaseSocket(int socket)
{

}

bool GSMMoteShieldV1ClientProvider::getStatusSocketClient(uint8_t socket)
{
	return (theGSMMoteShieldV1ModemCore.getStatus()==TRANSPARENT_CONNECTED);

};



