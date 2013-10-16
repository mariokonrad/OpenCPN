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

	public: // data
		virtual const Data & data() const;
		virtual void set_home_location(const wxString &);
		virtual void set_private_data_dir(const wxString &);
		virtual void set_tc_data_dir(const wxString &);
		virtual void set_log_file(const wxString &);

	public: // config
		virtual const Config & config() const;
		virtual void set_config_version_string(const wxString &);
		virtual void set_config_nav_message_shown(bool);
};

}

#endif
