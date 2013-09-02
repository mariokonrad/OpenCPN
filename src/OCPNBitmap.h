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

#ifndef __OCPNBITMAP__H__
#define __OCPNBITMAP__H__

#include <wx/bitmap.h>
#include "ocpn_pixel.h"

class wxImage;

/// Derived from wxBitmap
///
/// Why?....
/// wxWidgets does a very correct, but sometimes slow job of bitmap creation
/// and copying. ocpnBitmap is an optimization of wxBitmap for specific
/// platforms and color formats.
///
/// ocpn_Bitmap is optimized specifically for Windows and X11 Linux/Unix systems,
/// taking advantage of some known underlying data structures and formats.
///
/// There is (currently) no optimization for for other platforms,
/// such as GTK or MAC
///
/// The included methods are very different for MSW and X11 See the Code
class OCPNBitmap : public wxBitmap // TODO: right now, this class is never used (always disabled by preprocessor), why keep it?
{
		DECLARE_DYNAMIC_CLASS(OCPNBitmap)

	public:
		// default ctor creates an invalid bitmap, you must Create() it later
		OCPNBitmap();

		OCPNBitmap(unsigned char * pPix, int width, int height, int depth)
		{
			CreateFromData(pPix, width, height, depth);
		}

		OCPNBitmap(const wxImage & image, int depth)
		{
			CreateFromImage(image, depth);
		}

#ifdef __WXX11__
		OCPNBitmap(ocpnXImage * ocpn_Ximage, int width, int height, int depth)
		{
			CreateFromocpnXImage(ocpn_Ximage, width, height, depth);
		}
#endif

	protected:
		bool CreateFromData(void * pPix, int width, int height, int depth);
		bool CreateFromImage(const wxImage & image, int depth);

#ifdef __WXX11__
		bool CreateFromocpnXImage(ocpnXImage * poXI, int width, int height, int depth);
#endif
};

#endif
