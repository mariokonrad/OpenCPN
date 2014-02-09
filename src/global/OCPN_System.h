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

#ifndef __GLOBAL__OCPN_SYSTEM__H__
#define __GLOBAL__OCPN_SYSTEM__H__

#include <global/System.h>

namespace global {

class OCPN_System : public System
{
private:
	Data data_data;
	Config data_config;
	Debug data_debug;

public: // data
	virtual const Data& data() const;
	virtual void set_home_location(const wxString&);
	virtual void set_private_data_dir(const wxString&);
	virtual void set_tc_data_dir(const wxString&);
	virtual void set_config_file(const wxString&);
	virtual void set_log_file(const wxString&);
	virtual void set_sound_data_location(const wxString&);
	virtual void set_world_map_location(const wxString&);
	virtual void set_chartlist_fileame(const wxString&);
	virtual void set_init_chart_dir(const wxString&);
	virtual void set_plugin_dir(const wxString&);
	virtual void set_locale(const wxString&);
	virtual void set_csv_location(const wxString&);
	virtual void set_SENCPrefix(const wxString&);
	virtual void set_UserPresLibData(const wxString&);

public: // config
	virtual const Config& config() const;
	virtual void set_config_version_string(const wxString&);
	virtual void set_config_nav_message_shown(bool);
	virtual void set_config_memory_footprint(long);
	virtual void set_config_autosave_interval_seconds(long);
	virtual void set_config_CacheLimit(long);
	virtual void set_config_memCacheLimit(long);
	virtual void set_config_GPU_MemSize(long);
	virtual void set_config_nmea_use_gll(bool);
	virtual void set_config_SetSystemTime(bool);
	virtual void set_config_PlayShipsBells(bool);
	virtual void set_config_restore_stackindex(int);
	virtual void set_config_restore_dbindex(int);
	virtual void set_config_SDMMFormat(int);
	virtual void set_config_DistanceFormat(int);
	virtual void set_config_SpeedFormat(int);
	virtual void set_config_COMPortCheck(int);
	virtual void set_config_uploadConnection(const wxString&);
	virtual void set_config_GPS_Ident(const wxString&);
	virtual void set_config_GarminHostUpload(bool);
	virtual void set_config_filter_cogsog(bool);
	virtual void set_config_COGFilterSec(int);
	virtual void set_config_SOGFilterSec(int);

public: // debug
	virtual const Debug& debug() const;
	virtual void set_debug_gdal(bool);
	virtual void set_debug_nmea(long);
	virtual void set_debug_ogl(bool);
	virtual void set_debug_cm93(bool);
	virtual void set_debug_s57(bool);
	virtual void set_debug_total_NMEAerror_messages(int);
	virtual void inc_debug_total_NMEAerror_messages();
};

}

#endif
