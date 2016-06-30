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
#include <GSMMoteShieldV1DirectModemProvider.h>
#include <GSMMoteShieldV1ModemCore.h>
#include <HardwareSerial.h>
#include <Arduino.h>

#include <GSMMoteIO.h>



//Constructor
GSMMoteShieldV1DirectModemProvider::GSMMoteShieldV1DirectModemProvider(bool t)
{
	trace=true;
};

void GSMMoteShieldV1DirectModemProvider::begin()
{
	theGSMMoteShieldV1ModemCore.gss.begin(115200);
}

void GSMMoteShieldV1DirectModemProvider::restartModem()
{
	pinMode(__RESETPIN__, OUTPUT);
	digitalWrite(__RESETPIN__, HIGH);
	delay(300);
	digitalWrite(__RESETPIN__, LOW);
	delay(1000);

	pinMode(__POWERPIN__, OUTPUT);
	digitalWrite(__POWERPIN__, HIGH);
	delay(1100);
	digitalWrite(__POWERPIN__, LOW);
	delay(3000);


}

//To enable the debug process
void GSMMoteShieldV1DirectModemProvider::connect()
{
        theGSMMoteShieldV1ModemCore.registerActiveProvider(this);
}

//To disable the debug process
void GSMMoteShieldV1DirectModemProvider::disconnect()
{
        theGSMMoteShieldV1ModemCore.registerActiveProvider(0);
}

//Write to the modem by means of SoftSerial
size_t GSMMoteShieldV1DirectModemProvider::write(uint8_t c)
{	
        theGSMMoteShieldV1ModemCore.write(c);
}

//Detect if data to be read
int/*bool*/ GSMMoteShieldV1DirectModemProvider::available()
{
	if (theGSMMoteShieldV1ModemCore.gss.cb.peek(1)) return 1;
	else return 0;
} 

//Read data
int/*char*/ GSMMoteShieldV1DirectModemProvider::read()
{
	int dataRead;
	dataRead = theGSMMoteShieldV1ModemCore.gss.cb.read();
	//In case last char in xof mode.
	if (!(theGSMMoteShieldV1ModemCore.gss.cb.peek(0))) {
			theGSMMoteShieldV1ModemCore.gss.spaceAvailable();
			delay(100);
		}
	return dataRead;
} 

//Peek data
int/*char*/ GSMMoteShieldV1DirectModemProvider::peek()
{
	return theGSMMoteShieldV1ModemCore.gss.cb.peek(0);
} 

//Flush data
void GSMMoteShieldV1DirectModemProvider::flush()
{
	return theGSMMoteShieldV1ModemCore.gss.cb.flush();
}

String GSMMoteShieldV1DirectModemProvider::writeModemCommand(String ATcommand, int responseDelay)
{

  if(trace)
	Serial.println(ATcommand);
	
  // Flush other texts
  flush();
  
  //Enter debug mode.
  connect();
  //Send the AT command.
  println(ATcommand);

  delay(responseDelay);


  //Get response data from modem.
  String result = "";
  if(trace)
	theGSMMoteShieldV1ModemCore.gss.cb.debugBuffer();

  while (available())
  {
    char c = read();
    result += c;
  }
  if(trace)
	Serial.println(result);
  //Leave the debug mode.
  disconnect();
  return result;
}
