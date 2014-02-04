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

#ifndef __GLOBAL__SYSTEM__H__
#define __GLOBAL__SYSTEM__H__

#include <wx/string.h>

namespace global {

class System
{
public:
	virtual ~System()
	{
	}

public:
	struct Data
	{
		wxString home_location;
		wxString private_data_dir;
		wxString tc_data_dir;
		wxString config_file;
		wxString log_file;
		wxString sound_data_location;
		wxString world_map_location;
		wxString chartlist_filename;
		wxString init_chart_dir;
		wxString plugin_dir;
		wxString locale;
		wxString csv_location;
		wxString SENCPrefix;
		wxString UserPresLibData;
	};

	virtual const Data& data() const = 0;
	virtual void set_home_location(const wxString&) = 0;
	virtual void set_private_data_dir(const wxString&) = 0;
	virtual void set_tc_data_dir(const wxString&) = 0;
	virtual void set_config_file(const wxString&) = 0;
	virtual void set_log_file(const wxString&) = 0;
	virtual void set_sound_data_location(const wxString&) = 0;
	virtual void set_world_map_location(const wxString&) = 0;
	virtual void set_chartlist_fileame(const wxString&) = 0;
	virtual void set_init_chart_dir(const wxString&) = 0;
	virtual void set_plugin_dir(const wxString&) = 0;
	virtual void set_locale(const wxString&) = 0;
	virtual void set_csv_location(const wxString&) = 0;
	virtual void set_SENCPrefix(const wxString&) = 0;
	virtual void set_UserPresLibData(const wxString&) = 0;

public:
	struct Config
	{
		wxString version_string;
		bool nav_message_shown;
		long memory_footprint_kB;
		long autosave_interval_seconds;
		long CacheLimit;
		long memCacheLimit;
		long GPU_MemSize;
		bool nmea_UseGLL;
		bool SetSystemTime;
		bool PlayShipsBells;
		int restore_stackindex;
		int restore_dbindex;

		// 0 = "Degrees, Decimal minutes"
		// 1 = "Decimal degrees"
		// 2 = "Degrees, Minutes, Seconds"
		int SDMMFormat;

		// 0 = "Nautical miles"
		// 1 = "Statute miles"
		// 2 = "Kilometers"
		// 3 = "Meters"
		// ...
		// see DistanceUnit
		int DistanceFormat;

		// 0 = "kts",
		// 1 = "mph"
		// 2 = "km/h"
		// 3 = "m/s"
		// ...
		// see SpeedUnit
		int SpeedFormat;

		int COMPortCheck; /// Number of COM ports on windows to be checked
		wxString uploadConnection;
	};

	virtual const Config& config() const = 0;
	virtual void set_config_version_string(const wxString&) = 0;
	virtual void set_config_nav_message_shown(bool) = 0;
	virtual void set_config_memory_footprint(long) = 0;
	virtual void set_config_autosave_interval_seconds(long) = 0;
	virtual void set_config_CacheLimit(long) = 0;
	virtual void set_config_memCacheLimit(long) = 0;
	virtual void set_config_GPU_MemSize(long) = 0;
	virtual void set_config_nmea_use_gll(bool) = 0;
	virtual void set_config_SetSystemTime(bool) = 0;
	virtual void set_config_PlayShipsBells(bool) = 0;
	virtual void set_config_restore_stackindex(int) = 0;
	virtual void set_config_restore_dbindex(int) = 0;
	virtual void set_config_SDMMFormat(int) = 0;
	virtual void set_config_DistanceFormat(int) = 0;
	virtual void set_config_SpeedFormat(int) = 0;
	virtual void set_config_COMPortCheck(int) = 0;
	virtual void set_config_uploadConnection(const wxString&) = 0;

public:
	struct Debug
	{
		bool gdal;
		long nmea;
		bool ogl;
		bool cm93;
		bool s57;
	};

	virtual const Debug& debug() const = 0;
	virtual void set_debug_gdal(bool) = 0;
	virtual void set_debug_nmea(long) = 0;
	virtual void set_debug_ogl(bool) = 0;
	virtual void set_debug_cm93(bool) = 0;
	virtual void set_debug_s57(bool) = 0;
};

}

#endif
