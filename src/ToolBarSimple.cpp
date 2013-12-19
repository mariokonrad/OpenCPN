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

#include "ToolBarSimple.h"
#include <StyleManager.h>
#include <Style.h>
#include <FontMgr.h>
#include <GrabberWin.h>
#include <ToolbarMOBDialog.h>
#include <OCPNFloatingToolbarDialog.h>
#include <ToolTipWin.h>
#include <ToolBarTool.h>

#include <UserColors.h>
#include <MainFrame.h>

#include <plugin/PlugInManager.h>

#include <vector>

extern OCPNFloatingToolbarDialog* g_FloatingToolbarDialog;
extern ToolBarSimple* g_toolbar;
extern ocpnStyle::StyleManager* g_StyleManager;
extern MainFrame* gFrame;
extern wxMenu* g_FloatingToolbarConfigMenu;

BEGIN_EVENT_TABLE(ToolBarSimple, wxControl)
	EVT_SIZE(ToolBarSimple::OnSize)
	EVT_PAINT(ToolBarSimple::OnPaint)
	EVT_KILL_FOCUS(ToolBarSimple::OnKillFocus)
	EVT_MOUSE_EVENTS(ToolBarSimple::OnMouseEvent)
	EVT_TIMER(TOOLTIPON_TIMER, ToolBarSimple::OnToolTipTimerEvent)
END_EVENT_TABLE()


wxToolBarToolBase * ToolBarSimple::CreateTool(
		int id,
		const wxString & label,
		const wxBitmap & bmpNormal,
		const wxBitmap & bmpDisabled,
		wxItemKind kind,
		wxObject * clientData,
		const wxString & shortHelp,
		const wxString& longHelp)
{
	return new ToolBarTool(this, id, label, bmpNormal, bmpDisabled, kind, clientData, shortHelp, longHelp);
}

ToolBarSimple::ToolBarSimple()
{
	Init();
}

ToolBarSimple::ToolBarSimple(
		wxWindow * parent,
		wxWindowID winid,
		const wxPoint & pos,
		const wxSize & size,
		long style,
		const wxString & name)
	: m_one_shot(500)
{
	Init();
	Create(parent, winid, pos, size, style, name);
}

void ToolBarSimple::Init()
{
	m_currentRowsOrColumns = 0;

	m_lastX = m_lastY = 0;

	m_maxWidth = m_maxHeight = 0;

	m_pressedTool = m_currentTool = -1;

	m_xPos = m_yPos = wxDefaultCoord;

	m_style = g_StyleManager->GetCurrentStyle(); // FIXME: do not store the style

	m_defaultWidth = 16;
	m_defaultHeight = 15;

	m_toggle_bg_color = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	m_toolOutlineColour.Set(_T("BLACK"));
	m_pToolTipWin = NULL;
	m_last_ro_tool = NULL;

	EnableTooltips();
}

wxToolBarToolBase* ToolBarSimple::DoAddTool(int id, const wxString& label, const wxBitmap& bitmap,
											const wxBitmap& bmpDisabled, wxItemKind kind,
											const wxString& shortHelp, const wxString& longHelp,
											wxObject* clientData, wxCoord xPos, wxCoord yPos)
{
	// rememeber the position for DoInsertTool()
	m_xPos = xPos;
	m_yPos = yPos;

	InvalidateBestSize();
	return InsertTool(GetToolsCount(), id, label, bitmap, bmpDisabled, kind, shortHelp, longHelp,
					  clientData);
}

wxToolBarToolBase* ToolBarSimple::AddTool(int toolid, const wxString& label, const wxBitmap& bitmap,
										  const wxBitmap& bmpDisabled, wxItemKind kind,
										  const wxString& shortHelp, const wxString& longHelp,
										  wxObject* data)
{
	InvalidateBestSize();
	ToolBarTool* tool = static_cast<ToolBarTool*>(InsertTool(
		GetToolsCount(), toolid, label, bitmap, bmpDisabled, kind, shortHelp, longHelp, data));
	return tool;
}

wxToolBarToolBase* ToolBarSimple::InsertTool(size_t pos, int id, const wxString& label,
											 const wxBitmap& bitmap, const wxBitmap& bmpDisabled,
											 wxItemKind kind, const wxString& shortHelp,
											 const wxString& longHelp, wxObject* clientData)
{
	wxCHECK_MSG(pos <= GetToolsCount(), (wxToolBarToolBase*)NULL,
				_T("invalid position in wxToolBar::InsertTool()"));

	wxToolBarToolBase* tool
		= CreateTool(id, label, bitmap, bmpDisabled, kind, clientData, shortHelp, longHelp);

	if (!InsertTool(pos, tool)) {
		delete tool;

		return NULL;
	}

	return tool;
}

wxToolBarToolBase* ToolBarSimple::InsertTool(size_t pos, wxToolBarToolBase* tool)
{
	wxCHECK_MSG(pos <= GetToolsCount(), (wxToolBarToolBase*)NULL,
				_T("invalid position in wxToolBar::InsertTool()"));

	if (!tool || !DoInsertTool(pos, tool)) {
		return NULL;
	}

	m_tools.Insert(pos, tool);

	return tool;
}

bool ToolBarSimple::DoInsertTool(size_t WXUNUSED(pos), wxToolBarToolBase* toolBase)
{
	ToolBarTool* tool = static_cast<ToolBarTool*>(toolBase);

	// Check if the plugin is inserting same-named tools. Make sure they have different names,
	// otherwise the style manager cannot differentiate between them.
	if (tool->isPluginTool) {
		for (unsigned int i = 0; i < GetToolsCount(); i++) {
			if (tool->GetToolname() == static_cast<ToolBarTool*>(m_tools.Item(i)->GetData())->GetToolname()) {
				tool->toolname << _T("1");
			}
		}
	}

	tool->m_x = m_xPos;
	if (tool->m_x == wxDefaultCoord)
		tool->m_x = m_style->GetLeftMargin();

	tool->m_y = m_yPos;
	if (tool->m_y == wxDefaultCoord)
		tool->m_y = m_style->GetTopMargin();

	if (tool->IsButton()) {
		tool->SetSize(GetToolSize());

		// Calculate reasonable max size in case Layout() not called
		if ((tool->m_x + tool->GetNormalBitmap().GetWidth() + m_style->GetLeftMargin())
			> m_maxWidth)
			m_maxWidth = static_cast<wxCoord>(tool->m_x + tool->GetWidth() + m_style->GetLeftMargin());

		if ((tool->m_y + tool->GetNormalBitmap().GetHeight() + m_style->GetTopMargin())
			> m_maxHeight)
			m_maxHeight = static_cast<wxCoord>(tool->m_y + tool->GetHeight() + m_style->GetTopMargin());
	} else {
		if (tool->IsControl()) {
			tool->SetSize(tool->GetControl()->GetSize());
		}
	}

	tool->b_hilite = false;

	return true;
}

bool ToolBarSimple::DoDeleteTool(size_t WXUNUSED(pos), wxToolBarToolBase* tool)
{
	// VZ: didn't test whether it works, but why not...
	tool->Detach();

	if (m_last_ro_tool == tool)
		m_last_ro_tool = NULL;

	Refresh(false);

	return true;
}

bool ToolBarSimple::Create(
		wxWindow * parent,
		wxWindowID id,
		const wxPoint & pos,
		const wxSize & size,
		long style,
		const wxString& name)
{
	if (!wxWindow::Create(parent, id, pos, size, style, name))
		return false;

	// Set it to grey (or other 3D face colour)
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));

	if (GetWindowStyleFlag() & wxTB_VERTICAL) {
		m_lastX = 7;
		m_lastY = 3;

		m_maxRows = 32000; // a lot
		m_maxCols = 1;
	} else {
		m_lastX = 3;
		m_lastY = 7;

		m_maxRows = 1;
		m_maxCols = 32000; // a lot
	}

	SetCursor(*wxSTANDARD_CURSOR);

	m_tooltip_timer.SetOwner(this, TOOLTIPON_TIMER);

	return true;
}

ToolBarSimple::~ToolBarSimple()
{
	if (m_pToolTipWin) {
		m_pToolTipWin->Destroy();
		m_pToolTipWin = NULL;
	}
}

void ToolBarSimple::KillTooltip()
{
	m_btooltip_show = false;

	if (m_pToolTipWin) {
		m_pToolTipWin->Hide();
		m_pToolTipWin->Destroy();
		m_pToolTipWin = NULL;
	}
	m_tooltip_timer.Stop();

	if (m_last_ro_tool) {
		if (m_last_ro_tool->IsEnabled()) {
			if (m_last_ro_tool->IsToggled()) {
				m_last_ro_tool->SetNormalBitmap(m_style->GetToolIcon(m_last_ro_tool->GetToolname(),
																	 ocpnStyle::TOOLICON_TOGGLED));
			} else {
				m_last_ro_tool->SetNormalBitmap(m_style->GetToolIcon(m_last_ro_tool->GetToolname(),
																	 ocpnStyle::TOOLICON_NORMAL));
			}
		}
	}
}

void ToolBarSimple::HideTooltip()
{
	if (m_pToolTipWin) {
		m_pToolTipWin->Hide();
	}
}

void ToolBarSimple::SetColorScheme(ColorScheme cs)
{
	if (m_pToolTipWin) {
		m_pToolTipWin->Destroy();
		m_pToolTipWin = NULL;
	}

	m_toolOutlineColour = GetGlobalColor(_T("UIBDR"));
	m_currentColorScheme = cs;
}

bool ToolBarSimple::Realize()
{
	m_currentRowsOrColumns = 0;
	m_LineCount = 1;
	m_lastX = m_style->GetLeftMargin();
	m_lastY = m_style->GetTopMargin();
	m_maxWidth = 0;
	m_maxHeight = 0;

	if (IsVertical())
		m_style->SetOrientation(wxTB_VERTICAL);
	else
		m_style->SetOrientation(wxTB_HORIZONTAL);

	wxSize toolSize = m_style->GetToolSize();
	int separatorSize = m_style->GetToolSeparation();

	ToolBarTool* lastTool = NULL;
	bool firstNode = true;
	wxToolBarToolsList::iterator node = m_tools.begin();

	while (node != m_tools.end()) {
		ToolBarTool* tool = static_cast<ToolBarTool*>(*node);
		tool->firstInLine = firstNode;
		tool->lastInLine = false;
		firstNode = false;

		if (tool->IsSeparator()) {
			if (GetWindowStyleFlag() & wxTB_HORIZONTAL) {
				if (m_currentRowsOrColumns >= m_maxCols)
					m_lastY += separatorSize;
				else
					m_lastX += separatorSize;
			} else {
				if (m_currentRowsOrColumns >= m_maxRows)
					m_lastX += separatorSize;
				else
					m_lastY += separatorSize;
			}
		} else if (tool->IsButton()) {
			if (!IsVertical()) {
				if (m_currentRowsOrColumns >= m_maxCols) {
					tool->firstInLine = true;
					if (lastTool && m_LineCount > 1)
						lastTool->lastInLine = true;
					m_LineCount++;
					m_currentRowsOrColumns = 0;
					m_lastX = m_style->GetLeftMargin();
					m_lastY += toolSize.y + m_style->GetTopMargin();
				}
				tool->m_x = static_cast<wxCoord>(m_lastX);
				tool->m_y = static_cast<wxCoord>(m_lastY);

				tool->trect = wxRect(tool->m_x, tool->m_y, toolSize.x, toolSize.y);
				tool->trect.Inflate(m_style->GetToolSeparation() / 2, m_style->GetTopMargin());

				m_lastX += toolSize.x + m_style->GetToolSeparation();
			} else {
				if (m_currentRowsOrColumns >= m_maxRows) {
					tool->firstInLine = true;
					if (lastTool)
						lastTool->lastInLine = true;
					m_LineCount++;
					m_currentRowsOrColumns = 0;
					m_lastX += toolSize.x + m_style->GetTopMargin();
					m_lastY = m_style->GetTopMargin();
				}
				tool->m_x = static_cast<wxCoord>(m_lastX);
				tool->m_y = static_cast<wxCoord>(m_lastY);

				tool->trect = wxRect(tool->m_x, tool->m_y, toolSize.x, toolSize.y);
				tool->trect.Inflate(m_style->GetToolSeparation() / 2, m_style->GetTopMargin());

				m_lastY += toolSize.y + m_style->GetToolSeparation();
			}
			m_currentRowsOrColumns++;
		} else if (tool->IsControl()) {
			tool->m_x = static_cast<wxCoord>(m_lastX);
			tool->m_y = static_cast<wxCoord>(m_lastY - (m_style->GetTopMargin() / 2));

			tool->trect = wxRect(tool->m_x, tool->m_y, tool->GetWidth(), tool->GetHeight());
			tool->trect.Inflate(m_style->GetToolSeparation() / 2, m_style->GetTopMargin());

			wxSize s = tool->GetControl()->GetSize();
			m_lastX += s.x + m_style->GetToolSeparation();
		}

		if (m_lastX > m_maxWidth)
			m_maxWidth = m_lastX;
		if (m_lastY > m_maxHeight)
			m_maxHeight = m_lastY;

		lastTool = tool;
		++node;
	}
	if (m_LineCount > 1 || IsVertical())
		lastTool->lastInLine = true;

	if (GetWindowStyleFlag() & wxTB_HORIZONTAL)
		m_maxHeight += toolSize.y;
	else
		m_maxWidth += toolSize.x;

	m_maxWidth += m_style->GetRightMargin();
	m_maxHeight += m_style->GetBottomMargin();

	SetSize(m_maxWidth, m_maxHeight);
	SetMinSize(wxSize(m_maxWidth, m_maxHeight));

	return true;
}

void ToolBarSimple::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);
	PrepareDC(dc);

	wxRegion ru = GetUpdateRegion();
	wxRect upRect = ru.GetBox();

	static int count = 0;
	// Prevent reentry of OnPaint which would cause wxMemoryDC errors.
	if (count > 0)
		return;
	count++;

	for (wxToolBarToolsList::iterator node = m_tools.begin(); node != m_tools.end(); ++node) {
		wxToolBarToolBase* tool = *node;
		ToolBarTool* tools = static_cast<ToolBarTool*>(tool);
		wxRect toolRect = tools->trect;

		if (toolRect.Intersects(upRect)) {

			if (tool->IsButton()) {
				DrawTool(dc, tool);
			} else if (tool->IsControl()) {
				if (tool->GetControl()->IsKindOf(CLASSINFO(wxStaticBitmap))) {
					wxStaticBitmap* psbm = static_cast<wxStaticBitmap*>(tool->GetControl());
					ToolBarTool* toolsimp = static_cast<ToolBarTool*>(tool);
					dc.DrawBitmap(psbm->GetBitmap(), toolsimp->m_x, toolsimp->m_y, false);
				}
			}
		}
	}

	count--;
}

void ToolBarSimple::OnSize(wxSizeEvent& WXUNUSED(event))
{
	if (GetAutoLayout())
		Layout();
}

void ToolBarSimple::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
	OnMouseEnter(m_pressedTool = m_currentTool = -1);
}

void ToolBarSimple::OnToolTipTimerEvent(wxTimerEvent&)
{
	if (!gFrame->IsActive())
		return;

	if (m_btooltip_show && IsShown() && m_pToolTipWin && (!m_pToolTipWin->IsShown())) {
		if (m_last_ro_tool) {
			wxString s = m_last_ro_tool->GetShortHelp();

			if (s.Len()) {
				m_pToolTipWin->SetString(s);

				wxPoint pos_in_toolbar(m_last_ro_tool->m_x, m_last_ro_tool->m_y);
				pos_in_toolbar.x += m_last_ro_tool->m_width + 2;

				// Quick hack for right docked toolbar, to avoid tooltip interference
				if ((g_FloatingToolbarDialog->GetDockX() == 1)
					&& (g_FloatingToolbarDialog->GetOrient() == wxTB_VERTICAL))
					pos_in_toolbar.y = m_last_ro_tool->m_y - 30;

				m_pToolTipWin->Move(0, 0); // workaround for gtk autocentre dialog behavior

				m_pToolTipWin->SetPosition(ClientToScreen(pos_in_toolbar));
				m_pToolTipWin->SetBitmap();
				m_pToolTipWin->Show();
				gFrame->Raise();
			}
		}
	}
}

void ToolBarSimple::OnMouseEvent(wxMouseEvent& event)
{
	wxCoord x, y;
	event.GetPosition(&x, &y);
	ToolBarTool* tool = static_cast<ToolBarTool*>(FindToolForPosition(x, y));
	if (event.LeftDown()) {
		CaptureMouse();
	}
	if (event.LeftUp()) {
		if (HasCapture())
			ReleaseMouse();
	}

	if (tool && tool->IsButton() && IsShown()) {

		// ToolTips
		if (NULL == m_pToolTipWin) {
			m_pToolTipWin = new ToolTipWin(GetParent());
			m_pToolTipWin->SetColorScheme(m_currentColorScheme);
			m_pToolTipWin->Hide();
		}

		if (tool != m_last_ro_tool)
			m_pToolTipWin->Hide();

		if (!m_pToolTipWin->IsShown()) {
			m_tooltip_timer.Start(m_one_shot, wxTIMER_ONE_SHOT);
		}

		// Tool Rollover highlighting
		if (tool != m_last_ro_tool) {
			if (tool->IsEnabled()) {
				tool->rollover = true;
				tool->bitmapOK = false;
			}
			if (m_last_ro_tool) {
				if (m_last_ro_tool->IsEnabled()) {
					m_last_ro_tool->rollover = false;
					m_last_ro_tool->bitmapOK = false;
				}
			}
			m_last_ro_tool = tool;
			g_toolbar->Refresh(false);
		}
	} else {
		// Tooltips
		if (m_pToolTipWin && m_pToolTipWin->IsShown())
			m_pToolTipWin->Hide();

		// Remove Highlighting
		if (m_last_ro_tool) {
			if (m_last_ro_tool->IsEnabled()) {
				m_last_ro_tool->rollover = false;
				m_last_ro_tool->bitmapOK = false;
			}
			g_toolbar->Refresh(false);
		}
	}

	m_last_ro_tool = tool;

	if (!tool) {
		if (m_currentTool > -1) {
			if (event.LeftIsDown())
				SpringUpButton(m_currentTool);
			m_currentTool = -1;
			OnMouseEnter(-1);
		}

		wxMouseEvent* pev = static_cast<wxMouseEvent*>(event.Clone());
		GetParent()->GetEventHandler()->AddPendingEvent(*pev);
		wxDELETE(pev);

		return;
	}

	if (!event.IsButton()) {
		if (tool->GetId() != m_currentTool) {
			// If the left button is kept down and moved over buttons,
			// press those buttons.
			if (event.LeftIsDown() && tool->IsEnabled()) {
				SpringUpButton(m_currentTool);

				if (tool->CanBeToggled()) {
					tool->Toggle();
				}

				DrawTool(tool);
			}

			m_currentTool = tool->GetId();
			OnMouseEnter(m_currentTool);
		}

		wxMouseEvent* pev = static_cast<wxMouseEvent*>(event.Clone());
		GetParent()->GetEventHandler()->AddPendingEvent(*pev);
		wxDELETE(pev);

		return;
	}

	// Left button pressed.
	if (event.LeftDown() && tool->IsEnabled()) {
		if (tool->CanBeToggled()) {
			tool->Toggle();
		}

		DrawTool(tool);
	} else if (event.RightDown()) {
		OnRightClick(tool->GetId(), x, y);
	}

	// Left Button Released.  Only this action confirms selection.
	// If the button is enabled and it is not a toggle tool and it is
	// in the pressed state, then raise the button and call OnLeftClick.
	//
	if (event.LeftUp() && tool->IsEnabled()) {
		// Pass the OnLeftClick event to tool
		if (!OnLeftClick(tool->GetId(), tool->IsToggled()) && tool->CanBeToggled()) {
			// If it was a toggle, and OnLeftClick says No Toggle allowed,
			// then change it back
			tool->Toggle();
		}
	}

	wxMouseEvent* pev = static_cast<wxMouseEvent*>(event.Clone());
	GetParent()->GetEventHandler()->AddPendingEvent(*pev);
	wxDELETE(pev);
	event.Skip();
}

// ----------------------------------------------------------------------------
// drawing
// ----------------------------------------------------------------------------

void ToolBarSimple::DrawTool(wxToolBarToolBase* tool)
{
	wxClientDC dc(this);
	DrawTool(dc, tool);
}

// NB! The current DrawTool code assumes that plugin tools are never disabled
// when they are present on the toolbar, since disabled plugins are removed.

void ToolBarSimple::DrawTool(wxDC& dc, wxToolBarToolBase* toolBase)
{
	ToolBarTool* tool = static_cast<ToolBarTool*>(toolBase);

	PrepareDC(dc);

	wxPoint drawAt(tool->m_x, tool->m_y);
	wxBitmap bmp;

	if (tool->bitmapOK) {
		if (tool->IsEnabled()) {
			bmp = tool->GetNormalBitmap();
			if (!bmp.IsOk())
				bmp = m_style->GetToolIcon(tool->GetToolname(), ocpnStyle::TOOLICON_NORMAL,
										   tool->rollover);
		} else {
			bmp = tool->GetDisabledBitmap();
			if (!bmp.IsOk())
				bmp = m_style->GetToolIcon(tool->GetToolname(), ocpnStyle::TOOLICON_DISABLED);
		}
	} else {
		if (tool->isPluginTool) {

			// First try getting the icon from the Style.
			// If it is not in the style we build a new icon from the style BG and the plugin icon.

			if (tool->IsToggled()) {
				bmp = m_style->GetToolIcon(tool->GetToolname(), ocpnStyle::TOOLICON_TOGGLED,
										   tool->rollover);
				if (bmp.GetDepth() == 1) {
					if (tool->rollover) {
						bmp = m_style->BuildPluginIcon(tool->pluginRolloverIcon,
													   ocpnStyle::TOOLICON_TOGGLED);
						if (!bmp.IsOk())
							bmp = m_style->BuildPluginIcon(tool->pluginNormalIcon,
														   ocpnStyle::TOOLICON_TOGGLED);
					} else
						bmp = m_style->BuildPluginIcon(tool->pluginNormalIcon,
													   ocpnStyle::TOOLICON_TOGGLED);
				}
			} else {
				bmp = m_style->GetToolIcon(tool->GetToolname(), ocpnStyle::TOOLICON_NORMAL,
										   tool->rollover);
				if (bmp.GetDepth() == 1) {
					if (tool->rollover) {
						bmp = m_style->BuildPluginIcon(tool->pluginRolloverIcon,
													   ocpnStyle::TOOLICON_NORMAL);
						if (!bmp.IsOk())
							bmp = m_style->BuildPluginIcon(tool->pluginNormalIcon,
														   ocpnStyle::TOOLICON_NORMAL);
					} else
						bmp = m_style->BuildPluginIcon(tool->pluginNormalIcon,
													   ocpnStyle::TOOLICON_NORMAL);
				}
			}
			tool->SetNormalBitmap(bmp);
			tool->bitmapOK = true;
		} else {
			if (tool->IsEnabled()) {
				if (tool->IsToggled())
					bmp = m_style->GetToolIcon(tool->GetToolname(), ocpnStyle::TOOLICON_TOGGLED,
											   tool->rollover);
				else
					bmp = m_style->GetToolIcon(tool->GetIconName(), ocpnStyle::TOOLICON_NORMAL,
											   tool->rollover);

				tool->SetNormalBitmap(bmp);
				tool->bitmapOK = true;
			} else {
				bmp = m_style->GetToolIcon(tool->GetToolname(), ocpnStyle::TOOLICON_DISABLED);
				tool->SetDisabledBitmap(bmp);
				tool->bitmapOK = true;
			}
		}
	}

	if (tool->firstInLine) {
		m_style->DrawToolbarLineStart(bmp);
	}
	if (tool->lastInLine) {
		m_style->DrawToolbarLineEnd(bmp);
	}

	if (bmp.GetWidth() != m_style->GetToolSize().x || bmp.GetHeight() != m_style->GetToolSize().y) {
		drawAt.x -= (bmp.GetWidth() - m_style->GetToolSize().x) / 2;
		drawAt.y -= (bmp.GetHeight() - m_style->GetToolSize().y) / 2;
	}

	dc.DrawBitmap(bmp, drawAt);
}

// ----------------------------------------------------------------------------
// toolbar geometry
// ----------------------------------------------------------------------------

wxToolBarToolBase* ToolBarSimple::FindToolForPosition(wxCoord x, wxCoord y)
{
	for (wxToolBarToolsList::iterator node = m_tools.begin(); node != m_tools.end(); ++node) {
		ToolBarTool* tool = static_cast<ToolBarTool*>(*node);
		if ((x >= tool->m_x) && (y >= tool->m_y) && (x < (tool->m_x + tool->GetWidth()))
			&& (y < (tool->m_y + tool->GetHeight()))) {
			return tool;
		}
	}

	return NULL;
}

void ToolBarSimple::InvalidateBitmaps()
{
	for (wxToolBarToolsList::iterator node = m_tools.begin(); node != m_tools.end(); ++node) {
		ToolBarTool* tool = static_cast<ToolBarTool*>(*node);
		tool->bitmapOK = false;
	}
}

wxRect ToolBarSimple::GetToolRect(int tool_id)
{
	wxRect rect;
	wxToolBarToolBase* tool = FindById(tool_id);
	if (tool) {
		ToolBarTool* otool = static_cast<ToolBarTool*>(tool);
		if (otool)
			rect = otool->trect;
	}

	return rect;
}

// ----------------------------------------------------------------------------
// tool state change handlers
// ----------------------------------------------------------------------------

void ToolBarSimple::DoEnableTool(wxToolBarToolBase* tool, bool WXUNUSED(enable))
{
	DrawTool(tool);
}

void ToolBarSimple::DoToggleTool(wxToolBarToolBase* tool, bool WXUNUSED(toggle))
{
	ToolBarTool* t = static_cast<ToolBarTool*>(tool);
	t->bitmapOK = false;
	DrawTool(tool);
}

// Okay, so we've left the tool we're in ... we must check if the tool we're
// leaving was a 'sprung push button' and if so, spring it back to the up
// state.
void ToolBarSimple::SpringUpButton(int id)
{
	wxToolBarToolBase* tool = FindById(id);

	if (tool && tool->CanBeToggled()) {
		if (tool->IsToggled())
			tool->Toggle();

		DrawTool(tool);
	}
}

// ----------------------------------------------------------------------------
// scrolling implementation
// ----------------------------------------------------------------------------

wxString ToolBarSimple::GetToolShortHelp(int id) const
{
	wxToolBarToolBase* tool = FindById(id);
	wxCHECK_MSG(tool, wxEmptyString, _T("no such tool"));

	return tool->GetShortHelp();
}

wxString ToolBarSimple::GetToolLongHelp(int id) const
{
	wxToolBarToolBase* tool = FindById(id);
	wxCHECK_MSG(tool, wxEmptyString, _T("no such tool"));

	return tool->GetLongHelp();
}

void ToolBarSimple::SetToolShortHelp(int id, const wxString& help)
{
	wxToolBarToolBase* tool = FindById(id);
	if (tool) {
		(void)tool->SetShortHelp(help);
	}
}

void ToolBarSimple::SetToolLongHelp(int id, const wxString& help)
{
	wxToolBarToolBase* tool = FindById(id);
	if (tool) {
		(void)tool->SetLongHelp(help);
	}
}

int ToolBarSimple::GetToolPos(int id) const
{
	size_t pos = 0;

	for (wxToolBarToolsList::const_iterator i = m_tools.begin(); i != m_tools.end(); ++i) {
		if ((*i)->GetId() == id)
			return pos;

		pos++;
	}

	return wxNOT_FOUND;
}

bool ToolBarSimple::GetToolState(int id) const
{
	wxToolBarToolBase* tool = FindById(id);
	wxCHECK_MSG(tool, false, _T("no such tool"));

	return tool->IsToggled();
}

bool ToolBarSimple::GetToolEnabled(int id) const
{
	wxToolBarToolBase* tool = FindById(id);
	wxCHECK_MSG(tool, false, _T("no such tool"));

	return tool->IsEnabled();
}

void ToolBarSimple::ToggleTool(int id, bool toggle)
{
	wxToolBarToolBase* tool = FindById(id);
	if (tool) {
		tool->Toggle(toggle);
		DoToggleTool(tool, toggle);
	}
	if (g_toolbar)
		g_toolbar->Refresh();
}

wxObject* ToolBarSimple::GetToolClientData(int id) const
{
	wxToolBarToolBase* tool = FindById(id);
	return tool ? tool->GetClientData() : (wxObject*)NULL;
}

void ToolBarSimple::SetToolClientData(int id, wxObject* clientData)
{
	wxToolBarToolBase* tool = FindById(id);

	wxCHECK_RET(tool, _T("no such tool in wxToolBar::SetToolClientData"));

	tool->SetClientData(clientData);
}

void ToolBarSimple::EnableTool(int id, bool enable)
{
	wxToolBarToolBase* tool = FindById(id);
	if (tool) {
		if (tool->Enable(enable)) {
			DoEnableTool(tool, enable);
		}
	}
	wxMenuItem* configItem = g_FloatingToolbarConfigMenu->FindItem(id);
	configItem->Check(true);
}

void ToolBarSimple::SetToolBitmaps(int id, wxBitmap* bmp, wxBitmap* bmpRollover)
{
	ToolBarTool* tool = static_cast<ToolBarTool*>(FindById(id));
	if (tool) {
		tool->pluginNormalIcon = bmp;
		tool->pluginRolloverIcon = bmpRollover;
		tool->bitmapOK = false;
	}
}

void ToolBarSimple::ClearTools()
{
	while (GetToolsCount()) {
		DeleteToolByPos(0);
	}
}

int ToolBarSimple::GetVisibleToolCount()
{
	return m_tools.size();
}

bool ToolBarSimple::DeleteToolByPos(size_t pos)
{
	wxCHECK_MSG(pos < GetToolsCount(), false,
				_T("invalid position in wxToolBar::DeleteToolByPos()"));

	wxToolBarToolsList::compatibility_iterator node = m_tools.Item(pos);

	if (!DoDeleteTool(pos, node->GetData())) {
		return false;
	}

	delete node->GetData();
	m_tools.Erase(node);

	return true;
}

bool ToolBarSimple::DeleteTool(int id)
{
	size_t pos = 0;
	wxToolBarToolsList::iterator node;
	for (node = m_tools.begin(); node != m_tools.end(); ++node) {
		if ((*node)->GetId() == id)
			break;

		pos++;
	}

	if ((node == m_tools.end()) || !DoDeleteTool(pos, *node)) {
		return false;
	}

	delete *node;
	m_tools.erase(node);

	return true;
}

wxToolBarToolBase* ToolBarSimple::AddSeparator()
{
	return InsertSeparator(GetToolsCount());
}

wxToolBarToolBase* ToolBarSimple::InsertSeparator(size_t pos)
{
	wxCHECK_MSG(pos <= GetToolsCount(), (wxToolBarToolBase*)NULL,
				_T("invalid position in wxToolBar::InsertSeparator()"));

	wxToolBarToolBase* tool
		= CreateTool(wxID_SEPARATOR, wxEmptyString, wxNullBitmap, wxNullBitmap, wxITEM_SEPARATOR,
					 (wxObject*)NULL, wxEmptyString, wxEmptyString);

	if (!tool || !DoInsertTool(pos, tool)) {
		delete tool;

		return NULL;
	}

	m_tools.Insert(pos, tool);

	return tool;
}

wxToolBarToolBase* ToolBarSimple::RemoveTool(int id)
{
	size_t pos = 0;
	wxToolBarToolsList::iterator node;
	for (node = m_tools.begin(); node != m_tools.end(); ++node) {
		if ((*node)->GetId() == id)
			break;

		pos++;
	}

	if (node == m_tools.end()) {
		// don't give any error messages - sometimes we might call RemoveTool()
		// without knowing whether the tool is or not in the toolbar
		return NULL;
	}

	if (!DoDeleteTool(pos, *node)) {
		return NULL;
	}

	m_tools.erase(node);

	return *node;
}

wxControl* ToolBarSimple::FindControl(int id)
{
	for (wxToolBarToolsList::const_iterator node = m_tools.begin(); node != m_tools.end(); ++node) {
		const wxToolBarToolBase* const tool = *node;
		if (tool->IsControl()) {
			wxControl* const control = tool->GetControl();

			if (!control) {
				wxFAIL_MSG(_T("NULL control in toolbar?"));
			} else if (control->GetId() == id) {
				// found
				return control;
			}
		}
	}

	return NULL;
}

wxToolBarToolBase* ToolBarSimple::FindById(int id) const
{
	for (wxToolBarToolsList::const_iterator i = m_tools.begin(); i != m_tools.end(); ++i) {
		if ((*i)->GetId() == id) {
			return *i;
		}
	}
	return NULL;
}

// ----------------------------------------------------------------------------
// event processing
// ----------------------------------------------------------------------------

// Only allow toggle if returns true
bool ToolBarSimple::OnLeftClick(int id, bool toggleDown)
{
	wxCommandEvent event(wxEVT_COMMAND_TOOL_CLICKED, id);
	event.SetEventObject(this);

	// we use SetInt() to make wxCommandEvent::IsChecked() return toggleDown
	event.SetInt(static_cast<int>(toggleDown));

	// and SetExtraLong() for backwards compatibility
	event.SetExtraLong(static_cast<long>(toggleDown));

	// Send events to this toolbar instead (and thence up the window hierarchy)
	GetEventHandler()->ProcessEvent(event);

	return true;
}

// Call when right button down.
void ToolBarSimple::OnRightClick(int id, long WXUNUSED(x), long WXUNUSED(y))
{
	wxCommandEvent event(wxEVT_COMMAND_TOOL_RCLICKED, id);
	event.SetEventObject(this);
	event.SetInt(id);

	HideTooltip();
	OCPNFloatingToolbarDialog* parent = static_cast<OCPNFloatingToolbarDialog*>(GetParent());
	parent->toolbarConfigChanged = false;
	wxMenu* contextMenu = new wxMenu();
	wxMenuItem* submenu
		= contextMenu->AppendSubMenu(g_FloatingToolbarConfigMenu, _("Visible buttons"));

	PopupMenu(contextMenu);

	contextMenu->Remove(submenu);
	delete contextMenu;

	if (parent->toolbarConfigChanged)
		gFrame->GetEventHandler()->AddPendingEvent(event);
}

// Called when the mouse cursor enters a tool bitmap (no button pressed).
// Argument is wxID_ANY if mouse is exiting the toolbar.
// Note that for this event, the id of the window is used,
// and the integer parameter of wxCommandEvent is used to retrieve
// the tool id.
void ToolBarSimple::OnMouseEnter(int id)
{
	wxCommandEvent event(wxEVT_COMMAND_TOOL_ENTER, GetId());
	event.SetEventObject(this);
	event.SetInt(id);

	wxFrame* frame = wxDynamicCast(GetParent(), wxFrame);
	if (frame) {
		wxString help;
		wxToolBarToolBase* tool = id == wxID_ANY ? (wxToolBarToolBase*)NULL : FindById(id);
		if (tool)
			help = tool->GetLongHelp();
		frame->DoGiveHelp(help, id != wxID_ANY);
	}

	(void)GetEventHandler()->ProcessEvent(event);
}

void ToolBarSimple::SetToolNormalBitmapEx(wxToolBarToolBase* tool, const wxString& iconName)
{
	if (tool) {
		ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

		wxBitmap bmp = style->GetToolIcon(iconName, ocpnStyle::TOOLICON_NORMAL);
		tool->SetNormalBitmap(bmp);
		ToolBarTool* otool = static_cast<ToolBarTool*>(tool);
		if (otool)
			otool->SetIconName(iconName);
	}
}

size_t ToolBarSimple::GetToolsCount() const
{
	return m_tools.size();
}

int ToolBarSimple::GetNoRowsOrColumns() const
{
	return m_currentRowsOrColumns;
}

int ToolBarSimple::GetLineCount() const
{
	return m_LineCount;
}

