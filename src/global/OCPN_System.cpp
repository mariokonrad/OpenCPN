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

#include "OCPN_System.h"

namespace global {

const System::Data& OCPN_System::data() const
{
	return data_data;
}

void OCPN_System::set_home_location(const wxString& home_location)
{
	data_data.home_location = home_location;
}

void OCPN_System::set_private_data_dir(const wxString& directory)
{
	data_data.private_data_dir = directory;
}

void OCPN_System::set_tc_data_dir(const wxString& directory)
{
	data_data.tc_data_dir = directory;
}

void OCPN_System::set_config_file(const wxString& filename)
{
	data_data.config_file = filename;
}

void OCPN_System::set_log_file(const wxString& log_file)
{
	data_data.log_file = log_file;
}

void OCPN_System::set_sound_data_location(const wxString& directory)
{
	data_data.sound_data_location = directory;
}

void OCPN_System::set_world_map_location(const wxString& location)
{
	data_data.world_map_location = location;
}

void OCPN_System::set_chartlist_fileame(const wxString& filename)
{
	data_data.chartlist_filename = filename;
}

void OCPN_System::set_init_chart_dir(const wxString& directory)
{
	data_data.init_chart_dir = directory;
}

void OCPN_System::set_plugin_dir(const wxString& value)
{
	data_data.plugin_dir = value;
}

void OCPN_System::set_locale(const wxString& value)
{
	data_data.locale = value;
}

void OCPN_System::set_csv_location(const wxString& value)
{
	data_data.csv_location = value;
}

void OCPN_System::set_SENCPrefix(const wxString& value)
{
	data_data.SENCPrefix = value;
}

void OCPN_System::set_UserPresLibData(const wxString& value)
{
	data_data.UserPresLibData = value;
}

void OCPN_System::set_exe_path(const wxString& value)
{
	data_data.exe_path = value;
}

void OCPN_System::set_current_tide_dataset(const std::vector<wxString>& value)
{
	data_data.current_tide_dataset = value;
}

const System::Config& OCPN_System::config() const
{
	return data_config;
}

void OCPN_System::set_config_version_string(const wxString& version_string)
{
	data_config.version_string = version_string;
}

void OCPN_System::set_config_nav_message_shown(bool flag)
{
	data_config.nav_message_shown = flag;
}

void OCPN_System::set_config_memory_footprint(long value_kB)
{
	data_config.memory_footprint_kB = value_kB;
}

void OCPN_System::set_config_autosave_interval_seconds(long value)
{
	data_config.autosave_interval_seconds = value;
}

void OCPN_System::set_config_CacheLimit(long value)
{
	data_config.CacheLimit = value;
}

void OCPN_System::set_config_memCacheLimit(long value)
{
	data_config.memCacheLimit = value;
}

void OCPN_System::set_config_GPU_MemSize(long value)
{
	data_config.GPU_MemSize = value;
}

void OCPN_System::set_config_nmea_use_gll(bool value)
{
	data_config.nmea_UseGLL = value;
}

void OCPN_System::set_config_SetSystemTime(bool value)
{
	data_config.SetSystemTime = value;
}

void OCPN_System::set_config_PlayShipsBells(bool value)
{
	data_config.PlayShipsBells = value;
}

void OCPN_System::set_config_restore_stackindex(int value)
{
	data_config.restore_stackindex = value;
}

void OCPN_System::set_config_restore_dbindex(int value)
{
	data_config.restore_dbindex = value;
}

void OCPN_System::set_config_SDMMFormat(int value)
{
	data_config.SDMMFormat = value;
}

void OCPN_System::set_config_DistanceFormat(int value)
{
	data_config.DistanceFormat = value;
}

void OCPN_System::set_config_SpeedFormat(int value)
{
	data_config.SpeedFormat = value;
}

void OCPN_System::set_config_COMPortCheck(int value)
{
	data_config.COMPortCheck = value;
}

void OCPN_System::set_config_uploadConnection(const wxString& value)
{
	data_config.uploadConnection = value;
}

void OCPN_System::set_config_GPS_Ident(const wxString& value)
{
	data_config.GPS_Ident = value;
}

void OCPN_System::set_config_GarminHostUpload(bool value)
{
	data_config.GarminHostUpload = value;
}

void OCPN_System::set_config_filter_cogsog(bool value)
{
	data_config.filter_cogsog = value;
}

void OCPN_System::set_config_COGFilterSec(int value)
{
	data_config.COGFilterSec = value;
}

void OCPN_System::set_config_SOGFilterSec(int value)
{
	data_config.SOGFilterSec = value;
}

void OCPN_System::set_config_portable(bool value)
{
	data_config.portable = value;
}

void OCPN_System::set_config_assume_azerty(bool value)
{
	data_config.assume_azerty = value;
}

const System::Debug& OCPN_System::debug() const
{
	return data_debug;
}

void OCPN_System::set_debug_gdal(bool value)
{
	data_debug.gdal = value;
}

void OCPN_System::set_debug_nmea(long value)
{
	data_debug.nmea = value;
}

void OCPN_System::set_debug_ogl(bool value)
{
	data_debug.ogl = value;
}

void OCPN_System::set_debug_cm93(bool value)
{
	data_debug.cm93 = value;
}

void OCPN_System::set_debug_s57(bool value)
{
	data_debug.s57 = value;
}

void OCPN_System::set_debug_total_NMEAerror_messages(int value)
{
	data_debug.total_NMEAerror_messages = value;
}

void OCPN_System::inc_debug_total_NMEAerror_messages()
{
	data_debug.total_NMEAerror_messages++;
}

}

