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

#include "OCPNMemDC.h"

namespace graphics {

IMPLEMENT_DYNAMIC_CLASS(OCPNMemDC, wxMemoryDC)

OCPNMemDC::OCPNMemDC()
{
}


#ifdef ocpnUSE_DIBSECTION
void OCPNMemDC::SelectObject(wxDIB& dib)
{
	// select old bitmap out of the device context
	if ( m_oldBitmap )
	{
		::SelectObject(GetHdc(), (HBITMAP) m_oldBitmap);
		if ( m_selectedBitmap.Ok() )
		{
			m_selectedBitmap = wxNullBitmap;
		}
	}

	// check for whether the bitmap is already selected into a device context
	//    wxASSERT_MSG( !bitmap.GetSelectedInto() ||
	//                  (bitmap.GetSelectedInto() == this),
	//                  wxT("Bitmap is selected in another wxMemoryDC, delete the first wxMemoryDC or use SelectObject(NULL)") );

	m_pselectedDIB = &dib;
	HBITMAP hDIB = m_pselectedDIB->GetHandle();
	if ( !hDIB)
		return; // already selected


	hDIB = (HBITMAP)::SelectObject(GetHdc(), hDIB);

	if ( !hDIB )
	{
		wxLogLastError(wxT("SelectObject(OCPNMemDC, DIB)"));
		wxFAIL_MSG(wxT("Couldn't select a DIB into OCPNMemDC"));
	} else if (!m_oldBitmap) {
		m_oldBitmap = hDIB;
	}
}

#endif

}

