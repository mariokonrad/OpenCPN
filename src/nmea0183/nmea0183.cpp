/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Samuel R. Blackburn, David S Register           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#include "nmea0183.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

NMEA0183::NMEA0183()
{
	initialize();

//	response_table.push_back( (RESPONSE *) &Aam );
//	response_table.push_back( (RESPONSE *) &Alm );
//	response_table.push_back( (RESPONSE *) &Apb );
//	response_table.push_back( (RESPONSE *) &Asd );
//	response_table.push_back( (RESPONSE *) &Bec );
//	response_table.push_back( (RESPONSE *) &Bod );
//	response_table.push_back( (RESPONSE *) &Bwc );
//	response_table.push_back( (RESPONSE *) &Bwr );
//	response_table.push_back( (RESPONSE *) &Bww );
//	response_table.push_back( (RESPONSE *) &Dbt );
//	response_table.push_back( (RESPONSE *) &Dcn );
//	response_table.push_back( (RESPONSE *) &Dpt );
//	response_table.push_back( (RESPONSE *) &Fsi );
//	response_table.push_back( (RESPONSE *) &Gga );
//	response_table.push_back( (RESPONSE *) &Glc );
//	response_table.push_back( (RESPONSE *) &Gll );
//	response_table.push_back( (RESPONSE *) &Gxa );
//	response_table.push_back( (RESPONSE *) &Hsc );
//	response_table.push_back( (RESPONSE *) &Lcd );
//	response_table.push_back( (RESPONSE *) &Mtw );
//	response_table.push_back( (RESPONSE *) &Mwv );
//	response_table.push_back( (RESPONSE *) &Oln );
//	response_table.push_back( (RESPONSE *) &Osd );
//	response_table.push_back( (RESPONSE *) &Proprietary );
//	response_table.push_back( (RESPONSE *) &Rma );
	response_table.push_back((RESPONSE*)&Hdm);
	response_table.push_back((RESPONSE*)&Hdg);
	response_table.push_back((RESPONSE*)&Hdt);
	response_table.push_back((RESPONSE*)&Rmb);
	response_table.push_back((RESPONSE*)&Rmc);
	response_table.push_back((RESPONSE*)&Wpl);
	response_table.push_back((RESPONSE*)&Rte);
	response_table.push_back((RESPONSE*)&Gll);
	response_table.push_back((RESPONSE*)&Vtg);
	response_table.push_back((RESPONSE*)&Gsv);
	response_table.push_back((RESPONSE*)&Gga);
	response_table.push_back((RESPONSE*)&GPwpl);
	response_table.push_back((RESPONSE*)&Apb);
//	response_table.push_back( (RESPONSE *) &Rot );
//	response_table.push_back( (RESPONSE *) &Rpm );
//	response_table.push_back( (RESPONSE *) &Rsa );
//	response_table.push_back( (RESPONSE *) &Rsd );
//	response_table.push_back( (RESPONSE *) &Sfi );
//	response_table.push_back( (RESPONSE *) &Stn );
//	response_table.push_back( (RESPONSE *) &Trf );
//	response_table.push_back( (RESPONSE *) &Ttm );
//	response_table.push_back( (RESPONSE *) &Vbw );
//	response_table.push_back( (RESPONSE *) &Vhw );
//	response_table.push_back( (RESPONSE *) &Vdr );
//	response_table.push_back( (RESPONSE *) &Vlw );
//	response_table.push_back( (RESPONSE *) &Vpw );
//	response_table.push_back( (RESPONSE *) &Vtg );
//	response_table.push_back( (RESPONSE *) &Wcv );
//	response_table.push_back( (RESPONSE *) &Wnc );
//	response_table.push_back( (RESPONSE *) &Xdr );
//	response_table.push_back( (RESPONSE *) &Xte );
//	response_table.push_back( (RESPONSE *) &Xtr );
//	response_table.push_back( (RESPONSE *) &Zda );
//	response_table.push_back( (RESPONSE *) &Zfo );
//	response_table.push_back( (RESPONSE *) &Ztg );

   sort_response_table();
   set_container_pointers();
}

NMEA0183::~NMEA0183()
{
   initialize();
}

void NMEA0183::initialize(void)
{
	ErrorMessage.Empty();
}

void NMEA0183::set_container_pointers(void)
{
	int index = 0;
	int number_of_entries_in_table = response_table.size();

	RESPONSE* this_response = (RESPONSE*)NULL;

	index = 0;

	while (index < number_of_entries_in_table) {
		this_response = (RESPONSE*)response_table[index];
		this_response->SetContainer(this);
		index++;
	}
}

void NMEA0183::sort_response_table(void)
{
}

bool NMEA0183::IsGood(void) const
{
	// NMEA 0183 sentences begin with $ and and with CR LF

	if (sentence.Sentence[0] != '$') {
		return false;
	}

	// Next to last character must be a CR

	return true;
}

bool NMEA0183::PreParse(void)
{
	if (IsGood()) {
		wxString mnemonic = sentence.Field(0);

		// See if this is a proprietary field

		if (mnemonic.Left(1) == 'P')
			mnemonic = _T("P");
		else
			mnemonic = mnemonic.Right(3);

		LastSentenceIDReceived = mnemonic;

		return true;
	}
	return false;
}

bool NMEA0183::Parse(void)
{
	bool return_value = false;

	if (PreParse()) {
		wxString mnemonic = sentence.Field(0);

		// See if this is a proprietary field

		if (mnemonic.Left(1) == 'P') {
			mnemonic = _T("P");
		} else {
			mnemonic = mnemonic.Right(3);
		}

		// Set up our default error message

		ErrorMessage = mnemonic;
		ErrorMessage += _T(" is an unknown type of sentence");

		LastSentenceIDReceived = mnemonic;

		RESPONSE* response_p = (RESPONSE*)NULL;

		// Traverse the response list to find a mnemonic match

		int comparison = 0;
		for (MRL::iterator node = response_table.begin(); node != response_table.end(); ++node) {
			RESPONSE* resp = *node;

			comparison = mnemonic.Cmp(resp->Mnemonic);

			if (comparison == 0) {
				response_p = resp;
				return_value = response_p->Parse(sentence);

				// Set your ErrorMessage

				if (return_value == true) {
					ErrorMessage = _T("No Error");
					LastSentenceIDParsed = response_p->Mnemonic;
					TalkerID = talker_id(sentence);
					ExpandedTalkerID = expand_talker_id(TalkerID);
				} else {
					ErrorMessage = response_p->ErrorMessage;
				}
				break;
			}
		}
	} else {
		return_value = false;
	}

	return return_value;
}

wxArrayString NMEA0183::GetRecognizedArray(void)
{
	wxArrayString ret;

	for (MRL::iterator node = response_table.begin(); node != response_table.end(); ++node) {
		ret.Add((*node)->Mnemonic);
	}

	return ret;
}

NMEA0183& NMEA0183::operator<<(wxString& source)
{
	sentence = source;
	return *this;
}

NMEA0183& NMEA0183::operator>>(wxString& destination)
{
	destination = sentence;

	return *this;
}

