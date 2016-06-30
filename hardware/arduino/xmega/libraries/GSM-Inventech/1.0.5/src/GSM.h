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
#ifndef _GSMMoteSIMPLIFIERFILE_
#define _GSMMoteSIMPLIFIERFILE_

// This file simplifies the use of the GSMMote library
// First we include everything. 

#include <GSMMoteCircularBuffer.h>
#include <GSMMoteMobileCellManagement.h>
#include <GSMMoteMobileClientService.h>
#include <GSMMoteMobileNetworkRegistry.h>
#include <GSMMoteMobileServerService.h>
#include <GSMMoteShieldV1AccessProvider.h>
#include <GSMMoteShieldV1BandManagement.h>
#include <GSMMoteShieldV1ClientProvider.h>
#include <GSMMoteShieldV1DataNetworkProvider.h>
#include <GSMMoteShieldV1ModemVerification.h>
#include <GSMMoteShieldV1CellManagement.h>
#include <GSMMoteShieldV1PinManagement.h>
#include <GSMMoteShieldV1ScanNetworks.h>
#include <GSMMoteSMSService.h>
#include <GSMMoteVoiceCallService.h>

#define GSM GSMMoteShieldV1AccessProvider
#define GPRS GSMMoteShieldV1DataNetworkProvider
#define GSMClient GSMMoteMobileClientService
#define GSMServer GSMMoteMobileServerService
#define GSMVoiceCall GSMMoteVoiceCallService
#define GSM_SMS GSMMoteSMSService

#define GSMPIN GSMMoteShieldV1PinManagement
#define GSMModem GSMMoteShieldV1ModemVerification
#define GSMCell GSMMoteShieldV1CellManagement
#define GSMBand GSMMoteShieldV1BandManagement
#define GSMScanner GSMMoteShieldV1ScanNetworks

#endif