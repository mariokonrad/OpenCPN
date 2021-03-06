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

#include "OCPNFloatingToolbarDialog.h"
#include <GrabberWin.h>
#include <ToolbarMOBDialog.h>
#include <ToolBarSimple.h>
#include <ToolBarTool.h>
#include <MessageBox.h>
#include <GUI_IDs.h>
#include <ChartCanvas.h>
#include <MainFrame.h>

#include <gui/StyleManager.h>
#include <gui/Style.h>

#include <global/OCPN.h>
#include <global/GUI.h>
#include <global/ColorManager.h>

#include <wx/sizer.h>
#include <wx/menu.h>

extern ChartCanvas* cc1;
extern wxMenu* g_FloatingToolbarConfigMenu;
extern MainFrame* gFrame;
extern bool g_bresponsive;

BEGIN_EVENT_TABLE(OCPNFloatingToolbarDialog, wxDialog)
	EVT_MOUSE_EVENTS(OCPNFloatingToolbarDialog::MouseEvent)
	EVT_MENU(wxID_ANY, OCPNFloatingToolbarDialog::OnToolLeftClick)
	EVT_TIMER(FADE_TIMER, OCPNFloatingToolbarDialog::FadeTimerEvent)
	EVT_WINDOW_CREATE(OCPNFloatingToolbarDialog::OnWindowCreate)
END_EVENT_TABLE()

OCPNFloatingToolbarDialog::OCPNFloatingToolbarDialog(wxWindow* parent, wxPoint position,
													 long orient)
	: m_ptoolbar(NULL)
	, m_orient(orient)
	, m_opacity(255)
	, m_position(position)
	, m_dock_x(0)
	, m_dock_y(0)
	, m_block(false)
	, m_marginsInvisible(false)
{
	long wstyle = wxNO_BORDER | wxFRAME_NO_TASKBAR;
#ifndef __WXMAC__
	wstyle |= wxFRAME_SHAPED;
#endif

#ifdef __WXOSX__
	wstyle |= wxSTAY_ON_TOP;
#endif
	wxDialog::Create(parent, -1, _T("ocpnToolbarDialog"), wxPoint(-1, -1), wxSize(-1, -1), wstyle);

	m_fade_timer.SetOwner(this, FADE_TIMER);
	if (global::OCPN::get().gui().toolbar().transparent) {
		m_fade_timer.Start(5000);
	}

	m_pGrabberwin = new GrabberWin(this);

	// A top-level sizer
	m_topSizer = new wxBoxSizer(wxHORIZONTAL);
	SetSizer(m_topSizer);

	gui::Style& style = global::OCPN::get().styleman().current();
	m_marginsInvisible = style.isMarginsInvisible();

	if (g_bresponsive)
		m_marginsInvisible = true;

	Hide();
}

OCPNFloatingToolbarDialog::~OCPNFloatingToolbarDialog()
{
	DestroyToolBar();
}

void OCPNFloatingToolbarDialog::OnWindowCreate(wxWindowCreateEvent &)
{
	Realize();
}

void OCPNFloatingToolbarDialog::SetColorScheme(global::ColorScheme cs)
{
	const global::ColorManager& colors = global::OCPN::get().color();

	wxColour back_color = colors.get_color(_T("GREY2"));

	// Set background
	SetBackgroundColour(back_color);
	ClearBackground();

	if (m_ptoolbar) {
		wxColour back_color = colors.get_color(_T("GREY2"));

		// Set background
		m_ptoolbar->SetBackgroundColour(back_color);
		m_ptoolbar->ClearBackground();

		m_ptoolbar->SetToggledBackgroundColour(colors.get_color(_T("GREY1")));

		m_ptoolbar->SetColorScheme(cs);
		m_ptoolbar->Refresh(true);
	}

	if (m_pGrabberwin)
		m_pGrabberwin->SetColorScheme(cs);
}

void OCPNFloatingToolbarDialog::SetGeometry()
{
	if (!m_ptoolbar)
		return;

	const gui::Style& style = global::OCPN::get().styleman().current();
	wxSize style_tool_size = style.GetToolSize();
	if (g_bresponsive) {
		style_tool_size.x *= 2;
		style_tool_size.y *= 2;
	}
	m_ptoolbar->SetToolBitmapSize(style_tool_size);

	wxSize tool_size = m_ptoolbar->GetToolBitmapSize();

	if (m_orient == wxTB_VERTICAL)
		m_ptoolbar->SetMaxRowsCols(
			(cc1->GetSize().y / (tool_size.y + style.GetToolSeparation())) - 1, 100);
	else
		m_ptoolbar->SetMaxRowsCols(
			100, (cc1->GetSize().x / (tool_size.x + style.GetToolSeparation())) - 1);
}

void OCPNFloatingToolbarDialog::RePosition()
{
	if (m_block)
		return;

	if (GetParent() && m_ptoolbar) {
		wxSize cs = GetParent()->GetClientSize();
		if (-1 == m_dock_x)
			m_position.x = 0;
		else if (1 == m_dock_x)
			m_position.x = cs.x - GetSize().x;

		if (-1 == m_dock_y)
			m_position.y = 0;
		else if (1 == m_dock_y)
			m_position.y = cs.y - GetSize().y;

		m_position.x = wxMin(cs.x - GetSize().x, m_position.x);
		m_position.y = wxMin(cs.y - GetSize().y, m_position.y);

		m_position.x = wxMax(0, m_position.x);
		m_position.y = wxMax(0, m_position.y);

		wxPoint screen_pos = GetParent()->ClientToScreen(m_position);
		Move(screen_pos);
	}
}

void OCPNFloatingToolbarDialog::Submerge()
{
	Hide();
	if (m_ptoolbar)
		m_ptoolbar->KillTooltip();
}

void OCPNFloatingToolbarDialog::Surface()
{
#ifndef __WXOSX__
	Hide();
	Move(0, 0);
#endif

	RePosition();
	Show();
	if (m_ptoolbar)
		m_ptoolbar->EnableTooltips();
}

void OCPNFloatingToolbarDialog::HideTooltip()
{
	if (m_ptoolbar)
		m_ptoolbar->HideTooltip();
}

void OCPNFloatingToolbarDialog::ShowTooltips()
{
	if (m_ptoolbar)
		m_ptoolbar->EnableTooltips();
}

void OCPNFloatingToolbarDialog::ToggleOrientation()
{
	wxPoint old_screen_pos = GetParent()->ClientToScreen(m_position);

	if (m_orient == wxTB_HORIZONTAL) {
		m_orient = wxTB_VERTICAL;
		m_ptoolbar->SetWindowStyleFlag(m_ptoolbar->GetWindowStyleFlag() & ~wxTB_HORIZONTAL);
		m_ptoolbar->SetWindowStyleFlag(m_ptoolbar->GetWindowStyleFlag() | wxTB_VERTICAL);
	} else {
		m_orient = wxTB_HORIZONTAL;
		m_ptoolbar->SetWindowStyleFlag(m_ptoolbar->GetWindowStyleFlag() & ~wxTB_VERTICAL);
		m_ptoolbar->SetWindowStyleFlag(m_ptoolbar->GetWindowStyleFlag() | wxTB_HORIZONTAL);
	}

	wxPoint grabber_point_abs = ClientToScreen(m_pGrabberwin->GetPosition());

	global::OCPN::get().styleman().current().SetOrientation(m_orient);
	m_ptoolbar->InvalidateBitmaps();

	SetGeometry();
	Realize();

	wxPoint pos_abs = grabber_point_abs;
	pos_abs.x -= m_pGrabberwin->GetPosition().x;
	MoveDialogInScreenCoords(pos_abs, old_screen_pos);

	RePosition();

	Show(); // this seems to be necessary on GTK to kick the sizer into gear...(FS#553)
	Refresh();
}

void OCPNFloatingToolbarDialog::MouseEvent(wxMouseEvent& event)
{
	if (global::OCPN::get().gui().toolbar().transparent) {
		if (event.Entering() && (m_opacity < 255)) {
			SetTransparent(255);
			m_opacity = 255;
		}
		m_fade_timer.Start(5000); // retrigger the continuous timer
	}
}

void OCPNFloatingToolbarDialog::FadeTimerEvent(wxTimerEvent&)
{
	if (global::OCPN::get().gui().toolbar().transparent) {
		if (!global::OCPN::get().gui().view().opengl)
			DoFade(128);
	}

	m_fade_timer.Start(5000); // retrigger the continuous timer
}

void OCPNFloatingToolbarDialog::DoFade(int value)
{
	if (value != m_opacity)
		SetTransparent(value);
	m_opacity = value;
}

void OCPNFloatingToolbarDialog::RefreshFadeTimer()
{
	SetTransparent(255);
	m_opacity = 255;
	m_fade_timer.Start(500); // retrigger the continuous timer
}

void OCPNFloatingToolbarDialog::MoveDialogInScreenCoords(wxPoint posn, wxPoint posn_old)
{
	wxPoint pos_in_parent = GetParent()->ScreenToClient(posn);
	wxPoint pos_in_parent_old = GetParent()->ScreenToClient(posn_old);

	// "Docking" support
#define DOCK_MARGIN 40

	// X
	m_dock_x = 0;
	if (pos_in_parent.x < pos_in_parent_old.x) { // moving left
		if (pos_in_parent.x < DOCK_MARGIN) {
			pos_in_parent.x = 0;
			m_dock_x = -1;
		}
	} else if (pos_in_parent.x > pos_in_parent_old.x) { // moving right
		int max_right = GetParent()->GetClientSize().x - GetSize().x;
		if (pos_in_parent.x > (max_right - DOCK_MARGIN)) {
			pos_in_parent.x = max_right;
			m_dock_x = 1;
		}
	}

	// Y
	m_dock_y = 0;
	if (pos_in_parent.y < pos_in_parent_old.y) { // moving up
		if (pos_in_parent.y < DOCK_MARGIN) {
			pos_in_parent.y = 0;
			m_dock_y = -1;
		}
	} else if (pos_in_parent.y > pos_in_parent_old.y) { // moving down
		int max_down = GetParent()->GetClientSize().y - GetSize().y;
		if (pos_in_parent.y > (max_down - DOCK_MARGIN)) {
			pos_in_parent.y = max_down;
			m_dock_y = 1;
		}
	}

	m_position = pos_in_parent;

	wxPoint final_pos = GetParent()->ClientToScreen(pos_in_parent);

	Move(final_pos);
}

void OCPNFloatingToolbarDialog::Realize()
{
	if (!m_ptoolbar)
		return;

	gui::Style& style = global::OCPN::get().styleman().current();

	m_ptoolbar->Realize();

	m_topSizer->Clear();
	m_topSizer->Add(m_ptoolbar);
	m_topSizer->Add(m_pGrabberwin, 0, wxTOP, style.GetTopMargin());

	m_topSizer->Layout();
	Fit();

	// Update "Dock" parameters
	if (m_position.x == 0)
		m_dock_x = -1;
	else if (m_position.x == GetParent()->GetClientSize().x - GetSize().x)
		m_dock_x = 1;

	if (m_position.y == 0)
		m_dock_y = -1;
	else if (m_position.y == GetParent()->GetClientSize().y - GetSize().y)
		m_dock_y = 1;

	// Now create a bitmap mask forthe frame shape.

	if (m_marginsInvisible) {
		wxSize tool_size = m_ptoolbar->GetToolBitmapSize();

		// Determine whether the tool icons are meant (by style) to join without speces between
		// This will determine what type of region to draw.
		bool b_overlap = false;

		wxToolBarToolsList::compatibility_iterator node1 = m_ptoolbar->m_tools.GetFirst();
		wxToolBarToolsList::compatibility_iterator node2 = node1->GetNext();

		wxToolBarToolBase* tool1 = node1->GetData();
		ToolBarTool* tools1 = (ToolBarTool*)tool1;

		wxToolBarToolBase* tool2 = node2->GetData();
		ToolBarTool* tools2 = (ToolBarTool*)tool2;

		if ((tools1->m_x + tools1->m_width) >= tools2->m_x)
			b_overlap = true;

		int toolCount = m_ptoolbar->GetVisibleToolCount();

		wxPoint upperLeft(style.GetLeftMargin(), style.GetTopMargin());
		wxSize visibleSize;
		if (m_ptoolbar->IsVertical()) {
			int noTools = m_ptoolbar->GetMaxRows();
			if (noTools > toolCount)
				noTools = toolCount;
			visibleSize.x = m_ptoolbar->GetLineCount() * (tool_size.x + style.GetTopMargin());
			visibleSize.y = noTools * (tool_size.y + style.GetToolSeparation());
			visibleSize.x -= style.GetTopMargin();
			visibleSize.y -= style.GetToolSeparation();
		} else {
			int noTools = m_ptoolbar->GetMaxCols();
			if (noTools > toolCount)
				noTools = toolCount;
			visibleSize.x = noTools * (tool_size.x + style.GetToolSeparation());
			visibleSize.y = m_ptoolbar->GetLineCount() * (tool_size.y + style.GetTopMargin());
			visibleSize.x -= style.GetToolSeparation();
			visibleSize.y -= style.GetTopMargin();
		}

		wxBitmap shape(visibleSize.x + tool_size.x, visibleSize.y + tool_size.y); // + fluff
		wxMemoryDC sdc(shape);
		sdc.SetBackground(*wxWHITE_BRUSH);
		sdc.SetBrush(*wxBLACK_BRUSH);
		sdc.SetPen(*wxBLACK_PEN);
		sdc.Clear();

		if (b_overlap) {
			int lines = m_ptoolbar->GetLineCount();
			for (int i = 1; i <= lines; i++) {
				if (m_ptoolbar->IsVertical()) {
					wxSize barsize(tool_size.x, visibleSize.y);
					if (i == lines && i > 1) {
						int toolsInLastLine = toolCount % m_ptoolbar->GetMaxRows();
						if (toolsInLastLine == 0)
							toolsInLastLine = m_ptoolbar->GetMaxRows();
						int emptySpace = (m_ptoolbar->GetMaxRows() - toolsInLastLine);
						barsize.y -= emptySpace * (tool_size.y + style.GetToolSeparation());
					}
					if (i == lines) {
						// Also do grabber here, since it is to the right of the last line.
						wxRect grabMask(upperLeft, barsize);
						grabMask.width += style.GetIcon(_T("grabber")).GetWidth();
						grabMask.height = style.GetIcon(_T("grabber")).GetHeight();
						sdc.DrawRoundedRectangle(grabMask, style.GetToolbarCornerRadius());
					}
					sdc.DrawRoundedRectangle(upperLeft, barsize, style.GetToolbarCornerRadius());
					upperLeft.x += style.GetTopMargin() + tool_size.x;
				} else {
					wxSize barsize(visibleSize.x, tool_size.y);

					if (i == 1) {
						barsize.x += style.GetIcon(_T("grabber")).GetWidth();
					}
					if (i == lines && i > 1) {
						int toolsInLastLine = toolCount % m_ptoolbar->GetMaxCols();
						if (toolsInLastLine == 0)
							toolsInLastLine = m_ptoolbar->GetMaxCols();
						int emptySpace = (m_ptoolbar->GetMaxCols() - toolsInLastLine);
						barsize.x -= emptySpace * (tool_size.x + style.GetToolSeparation());
					}

					const gui::Style& style = global::OCPN::get().styleman().current();

					sdc.DrawRoundedRectangle(upperLeft, barsize, style.GetToolbarCornerRadius());
					upperLeft.y += style.GetTopMargin() + tool_size.y;
				}
			}
		} else {
			for (wxToolBarToolsList::compatibility_iterator node = m_ptoolbar->m_tools.GetFirst();
				 node; node = node->GetNext()) {
				wxToolBarToolBase* tool = node->GetData();
				ToolBarTool* tools = (ToolBarTool*)tool;
				wxRect toolRect = tools->trect;

				sdc.DrawRoundedRectangle(tools->m_x, tools->m_y, tool_size.x, tool_size.y,
										 style.GetToolbarCornerRadius());
			}
		}
#ifndef __WXMAC__
		SetShape(wxRegion(shape, *wxWHITE, 10));
#endif
	}
}

void OCPNFloatingToolbarDialog::OnToolLeftClick(wxCommandEvent& event)
{
	// First see if it was actually the context menu that was clicked.

	if (event.GetId() >= ID_PLUGIN_BASE + 100) {

		int itemId = event.GetId() - ID_PLUGIN_BASE - 100;
		bool toolIsChecked = g_FloatingToolbarConfigMenu->FindItem(event.GetId())->IsChecked();

		global::GUI& gui = global::OCPN::get().gui();

		if (toolIsChecked) {
			gui.set_toolbar_config_at(itemId, _T('X'));
		} else {
			if (itemId + ID_ZOOMIN == ID_MOB) {
				ToolbarMOBDialog mdlg(this);
				int dialog_ret = mdlg.ShowModal();
				int answer = mdlg.GetSelection();

				if (answer == 0 || answer == 1 || dialog_ret == wxID_CANCEL) {
					g_FloatingToolbarConfigMenu->FindItem(event.GetId())->Check(true);
					if (answer == 1 && dialog_ret == wxID_OK) {
						global::OCPN::get().gui().set_view_permanent_mob_icon(true);
						delete g_FloatingToolbarConfigMenu;
						g_FloatingToolbarConfigMenu = new wxMenu();
						toolbarConfigChanged = true;
					}
					return;
				}
			}

			if (m_ptoolbar->GetVisibleToolCount() == 1) {
				OCPNMessageBox(this, _("You can't hide the last tool from the toolbar\nas this would make it inaccessible."),
							   _("OpenCPN Alert"), wxOK);
				g_FloatingToolbarConfigMenu->FindItem(event.GetId())->Check(true);
				return;
			}

			gui.set_toolbar_config_at(itemId, _T('.'));
		}
		toolbarConfigChanged = true;
		return;
	}

	// No it was a button that was clicked.
	// Since Dialog events don't propagate automatically, we send it explicitly
	// (instead of relying on event.Skip()). Send events up the window hierarchy

	GetParent()->GetEventHandler()->AddPendingEvent(event);
	gFrame->Raise();
}

ToolBarSimple* OCPNFloatingToolbarDialog::GetToolbar()
{
	if (!m_ptoolbar) {
		const global::ColorManager& colors = global::OCPN::get().color();
		long winstyle = wxNO_BORDER | wxTB_FLAT | m_orient;

		m_ptoolbar = new ToolBarSimple(this, -1, wxPoint(-1, -1), wxSize(-1, -1), winstyle);

		m_ptoolbar->SetBackgroundColour(colors.get_color(_T("GREY2")));
		m_ptoolbar->ClearBackground();
		m_ptoolbar->SetToggledBackgroundColour(colors.get_color(_T("GREY1")));
		m_ptoolbar->SetColorScheme(global::OCPN::get().gui().view().color_scheme);

		SetGeometry();
	}

	return m_ptoolbar;
}

void OCPNFloatingToolbarDialog::DestroyToolBar()
{
	if (m_ptoolbar) {
		m_ptoolbar->ClearTools();
		delete m_ptoolbar;
		m_ptoolbar = NULL;
	}
}

void OCPNFloatingToolbarDialog::EnableTooltips()
{
	if (m_ptoolbar)
		m_ptoolbar->EnableTooltips();
}

void OCPNFloatingToolbarDialog::DisableTooltips()
{
	if (m_ptoolbar)
		m_ptoolbar->DisableTooltips();
}

void OCPNFloatingToolbarDialog::LockPosition(bool lock)
{
	m_block = lock;
}

long OCPNFloatingToolbarDialog::GetOrient() const
{
	return m_orient;
}

int OCPNFloatingToolbarDialog::GetDockX() const
{
	return m_dock_x;
}

int OCPNFloatingToolbarDialog::GetDockY() const
{
	return m_dock_y;
}

bool OCPNFloatingToolbarDialog::IsToolbarShown() const
{
	return m_ptoolbar != 0;
}

