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

// FIXME: merge MarkInfoDef and MarkInfoImpl

#ifndef __MARKINFO__H__
#define __MARKINFO__H__

#include "RoutePoint.h"

#include <global/ColorScheme.h>

#include <wx/dialog.h>

class wxBoxSizer;
class wxStaticBoxSizer;
class wxBitmapComboBox;
class wxButton;
class wxCheckBox;
class wxStaticText;
class wxTextCtrl;
class wxHyperlinkCtrl;
class wxHyperlinkEvent;
class wxMenu;
class wxToggleButton;
class wxNotebook;
class wxPanel;
class wxStaticBitmap;
class wxScrolledWindow;

class LinkPropDialog;

class MarkInfoDef : public wxDialog
{
public:
	MarkInfoDef(wxWindow* parent, wxWindowID id = wxID_ANY,
				const wxString& title = _("Waypoint Properties"),
				const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(450, 550),
				long style = wxDEFAULT_DIALOG_STYLE | wxMAXIMIZE_BOX | wxRESIZE_BORDER);

	virtual ~MarkInfoDef();
	void hyperlink17OnContextMenu(wxMouseEvent& event);

protected:
	wxBoxSizer* bSizerLinks;
	wxBitmapComboBox* m_bcomboBoxIcon;
	wxStaticBitmap* m_bitmapIcon;
	wxButton* m_buttonAddLink;
	wxButton* m_buttonExtDescription;
	wxCheckBox* m_checkBoxShowName;
	wxCheckBox* m_checkBoxVisible;
	wxHyperlinkCtrl* m_hyperlink17;
	wxObject* m_contextObject;
	wxMenu* m_menuLink;
	wxNotebook* m_notebookProperties;
	wxScrolledWindow* m_panelBasicProperties;
	wxPanel* m_panelDescription;
	wxPanel* m_panelExtendedProperties;
	wxScrolledWindow* m_scrolledWindowLinks;
	wxStdDialogButtonSizer* m_sdbSizerButtons;
	wxButton* m_sdbSizerButtonsCancel;
	wxButton* m_sdbSizerButtonsOK;
	wxStaticText* m_staticTextDescription;
	wxStaticText* m_staticTextEditEnabled;
	wxStaticText* m_staticTextGpx;
	wxStaticText* m_staticTextGuid;
	wxStaticText* m_staticTextIcon;
	wxStaticText* m_staticTextLatitude;
	wxStaticText* m_staticTextLayer;
	wxStaticText* m_staticTextLongitude;
	wxStaticText* m_staticTextName;
	wxTextCtrl* m_textCtrlExtDescription;
	wxTextCtrl* m_textCtrlGpx;
	wxTextCtrl* m_textCtrlGuid;
	wxTextCtrl* m_textDescription;
	wxTextCtrl* m_textLatitude;
	wxTextCtrl* m_textLongitude;
	wxTextCtrl* m_textName;
	wxToggleButton* m_toggleBtnEdit;
	wxStaticBoxSizer* sbSizerLinks;
	wxSize m_defaultClientSize;

	// Virtual event handlers, overide them in your derived class
	virtual void OnPositionCtlUpdated(wxCommandEvent& event);
	virtual void OnDescChangedBasic(wxCommandEvent& event);
	virtual void OnExtDescriptionClick(wxCommandEvent& event);
	virtual void OnDeleteLink(wxCommandEvent& event);
	virtual void OnEditLink(wxCommandEvent& event);
	virtual void OnAddLink(wxCommandEvent& event);
	virtual void OnEditLinkToggle(wxCommandEvent& event);
	virtual void OnDescChangedExt(wxCommandEvent& event);
	virtual void OnMarkInfoCancelClick(wxCommandEvent& event);
	virtual void OnMarkInfoOKClick(wxCommandEvent& event);
	void OnCopyPasteLatLon(wxCommandEvent& event);
};

class MarkInfoImpl : public MarkInfoDef
{
public:
	void SetColorScheme(global::ColorScheme cs);
	void OnMarkInfoOKClick(wxCommandEvent& event);
	void OnMarkInfoCancelClick(wxCommandEvent& event);
	void SetRoutePoint(RoutePoint* pRP);

	void SetDialogTitle(const wxString& title);
	RoutePoint* GetRoutePoint(void);

	bool UpdateProperties(bool positionOnly = false);
	void ValidateMark(void);
	void InitialFocus(void);
	void OnRightClick(wxCommandEvent& event);

	MarkInfoImpl(wxWindow* parent, wxWindowID id = wxID_ANY,
				 const wxString& title = _("Waypoint Information"),
				 const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(450, 550),
				 long style = wxDEFAULT_DIALOG_STYLE | wxMAXIMIZE_BOX | wxRESIZE_BORDER);

	virtual ~MarkInfoImpl();

	void hyperlinkContextMenu(wxMouseEvent& event);

protected:
	virtual void OnPositionCtlUpdated(wxCommandEvent& event);
	void OnDeleteLink(wxCommandEvent& event);
	void OnEditLink(wxCommandEvent& event);
	void OnAddLink(wxCommandEvent& event);
	void OnEditLinkToggle(wxCommandEvent& event);
	void OnDescChangedBasic(wxCommandEvent& event);
	void OnDescChangedExt(wxCommandEvent& event);
	void OnExtDescriptionClick(wxCommandEvent& event);

private:
	void build_hyperlink_list();
	void add_hyperlink(const wxString& desc, const wxString& link, bool on_layer = false);

	RoutePoint* m_pRoutePoint;
	Hyperlinks m_pMyLinkList;
	void OnHyperLinkClick(wxHyperlinkEvent& event);
	LinkPropDialog* m_pLinkProp;
	bool SaveChanges();
	wxHyperlinkCtrl* m_pEditedLink;

	int m_current_icon_Index;
	double m_lat_save;
	double m_lon_save;
	wxString m_IconName_save;
	bool m_bShowName_save;
	bool m_bIsVisible_save;
};

#endif
