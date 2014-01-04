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

#include "RenderFromHPGL.h"
#include "dychart.h"

#include <chart/s52plib.h>

#include <wx/dc.h>
#include <wx/tokenzr.h>
#include <wx/log.h>

extern double g_GLMinLineWidth;

namespace chart {

RenderFromHPGL::RenderFromHPGL(s52plib* plibarg)
	: plib(plibarg)
	, renderToDC(false)
	, renderToOpenGl(false)
	, renderToGCDC(false)
	, havePushedOpenGlAttrib(false)
{}

void RenderFromHPGL::SetTargetDC(wxDC* pdc)
{
	targetDC = pdc;
	renderToDC = true;
	renderToOpenGl = false;
	renderToGCDC = false;
}

void RenderFromHPGL::SetTargetOpenGl()
{
	renderToOpenGl = true;
	renderToDC = false;
	renderToGCDC = false;
}

void RenderFromHPGL::SetTargetGCDC(wxGCDC* gdc)
{
	targetGCDC = gdc;
	renderToGCDC = true;
	renderToDC = false;
	renderToOpenGl = false;
}

const char* RenderFromHPGL::findColorNameInRef(char colorCode, char* col)
{
	int noColors = strlen(col) / 6;
	for (int i = 0; i < noColors; i++) {
		if (*col + i == colorCode)
			return col + i + 1;
	}
	return col + 1; // Default to first color if not found.
}

wxPoint RenderFromHPGL::ParsePoint(wxString& argument)
{
	long x;
	long y;
	int colon = argument.Index(',');
	argument.Left(colon).ToLong(&x);
	argument.Mid(colon + 1).ToLong(&y);
	return wxPoint(x, y);
}

void RenderFromHPGL::SetPen() // FIXME: prime candidate for polymorphism
{
	scaleFactor = 100.0 / plib->GetPPMM();

	if (renderToDC) {
		pen = wxThePenList->FindOrCreatePen(penColor, penWidth, wxSOLID);
		brush = wxTheBrushList->FindOrCreateBrush(penColor, wxSOLID);
		targetDC->SetPen(*pen);
		targetDC->SetBrush(*brush);
	}
#ifdef ocpnUSE_GL
	if (renderToOpenGl) {
		if (!havePushedOpenGlAttrib) {
			glPushAttrib(GL_COLOR_BUFFER_BIT | GL_LINE_BIT | GL_HINT_BIT);
			glColor4ub(penColor.Red(), penColor.Green(), penColor.Blue(), penColor.Alpha());
			glLineWidth(wxMax(g_GLMinLineWidth, (float)penWidth * 0.7));
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			havePushedOpenGlAttrib = true;
		}
	}
#endif
	if (renderToGCDC) {
		pen = wxThePenList->FindOrCreatePen(penColor, penWidth, wxSOLID);
		brush = wxTheBrushList->FindOrCreateBrush(penColor, wxSOLID);
		targetGCDC->SetPen(*pen);
		targetGCDC->SetBrush(*brush);
	}
}

void RenderFromHPGL::Line(wxPoint from, wxPoint to) // FIXME: prime candidate for polymorphism
{
	if (renderToDC) {
		targetDC->DrawLine(from, to);
	}
#ifdef ocpnUSE_GL
	if (renderToOpenGl) {
		glBegin(GL_LINES);
		glVertex2i(from.x, from.y);
		glVertex2i(to.x, to.y);
		glEnd();
	}
#endif
	if (renderToGCDC) {
		targetGCDC->DrawLine(from, to);
	}
}

void RenderFromHPGL::Circle(wxPoint center, int radius,
							bool filled) // FIXME: prime candidate for polymorphism
{
	if (renderToDC) {
		if (filled)
			targetDC->SetBrush(*brush);
		else
			targetDC->SetBrush(*wxTRANSPARENT_BRUSH);
		targetDC->DrawCircle(center, radius);
	}
#ifdef ocpnUSE_GL
	if (renderToOpenGl) {
		int noSegments = 2 + (radius * 4);
		if (noSegments > 200)
			noSegments = 200;
		glBegin(GL_LINE_STRIP);
		for (double a = 0; a <= 2 * M_PI; a += 2 * M_PI / noSegments)
			glVertex2d(center.x + radius * sin(a), center.y + radius * cos(a));
		glEnd();
	}
#endif
	if (renderToGCDC) {
		if (filled)
			targetGCDC->SetBrush(*brush);
		else
			targetGCDC->SetBrush(*wxTRANSPARENT_BRUSH);

		targetGCDC->DrawCircle(center, radius);

		// wxGCDC doesn't update min/max X/Y properly for DrawCircle.
		targetGCDC->SetPen(*wxTRANSPARENT_PEN);
		targetGCDC->DrawPoint(center.x - radius, center.y);
		targetGCDC->DrawPoint(center.x + radius, center.y);
		targetGCDC->DrawPoint(center.x, center.y - radius);
		targetGCDC->DrawPoint(center.x, center.y + radius);
		targetGCDC->SetPen(*pen);
	}
}

void RenderFromHPGL::Polygon() // FIXME: prime candidate for polymorphism
{
	if (renderToDC) {
		targetDC->DrawPolygon(noPoints, polygon);
	}
#ifdef ocpnUSE_GL
	if (renderToOpenGl) {
		glBegin(GL_POLYGON);
		for (int ip = 1; ip < noPoints; ip++)
			glVertex2i(polygon[ip].x, polygon[ip].y);
		glEnd();
	}
#endif
	if (renderToGCDC) {
		targetGCDC->DrawPolygon(noPoints, polygon);
	}
}

void RenderFromHPGL::RotatePoint(wxPoint& point, double angle)
{
	if (angle == 0.0)
		return;

	double sin_rot = sin(angle * M_PI / 180.0);
	double cos_rot = cos(angle * M_PI / 180.0);

	double xp = (point.x * cos_rot) - (point.y * sin_rot);
	double yp = (point.x * sin_rot) + (point.y * cos_rot);

	point.x = (int)xp;
	point.y = (int)yp;
}

bool RenderFromHPGL::Render(char* str, char* col, wxPoint& r, wxPoint& pivot, double rot_angle)
{
	wxPoint lineStart;
	wxPoint lineEnd;

	wxStringTokenizer commands(wxString(str, wxConvUTF8), _T(";"));
	while (commands.HasMoreTokens()) {
		wxString command = commands.GetNextToken();
		wxString arguments = command.Mid(2);
		command = command.Left(2);

		if (command == _T("SP")) {
			S52color* color = plib->getColor(findColorNameInRef(arguments.GetChar(0), col));
			penColor = wxColor(color->R, color->G, color->B);
			brushColor = penColor;
		} else if (command == _T("SW")) {
			arguments.ToLong(&penWidth);
		} else if (command == _T("ST")) {
			// Transparency is ignored for now.
		} else if (command == _T("PU")) {
			SetPen();
			lineStart = ParsePoint(arguments);
			lineStart -= pivot;
			RotatePoint(lineStart, rot_angle);
			lineStart.x /= scaleFactor;
			lineStart.y /= scaleFactor;
			lineStart += r;
		} else if (command == _T("PD")) {
			if (arguments.Length() == 0) {
				lineEnd = lineStart;
				lineEnd.x++;
			} else {
				lineEnd = ParsePoint(arguments);
				lineEnd -= pivot;
				RotatePoint(lineEnd, rot_angle);
				lineEnd.x /= scaleFactor;
				lineEnd.y /= scaleFactor;
				lineEnd += r;
			}
			Line(lineStart, lineEnd);
			lineStart = lineEnd; // For next line.
		} else if (command == _T("CI")) {
			long radius;
			arguments.ToLong(&radius);
			radius = (int)radius / scaleFactor;
			Circle(lineStart, radius);
		} else if (command == _T("PM")) {
			noPoints = 1;
			polygon[0] = lineStart;

			if (arguments == _T("0")) {
				do {
					command = commands.GetNextToken();
					arguments = command.Mid(2);
					command = command.Left(2);

					if (command == _T("AA")) {
						wxLogWarning(_T("RenderHPGL: AA instruction not implemented."));
					} else if (command == _T("CI")) {
						long radius;
						arguments.ToLong(&radius);
						radius = (int)radius / scaleFactor;
						Circle(lineStart, radius, HPGL_FILLED);
					} else if (command == _T("PD")) {
						wxStringTokenizer points(arguments, _T(","));
						while (points.HasMoreTokens()) {
							long x, y;
							points.GetNextToken().ToLong(&x);
							points.GetNextToken().ToLong(&y);
							lineEnd = wxPoint(x, y);
							lineEnd -= pivot;
							RotatePoint(lineEnd, rot_angle);
							lineEnd.x /= scaleFactor;
							lineEnd.y /= scaleFactor;
							lineEnd += r;
							polygon[noPoints++] = lineEnd; // FIXME: buffer overflow
						}
					}
				} while (command != _T("PM"));
			}
		} else if (command == _T("FP")) {
			SetPen();
			Polygon();
		} else {
			// Only get here if non of the other cases did a continue.
			wxString msg(_T("RenderHPGL: The '%s' instruction is not implemented."));
			msg += wxString(command);
			wxLogWarning(msg);
		}
	}
#ifdef ocpnUSE_GL
	if (havePushedOpenGlAttrib)
		glPopAttrib();
	havePushedOpenGlAttrib = false;
#endif
	return true;
}

}

