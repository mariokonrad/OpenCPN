/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Sean D'Epagnier                                 *
 *   sean at depagnier dot com                                             *
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

#ifndef __OCPNDC_H__
#define __OCPNDC_H__

#include <vector>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/font.h>
#include <wx/colour.h>
#include "decl_exp.h"

#if wxUSE_GRAPHICS_CONTEXT
class wxGraphicsContext;
#endif

class wxGLCanvas;
class wxDC;

class DECL_EXP ocpnDC // FIXME: redesign: do not support wxGLCanvas and wxDC, use polymorphism
{
public:
	ocpnDC(wxGLCanvas& canvas);
	ocpnDC(wxDC& pdc);
	ocpnDC();

	~ocpnDC();

	void SetBackground(const wxBrush& brush);
	void SetPen(const wxPen& pen);
	void SetBrush(const wxBrush& brush);
	void SetTextForeground(const wxColour& colour);
	void SetFont(const wxFont& font);

	const wxPen& GetPen() const;
	const wxBrush& GetBrush() const;
	const wxFont& GetFont() const;

	void GetSize(wxCoord* width, wxCoord* height) const;

	void StrokeLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2);
	void StrokeLine(wxPoint a, wxPoint b);
	void StrokeLines(int n, wxPoint* points);
	void StrokePolygon(int n, wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0);
	void StrokeCircle(wxCoord x, wxCoord y, wxCoord radius);

	void Clear();
	void DrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, bool b_hiqual = true);
	void DrawLines(int n, wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0,
				   bool b_hiqual = true);
	void DrawRectangle(wxCoord x, wxCoord y, wxCoord w, wxCoord h);
	void DrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord w, wxCoord h, wxCoord rr);
	void DrawCircle(wxCoord x, wxCoord y, wxCoord radius);
	void DrawCircle(const wxPoint& pt, wxCoord radius);
	void DrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height);
	void DrawPolygon(int n, wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0);
	void DrawBitmap(const wxBitmap& bitmap, wxCoord x, wxCoord y, bool usemask);
	void DrawText(const wxString& text, wxCoord x, wxCoord y);

	void DrawPolygonTessellated(int n, wxPoint points[], wxCoord xoffset = 0, wxCoord yoffset = 0);

	void GetTextExtent(const wxString& string, wxCoord* w, wxCoord* h, wxCoord* descent = NULL,
					   wxCoord* externalLeading = NULL, wxFont* font = NULL) const;

	void ResetBoundingBox();
	void CalcBoundingBox(wxCoord x, wxCoord y);

	void DestroyClippingRegion();

	void AlphaBlending(int x, int y, int size_x, int size_y, float radius, wxColour color,
					   unsigned char transparency);

	wxDC* GetDC() const;

	// this is static public, because it must be accessible by the GLU tesselation
	// callback functions. It is only in use during the runtime of DrawPolygonTessellated
	static wxArrayPtrVoid gTesselatorVertices;

private:
	bool ConfigurePen();
	bool ConfigureBrush();

	static void SetGLAttrs(bool highQuality);
	void DrawGLThickLine(double x1, double y1, double x2, double y2, wxPen pen,
						 bool b_hiqual) const;
	void drawrrhelper(wxCoord x, wxCoord y, wxCoord r, double st, double et) const;
	void SetGLStipple() const;

	void GLDrawBlendData(wxCoord x, wxCoord y, wxCoord w, wxCoord h, int format,
						 const unsigned char* data);

	wxGLCanvas* glcanvas;
	wxDC* dc;
#if wxUSE_GRAPHICS_CONTEXT
	wxGraphicsContext* pgc;
#endif

	wxPen m_pen;
	wxBrush m_brush;
	wxColour m_textforegroundcolour;
	wxFont m_font;

	unsigned int tex;
};

#endif
