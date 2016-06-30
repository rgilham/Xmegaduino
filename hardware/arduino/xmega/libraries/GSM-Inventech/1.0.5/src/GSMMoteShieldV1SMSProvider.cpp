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
#include <GSMMoteShieldV1SMSProvider.h>
#include <Arduino.h>
	
GSMMoteShieldV1SMSProvider::GSMMoteShieldV1SMSProvider()
{
	theGSMMoteSMSProvider=this;
};

//Send SMS begin function.
int GSMMoteShieldV1SMSProvider::beginSMS(const char* to)
{
	if((theGSMMoteShieldV1ModemCore.getStatus() != GSM_READY)&&(theGSMMoteShieldV1ModemCore.getStatus() != GPRS_READY))
	  return 2;

	theGSMMoteShieldV1ModemCore.setPhoneNumber((char*)to);
	theGSMMoteShieldV1ModemCore.openCommand(this,BEGINSMS);
	beginSMSContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Send SMS continue function.
void GSMMoteShieldV1SMSProvider::beginSMSContinue()
{
	bool resp;
	// 1: Send AT
	// 2: wait for > and write text
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CMGS=\""), false);
		theGSMMoteShieldV1ModemCore.print(theGSMMoteShieldV1ModemCore.getPhoneNumber());
		theGSMMoteShieldV1ModemCore.print("\"\r");
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp, ">"))
		{
			if (resp) theGSMMoteShieldV1ModemCore.closeCommand(1);
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		break;
	}
}

//Send SMS write function.
void GSMMoteShieldV1SMSProvider::writeSMS(char c)
{
	theGSMMoteShieldV1ModemCore.write(c);
}

//Send SMS begin function.
int GSMMoteShieldV1SMSProvider::endSMS()
{
	theGSMMoteShieldV1ModemCore.openCommand(this,ENDSMS);
	endSMSContinue();
	while(ready()==0) delay(100);
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Send SMS continue function.
void GSMMoteShieldV1SMSProvider::endSMSContinue()
{
	bool resp;
	// 1: Send #26
	// 2: wait for OK
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		theGSMMoteShieldV1ModemCore.write(26);
		theGSMMoteShieldV1ModemCore.print("\r");
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			if (resp) 
				theGSMMoteShieldV1ModemCore.closeCommand(1);
			else 
				theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		break;
	}
}

//Available SMS main function.
int GSMMoteShieldV1SMSProvider::availableSMS()
{
	flagReadingSMS = 0;
	theGSMMoteShieldV1ModemCore.openCommand(this,AVAILABLESMS);
	availableSMSContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Available SMS continue function.
void GSMMoteShieldV1SMSProvider::availableSMSContinue()
{
	// 1:  AT+CMGL="REC UNREAD",1
	// 2: Receive +CMGL: _id_ ... READ","_numero_" ... \n_mensaje_\nOK
	// 3: Send AT+CMGD= _id_
	// 4: Receive OK
	// 5: Remaining SMS text in case full buffer.
	// This implementation really does not care much if the modem aswers trash to CMGL
	bool resp;
	//int msglength_aux;
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:	
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CMGL=\"REC UNREAD\",1"));
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(parseCMGL_available(resp))
			{
				if (!resp) theGSMMoteShieldV1ModemCore.closeCommand(4);
				else theGSMMoteShieldV1ModemCore.closeCommand(1);
			}
		break;
	}
	  
}	
		
//SMS available parse.
bool GSMMoteShieldV1SMSProvider::parseCMGL_available(bool& rsp)
{
	fullBufferSMS = (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()<=4);
	if (!(theGSMMoteShieldV1ModemCore.theBuffer().chopUntil("+CMGL:", true)))
		rsp = false;
	else 
		rsp = true;
	idSMS=theGSMMoteShieldV1ModemCore.theBuffer().readInt();

	//If there are 2 SMS in buffer, response is ...CRLFCRLF+CMGL
	twoSMSinBuffer = theGSMMoteShieldV1ModemCore.theBuffer().locate("\r\n\r\n+");

	checkSecondBuffer = 0;
	
	return true;
}

//remoteNumber SMS function.
int GSMMoteShieldV1SMSProvider::remoteSMSNumber(char* number, int nlength)
{
	theGSMMoteShieldV1ModemCore.theBuffer().extractSubstring("READ\",\"", "\"", number, nlength);	
	
	return 1;
}

//remoteNumber SMS function.
int GSMMoteShieldV1SMSProvider::readSMS()
{
	char charSMS;
	//First char.
	if (!flagReadingSMS) 
	{
		flagReadingSMS = 1;
		theGSMMoteShieldV1ModemCore.theBuffer().chopUntil("\n", true);
	}
	charSMS = theGSMMoteShieldV1ModemCore.theBuffer().read(); 
	
	//Second Buffer.
	if (checkSecondBuffer)
	{
		checkSecondBuffer = 0;
		twoSMSinBuffer = theGSMMoteShieldV1ModemCore.theBuffer().locate("\r\n\r\n+");
	}

	//Case the last char in buffer.
	if ((!twoSMSinBuffer)&&fullBufferSMS&&(theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()==127))
	{
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		fullBufferSMS = 0;
		checkSecondBuffer = 1;
		theGSMMoteShieldV1ModemCore.openCommand(this,XON);
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		delay(10);
		
		return charSMS;
	}
	//Case two SMS in buffer
	else if (twoSMSinBuffer)
	{
		if (theGSMMoteShieldV1ModemCore.theBuffer().locate("\r\n\r\n+")) 
		{
					return charSMS;
		}
		else 
		{
			theGSMMoteShieldV1ModemCore.theBuffer().flush();
			theGSMMoteShieldV1ModemCore.openCommand(this,XON);
			theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
			delay(10);
			return 0;
		}
	}
	//Case 1 SMS and buffer not full
	else if (!fullBufferSMS)
	{
		if (theGSMMoteShieldV1ModemCore.theBuffer().locate("\r\n\r\nOK")) 
		{
			return charSMS;
		}
		else 
		{
			theGSMMoteShieldV1ModemCore.theBuffer().flush();
			theGSMMoteShieldV1ModemCore.openCommand(this,XON);
			theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
			delay(10);
			return 0;
		}
	}
	//Case to read all the chars in buffer to the end.
	else 
	{
		return charSMS;		
	}
}	

//Read socket main function.
int GSMMoteShieldV1SMSProvider::peekSMS()
{
	if (!flagReadingSMS) 
	{
		flagReadingSMS = 1;
		theGSMMoteShieldV1ModemCore.theBuffer().chopUntil("\n", true);
	}

	return theGSMMoteShieldV1ModemCore.theBuffer().peek(0); 
}
	
//Flush SMS main function.
void GSMMoteShieldV1SMSProvider::flushSMS()
{

	//With this, sms data can fill up to 2x128+5x128 bytes.
	for (int aux = 0;aux<5;aux++)
	{
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		delay(10);
	}
		
	theGSMMoteShieldV1ModemCore.openCommand(this,FLUSHSMS);
	flushSMSContinue();
}

//Send SMS continue function.
void GSMMoteShieldV1SMSProvider::flushSMSContinue()
{
	bool resp;
	// 1: Deleting SMS
	// 2: wait for OK
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CMGD="), false);
		theGSMMoteShieldV1ModemCore.print(idSMS);
		theGSMMoteShieldV1ModemCore.print("\r");
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			if (resp) theGSMMoteShieldV1ModemCore.closeCommand(1);
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		break;
	}
}

void GSMMoteShieldV1SMSProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
/*		case XON:
			if (flagReadingSocket) 
				{
//					flagReadingSocket = 0;
					fullBufferSocket = (theGSMMoteShieldV1ModemCore.theBuffer().availableBytes()<3);
				}
			else theGSMMoteShieldV1ModemCore.openCommand(this,NONE);
			break;
*/		case NONE:
			theGSMMoteShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case BEGINSMS:
			beginSMSContinue();
			break;
		case ENDSMS:
			endSMSContinue();
			break;
		case AVAILABLESMS:
			availableSMSContinue();
			break;
		case FLUSHSMS:
			flushSMSContinue();
			break;
	}
}
