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

#ifndef __OCPNMEMDC__H__
#define __OCPNMEMDC__H__

#include <wx/dcmemory.h>
#include "ocpn_pixel.h"

class OCPNMemDC : public wxMemoryDC
{
		DECLARE_DYNAMIC_CLASS(OCPNMemDC)

	public:
		OCPNMemDC();

		// Satisfy wxX11 2.8.0
		void SelectObject(wxBitmap& bitmap){wxMemoryDC::SelectObject(bitmap);}

		// Add a method to select a DIB section directly into the DC
#ifdef ocpnUSE_DIBSECTION
		void SelectObject(wxDIB & dib);
#endif

#ifdef ocpnUSE_DIBSECTION
	private:
		wxDIB * m_pselectedDIB;
#endif
};

#endif
