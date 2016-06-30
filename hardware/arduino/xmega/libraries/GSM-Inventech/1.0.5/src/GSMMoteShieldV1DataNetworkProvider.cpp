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
#include <GSMMoteShieldV1DataNetworkProvider.h>
#include <Arduino.h>

const char _command_CGATT[] PROGMEM = "AT+CGATT=";
const char _command_SEPARATOR[] PROGMEM = "\",\"";

//Attach GPRS main function.	
GSMMote_NetworkStatus_t GSMMoteShieldV1DataNetworkProvider::attachGPRS(char* apn, char* user_name, char* password, bool synchronous)
{					
	user = user_name;
	passwd = password;
	// A sad use of byte reuse
	theGSMMoteShieldV1ModemCore.setPhoneNumber(apn);

	theGSMMoteShieldV1ModemCore.openCommand(this,ATTACHGPRS);
	theGSMMoteShieldV1ModemCore.setStatus(CONNECTING);

	attachGPRSContinue();

	// If synchronous, wait till attach is over, or not.
	if(synchronous)
	{
		// if we shorten this delay, the command fails
		while(ready()==0) 
			delay(100); 
	}

	return theGSMMoteShieldV1ModemCore.getStatus();	
}

//Atthach GPRS continue function.
void GSMMoteShieldV1DataNetworkProvider::attachGPRSContinue()
{
	bool resp;
	// 1: Attach to GPRS service "AT+CGATT=1"
	// 2: Wait attach OK and Set the context 0 as FGCNT "AT+QIFGCNT=0"
	// 3: Wait context OK and Set bearer type as GPRS, APN, user name and pasword "AT+QICSGP=1..."
	// 4: Wait bearer OK and Enable the function of MUXIP "AT+QIMUX=1" 
	// 5: Wait for disable MUXIP OK and Set the session mode as non transparent "AT+QIMODE=0"
	// 6: Wait for session mode OK and Enable notification when data received "AT+QINDI=1"
	// 8: Wait domain name OK and Register the TCP/IP stack "AT+QIREGAPP"
	// 9: Wait for Register OK and Activate FGCNT "AT+QIACT"
	// 10: Wait for activate OK
	
	int ct=theGSMMoteShieldV1ModemCore.getCommandCounter();
	if(ct==1)
	{
		//AT+CGATT	
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_CGATT,false);
		theGSMMoteShieldV1ModemCore.print(1);
		theGSMMoteShieldV1ModemCore.print('\r');
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
	}
	else if(ct==2)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				//AT+QIFGCNT
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIFGCNT=0"));
				theGSMMoteShieldV1ModemCore.setCommandCounter(3);
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
				// Great. Go for the next step
				//DEBUG
				//Serial.println("AT+QICSGP.");	
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QICSGP=1,\""),false);
				theGSMMoteShieldV1ModemCore.print(theGSMMoteShieldV1ModemCore.getPhoneNumber());
				theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_SEPARATOR,false);
				theGSMMoteShieldV1ModemCore.print(user);
				theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_SEPARATOR,false);
				theGSMMoteShieldV1ModemCore.print(passwd);
				theGSMMoteShieldV1ModemCore.print("\"\r");
				theGSMMoteShieldV1ModemCore.setCommandCounter(4);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==4)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				// AT+QIMUX=1 for multisocket
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIMUX=0"));
				theGSMMoteShieldV1ModemCore.setCommandCounter(5);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==5)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				//AT+QIMODE=0 for multisocket
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIMODE=1"));
				theGSMMoteShieldV1ModemCore.setCommandCounter(6);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==6)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				// AT+QINDI=1
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QINDI=1"));
				theGSMMoteShieldV1ModemCore.setCommandCounter(8);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==8)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				// AT+QIREGAPP
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIREGAPP"));
				theGSMMoteShieldV1ModemCore.setCommandCounter(9);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==9)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if(resp)
			{
				// AT+QIACT	
				theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QIACT"));
				theGSMMoteShieldV1ModemCore.setCommandCounter(10);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
	else if(ct==10)
	{
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			if (resp) 
				{
					theGSMMoteShieldV1ModemCore.setStatus(GPRS_READY);
					theGSMMoteShieldV1ModemCore.closeCommand(1);
				}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}
	}
}

//Detach GPRS main function.
GSMMote_NetworkStatus_t GSMMoteShieldV1DataNetworkProvider::detachGPRS(bool synchronous)
{
	theGSMMoteShieldV1ModemCore.openCommand(this,DETACHGPRS);
	theGSMMoteShieldV1ModemCore.setStatus(CONNECTING);
	detachGPRSContinue();
	
	if(synchronous)
	{
		while(ready()==0) 
			delay(1); 
	}
	
	return theGSMMoteShieldV1ModemCore.getStatus();
}

void GSMMoteShieldV1DataNetworkProvider::detachGPRSContinue()
{
	bool resp;
	// 1: Detach to GPRS service "AT+CGATT=0"
	// 2: Wait dettach +PDP DEACT 
	// 3: Wait for OK

	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		//AT+CGATT=0
		theGSMMoteShieldV1ModemCore.genericCommand_rq(_command_CGATT,false);
		theGSMMoteShieldV1ModemCore.print(0);
		theGSMMoteShieldV1ModemCore.print('\r');
		theGSMMoteShieldV1ModemCore.setCommandCounter(2);
		break;
	case 2:
		char auxLocate[12];
		prepareAuxLocate(PSTR("+PDP DEACT"), auxLocate);
		if(theGSMMoteShieldV1ModemCore.theBuffer().locate(auxLocate))
	    {
			if(resp)
			{
				// Received +PDP DEACT;				
				theGSMMoteShieldV1ModemCore.setCommandCounter(3);
			}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}	
		break;
	case 3:
		if(theGSMMoteShieldV1ModemCore.genericParse_rsp(resp))
	    {
			// OK received
			if (resp) 
				{
					theGSMMoteShieldV1ModemCore.setStatus(GSM_READY);
					theGSMMoteShieldV1ModemCore.closeCommand(1);
				}
			else theGSMMoteShieldV1ModemCore.closeCommand(3);
		}		
		theGSMMoteShieldV1ModemCore.theBuffer().flush();
		theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
		break;	
	}
}

//QILOCIP parse.
bool GSMMoteShieldV1DataNetworkProvider::parseQILOCIP_rsp(char* LocalIP, int LocalIPlength, bool& rsp)
{
	if (!(theGSMMoteShieldV1ModemCore.theBuffer().extractSubstring("\r\n","\r\n", LocalIP, LocalIPlength)))
		rsp = false;
	else 
		rsp = true;
	return true;
}

//Get IP main function.
int GSMMoteShieldV1DataNetworkProvider::getIP(char* LocalIP, int LocalIPlength)
{
	theGSMMoteShieldV1ModemCore.setPhoneNumber(LocalIP);
	theGSMMoteShieldV1ModemCore.setPort(LocalIPlength);
	theGSMMoteShieldV1ModemCore.openCommand(this,GETIP);
	getIPContinue();
	return theGSMMoteShieldV1ModemCore.getCommandError();
}

void GSMMoteShieldV1DataNetworkProvider::getIPContinue()
{

	bool resp;
	// 1: Read Local IP "AT+QILOCIP"
	// 2: Waiting for IP.

	switch (theGSMMoteShieldV1ModemCore.getCommandCounter()) {
	case 1:
		//AT+QILOCIP
		theGSMMoteShieldV1ModemCore.genericCommand_rq(PSTR("AT+QILOCIP"));
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
}

//Get IP with IPAddress object
IPAddress GSMMoteShieldV1DataNetworkProvider::getIPAddress() {
	char ip_temp[15]="";
	getIP(ip_temp, 15);
	unsigned long m=millis();

	while((millis()-m)<10*1000 && (!ready())){
		// wait for a response from the modem:
		delay(100);
	} 
	IPAddress ip;
	inet_aton(ip_temp, ip);
	return ip;
}

int GSMMoteShieldV1DataNetworkProvider::inet_aton(const char* aIPAddrString, IPAddress& aResult)
{
    // See if we've been given a valid IP address
    const char* p =aIPAddrString;
    while (*p &&
           ( (*p == '.') || (*p >= '0') || (*p <= '9') ))
    {
        p++;
    }

    if (*p == '\0')
    {
        // It's looking promising, we haven't found any invalid characters
        p = aIPAddrString;
        int segment =0;
        int segmentValue =0;
        while (*p && (segment < 4))
        {
            if (*p == '.')
            {
                // We've reached the end of a segment
                if (segmentValue > 255)
                {
                    // You can't have IP address segments that don't fit in a byte
                    return 0;
                }
                else
                {
                    aResult[segment] = (byte)segmentValue;
                    segment++;
                    segmentValue = 0;
                }
            }
            else
            {
                // Next digit
                segmentValue = (segmentValue*10)+(*p - '0');
            }
            p++;
        }
        // We've reached the end of address, but there'll still be the last
        // segment to deal with
        if ((segmentValue > 255) || (segment > 3))
        {
            // You can't have IP address segments that don't fit in a byte,
            // or more than four segments
            return 0;
        }
        else
        {
            aResult[segment] = (byte)segmentValue;
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

//Response management.
void GSMMoteShieldV1DataNetworkProvider::manageResponse(byte from, byte to)
{
	switch(theGSMMoteShieldV1ModemCore.getOngoingCommand())
	{
		case ATTACHGPRS:
			attachGPRSContinue();
			break;	
		case DETACHGPRS:
			detachGPRSContinue();
			break;
		case GETIP:
			getIPContinue();
			break;		
	}
}
