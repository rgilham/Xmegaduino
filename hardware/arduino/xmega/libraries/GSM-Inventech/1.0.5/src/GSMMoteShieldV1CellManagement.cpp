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
#include <GSMMoteShieldV1CellManagement.h>

GSMMoteShieldV1CellManagement::GSMMoteShieldV1CellManagement()
{
}

bool GSMMoteShieldV1CellManagement::parseQCCID_available(bool& rsp)
{
/*	char c;
	bool iccidFound = false;
	int i = 0;
	
	while(((c = theGSMMoteShieldV1ModemCore.theBuffer().read()) != 0) & (i < 19))
	{
		if((c < 58) & (c > 47))
			iccidFound = true;
		
		if(iccidFound)
		{
			bufferICCID[i] = c;
			i++;
		}
	}
	bufferICCID[i]=0;*/
	
	return true;
}			

bool GSMMoteShieldV1CellManagement::parseQENG_available(bool& rsp)
{
/*	char c;
	char location[50] = "";
	int i = 0;
	
	if (!(theGSMMoteShieldV1ModemCore.theBuffer().chopUntil("+QENG: ", true)))
		rsp = false;
	else 
		rsp = true;
	
	if (!(theGSMMoteShieldV1ModemCore.theBuffer().chopUntil("+QENG:", true)))
		rsp = false;
	else 
		rsp = true;
	
	while(((c = theGSMMoteShieldV1ModemCore.theBuffer().read()) != 0) & (i < 50))
	{
		location[i] = c;
		i++;
	}
	location[i]=0;
	
	char* res_tok = strtok(location, ",");
	res_tok=strtok(NULL, ",");
	strcpy(countryCode, res_tok);
	res_tok=strtok(NULL, ",");
	strcpy(networkCode, res_tok);
	res_tok=strtok(NULL, ",");
	strcpy(locationArea, res_tok);
	res_tok=strtok(NULL, ",");
	strcpy(cellId, res_tok);*/
	
	return true;
}			

int GSMMoteShieldV1CellManagement::getLocation(char *country, char *network, char *area, char *cell)
{
	if((theGSMMoteShieldV1ModemCore.getStatus() != GSM_READY) && (theGSMMoteShieldV1ModemCore.getStatus() != GPRS_READY))
		return 2;
	
	countryCode=country;
	networkCode=network;
	locationArea=area;
	cellId=cell;
	
	theGSMMoteShieldV1ModemCore.openCommand(this,GETLOCATION);
	getLocationContinue();
	
	unsigned long timeOut = millis();
	while(((millis() - timeOut) < 5000) & (ready() == 0));

	return theGSMMoteShieldV1ModemCore.getCommandError();
}

void GSMMoteShieldV1CellManagement::getLocationContinue()
{
	bool resp;
	
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		//theGSMMoteShieldV1ModemCore.gss.tunedDelay(3000);
/*		delay(3000);
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QENG=1"), false);
		theGSMMoteShieldV1ModemCore.print("\r");*/
		break;
	case 2:
		if (theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
/*			theGSMMoteShieldV1ModemCore.gss.tunedDelay(3000);
			delay(3000);
			theGSMMoteShieldV1ModemCore.setCommandCounter(3);
			theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QENG?"), false);
			theGSMMoteShieldV1ModemCore.print("\r");*/
		}
		else theGSMMoteShieldV1ModemCore.closeCommand(1);
		break;
	case 3:
		if (resp)
		{
			parseQENG_available(resp);
			theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
		else theGSMMoteShieldV1ModemCore.closeCommand(2);
		break;
	}
}

int GSMMoteShieldV1CellManagement::getICCID(char *iccid)
{
	if((theGSMMoteShieldV1ModemCore.getStatus() != GSM_READY) && (theGSMMoteShieldV1ModemCore.getStatus() != GPRS_READY))
		return 2;
	
	bufferICCID=iccid;
	theGSMMoteShieldV1ModemCore.openCommand(this,GETICCID);
	getICCIDContinue();
	
	unsigned long timeOut = millis();
	while(((millis() - timeOut) < 5000) & (ready() == 0));
		
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

void GSMMoteShieldV1CellManagement::getICCIDContinue()
{
	bool resp;
	
	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
    case 1:
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QCCID"), false);
		theGSMMoteShieldV1ModemCore.print("\r");
		break;
	case 2:
		if (theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
		{
			parseQCCID_available(resp);
			theGSMMoteShieldV1ModemCore.closeCommand(2);
		}
		else theGSMMoteShieldV1ModemCore.closeCommand(1);
		break;
	}
}

void GSMMoteShieldV1CellManagement::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case NONE:
			//theGSMMoteShieldV1ModemCore.gss.cb.deleteToTheEnd(from);
			break;
		case GETLOCATION:
			getLocationContinue();
			break;
		case GETICCID:
			getICCIDContinue();
			break;
	}
}
