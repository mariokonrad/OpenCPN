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

#ifndef __RENDERFROMHPGL__H__
#define __RENDERFROMHPGL__H__

#include <wx/string.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>

#define HPGL_FILLED true

class wxDC;
class wxGCDC;
class wxBrush;
class wxPen;
class s52plib;

class RenderFromHPGL
{
	public:
		RenderFromHPGL(s52plib * plibarg);

		void SetTargetDC(wxDC * pdc);
		void SetTargetOpenGl();
		void SetTargetGCDC(wxGCDC * gdc);
		bool Render(char * str, char * col, wxPoint & r, wxPoint & pivot, double rot_angle = 0);

	private:
		const char* findColorNameInRef(char colorCode, char * col);
		void RotatePoint(wxPoint & point, double angle);
		wxPoint ParsePoint(wxString & argument);
		void SetPen();
		void Line(wxPoint from, wxPoint to);
		void Circle(wxPoint center, int radius, bool filled = false);
		void Polygon();

		s52plib * plib;
		int scaleFactor;

		wxDC* targetDC;
		wxGCDC* targetGCDC;

		wxColor penColor;
		wxPen* pen;
		wxColor brushColor;
		wxBrush* brush;
		long penWidth;

		int noPoints;
		wxPoint polygon[100];

		bool renderToDC;
		bool renderToOpenGl;
		bool renderToGCDC;
		bool havePushedOpenGlAttrib;
};

#endif
