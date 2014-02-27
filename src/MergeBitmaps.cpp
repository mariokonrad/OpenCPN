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

#include "MergeBitmaps.h"

#include <wx/image.h>

// This function can be used to create custom bitmap blending for platforms
// where 32 bit bitmap ops are broken. Can hopefully be removed for wxWidgets 3.0...
wxBitmap MergeBitmaps(wxBitmap back, wxBitmap front, wxSize offset)
{
	wxBitmap merged(back.GetWidth(), back.GetHeight(), back.GetDepth());
#if (!wxCHECK_VERSION(2,9,4) && (defined(__WXGTK__) || defined(__WXMAC__)))

	// Manual alpha blending for broken wxWidgets platforms.
	merged.UseAlpha();
	back.UseAlpha();
	front.UseAlpha();

	wxImage im_front = front.ConvertToImage();
	wxImage im_back = back.ConvertToImage();
	// Only way to make result have alpha channel in wxW 2.8.
	wxImage im_result = back.ConvertToImage();

	unsigned char* presult = im_result.GetData();
	unsigned char* pback = im_back.GetData();
	unsigned char* pfront = im_front.GetData();

	unsigned char* afront = NULL;
	if (im_front.HasAlpha())
		afront = im_front.GetAlpha();

	unsigned char* aback = NULL;
	if (im_back.HasAlpha())
		aback = im_back.GetAlpha();

	unsigned char* aresult = NULL;
	if (im_result.HasAlpha())
		aresult = im_result.GetAlpha();

	// Do alpha blending, associative version of "over" operator.

	for (int i = 0; i < back.GetHeight(); i++) {
		for (int j = 0; j < back.GetWidth(); j++) {

			int fX = j - offset.x;
			int fY = i - offset.y;

			bool inFront = true;
			if (fX < 0 || fY < 0)
				inFront = false;
			if (fX >= front.GetWidth())
				inFront = false;
			if (fY >= front.GetHeight())
				inFront = false;

			if (inFront) {
				double alphaF = (double)(*afront++) / 256.0;
				double alphaB = (double)(*aback++) / 256.0;
				double alphaRes = alphaF + alphaB * (1.0 - alphaF);
				unsigned char a = alphaRes * 256;
				*aresult++ = a;
				unsigned char r = (*pfront++ * alphaF + *pback++ * alphaB * (1.0 - alphaF))
								  / alphaRes;
				*presult++ = r;
				unsigned char g = (*pfront++ * alphaF + *pback++ * alphaB * (1.0 - alphaF))
								  / alphaRes;
				*presult++ = g;
				unsigned char b = (*pfront++ * alphaF + *pback++ * alphaB * (1.0 - alphaF))
								  / alphaRes;
				*presult++ = b;
			} else {
				*aresult++ = *aback++;
				*presult++ = *pback++;
				*presult++ = *pback++;
				*presult++ = *pback++;
			}
		}
	}

	merged = wxBitmap(im_result);

#else
	wxMemoryDC mdc(merged);
	mdc.DrawBitmap(back, 0, 0, true);
	mdc.DrawBitmap(front, offset.x, offset.y, true);
	mdc.SelectObject(wxNullBitmap);
#endif

	return merged;
}

