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

#include "ConsoleCanvas.h"
#include <AnnunText.h>
#include <CDI.h>
#include <MainFrame.h>
#include <RoutePoint.h>
#include <Route.h>

#include <navigation/MagneticVariation.h>

#include <global/OCPN.h>
#include <global/ColorManager.h>
#include <global/Navigation.h>
#include <global/GUI.h>

#include <navigation/RouteManager.h>

#include <cstdlib>
#include <cmath>
#include <ctime>

#include <wx/datetime.h>

extern MainFrame* gFrame;

enum eMenuItems {
	ID_NAVLEG,
	ID_NAVROUTE,
	ID_NAVHIGHWAY
} menuItems;

BEGIN_EVENT_TABLE(ConsoleCanvas, wxWindow)
	EVT_PAINT(ConsoleCanvas::OnPaint)
	EVT_SHOW(ConsoleCanvas::OnShow)
	EVT_CONTEXT_MENU(ConsoleCanvas::OnContextMenu)
	EVT_MENU(ID_NAVLEG, ConsoleCanvas::OnContextMenuSelection)
	EVT_MENU(ID_NAVROUTE, ConsoleCanvas::OnContextMenuSelection)
	EVT_MENU(ID_NAVHIGHWAY, ConsoleCanvas::OnContextMenuSelection)
END_EVENT_TABLE()

// Define a constructor for my canvas
ConsoleCanvas::ConsoleCanvas(wxWindow* frame)
{
	pbackBrush = NULL;
	m_bNeedClear = false;

	long style = wxSIMPLE_BORDER | wxCLIP_CHILDREN;
#ifdef __WXOSX__
	style |= wxSTAY_ON_TOP;
#endif

	wxDialog::Create(frame, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize, style);

	m_pParent = frame;

	m_pitemBoxSizerLeg = new wxBoxSizer(wxVERTICAL);

	pThisLegText = new wxStaticText(this, -1, _("This Leg"));
	pThisLegText->Fit();
	m_pitemBoxSizerLeg->Add(pThisLegText, 0, wxALIGN_CENTER_HORIZONTAL, 2);

	pThisLegFont = wxTheFontList->FindOrCreateFont(10, wxDEFAULT, wxNORMAL, wxBOLD);

	pThisLegText->SetFont(*pThisLegFont);

	pXTE = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
	pXTE->SetALabel(_T("XTE"));
	m_pitemBoxSizerLeg->Add(pXTE, 1, wxALIGN_LEFT | wxALL, 2);

	pBRG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
	pBRG->SetALabel(_T("BRG"));
	m_pitemBoxSizerLeg->Add(pBRG, 1, wxALIGN_LEFT | wxALL, 2);

	pVMG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
	pVMG->SetALabel(_T("VMG"));
	m_pitemBoxSizerLeg->Add(pVMG, 1, wxALIGN_LEFT | wxALL, 2);

	pRNG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
	pRNG->SetALabel(_T("RNG"));
	m_pitemBoxSizerLeg->Add(pRNG, 1, wxALIGN_LEFT | wxALL, 2);

	pTTG = new AnnunText(this, -1, _("Console Legend"), _("Console Value"));
	pTTG->SetALabel(_T("TTG"));
	m_pitemBoxSizerLeg->Add(pTTG, 1, wxALIGN_LEFT | wxALL, 2);

	// Create CDI Display Window

	pCDI = new CDI(this, -1, wxSIMPLE_BORDER, _T("CDI"));
	m_pitemBoxSizerLeg->AddSpacer(5);
	m_pitemBoxSizerLeg->Add(pCDI, 0, wxALL | wxEXPAND, 2);

	m_bShowRouteTotal = false;

	SetSizer(m_pitemBoxSizerLeg); // use the sizer for layout
	m_pitemBoxSizerLeg->SetSizeHints(this);
	Layout();
	Fit();

	Hide();
}

ConsoleCanvas::~ConsoleCanvas()
{
	delete pCDI;
}

void ConsoleCanvas::SetColorScheme(global::ColorScheme cs)
{
	wxColour color = global::OCPN::get().color().get_color(_T("DILG1"));

	pbackBrush = wxTheBrushList->FindOrCreateBrush(color, wxSOLID);
	SetBackgroundColour(color);

	// Also apply color scheme to all known children

	pThisLegText->SetBackgroundColour(color);

	pXTE->SetColorScheme(cs);
	pBRG->SetColorScheme(cs);
	pRNG->SetColorScheme(cs);
	pTTG->SetColorScheme(cs);
	pVMG->SetColorScheme(cs);
	pCDI->SetColorScheme(cs);
}

void ConsoleCanvas::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);

	if (global::OCPN::get().routeman().GetpActiveRoute()) {
		if (m_bNeedClear) {
			pThisLegText->Refresh();
			m_bNeedClear = false;
		}

		UpdateRouteData();
	}

	if (!global::OCPN::get().gui().view().show_active_route_highway)
		pCDI->Hide();
}

void ConsoleCanvas::OnShow(wxShowEvent&)
{
	pCDI->Show(global::OCPN::get().gui().view().show_active_route_highway);
	m_pitemBoxSizerLeg->SetSizeHints(this);
}

void ConsoleCanvas::LegRoute()
{
	if (m_bShowRouteTotal)
		pThisLegText->SetLabel(_("Route"));
	else
		pThisLegText->SetLabel(_("This Leg"));

	pThisLegText->Refresh(true);
	RefreshConsoleData();
}

void ConsoleCanvas::OnContextMenu(wxContextMenuEvent&)
{
	wxMenu* contextMenu = new wxMenu();
	wxMenuItem* btnLeg
		= new wxMenuItem(contextMenu, ID_NAVLEG, _("This Leg"), _T(""), wxITEM_RADIO);
	wxMenuItem* btnRoute
		= new wxMenuItem(contextMenu, ID_NAVROUTE, _("Full Route"), _T(""), wxITEM_RADIO);
	wxMenuItem* btnHighw
		= new wxMenuItem(contextMenu, ID_NAVHIGHWAY, _("Show Highway"), _T(""), wxITEM_CHECK);
	contextMenu->Append(btnLeg);
	contextMenu->Append(btnRoute);
	contextMenu->AppendSeparator();
	contextMenu->Append(btnHighw);

	btnLeg->Check(!m_bShowRouteTotal);
	btnRoute->Check(m_bShowRouteTotal);
	btnHighw->Check(global::OCPN::get().gui().view().show_active_route_highway);

	PopupMenu(contextMenu);

	delete contextMenu;
}

void ConsoleCanvas::OnContextMenuSelection(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case ID_NAVLEG:
			m_bShowRouteTotal = false;
			LegRoute();
			break;

		case ID_NAVROUTE:
			m_bShowRouteTotal = true;
			LegRoute();
			break;

		case ID_NAVHIGHWAY: {
			global::GUI& gui = global::OCPN::get().gui();
			gui.set_view_show_active_route_highway(!gui.view().show_active_route_highway);
			if (gui.view().show_active_route_highway) {
				pCDI->Show();
			} else {
				pCDI->Hide();
			}
			m_pitemBoxSizerLeg->SetSizeHints(this);
			} break;
	}
}

void ConsoleCanvas::UpdateRouteData()
{
	navigation::RouteManager& routemanager = global::OCPN::get().routeman();
	if (!routemanager.GetpActiveRoute())
		return;

	if (!routemanager.is_data_valid())
		return;

	// Range
	wxString srng;
	float rng = routemanager.GetCurrentRngToActivePoint();
	float nrng = routemanager.GetCurrentRngToActiveNormalArrival();

	// show if there is more than 10% difference in ranges, etc...
	double deltarng = fabs(rng - nrng);
	if ((deltarng > 0.01) && ((deltarng / rng) > 0.10) && (rng < 10.0)) {
		if (nrng < 10.0)
			srng.Printf(_T("%5.2f/%5.2f"), rng, nrng);
		else
			srng.Printf(_T("%5.1f/%5.1f"), rng, nrng);
	} else {
		if (rng < 10.0)
			srng.Printf(_T("%6.2f"), rng);
		else
			srng.Printf(_T("%6.1f"), rng);
	}

	if (!m_bShowRouteTotal)
		pRNG->SetAValue(srng);

	// Brg
	float dcog = routemanager.GetCurrentBrgToActivePoint();
	if (dcog >= 359.5)
		dcog = 0;

	wxString cogstr;
	if (global::OCPN::get().gui().view().ShowMag)
		cogstr << wxString::Format(wxString("%6.0f(M)", wxConvUTF8),
								   navigation::GetTrueOrMag(dcog));
	else
		cogstr << wxString::Format(wxString("%6.0f", wxConvUTF8), navigation::GetTrueOrMag(dcog));

	pBRG->SetAValue(cogstr);

	// XTE
	wxString str_buf;
	str_buf.Printf(_T("%6.2f"), routemanager.GetCurrentXTEToActivePoint());
	pXTE->SetAValue(str_buf);
	if (routemanager.GetXTEDir() < 0)
		pXTE->SetALabel(wxString(_("XTE         L")));
	else
		pXTE->SetALabel(wxString(_("XTE         R")));

	// VMG
	// VMG is always to next waypoint, not to end of route
	// VMG is SOG x cosine (difference between COG and BRG to Waypoint)
	const global::Navigation::Data& nav = global::OCPN::get().nav().get_data();
	double VMG = 0.0;
	if (!wxIsNaN(nav.cog) && !wxIsNaN(nav.sog)) {
		double BRG;
		BRG = routemanager.GetCurrentBrgToActivePoint();
		VMG = nav.sog * cos((BRG - nav.cog) * M_PI / 180.0);
		str_buf.Printf(_T("%6.2f"), VMG);
	} else
		str_buf = _T("---");

	pVMG->SetAValue(str_buf);

	// TTG
	// In all cases, ttg/eta are declared invalid if VMG <= 0.

	// If showing only "this leg", use VMG for calculation of ttg
	wxString ttg_s;
	if ((VMG > 0.0) && !wxIsNaN(nav.cog) && !wxIsNaN(nav.sog)) {
		float ttg_sec = (rng / VMG) * 3600.0;
		wxTimeSpan ttg_span(0, 0, long(ttg_sec), 0);
		ttg_s = ttg_span.Format();
	} else {
		ttg_s = _T("---");
	}

	if (!m_bShowRouteTotal)
		pTTG->SetAValue(ttg_s);

	// Remainder of route
	float trng = rng;

	Route* prt = routemanager.GetpActiveRoute();

	int n_addflag = 0;
	for (RoutePointList::iterator node = prt->routepoints().begin();
		 node != prt->routepoints().end(); ++node) {
		RoutePoint* prp = *node;
		if (n_addflag)
			trng += prp->segment.length;

		if (prp == prt->m_pRouteActivePoint)
			n_addflag++;
	}

	// total rng
	wxString strng;
	if (trng < 10.0)
		strng.Printf(_T("%6.2f"), trng);
	else
		strng.Printf(_T("%6.1f"), trng);

	if (m_bShowRouteTotal)
		pRNG->SetAValue(strng);

	// total ttg
	// If showing total route ttg/ETA, use speed over ground for calculation

	wxString tttg_s;
	wxTimeSpan tttg_span;
	if (VMG > 0.) {
		float tttg_sec = (trng / nav.sog) * 3600.0;
		tttg_span = wxTimeSpan::Seconds((long)tttg_sec);
		tttg_s = tttg_span.Format();
	} else {
		tttg_span = wxTimeSpan::Seconds(0);
		tttg_s = _T("---");
	}

	if (m_bShowRouteTotal)
		pTTG->SetAValue(tttg_s);

	// total ETA to be shown on XTE panel
	if (m_bShowRouteTotal) {
		wxDateTime dtnow, eta;
		dtnow.SetToCurrent();
		eta = dtnow.Add(tttg_span);
		wxString seta;

		if (VMG > 0.0)
			seta = eta.Format(_T("%H:%M"));
		else
			seta = _T("---");

		pXTE->SetAValue(seta);
		pXTE->SetALabel(wxString(_("ETA          ")));
	}

	pRNG->Refresh();
	pBRG->Refresh();
	pVMG->Refresh();
	pTTG->Refresh();
	pXTE->Refresh();
}

void ConsoleCanvas::RefreshConsoleData(void)
{
	UpdateRouteData();

	pRNG->Refresh();
	pBRG->Refresh();
	pVMG->Refresh();
	pTTG->Refresh();
	pXTE->Refresh();
	pCDI->Refresh();
}

void ConsoleCanvas::ShowWithFreshFonts(void)
{
	Hide();
	Move(0, 0);

	UpdateFonts();
	gFrame->PositionConsole();
	Show();
}

void ConsoleCanvas::UpdateFonts(void)
{
	pBRG->RefreshFonts();
	pXTE->RefreshFonts();
	pTTG->RefreshFonts();
	pRNG->RefreshFonts();
	pVMG->RefreshFonts();

	m_pitemBoxSizerLeg->SetSizeHints(this);
	Layout();
	Fit();

	Refresh();
}

void ConsoleCanvas::clear_background(void)
{
	pCDI->ClearBackground();
}

