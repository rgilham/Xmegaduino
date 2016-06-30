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
#include <GSMMoteShieldV1MultiClientProvider.h>
#include <GSMMoteShieldV1ModemCore.h>

const char _command_MultiQISRVC[] PROGMEM = "AT+QISRVC=";

#define __TOUTFLUSH__ 10000

GSMMoteShieldV1MultiClientProvider::GSMMoteShieldV1MultiClientProvider()
{
	theGSMMoteMobileClientProvider=this;
	theGSMMoteShieldV1ModemCore.registerUMProvider(this);
};

//Response management.
void GSMMoteShieldV1MultiClientProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case XON:
			if (flagReadingSocket) 
				{
//					flagReadingSocket = 0;
					fullBufferSocket = (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()<3);
				}
			else theGSMMoteShieldV1ModemCore.setOngoingCommand(NONE);
			break;
		case NONE:
			theGSMMoteShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case CONNECTTCPCLIENT:
			connectTCPClientContinue();
			break;
		case DISCONNECTTCP:
			disconnectTCPContinue();
			break;	
	 	case BEGINWRITESOCKET:
			beginWriteSocketContinue();
			break;
		case ENDWRITESOCKET:
			endWriteSocketContinue();
			break;	
		case AVAILABLESOCKET:
			availableSocketContinue();
			break;	
		case FLUSHSOCKET:
			fullBufferSocket = (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()<3);
			flushSocketContinue();
			break;	
	}
}

//Connect TCP main function.
int GSMMoteShieldV1MultiClientProvider::connectTCPClient(const char* server, int port, int id_socket)
{
	theGSMMoteShieldV1ModemCore.setPort(port);		
	idSocket = id_socket;
	
	theGSMMoteShieldV1ModemCore.setPhoneNumber((char*)server);
	theGSMMoteShieldV1ModemCore.openCommand(this,CONNECTTCPCLIENT);
	connectTCPClientContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();	
}

int GSMMoteShieldV1MultiClientProvider::connectTCPClient(IPAddress add, int port, int id_socket)
{
	remoteIP=add;
	theGSMMoteShieldV1ModemCore.setPhoneNumber(0);
	return connectTCPClient(0, port, id_socket);
}

//Connect TCP continue function.
void GSMMoteShieldV1MultiClientProvider::connectTCPClientContinue()
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
				theGSMMoteShieldV1ModemCore.print(idSocket);
				theGSMMoteShieldV1ModemCore.print(",\"TCP\",\"");
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
		prepareAuxLocate(PSTR("CONNECT OK"), auxLocate);
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp,auxLocate))
	    {
			// Response received
			if(resp)
			{
				// Received CONNECT OK
				// Great. We're done
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			}
			else 
				theGSMMoteShieldV1ModemCore.closeCommand(3);
		}		
		break;
		
	}
}

//Disconnect TCP main function.
int GSMMoteShieldV1MultiClientProvider::disconnectTCP(bool client1Server0, int id_socket)
{		
	idSocket = id_socket;
	
	// First of all, we will flush the socket synchronously
	unsigned long m;
	m=millis();
	flushSocket();
	while(((millis()-m)< __TOUTFLUSH__ )&&(ready()==0)) 
		delay(10);
		
	// Could not flush the communications... strange
	if(ready()==0)
	{
		theGSMMoteShieldV1ModemCore.setCommandError(2);
		return theGSMMoteShieldV1ModemCore.getCommandError();
	}
		
	// Set up the command
	client1_server0 = client1Server0;
	flagReadingSocket=0;
	theGSMMoteShieldV1ModemCore.openCommand(this,DISCONNECTTCP);
	disconnectTCPContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Disconnect TCP continue function
void GSMMoteShieldV1MultiClientProvider::disconnectTCPContinue()
{
	bool resp;
	// 1: Send AT+QISRVC
	// 2: "AT+QICLOSE"
	// 3: Wait for OK
	
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_MultiQISRVC, false);
		if (client1_server0) theGSMMoteShieldV1ModemCore.print('1');
		else theGSMMoteShieldV1ModemCore.print('2');
		theGSMMoteShieldV1ModemCore.print('\r');
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		// Parse response to QISRVC
		theGSMMoteShieldV1ModemCore.genericParse_rsp(resp);
		if(resp)
		{
			// Send QICLOSE command
			theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QICLOSE="),false);
			theGSMMoteShieldV1ModemCore.print(idSocket);
			theGSMMoteShieldV1ModemCore.print('\r');
			theGSMMoteShieldV1ModemCore.setCommandCounter(3);
		}
		else 
			theGSMMoteShieldV1ModemCore.closeCommand(3);
		break;
	case 3:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {		
			theGSMMoteShieldV1ModemCore.setCommandCounter(0);
			if (resp) 
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			else 
				theGSMMoteShieldV1ModemCore.closeCommand(3);
		}	
		break;
	}
}

//Write socket first chain main function.
void GSMMoteShieldV1MultiClientProvider::beginWriteSocket(bool client1Server0, int id_socket)
{
	idSocket = id_socket;	
	client1_server0 = client1Server0;
	theGSMMoteShieldV1ModemCore.openCommand(this,BEGINWRITESOCKET);
	beginWriteSocketContinue();
}

//Write socket first chain continue function.
void GSMMoteShieldV1MultiClientProvider::beginWriteSocketContinue()
{
	bool resp;
	// 1: Send AT+QISRVC
	// 2: Send AT+QISEND
	// 3: wait for > and Write text
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		// AT+QISRVC
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_MultiQISRVC, false);
		if (client1_server0) 
			theGSMMoteShieldV1ModemCore.print('1');
		else 
			theGSMMoteShieldV1ModemCore.print('2');
		theGSMMoteShieldV1ModemCore.print('\r');
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
    case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			// Response received
			if(resp)
			{
				// AT+QISEND
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QISEND="), false);
				theGSMMoteShieldV1ModemCore.print(idSocket);
				theGSMMoteShieldV1ModemCore.print('\r');
				theGSMMoteShieldV1ModemCore.setCommandCounter(3);
			}
			else
			{
				theGSMMoteShieldV1ModemCore.closeCommand(3);
			}
		}	
		break;
	case 3:
		char aux[2];
		aux[0]='>';
		aux[1]=0;
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp, aux))
		{
			if(resp)
			{
				// Received ">"
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			}
			else
			{
				theGSMMoteShieldV1ModemCore.closeCommand(3);
			}
		}
		break;
	}
}

//Write socket next chain function.
void GSMMoteShieldV1MultiClientProvider::writeSocket(const char* buf)
{
	theGSMMoteShieldV1ModemCore.print(buf);
}

//Write socket character function.
void GSMMoteShieldV1MultiClientProvider::writeSocket(char c)
{
	theGSMMoteShieldV1ModemCore.print(c);
}

//Write socket last chain main function.
void GSMMoteShieldV1MultiClientProvider::endWriteSocket()
{		
	theGSMMoteShieldV1ModemCore.openCommand(this,ENDWRITESOCKET);
	endWriteSocketContinue();
}

//Write socket last chain continue function.
void GSMMoteShieldV1MultiClientProvider::endWriteSocketContinue()
{
	bool resp;
	// 1: Write text (ctrl-Z)
	// 2: Wait for OK
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		theGSMMoteShieldV1ModemCore.write(26); // Ctrl-Z
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			// OK received
			if (resp) theGSMMoteShieldV1ModemCore.closeCommand(1);
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		break;
	}
}

//Available socket main function.
int GSMMoteShieldV1MultiClientProvider::availableSocket(bool client1Server0, int id_socket)
{
	if(flagReadingSocket==1)
	{
		theGSMMoteShieldV1ModemCore.setCommandError(1);
		return 1;
	}
	client1_server0 = client1Server0;
	idSocket = id_socket;	
	theGSMMoteShieldV1ModemCore.openCommand(this,AVAILABLESOCKET);
	availableSocketContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Available socket continue function.
void GSMMoteShieldV1MultiClientProvider::availableSocketContinue()
{
	bool resp;
	// 1: AT+QIRD
	// 2: Wait for OK and Next necessary AT+QIRD

	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIRD=0,"),false);
		if (client1_server0) 
			theGSMMoteShieldV1ModemCore.print('1');
		else 
			theGSMMoteShieldV1ModemCore.print('2');
		theGSMMoteShieldV1ModemCore.print(',');
		theGSMMoteShieldV1ModemCore.print(idSocket);
		theGSMMoteShieldV1ModemCore.print(",1500");
		// theGSMMoteShieldV1ModemCore.print(",120");
		theGSMMoteShieldV1ModemCore.print('\r');
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(parseQIRD_head(resp))
		{
			if (!resp)
			{
				theGSMMoteShieldV1ModemCore.closeCommand(4);
			}
			else 
			{
				flagReadingSocket=1;
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			}
		}
		else 
		{
			theGSMMoteShieldV1ModemCore.closeCommand(3);	
		}
		break;
	}
}
	
//Read Socket Parse head.
bool GSMMoteShieldV1MultiClientProvider::parseQIRD_head(bool& rsp)
{
	char _qird [8];
	prepareAuxLocate(PSTR("+QIRD:"), _qird);
	fullBufferSocket = (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()<3);
	if(theGSMMoteShieldV1ModemCore.theBuffer().locate(_qird)) 
	{		
		theGSMMoteShieldV1ModemCore.theBuffer().chopUntil(_qird, true);
		// Saving more memory, reuse _qird
		_qird[0]='\n';
		_qird[1]=0;
		theGSMMoteShieldV1ModemCore.theBuffer().chopUntil(_qird, true);
		rsp = true;			
		return true;
	}
	else if(theGSMMoteShieldV1ModemCore.theBuffer().locate("OK")) 
	{
		rsp = false;
		return true;
	}
	else
	{
		rsp = false;
		return false;
	}
}
/*		
//Read socket main function.
int GSMMoteShieldV1MultiClientProvider::readSocket()
{
	char charSocket;
	charSocket = theGSMMoteShieldV1ModemCore.theBuffer().read(); 
	//Case buffer not full
	if (!fullBufferSocket)
	{	
		//The last part of the buffer after data is CRLFOKCRLF
		if (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==125)
		{
			//Start again availableSocket function.
			flagReadingSocket=0;
			theGSMMoteShieldV1ModemCore.openCommand(this,AVAILABLESOCKET);
			availableSocketContinue();					
		}
	}
	else if (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==127)
	{
		// The buffer is full, no more action is possible until we have read()
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		flagReadingSocket = 1;
		theGSMMoteShieldV1ModemCore.openCommand(this,XON);
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		//A small delay to assure data received after xon.
		delay(10);
	}
	//To distinguish the case no more available data in socket.			
	if (ready()==1)	
		return charSocket;
	else 
		return 0;
}	
*/
int GSMMoteShieldV1MultiClientProvider::readSocket()
{
	char charSocket;
	
	if(theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==0)
	{
		Serial.println();Serial.println("*");
		return 0;
	}
		
	charSocket = theGSMMoteShieldV1ModemCore.theBuffer().read(); 
	//Case buffer not full
	if (!fullBufferSocket)
	{	
		//The last part of the buffer after data is CRLFOKCRLF
		if (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==125)
		{
			//Start again availableSocket function.
			flagReadingSocket=0;
			theGSMMoteShieldV1ModemCore.openCommand(this,AVAILABLESOCKET);
			availableSocketContinue();					
		}
	}
	else if (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()>=100)
	{
		// The buffer was full, we have to let the data flow again
		// theGSMMoteShieldV1ModemCore.theBuffer().flush();
		flagReadingSocket = 1;
		theGSMMoteShieldV1ModemCore.openCommand(this,XON);
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		//A small delay to assure data received after xon.
		delay(100);
		if(theGSMMoteShieldV1ModemCore.theBuffer().availableBytes() >=6)
			fullBufferSocket=false;
	}

	return charSocket;

}

//Read socket main function.
int GSMMoteShieldV1MultiClientProvider::peekSocket()
{
	return theGSMMoteShieldV1ModemCore.theBuffer().peek(0); 
}


//Flush SMS main function.
void GSMMoteShieldV1MultiClientProvider::flushSocket()
{
	flagReadingSocket=0;
	theGSMMoteShieldV1ModemCore.openCommand(this,FLUSHSOCKET);
	flushSocketContinue();
}

//Send SMS continue function.
void GSMMoteShieldV1MultiClientProvider::flushSocketContinue()
{
	bool resp;
	// 1: Deleting SMS
	// 2: wait for OK
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		//DEBUG
		//Serial.println("Flushing Socket.");	
			theGSMMoteShieldV1ModemCore.theBuffer().flush();
			if (fullBufferSocket) 
				{
					//Serial.println("Buffer flushed.");
					theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
				}
			else 
				{
					//Serial.println("Socket flushed completely.");
					theGSMMoteShieldV1ModemCore.closeCommand(1);
				}
		break;
	}
}

//URC recognize.
// Momentarily, we will not recognize "closes" in client mode
bool GSMMoteShieldV1MultiClientProvider::recognizeUnsolicitedEvent(byte oldTail)
{
	return false;
}

int GSMMoteShieldV1MultiClientProvider::getSocket(int socket)
{
	if(socket==-1)
	{
		int i;
		for(i=minSocket(); i<=maxSocket(); i++)
		{
			if (!(sockets&(0x0001<<i)))
			{
				sockets|=((0x0001)<<i);
				return i;
			}	
		}
	}
	else
	{
		if (!(sockets&(0x0001<<socket)))
		{
			sockets|=((0x0001)<<socket);
			return socket;
		}	
	}
	return -1;
}

void GSMMoteShieldV1MultiClientProvider::releaseSocket(int socket)
{
	if (sockets&((0x0001)<<socket))
		sockets^=((0x0001)<<socket);
}

bool GSMMoteShieldV1MultiClientProvider::getStatusSocketClient(uint8_t socket)
{
	if(socket>8)
		return 0;
	if(sockets&(0x0001<<socket))
		return 1;
	else
		return 0;
};



