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

#ifndef __OCPNFLOATINGTOOLBARDIALOG__H__
#define __OCPNFLOATINGTOOLBARDIALOG__H__

#include <wx/dialog.h>
#include <wx/timer.h>
#include <global/ColorScheme.h>

#define FADE_TIMER 2

class ToolBarSimple;
class GrabberWin;
class wxBoxSizer;

class OCPNFloatingToolbarDialog : public wxDialog
{
	DECLARE_EVENT_TABLE()

public:
	OCPNFloatingToolbarDialog(wxWindow* parent, wxPoint position, long orient);
	virtual ~OCPNFloatingToolbarDialog();

	void OnClose(wxCloseEvent& event);
	void OnWindowCreate(wxWindowCreateEvent& event);
	void OnToolLeftClick(wxCommandEvent& event);
	void MouseEvent(wxMouseEvent& event);
	void FadeTimerEvent(wxTimerEvent& event);
	bool IsToolbarShown() const;
	void Realize();
	ToolBarSimple* GetToolbar();
	void Submerge();
	void Surface();
	void HideTooltip();
	void ShowTooltips();
	void EnableTooltips();
	void DisableTooltips();

	void DestroyToolBar();
	void ToggleOrientation();
	void MoveDialogInScreenCoords(wxPoint posn, wxPoint posn_old);
	void RePosition();
	void LockPosition(bool lock);
	void SetColorScheme(global::ColorScheme cs);

	void SetGeometry();
	long GetOrient() const;
	void RefreshFadeTimer();
	int GetDockX() const;
	int GetDockY() const;
	bool toolbarConfigChanged;

private:
	void DoFade(int value);

	ToolBarSimple* m_ptoolbar;
	wxBoxSizer* m_topSizer;

	GrabberWin* m_pGrabberwin;

	long m_orient;
	wxTimer m_fade_timer;
	int m_opacity;

	wxPoint m_position;
	int m_dock_x;
	int m_dock_y;
	bool m_block;
};

#endif
