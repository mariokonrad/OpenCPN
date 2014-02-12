/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 * Copyright (C) 2010 by David S. Register                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program; if not, write to the                           *
 * Free Software Foundation, Inc.,                                         *
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.           *
 **************************************************************************/

#include "DefaultRouteTracker.h"
#include <wx/jsonreader.h>
#include <plugin/PlugInManager.h>
#include <ToolBarSimple.h>
#include <RouteManagerDialog.h>
#include <GUI_IDs.h>
#include <Track.h>

#include <global/OCPN.h>
#include <global/Navigation.h>

#include <navigation/RouteManager.h>

extern PlugInManager* g_pi_manager;
extern ToolBarSimple* g_toolbar;
extern RouteManagerDialog* pRouteManagerDialog;
extern RouteList* pRouteList;

namespace navigation {

DefaultRouteTracker::DefaultRouteTracker()
	: track(NULL)
	, active(false)
{
}

DefaultRouteTracker::~DefaultRouteTracker()
{
	stop();
}

void DefaultRouteTracker::start()
{
	// TODO: what to do if track (and/or active) is already set, repeated start should be robust

	active = true;

	track = new Track; // FIXME: this should be delegated to some sort of factory
	pRouteList->push_back(track);

	track->Start(); // FIXME: this is actually a problem: tracks should not receive position events themselfes.

	notify_toolbar();
	notify_routemanager();
}

void DefaultRouteTracker::stop(bool do_add_point, bool process_track)
{
	do_stop(do_add_point, process_track);
}

Track* DefaultRouteTracker::do_stop(bool do_add_point, bool process_track)
{
	Track* return_val = track;

	if (track && process_track) {
		navigation::RouteManager& routemanager = global::OCPN::get().routeman();
		notify_plugins_stop();
		track->Stop(do_add_point);
		if (track->GetnPoints() < 2) {
			routemanager.DeleteRoute(track);
			return_val = NULL;
		} else {
			const global::Navigation::Track& nav_track = global::OCPN::get().nav().get_track();
			if (nav_track.TrackDaily) {
				Track* extended_track = track->DoExtendDaily();
				if (extended_track) {
					routemanager.DeleteRoute(track);
					return_val = extended_track;
				}
			}
		}
	}

	track = NULL;
	active = false;

	notify_routemanager();
	notify_toolbar();

	return return_val;
}

void DefaultRouteTracker::restart()
{
	if (!track)
		return;

	Track* previous = do_stop(true);
	start();

	// Set the restarted track's current state such that the current track point's attributes match
	// the attributes of the last point of the track that was just stopped at midnight.

	if (previous) {
		RoutePoint* midnight_point = previous->GetLastPoint();
		track->AdjustCurrentTrackPoint(midnight_point);
	}

	notify_routemanager();
}

void DefaultRouteTracker::set_precision(long precision)
{
	if (!track)
		return;

	// TODO: do some sanity checks (value >=0, etc.)?

	track->SetPrecision(precision);
}

bool DefaultRouteTracker::has_active_track() const
{
	return track != NULL;
}

bool DefaultRouteTracker::is_active_track(const Route* route) const
{
	return true
		&& route
		&& track
		&& track == route
		;
}

bool DefaultRouteTracker::is_active() const
{
	return active;
}

bool DefaultRouteTracker::is_running() const
{
	return true
		&& track
		&& track->IsRunning()
		;
}

geo::Position DefaultRouteTracker::get_last_position() const
{
	if (!track)
		return geo::Position();

	return track->GetLastPoint()->get_position(); // FIXME: trainwreck (and uses RoutePoint, should be delegated to Route::get_last_position() -> geo::Position)
}

void DefaultRouteTracker::notify_toolbar() const
{
	if (g_toolbar)
		g_toolbar->ToggleTool(ID_TRACK, is_active());
}

void DefaultRouteTracker::notify_routemanager() const
{
	if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
		pRouteManagerDialog->UpdateTrkListCtrl();
		pRouteManagerDialog->UpdateRouteListCtrl();
	}
}

void DefaultRouteTracker::notify_plugins_start() const
{
	// FIXME: this mechanism is backwards: plugins should register for events, not having the
	//        application sending actively data to plugins (observer pattern)

	wxJSONValue v;
	wxString name = track->get_name();
	if (name.IsEmpty()) {
		const RoutePoint* rp = track->GetPoint(1);
		if (rp && rp->GetCreateTime().IsValid())
			name = rp->GetCreateTime().FormatISODate() + _T(" ")
				   + rp->GetCreateTime().FormatISOTime();
		else
			name = _("(Unnamed Track)");
	}
	v[_T("Name")] = name;
	v[_T("GUID")] = track->guid();
	g_pi_manager->SendJSONMessageToAllPlugins(wxString(_T("OCPN_TRK_ACTIVATED")), v);
}

void DefaultRouteTracker::notify_plugins_stop() const
{
	// FIXME: this mechanism is backwards: plugins should register for events, not having the
	//        application sending actively data to plugins (observer pattern)

	wxJSONValue v;
	v[_T("GUID")] = track->guid();
	g_pi_manager->SendJSONMessageToAllPlugins(wxString(_T("OCPN_TRK_DEACTIVATED")), v);
}

}

