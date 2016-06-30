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
#include <GSMMoteShieldV1VoiceProvider.h>
#include <Arduino.h>

GSMMoteShieldV1VoiceProvider::GSMMoteShieldV1VoiceProvider()
 {
	phonelength=0;
	theGSMMoteMobileVoiceProvider=this;
 }
 
 void GSMMoteShieldV1VoiceProvider::initialize()
 {
 	theGSMMoteShieldV1ModemCore.registerUMProvider(this);
 }
 
//Voice Call main function.
int GSMMoteShieldV1VoiceProvider::voiceCall(const char* to)
{
	theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("ATD"),false);
	theGSMMoteShieldV1ModemCore.print(to);
	theGSMMoteShieldV1ModemCore.print(";\r");
	setvoiceCallStatus(CALLING);
	return 1;
}

//Retrieve calling number main function.
int GSMMoteShieldV1VoiceProvider::retrieveCallingNumber (char* buffer, int bufsize)
{
	theGSMMoteShieldV1ModemCore.setPhoneNumber(buffer);
	phonelength = bufsize;
	theGSMMoteShieldV1ModemCore.setCommandError(0);
	theGSMMoteShieldV1ModemCore.setCommandCounter(1);
	theGSMMoteShieldV1ModemCore.openCommand(this,RETRIEVECALLINGNUMBER);
	retrieveCallingNumberContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Retrieve calling number Continue function.
void GSMMoteShieldV1VoiceProvider::retrieveCallingNumberContinue()
{
	// 1:  AT+CLCC
	// 2: Receive +CLCC: 1,1,4,0,0,"num",129,""
	// This implementation really does not care much if the modem aswers trash to CMGL
	bool resp;
	//int msglength_aux;
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:	
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+CLCC"));
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(parseCLCC(theGSMMoteShieldV1ModemCore.getPhoneNumber(), phonelength))
		{
			theGSMMoteShieldV1ModemCore.closeCommand(1);
		}
		break;
	}	
}	

//CLCC parse.	
bool GSMMoteShieldV1VoiceProvider::parseCLCC(char* number, int nlength)
{
	theGSMMoteShieldV1ModemCore.theBuffer().extractSubstring("+CLCC: 1,1,4,0,0,\"","\"", number, nlength);
	theGSMMoteShieldV1ModemCore.theBuffer().flush();
	return true;
}	

//Answer Call main function.
int GSMMoteShieldV1VoiceProvider::answerCall()
{
	theGSMMoteShieldV1ModemCore.setCommandError(0);
	theGSMMoteShieldV1ModemCore.setCommandCounter(1);
	theGSMMoteShieldV1ModemCore.openCommand(this,ANSWERCALL);
	answerCallContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Answer Call continue function.
void GSMMoteShieldV1VoiceProvider::answerCallContinue()
{
	// 1: ATA
	// 2: Waiting for OK
	
	// This implementation really does not care much if the modem aswers trash to CMGL
	bool resp;
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		// ATA ;
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("ATA"));
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		   {
			   setvoiceCallStatus(TALKING);
			   if (resp) theGSMMoteShieldV1ModemCore.closeCommand(1);
			   else theGSMMoteShieldV1ModemCore.closeCommand(3);
			}
		break;
	}
}
		
//Hang Call main function.		
int GSMMoteShieldV1VoiceProvider::hangCall()
{
	theGSMMoteShieldV1ModemCore.setCommandError(0);
	theGSMMoteShieldV1ModemCore.setCommandCounter(1);
	theGSMMoteShieldV1ModemCore.openCommand(this,HANGCALL);
	hangCallContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

//Hang Call continue function.
void GSMMoteShieldV1VoiceProvider::hangCallContinue()
{
	// 1: ATH
	// 2: Waiting for OK
	
	bool resp;
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		//ATH
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("ATH"));
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
		   setvoiceCallStatus(IDLE_CALL);
		   if (resp) theGSMMoteShieldV1ModemCore.closeCommand(1);
		   else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		break;
	}
}		

//Response management.
void GSMMoteShieldV1VoiceProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case ANSWERCALL:
			answerCallContinue();
			break;
		case HANGCALL:
			hangCallContinue();
			break;
		case RETRIEVECALLINGNUMBER:
			retrieveCallingNumberContinue();
			break;	

	}
}

//URC recognize.
bool GSMMoteShieldV1VoiceProvider::recognizeUnsolicitedEvent(byte oldTail)
{

	int nlength;
	char auxLocate [15];
	//RING.
	prepareAuxLocate(PSTR("RING"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.theBuffer().locate(auxLocate))
	{
		// RING
		setvoiceCallStatus(RECEIVINGCALL);
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		return true;
	}
	
	//CALL ACEPTED.
	prepareAuxLocate(PSTR("+COLP:"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.theBuffer().locate(auxLocate))
	{
		//DEBUG
		//Serial.println("Call Accepted.");
		setvoiceCallStatus(TALKING);
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		return true;
	}	
	
	//NO CARRIER.
	prepareAuxLocate(PSTR("NO CARRIER"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.theBuffer().locate(auxLocate))
	{
		//DEBUG
		//Serial.println("NO CARRIER received.");
		setvoiceCallStatus(IDLE_CALL);
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		return true;
	}
	
	//BUSY.
	prepareAuxLocate(PSTR("BUSY"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.theBuffer().locate(auxLocate))
	{
		//DEBUG	
		//Serial.println("BUSY received.");
		setvoiceCallStatus(IDLE_CALL);
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		return true;
	}	
	
	//CALL RECEPTION.
	prepareAuxLocate(PSTR("+CLIP:"), auxLocate);
	if(theGSMMoteShieldV1ModemCore.theBuffer().locate(auxLocate))
	{
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		setvoiceCallStatus(RECEIVINGCALL);
		return true;
	}
	
	return false;
}


