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

#ifndef __OCPN_ALERTDIALOG__H__
#define __OCPN_ALERTDIALOG__H__

#include <wx/dialog.h>

class OCPN_AlertDialog : public wxDialog
{
		DECLARE_CLASS(OCPN_AlertDialog)
		DECLARE_EVENT_TABLE()

	public:

		OCPN_AlertDialog();
		virtual ~OCPN_AlertDialog();
		virtual void Init();
		virtual bool Create(
				wxWindow *parent,
				wxWindowID id = wxID_ANY,
				const wxString& caption = _("OpenCPN Alert"),
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

	private:
		wxWindow * m_pparent;
};

#endif
