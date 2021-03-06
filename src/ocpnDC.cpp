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

#include "ocpnDC.h"

#include <global/OCPN.h>
#include <global/GUI.h>

#ifdef __MSVC__
	#include <windows.h>
#endif

#include <vector>

#ifdef ocpnUSE_GL
	#include <wx/glcanvas.h>
	#ifdef __WXMSW__
		#include "GL/gl.h"  // local copy for Windows
		#include "GL/glu.h"
		#include "GL/glext.h"
	#else
		#include <GL/gl.h>
		#include <GL/glu.h>
		#include <GL/glext.h>
	#endif
#endif

#include <wx/graphics.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/log.h>
#include <wx/image.h>

wxArrayPtrVoid ocpnDC::gTesselatorVertices;

// pass the dc to the constructor, or NULL to use opengl
ocpnDC::ocpnDC(wxGLCanvas & canvas)
	: glcanvas(&canvas)
	, dc(NULL)
	, pgc(NULL)
	, m_pen(wxNullPen)
	, m_brush(wxNullBrush)
	, m_textforegroundcolour(0, 0, 0)
	, tex(0)
{
#ifdef ocpnUSE_GL
	glGenTextures(1, &tex);
#endif
}

ocpnDC::ocpnDC(wxDC & pdc)
	: glcanvas(NULL)
	, dc(&pdc)
	, pgc(NULL)
	, m_pen(wxNullPen)
	, m_brush(wxNullBrush)
	, m_textforegroundcolour(0, 0, 0)
	, tex(0)
{
#if wxUSE_GRAPHICS_CONTEXT
	wxMemoryDC * pmdc = wxDynamicCast(dc, wxMemoryDC);
	if (pmdc)
		pgc = wxGraphicsContext::Create(*pmdc);
	else {
		wxClientDC *pcdc = wxDynamicCast(dc, wxClientDC);
		if (pcdc)
			pgc = wxGraphicsContext::Create(*pcdc);
	}
#endif
}

ocpnDC::ocpnDC()
	: glcanvas(NULL)
	, dc(NULL)
	, pgc(NULL)
	, m_pen(wxNullPen)
	, m_brush(wxNullBrush)
	, tex(0)
{}

ocpnDC::~ocpnDC()
{
	if (pgc)
		delete pgc;
#ifdef ocpnUSE_GL
	if (glcanvas)
		glDeleteTextures(1, &tex);
#endif
}

wxDC* ocpnDC::GetDC() const
{
	return dc;
}

void ocpnDC::DestroyClippingRegion()
{}

void ocpnDC::Clear()
{
	if (dc) {
		dc->Clear();
	} else {
#ifdef ocpnUSE_GL
		wxBrush tmpBrush = m_brush;
		int w, h;
		SetBrush(wxBrush(glcanvas->GetBackgroundColour()));
		glcanvas->GetSize(&w, &h);
		DrawRectangle(0, 0, w, h);
		SetBrush(tmpBrush);
#endif
	}
}

void ocpnDC::SetBackground(const wxBrush& brush)
{
	if (dc)
		dc->SetBackground(brush);
	else {
#ifdef ocpnUSE_GL
		glcanvas->SetBackgroundColour(brush.GetColour());
#endif
	}
}

void ocpnDC::SetGLAttrs(bool highQuality)
{
#ifdef ocpnUSE_GL
	// Enable anti-aliased polys, at best quality
	if (highQuality) {
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	} else {
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_BLEND);
	}
#endif
}

void ocpnDC::SetPen(const wxPen& pen)
{
	if (dc) {
		if (pen == wxNullPen)
			dc->SetPen(*wxTRANSPARENT_PEN);
		else
			dc->SetPen(pen);
	} else
		m_pen = pen;
}

void ocpnDC::SetBrush(const wxBrush& brush)
{
	if (dc)
		dc->SetBrush(brush);
	else
		m_brush = brush;
}

void ocpnDC::SetTextForeground(const wxColour& colour)
{
	if (dc)
		dc->SetTextForeground(colour);
	else
		m_textforegroundcolour = colour;
}

void ocpnDC::SetFont(const wxFont& font)
{
	if (dc)
		dc->SetFont(font);
	else
		m_font = font;
}

const wxPen& ocpnDC::GetPen() const
{
	if (dc)
		return dc->GetPen();
	return m_pen;
}

const wxBrush& ocpnDC::GetBrush() const
{
	if (dc)
		return dc->GetBrush();
	return m_brush;
}

const wxFont& ocpnDC::GetFont() const
{
	if (dc)
		return dc->GetFont();
	return m_font;
}

void ocpnDC::GetSize(wxCoord* width, wxCoord* height) const
{
	if (dc)
		dc->GetSize(width, height);
	else {
#ifdef ocpnUSE_GL
		glcanvas->GetSize(width, height);
#endif
	}
}

void ocpnDC::SetGLStipple() const
{
#ifdef ocpnUSE_GL
	switch (m_pen.GetStyle()) {
		case wxDOT:
			glLineStipple(1, 0x3333);
			glEnable(GL_LINE_STIPPLE);
			break;

		case wxLONG_DASH:
			glLineStipple(1, 0xFFF8);
			glEnable(GL_LINE_STIPPLE);
			break;

		case wxSHORT_DASH:
			glLineStipple(1, 0x3F3F);
			glEnable(GL_LINE_STIPPLE);
			break;

		case wxDOT_DASH:
			glLineStipple(1, 0x8FF1);
			glEnable(GL_LINE_STIPPLE);
			break;

		default:
			break;
	}
#endif
}

// Draws a line between (x1,y1) - (x2,y2) with a start thickness of t1
void ocpnDC::DrawGLThickLine(double x1, double y1, double x2, double y2, wxPen pen,
							 bool b_hiqual) const
{
#ifdef ocpnUSE_GL

	double angle = atan2(y2 - y1, x2 - x1);
	double t1 = pen.GetWidth();
	double t2sina1 = t1 / 2 * sin(angle);
	double t2cosa1 = t1 / 2 * cos(angle);

	glPushAttrib(GL_COLOR_BUFFER_BIT | GL_HINT_BIT | GL_POLYGON_BIT); // Save state
	ocpnDC::SetGLAttrs(b_hiqual);

	// n.b.  The dwxDash interpretation for GL only allows for 2 elements in the dash table.
	// The first is assumed drawn, second is assumed space
	wxDash* dashes;
	int n_dashes = pen.GetDashes(&dashes);
	if (n_dashes) {
		double lpix = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
		double lrun = 0.;
		double xa = x1;
		double ya = y1;
		double ldraw = t1 * dashes[0];
		double lspace = t1 * dashes[1];

		while (lrun < lpix) {
			// Dash
			double xb = xa + ldraw * cos(angle);
			double yb = ya + ldraw * sin(angle);

			if ((lrun + ldraw) >= lpix) // last segment is partial draw
			{
				xb = x2;
				yb = y2;
			}

			glBegin(GL_TRIANGLES);
			glVertex2f(xa + t2sina1, ya - t2cosa1);
			glVertex2f(xb + t2sina1, yb - t2cosa1);
			glVertex2f(xb - t2sina1, yb + t2cosa1);

			glVertex2f(xb - t2sina1, yb + t2cosa1);
			glVertex2f(xa - t2sina1, ya + t2cosa1);
			glVertex2f(xa + t2sina1, ya - t2cosa1);
			glEnd();

			xa = xb;
			ya = yb;
			lrun += ldraw;

			// Space
			xb = xa + lspace * cos(angle);
			yb = ya + lspace * sin(angle);

			xa = xb;
			ya = yb;
			lrun += lspace;
		}
	} else {
		glBegin(GL_TRIANGLES);
		glVertex2f(x1 + t2sina1, y1 - t2cosa1);
		glVertex2f(x2 + t2sina1, y2 - t2cosa1);
		glVertex2f(x2 - t2sina1, y2 + t2cosa1);
		glVertex2f(x2 - t2sina1, y2 + t2cosa1);
		glVertex2f(x1 - t2sina1, y1 + t2cosa1);
		glVertex2f(x1 + t2sina1, y1 - t2cosa1);
		glEnd();
	}
	glPopAttrib();
#endif
}

void ocpnDC::DrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2, bool b_hiqual)
{
	if (dc)
		dc->DrawLine(x1, y1, x2, y2);
#ifdef ocpnUSE_GL
	else {
		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_ENABLE_BIT | GL_HINT_BIT); // Save state

		if (ConfigurePen()) {

			glDisable(GL_MULTISAMPLE);
			glDisable(GL_LINE_STIPPLE);

			bool b_draw_thick = false;
			const float min_line_width = global::OCPN::get().gui().view().GLMinLineWidth;

			float pen_width = wxMax(min_line_width, m_pen.GetWidth());

			// Enable anti-aliased lines, at best quality
			if (b_hiqual) {
				SetGLStipple();

				glEnable(GL_LINE_SMOOTH);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

				if (pen_width > 1.0) {
					GLint parms[2];
					glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0]);
					if (pen_width > parms[1])
						b_draw_thick = true;
					else
						glLineWidth(pen_width);
				} else
					glLineWidth(pen_width);
			} else {
				glDisable(GL_LINE_SMOOTH);
				glDisable(GL_BLEND);

				if (pen_width > 1) {
					GLint parms[2];
					glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
					if (pen_width > parms[1])
						b_draw_thick = true;
					else
						glLineWidth(pen_width);
				} else
					glLineWidth(pen_width);
			}

			if (b_draw_thick) {
				DrawGLThickLine(x1, y1, x2, y2, m_pen, b_hiqual);
			} else {
				wxDash* dashes;
				int n_dashes = m_pen.GetDashes(&dashes);
				if (n_dashes) {
					double angle = atan2((double)(y2 - y1), (double)(x2 - x1));
					double cosa = cos(angle);
					double sina = sin(angle);
					double t1 = m_pen.GetWidth();

					double lpix
						= sqrt((double)(x1 - x2) * (x1 - x2) + (double)(y1 - y2) * (y1 - y2));
					double lrun = 0.;
					double xa = x1;
					double ya = y1;
					double ldraw = t1 * dashes[0];
					double lspace = t1 * dashes[1];

					while (lrun < lpix) {
						// Dash
						double xb = xa + ldraw * cosa;
						double yb = ya + ldraw * sina;

						if ((lrun + ldraw) >= lpix) { // last segment is partial draw
							xb = x2;
							yb = y2;
						}

						glBegin(GL_LINES);
						glVertex2f(xa, ya);
						glVertex2f(xb, yb);
						glEnd();

						xa = xa + (lspace + ldraw) * cosa;
						ya = ya + (lspace + ldraw) * sina;
						lrun += lspace + ldraw;
					}
				} else // not dashed
				{
					glBegin(GL_LINES);
					glVertex2i(x1, y1);
					glVertex2i(x2, y2);
					glEnd();
				}
			}
		}

		glPopAttrib();
	}
#endif
}

void ocpnDC::DrawLines(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset, bool b_hiqual)
{
	if (dc)
		dc->DrawLines(n, points, xoffset, yoffset);
#ifdef ocpnUSE_GL
	else if (ConfigurePen()) {

		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT); // Save state
		SetGLAttrs(b_hiqual);

		bool b_draw_thick = false;
		const float min_line_width = global::OCPN::get().gui().view().GLMinLineWidth;

		glDisable(GL_LINE_STIPPLE);
		SetGLStipple();

		// Enable anti-aliased lines, at best quality
		if (b_hiqual) {
			if (m_pen.GetWidth() > 1) {
				GLint parms[2];
				glGetIntegerv(GL_SMOOTH_LINE_WIDTH_RANGE, &parms[0]);
				if (m_pen.GetWidth() > parms[1])
					b_draw_thick = true;
				else
					glLineWidth(wxMax(min_line_width, m_pen.GetWidth()));
			} else
				glLineWidth(wxMax(min_line_width, 1));
		} else {
			if (m_pen.GetWidth() > 1) {
				GLint parms[2];
				glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, &parms[0]);
				if (m_pen.GetWidth() > parms[1])
					b_draw_thick = true;
				else
					glLineWidth(wxMax(min_line_width, m_pen.GetWidth()));
			} else
				glLineWidth(wxMax(min_line_width, 1));
		}

		if (b_draw_thick) {
			wxPoint p0 = points[0];
			for (int i = 1; i < n; i++) {
				DrawGLThickLine(p0.x + xoffset, p0.y + yoffset, points[i].x + xoffset,
								points[i].y + yoffset, m_pen, b_hiqual);
				p0 = points[i];
			}
		} else {
			glBegin(GL_LINE_STRIP);
			for (int i = 0; i < n; i++)
				glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
			glEnd();
		}
		glPopAttrib(); // restore state
	}
#endif
}

void ocpnDC::StrokeLine(wxPoint a, wxPoint b)
{
	StrokeLine(a.x, a.y, b.x, b.y);
}

void ocpnDC::StrokeLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
#if wxUSE_GRAPHICS_CONTEXT
	if (pgc) {
		pgc->SetPen(dc->GetPen());
		pgc->StrokeLine(x1, y1, x2, y2);

		dc->CalcBoundingBox(x1, y1);
		dc->CalcBoundingBox(x2, y2);
	} else
#endif
		DrawLine(x1, y1, x2, y2, true);
}

void ocpnDC::StrokeLines(int n, wxPoint* points)
{
#if wxUSE_GRAPHICS_CONTEXT
	if (pgc) {
		wxPoint2DDouble* dPoints = (wxPoint2DDouble*)malloc(n * sizeof(wxPoint2DDouble));
		for (int i = 0; i < n; i++) {
			dPoints[i].m_x = points[i].x;
			dPoints[i].m_y = points[i].y;
		}
		pgc->SetPen(dc->GetPen());
		pgc->StrokeLines(n, dPoints);
		free(dPoints);
	} else
#endif
		DrawLines(n, points, 0, 0, true);
}

void ocpnDC::DrawRectangle(wxCoord x, wxCoord y, wxCoord w, wxCoord h)
{
	if (dc)
		dc->DrawRectangle(x, y, w, h);
#ifdef ocpnUSE_GL
	else {
		if (ConfigureBrush()) {
			glBegin(GL_QUADS);
			glVertex2i(x, y);
			glVertex2i(x + w, y);
			glVertex2i(x + w, y + h);
			glVertex2i(x, y + h);
			glEnd();
		}

		if (ConfigurePen()) {
			glBegin(GL_LINE_LOOP);
			glVertex2i(x, y);
			glVertex2i(x + w, y);
			glVertex2i(x + w, y + h);
			glVertex2i(x, y + h);
			glEnd();
		}
	}
#endif
}

// draw the arc along corners
void ocpnDC::drawrrhelper(wxCoord x, wxCoord y, wxCoord r, double st, double et) const
{
#ifdef ocpnUSE_GL
	const int slices = 10;
	double dt = (et - st) / slices;
	for (double t = st; t <= et; t += dt)
		glVertex2i(x + r * cos(t), y + r * sin(t));
#endif
}

void ocpnDC::DrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord w, wxCoord h, wxCoord r)
{
	if (dc)
		dc->DrawRoundedRectangle(x, y, w, h, r);
#ifdef ocpnUSE_GL
	else {
		wxCoord x0 = x;
		wxCoord x1 = x + r;
		wxCoord x2 = x + w - r;
		wxCoord x3 = x + h;
		wxCoord y0 = y;
		wxCoord y1 = y + r;
		wxCoord y2 = y + h - r;
		wxCoord y3 = y + h;
		if (ConfigureBrush()) {
			glBegin(GL_QUADS);
			glVertex2i(x0, y1);
			glVertex2i(x1, y1);
			glVertex2i(x1, y2);
			glVertex2i(x0, y2);

			glVertex2i(x1, y0);
			glVertex2i(x2, y0);
			glVertex2i(x2, y3);
			glVertex2i(x0, y3);

			glVertex2i(x2, y1);
			glVertex2i(x3, y1);
			glVertex2i(x3, y2);
			glVertex2i(x2, y2);
			glEnd();

			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(x1, y2);
			drawrrhelper(x1, y2, r, -M_PI, -M_PI / 2);
			glEnd();

			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(x2, y2);
			drawrrhelper(x2, y2, r, -M_PI / 2, 0);
			glEnd();

			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(x2, y1);
			drawrrhelper(x2, y1, r, 0, M_PI / 2);
			glEnd();

			glBegin(GL_TRIANGLE_FAN);
			glVertex2i(x1, y1);
			drawrrhelper(x1, y1, r, M_PI / 2, M_PI);
			glEnd();
		}

		if (ConfigurePen()) {
			glBegin(GL_LINE_LOOP);
			drawrrhelper(x1, y2, r, -M_PI, -M_PI / 2);
			drawrrhelper(x2, y2, r, -M_PI / 2, 0);
			drawrrhelper(x2, y1, r, 0, M_PI / 2);
			drawrrhelper(x1, y1, r, M_PI / 2, M_PI);
			glEnd();
		}
	}
#endif
}

void ocpnDC::DrawCircle(const wxPoint& pt, wxCoord radius)
{
	DrawCircle(pt.x, pt.y, radius);
}

void ocpnDC::DrawCircle(wxCoord x, wxCoord y, wxCoord radius)
{
	DrawEllipse(x - radius, y - radius, 2 * radius, 2 * radius);
}

void ocpnDC::StrokeCircle(wxCoord x, wxCoord y, wxCoord radius)
{
#if wxUSE_GRAPHICS_CONTEXT
	if (pgc) {
		wxGraphicsPath gpath = pgc->CreatePath();
		gpath.AddCircle(x, y, radius);

		pgc->SetPen(GetPen());
		pgc->SetBrush(GetBrush());
		pgc->DrawPath(gpath);

		// keep dc dirty box up-to-date
		dc->CalcBoundingBox(x + radius + 2, y + radius + 2);
		dc->CalcBoundingBox(x - radius - 2, y - radius - 2);
	} else
#endif
		DrawCircle(x, y, radius);
}

void ocpnDC::DrawEllipse(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
	if (dc)
		dc->DrawEllipse(x, y, width, height);
#ifdef ocpnUSE_GL
	else {
		double r1 = width / 2, r2 = height / 2;
		double cx = x + r1, cy = y + r2;

		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT); // Save state

		// Enable anti-aliased lines, at best quality
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		if (ConfigureBrush()) {
			glBegin(GL_TRIANGLE_FAN);
			glVertex2d(cx, cy);
			for (double a = 0; a <= 2 * M_PI; a += 2 * M_PI / 20)
				glVertex2d(cx + r1 * sin(a), cy + r2 * cos(a));
			glEnd();
		}

		if (ConfigurePen()) {
			glBegin(GL_LINE_STRIP);
			for (double a = 0; a <= 2 * M_PI; a += 2 * M_PI / 200)
				glVertex2d(cx + r1 * sin(a), cy + r2 * cos(a));
			glEnd();
		}

		glPopAttrib(); // restore state
	}
#endif
}

void ocpnDC::DrawPolygon(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
	if (dc) {
		dc->DrawPolygon(n, points, xoffset, yoffset);
	}
#ifdef ocpnUSE_GL
	else {
		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT | GL_POLYGON_BIT); // Save
																						// state

		SetGLAttrs(true);

		if (ConfigureBrush()) {
			glBegin(GL_POLYGON);
			for (int i = 0; i < n; i++)
				glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
			glEnd();
		}

		if (ConfigurePen()) {
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < n; i++)
				glVertex2i(points[i].x + xoffset, points[i].y + yoffset);
			glEnd();
		}
		glPopAttrib();
	}
#endif
}

#ifdef ocpnUSE_GL

typedef union
{
	GLdouble data[6];
	struct sGLvertex
	{
		GLdouble x;
		GLdouble y;
		GLdouble z;
		GLdouble r;
		GLdouble g;
		GLdouble b;
	} info;
} GLvertex;

void APIENTRY ocpnDCcombineCallback(GLdouble coords[3], GLdouble* vertex_data[4], GLfloat weight[4],
									GLdouble** dataOut)
{
	GLvertex* vertex;

	vertex = new GLvertex();
	ocpnDC::gTesselatorVertices.Add(vertex);

	vertex->info.x = coords[0];
	vertex->info.y = coords[1];
	vertex->info.z = coords[2];

	for (int i = 3; i < 6; i++) {
		vertex->data[i] = weight[0] * vertex_data[0][i] + weight[1] * vertex_data[1][i];
	}

	*dataOut = &(vertex->data[0]);
}

void APIENTRY ocpnDCvertexCallback(GLvoid* arg)
{
	GLvertex* vertex;
	vertex = (GLvertex*)arg;
	glVertex2d(vertex->info.x, vertex->info.y);
}

void APIENTRY ocpnDCerrorCallback(GLenum errorCode)
{
	const GLubyte* estring;
	estring = gluErrorString(errorCode);
	wxLogMessage(_T("OpenGL Tessellation Error: %s"), estring);
}

void APIENTRY ocpnDCbeginCallback(GLenum type)
{
	glBegin(type);
}

void APIENTRY ocpnDCendCallback()
{
	glEnd();
}
#endif // ocpnUSE_GL

void ocpnDC::DrawPolygonTessellated(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
	if (dc) {
		dc->DrawPolygon(n, points, xoffset, yoffset);
	}
#ifdef ocpnUSE_GL
	else {
		if (n < 5) {
			DrawPolygon(n, points, xoffset, yoffset);
			return;
		}

		glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT | GL_POLYGON_BIT); // Save
																						// state
		SetGLAttrs(false);

		static GLUtesselator* tobj = NULL;
		if (!tobj)
			tobj = gluNewTess();

		gluTessCallback(tobj, GLU_TESS_VERTEX, (_GLUfuncptr) & ocpnDCvertexCallback);
		gluTessCallback(tobj, GLU_TESS_BEGIN, (_GLUfuncptr) & ocpnDCbeginCallback);
		gluTessCallback(tobj, GLU_TESS_END, (_GLUfuncptr) & ocpnDCendCallback);
		gluTessCallback(tobj, GLU_TESS_COMBINE, (_GLUfuncptr) & ocpnDCcombineCallback);
		gluTessCallback(tobj, GLU_TESS_ERROR, (_GLUfuncptr) & ocpnDCerrorCallback);

		gluTessNormal(tobj, 0, 0, 1);
		gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

		if (ConfigureBrush()) {
			gluTessBeginPolygon(tobj, NULL);
			gluTessBeginContour(tobj);

			for (int i = 0; i < n; i++) {
				GLvertex* vertex = new GLvertex();
				gTesselatorVertices.Add(vertex);
				vertex->info.x = (GLdouble)points[i].x;
				vertex->info.y = (GLdouble)points[i].y;
				vertex->info.z = (GLdouble)0.0;
				vertex->info.r = (GLdouble)0.0;
				vertex->info.g = (GLdouble)0.0;
				vertex->info.b = (GLdouble)0.0;
				gluTessVertex(tobj, (GLdouble*)vertex, (GLdouble*)vertex);
			}
			gluTessEndContour(tobj);
			gluTessEndPolygon(tobj);
		}

		glPopAttrib();

		for (unsigned int i = 0; i < gTesselatorVertices.Count(); ++i)
			delete (GLvertex*)gTesselatorVertices.Item(i);
		gTesselatorVertices.Clear();
	}
#endif
}

void ocpnDC::StrokePolygon(int n, wxPoint points[], wxCoord xoffset, wxCoord yoffset)
{
#if wxUSE_GRAPHICS_CONTEXT
	if (pgc) {
		wxGraphicsPath gpath = pgc->CreatePath();
		gpath.MoveToPoint(points[0].x + xoffset, points[0].y + yoffset);
		for (int i = 1; i < n; i++)
			gpath.AddLineToPoint(points[i].x + xoffset, points[i].y + yoffset);
		gpath.AddLineToPoint(points[0].x + xoffset, points[0].y + yoffset);

		pgc->SetPen(GetPen());
		pgc->SetBrush(GetBrush());
		pgc->DrawPath(gpath);

		for (int i = 0; i < n; i++)
			dc->CalcBoundingBox(points[i].x + xoffset, points[i].y + yoffset);
	} else
#endif
		DrawPolygon(n, points, xoffset, yoffset);
}

void ocpnDC::DrawBitmap(const wxBitmap& bitmap, wxCoord x, wxCoord y, bool usemask)
{
	wxBitmap bmp;
	if (x < 0 || y < 0) {
		int dx = (x < 0 ? -x : 0);
		int dy = (y < 0 ? -y : 0);
		int w = bitmap.GetWidth() - dx;
		int h = bitmap.GetHeight() - dy;
		// picture is out of viewport
		if (w <= 0 || h <= 0)
			return;
		wxBitmap newBitmap = bitmap.GetSubBitmap(wxRect(dx, dy, w, h));
		x += dx;
		y += dy;
		bmp = newBitmap;
	} else {
		bmp = bitmap;
	}

	if (dc) {
		dc->DrawBitmap(bmp, x, y, usemask);
	}
#ifdef ocpnUSE_GL
	else {
		wxImage image = bmp.ConvertToImage();
		int w = image.GetWidth();
		int h = image.GetHeight();

		if (usemask) {
			unsigned char* d = image.GetData();
			unsigned char* a = image.GetAlpha();

			unsigned char mr, mg, mb;
			if (!image.GetOrFindMaskColour(&mr, &mg, &mb) && !a)
				printf("trying to use mask to draw a bitmap without alpha or mask\n");

			unsigned char* e = new unsigned char[4 * w * h];
			{
				for (int y = 0; y < h; y++)
					for (int x = 0; x < w; x++) {
						unsigned char r, g, b;
						int off = (y * image.GetWidth() + x);
						r = d[off * 3 + 0];
						g = d[off * 3 + 1];
						b = d[off * 3 + 2];

						e[off * 4 + 0] = r;
						e[off * 4 + 1] = g;
						e[off * 4 + 2] = b;

						e[off * 4 + 3] = a ? a[off]
										   : ((r == mr) && (g == mg) && (b == mb) ? 0 : 255);
					}
			}

			glColor4f(1, 1, 1, 1);
			GLDrawBlendData(x, y, w, h, GL_RGBA, e);
			delete[](e);
		} else {
			glRasterPos2i(x, y);
			glPixelZoom(1, -1); // draw data from top to bottom
			glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, image.GetData());
			glPixelZoom(1, 1);
		}
	}
#endif
}

void ocpnDC::DrawText(const wxString& text, wxCoord x, wxCoord y)
{
	if (dc) {
		dc->DrawText(text, x, y);
	}
#ifdef ocpnUSE_GL
		else {
		wxCoord w = 0;
		wxCoord h = 0;
#ifdef __WXMAC__
		wxBitmap tbmp(200, 200);
		wxMemoryDC mac_dc(tbmp);
		mac_dc.GetTextExtent(text, &w, &h, NULL, NULL, &m_font);
#else
		GetTextExtent(text, &w, &h);
#endif

		if (w && h) {
			// create bitmap of appropriate size and select it
			wxBitmap bmp(w, h);
			wxMemoryDC temp_dc;
			temp_dc.SelectObject(bmp);

			// fill bitmap with black
			temp_dc.SetBackground(wxBrush(wxColour(0, 0, 0)));
			temp_dc.Clear();

			// draw the text white
			temp_dc.SetFont(m_font);
			temp_dc.SetTextForeground(wxColour(255, 255, 255));
			temp_dc.DrawText(text, 0, 0);
			temp_dc.SelectObject(wxNullBitmap);

			// use the data in the bitmap for alpha channel,
			// and set the color to text foreground
			wxImage image = bmp.ConvertToImage();
			unsigned char* data = new unsigned char[image.GetWidth() * image.GetHeight()];
			unsigned char* im = image.GetData();
			for (int i = 0; i < w * h; i++)
				data[i] = im[3 * i];

			glColor4ub(m_textforegroundcolour.Red(), m_textforegroundcolour.Green(),
					   m_textforegroundcolour.Blue(), 255);
			GLDrawBlendData(x, y, w, h, GL_ALPHA, data);
			delete[] data;
		}
	}
#endif
}

void ocpnDC::GetTextExtent(const wxString& string, wxCoord* w, wxCoord* h, wxCoord* descent,
						   wxCoord* externalLeading, wxFont* font) const
{
	if (dc) {
		dc->GetTextExtent(string, w, h, descent, externalLeading, font);
	} else {
		wxFont f = m_font;
		if (font)
			f = *font;

		wxMemoryDC temp_dc;
		temp_dc.GetTextExtent(string, w, h, descent, externalLeading, &f);
	}
}

void ocpnDC::ResetBoundingBox()
{
	if (dc)
		dc->ResetBoundingBox();
}

void ocpnDC::CalcBoundingBox(wxCoord x, wxCoord y)
{
	if (dc)
		dc->CalcBoundingBox(x, y);
}

bool ocpnDC::ConfigurePen()
{
	if (m_pen == wxNullPen)
		return false;
	if (m_pen == *wxTRANSPARENT_PEN)
		return false;

#ifdef ocpnUSE_GL
	wxColour c = m_pen.GetColour();
	int width = m_pen.GetWidth();
	const float min_line_width = global::OCPN::get().gui().view().GLMinLineWidth;
	glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());
	glLineWidth(wxMax(min_line_width, width));
#endif
	return true;
}

bool ocpnDC::ConfigureBrush()
{
	if (m_brush == wxNullBrush || m_brush.GetStyle() == wxTRANSPARENT)
		return false;
#ifdef ocpnUSE_GL
	wxColour c = m_brush.GetColour();
	glColor4ub(c.Red(), c.Green(), c.Blue(), c.Alpha());
#endif
	return true;
}

void ocpnDC::GLDrawBlendData(wxCoord x, wxCoord y, wxCoord w, wxCoord h, int format,
							 const unsigned char* data)
{
#ifdef ocpnUSE_GL

	// Hmmmm.... I find that the texture version below does not work on my dodgy
	// OpenChrome gl drivers but the glDrawPixels works fine.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glRasterPos2i(x, y);
	glPixelZoom(1, -1);
	glDrawPixels(w, h, format, GL_UNSIGNED_BYTE, data);
	glPixelZoom(1, 1);
	glDisable(GL_BLEND);

	// I would prefer to just use glDrawPixels than need a texture,
	// but sometimes it did not perform alpha blending correctly,
	// this way always works
#endif
}

// render a rectangle at a given color and transparency
void ocpnDC::AlphaBlending(
		int x,
		int y,
		int size_x,
		int size_y,
		float radius, wxColour color,
		unsigned char transparency)
{
	wxDC* pdc = GetDC();
	if (pdc) {
		// Get wxImage of area of interest
		wxBitmap obm(size_x, size_y);
		wxMemoryDC mdc1;
		mdc1.SelectObject(obm);
		mdc1.Blit(0, 0, size_x, size_y, pdc, x, y);
		mdc1.SelectObject(wxNullBitmap);
		wxImage oim = obm.ConvertToImage();

		// Create destination image
		wxBitmap olbm(size_x, size_y);
		wxMemoryDC oldc(olbm);
		oldc.SetBackground(*wxBLACK_BRUSH);
		oldc.SetBrush(*wxWHITE_BRUSH);
		oldc.Clear();

		if (radius > 0.0)
			oldc.DrawRoundedRectangle(0, 0, size_x, size_y, radius);

		wxImage dest = olbm.ConvertToImage();
		unsigned char* dest_data
			= (unsigned char*)malloc(size_x * size_y * 3 * sizeof(unsigned char));
		unsigned char* bg = oim.GetData();
		unsigned char* box = dest.GetData();
		unsigned char* d = dest_data;

		float alpha = 1.0 - (float)transparency / 255.0;
		int sb = size_x * size_y;
		for (int i = 0; i < sb; i++) {
			float a = alpha;
			if (*box == 0 && radius > 0.0)
				a = 1.0;
			int r = ((*bg++) * a) + (1.0 - a) * color.Red();
			*d++ = r;
			box++;
			int g = ((*bg++) * a) + (1.0 - a) * color.Green();
			*d++ = g;
			box++;
			int b = ((*bg++) * a) + (1.0 - a) * color.Blue();
			*d++ = b;
			box++;
		}

		dest.SetData(dest_data);

		// Convert destination to bitmap and draw it
		wxBitmap dbm(dest);
		DrawBitmap(dbm, x, y, false);

		// on MSW, the dc Bounding box is not updated on DrawBitmap() method.
		// Do it explicitely here for all platforms.
		CalcBoundingBox(x, y);
		CalcBoundingBox(x + size_x, y + size_y);
	} else {
#ifdef ocpnUSE_GL
		// opengl version
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4ub(color.Red(), color.Green(), color.Blue(), transparency);

		glBegin(GL_QUADS);
		glVertex2i(x, y);
		glVertex2i(x + size_x, y);
		glVertex2i(x + size_x, y + size_y);
		glVertex2i(x, y + size_y);
		glEnd();

		glDisable(GL_BLEND);
#endif
	}
}

