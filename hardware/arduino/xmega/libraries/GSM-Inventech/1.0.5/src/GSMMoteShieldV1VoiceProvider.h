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

#ifndef _GSMMoteSHIELDV1VOICEPROVIDER_
#define _GSMMoteSHIELDV1VOICEPROVIDER_

#include <GSMMoteMobileVoiceProvider.h>
#include <GSMMoteShieldV1ModemCore.h>
#include <GSMMoteShieldV1BaseProvider.h>

class GSMMoteShieldV1VoiceProvider : public GSMMoteMobileVoiceProvider, public GSMMoteShieldV1BaseProvider
{
	public:
		
		/** Constructor */
		GSMMoteShieldV1VoiceProvider();
		
		/** initilizer, links with modem provider */
		void initialize();

		
		/** Manages modem response
			@param from 		Initial byte of buffer
			@param to 			Final byte of buffer
		 */
		void manageResponse(byte from, byte to);

		//Call functions.
		
		/** Launch a voice call
			@param number	 	Phone number to be called
			@return If asynchronous, returns 0. If synchronous, 1 if success, other if error
		 */
		int voiceCall(const char* number);
		
		/** Answer a voice call
			@return If asynchronous, returns 0. If synchronous, 1 if success, other if error
		 */
		int answerCall();
		
		/** Hang a voice call
			@return If asynchronous, returns 0. If synchronous, 1 if success, other if error
		 */
		int hangCall();
		
		/** Retrieve phone number of caller
			@param buffer		Buffer for copy phone number
			@param bufsize		Buffer size
			@return If asynchronous, returns 0. If synchronous, 1 if success, other if error
		 */
		int retrieveCallingNumber(char* buffer, int bufsize);
		
		/** Get last command status
			@return Returns 0 if last command is still executing, 1 success, >1 error
		 */
		int ready(){return GSMMoteShieldV1BaseProvider::ready();};
		
		/** Recognize URC
			@param oldTail		
			@return true if successful
		 */		
		bool recognizeUnsolicitedEvent(byte oldTail);
		
		/** Returns voice call status
			@return voice call status
		 */
		GSMMote_voiceCall_st getvoiceCallStatus(){ready(); return _voiceCallstatus;};
		
		/**	Set voice call status
			@param status		New status for voice call
		 */
		void setvoiceCallStatus(GSMMote_voiceCall_st status) { _voiceCallstatus = status; };

		
	private:
		
		int phonelength; // Phone number length
		
		GSMMote_voiceCall_st _voiceCallstatus; // The voiceCall status

		/** Continue to voice call function
		 */
		void voiceCallContinue();
		
		/** Continue to answer call function
		 */
		void answerCallContinue();
		
		/** Continue to hang call function
		 */
		void hangCallContinue();
		
		/** Continue to retrieve calling number function
		 */
		void retrieveCallingNumberContinue();
		
		/** Parse CLCC response from buffer
			@param number		Number initial for extract substring of response
			@param nlength		Substring length
			@return true if successful
		 */
		bool parseCLCC(char* number, int nlength);

};

#endif
