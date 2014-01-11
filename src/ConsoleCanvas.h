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

#ifndef __CONCANV_H__
#define __CONCANV_H__

#include <wx/dialog.h>
#include <global/ColorScheme.h>

#define ID_LEGROUTE 1000

class AnnunText;
class CDI;
class wxBoxSizer;
class wxStaticText;

class ConsoleCanvas : public wxDialog
{
	DECLARE_EVENT_TABLE()

public:
	ConsoleCanvas(wxWindow* frame);
	virtual ~ConsoleCanvas();
	void UpdateRouteData();
	void ShowWithFreshFonts(void);
	void UpdateFonts(void);
	void SetColorScheme(global::ColorScheme cs);
	void LegRoute();
	void OnContextMenu(wxContextMenuEvent& event);
	void OnContextMenuSelection(wxCommandEvent& event);
	void RefreshConsoleData(void);

	wxWindow* m_pParent;
	wxStaticText* pThisLegText;
	wxBoxSizer* m_pitemBoxSizerLeg;

	AnnunText* pXTE;
	AnnunText* pBRG;
	AnnunText* pRNG;
	AnnunText* pTTG;
	AnnunText* pVMG;
	CDI* pCDI;

	wxFont* pThisLegFont;
	bool m_bShowRouteTotal;
	bool m_bNeedClear;
	wxBrush* pbackBrush;

private:
	void OnPaint(wxPaintEvent& event);
	void OnShow(wxShowEvent& event);
};

#endif
