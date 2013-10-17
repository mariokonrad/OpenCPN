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

const System::Data & OCPN_System::data() const
{
	return data_data;
}

void OCPN_System::set_home_location(const wxString & home_location)
{
	data_data.home_location = home_location;
}

void OCPN_System::set_private_data_dir(const wxString & directory)
{
	data_data.private_data_dir = directory;
}

void OCPN_System::set_tc_data_dir(const wxString & directory)
{
	data_data.tc_data_dir = directory;
}

void OCPN_System::set_log_file(const wxString & log_file)
{
	data_data.log_file = log_file;
}

void OCPN_System::set_sound_data_location(const wxString & directory)
{
	data_data.sound_data_location = directory;
}

const System::Config & OCPN_System::config() const
{
	return data_config;
}

void OCPN_System::set_config_version_string(const wxString & version_string)
{
	data_config.version_string = version_string;
}

void OCPN_System::set_config_nav_message_shown(bool flag)
{
	data_config.nav_message_shown = flag;
}

}

