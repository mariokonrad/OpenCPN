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

#ifndef __TOOLBARSIMPLE__H__
#define __TOOLBARSIMPLE__H__

#include <wx/toolbar.h>
#include <wx/timer.h>

#include <global/ColorScheme.h>

#define TOOLTIPON_TIMER 10000

class ToolTipWin;
class GrabberWin;
class ToolBarTool;

// ----------------------------------------------------------------------------
// ToolBarSimple is a generic toolbar implementation in pure wxWidgets
//    Adapted from wxToolBarSimple( deprecated )
// ----------------------------------------------------------------------------

class ToolBarSimple : public wxControl
{
	DECLARE_EVENT_TABLE()

public:
	ToolBarSimple();

	ToolBarSimple(wxWindow* parent, wxWindowID winid, const wxPoint& pos = wxDefaultPosition,
				  const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTB_HORIZONTAL,
				  const wxString& name = wxToolBarNameStr);

	bool Create(wxWindow* parent, wxWindowID winid, const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize, long style = wxNO_BORDER | wxTB_HORIZONTAL,
				const wxString& name = wxToolBarNameStr);

	virtual ~ToolBarSimple();

	virtual void SetToggledBackgroundColour(wxColour c);
	virtual void SetColorScheme(global::ColorScheme cs);

	// implementation from now on
	// --------------------------

	// event handlers
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouseEvent(wxMouseEvent& event);
	void OnKillFocus(wxFocusEvent& event);
	void OnToolTipTimerEvent(wxTimerEvent& event);

	wxToolBarToolBase* AddTool(int toolid, const wxString& label, const wxBitmap& bitmap,
							   const wxBitmap& bmpDisabled, wxItemKind kind = wxITEM_NORMAL,
							   const wxString& shortHelp = wxEmptyString,
							   const wxString& longHelp = wxEmptyString, wxObject* data = NULL);

	wxToolBarToolBase* AddTool(int toolid, const wxString& label, const wxBitmap& bitmap,
							   const wxString& shortHelp = wxEmptyString,
							   wxItemKind kind = wxITEM_NORMAL)
	{
		return AddTool(toolid, label, bitmap, wxNullBitmap, kind, shortHelp);
	}

	wxToolBarToolBase* InsertTool(size_t pos, int id, const wxString& label, const wxBitmap& bitmap,
								  const wxBitmap& bmpDisabled, wxItemKind kind,
								  const wxString& shortHelp, const wxString& longHelp,
								  wxObject* clientData);

	wxToolBarToolBase* InsertTool(size_t pos, wxToolBarToolBase* tool);

	// Only allow toggle if returns true. Call when left button up.
	virtual bool OnLeftClick(int toolid, bool toggleDown);

	// Call when right button down.
	virtual void OnRightClick(int toolid, long x, long y);

	// Called when the mouse cursor enters a tool bitmap.
	// Argument is wxID_ANY if mouse is exiting the toolbar.
	virtual void OnMouseEnter(int toolid);

	size_t GetToolsCount() const;
	int GetNoRowsOrColumns() const;
	int GetLineCount() const;

	int GetVisibleToolCount();

	void SetToolNormalBitmapEx(wxToolBarToolBase* tool, const wxString& iconname);

	// get the control with the given id or return NULL
	virtual wxControl* FindControl(int toolid);

	// add a separator to the toolbar
	virtual wxToolBarToolBase* AddSeparator();
	virtual wxToolBarToolBase* InsertSeparator(size_t pos);

	// remove the tool from the toolbar: the caller is responsible for actually
	// deleting the pointer
	virtual wxToolBarToolBase* RemoveTool(int toolid);

	// delete tool either by index or by position
	virtual bool DeleteToolByPos(size_t pos);
	virtual bool DeleteTool(int toolid);

	// delete all tools
	virtual void ClearTools();

	// must be called after all buttons have been created to finish toolbar
	// initialisation
	virtual bool Realize();

	// tools state
	// -----------

	virtual void EnableTool(int toolid, bool enable);
	virtual void ToggleTool(int toolid, bool toggle);

	virtual void SetToolBitmaps(int toolid, wxBitmap* bmp, wxBitmap* bmpRollover);
	void InvalidateBitmaps();

	// set/get tools client data (not for controls)
	virtual wxObject* GetToolClientData(int toolid) const;
	virtual void SetToolClientData(int toolid, wxObject* clientData);

	// returns tool pos, or wxNOT_FOUND if tool isn't found
	virtual int GetToolPos(int id) const;

	// return true if the tool is toggled
	virtual bool GetToolState(int toolid) const;

	virtual bool GetToolEnabled(int toolid) const;

	virtual void SetToolShortHelp(int toolid, const wxString& helpString);
	virtual wxString GetToolShortHelp(int toolid) const;
	virtual void SetToolLongHelp(int toolid, const wxString& helpString);
	virtual wxString GetToolLongHelp(int toolid) const;

	// toolbar geometry
	// ----------------

	// the toolbar can wrap - limit the number of columns or rows it may take
	void SetMaxRowsCols(int rows, int cols);
	int GetMaxRows() const;
	int GetMaxCols() const;

	// get/set the size of the bitmaps used by the toolbar: should be called
	// before adding any tools to the toolbar
	virtual void SetToolBitmapSize(const wxSize& size);
	virtual wxSize GetToolBitmapSize() const;

	// the button size in some implementations is bigger than the bitmap size:
	// get the total button size (by default the same as bitmap size)
	virtual wxSize GetToolSize() const;
	virtual wxRect GetToolRect(int tool_id);

	// returns a (non separator) tool containing the point (x, y) or NULL if
	// there is no tool at this point (corrdinates are client)
	wxToolBarToolBase* FindToolForPosition(wxCoord x, wxCoord y);

	// find the tool by id
	wxToolBarToolBase* FindById(int toolid) const;

	// return true if this is a vertical toolbar, otherwise false
	bool IsVertical() const;

	void HideTooltip();
	void KillTooltip();

	void EnableTooltips();
	void DisableTooltips();

protected:
	// common part of all ctors
	void Init();

	// implement base class pure virtuals
	virtual wxToolBarToolBase* DoAddTool(int toolid, const wxString& label, const wxBitmap& bitmap,
										 const wxBitmap& bmpDisabled, wxItemKind kind,
										 const wxString& shortHelp = wxEmptyString,
										 const wxString& longHelp = wxEmptyString,
										 wxObject* clientData = NULL, wxCoord xPos = wxDefaultCoord,
										 wxCoord yPos = wxDefaultCoord);

	virtual bool DoInsertTool(size_t pos, wxToolBarToolBase* tool);
	virtual bool DoDeleteTool(size_t pos, wxToolBarToolBase* tool);

	virtual void DoEnableTool(wxToolBarToolBase* tool, bool enable);
	virtual void DoToggleTool(wxToolBarToolBase* tool, bool toggle);

	virtual wxToolBarToolBase* CreateTool(int winid, const wxString& label,
										  const wxBitmap& bmpNormal, const wxBitmap& bmpDisabled,
										  wxItemKind kind, wxObject* clientData,
										  const wxString& shortHelp, const wxString& longHelp);

	void DrawTool(wxToolBarToolBase* tool);
	virtual void DrawTool(wxDC& dc, wxToolBarToolBase* tool);
	virtual void SpringUpButton(int index);

private:
	// the list of all our tools
	wxToolBarToolsList m_tools;

	// the maximum number of toolbar rows/columns
	int m_maxRows;
	int m_maxCols;

	// the size of the toolbar bitmaps
	wxCoord m_defaultWidth;
	wxCoord m_defaultHeight;

	int m_currentRowsOrColumns;
	int m_LineCount;

	int m_pressedTool;
	int m_currentTool;

	wxCoord m_lastX;
	wxCoord m_lastY;
	wxCoord m_maxWidth;
	wxCoord m_maxHeight;
	wxCoord m_xPos;
	wxCoord m_yPos;

	wxColour m_toggle_bg_color;
	wxColour m_toolOutlineColour;
	ToolTipWin* m_pToolTipWin;
	ToolBarTool* m_last_ro_tool;

	wxTimer m_tooltip_timer;
	int m_one_shot;
	bool m_btooltip_show;
};

#endif
