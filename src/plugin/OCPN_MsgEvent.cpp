/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 **************************************************************************/

#include "OCPN_MsgEvent.h"

OCPN_MsgEvent::OCPN_MsgEvent(const OCPN_MsgEvent & event)
	: wxEvent(event)
	, m_MessageID(event.m_MessageID)
	, m_MessageText(event.m_MessageText)
{ }

OCPN_MsgEvent::OCPN_MsgEvent(wxEventType commandType, int id)
	: wxEvent(id, commandType)
{
}

OCPN_MsgEvent::~OCPN_MsgEvent()
{
}

wxEvent * OCPN_MsgEvent::Clone() const
{
	OCPN_MsgEvent *newevent = new OCPN_MsgEvent(*this);
	newevent->m_MessageID=this->m_MessageID.c_str();  // this enforces a deep copy of the string data
	newevent->m_MessageText=this->m_MessageText.c_str();
	return newevent;
}

wxString OCPN_MsgEvent::GetID()
{
	return m_MessageID;
}

wxString OCPN_MsgEvent::GetJSONText()
{
	return m_MessageText;
}

void OCPN_MsgEvent::SetID(const wxString &string)
{
	m_MessageID = string;
}

void OCPN_MsgEvent::SetJSONText(const wxString & string)
{
	m_MessageText = string;
}

