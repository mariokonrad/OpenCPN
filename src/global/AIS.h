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

#ifndef __GLOBAL__AIS__H__
#define __GLOBAL__AIS__H__

namespace global {

class AIS
{
public:
	virtual ~AIS()
	{
	}

public:
	struct Data
	{
		bool CPAMax;
		double CPAMax_NM;
		bool CPAWarn;
		double CPAWarn_NM;
		bool TCPA_Max;
	};

	virtual const Data& get_data() const = 0;
	virtual void set_CPAMax(bool) = 0;
	virtual void set_CPAMax_NM(double) = 0;
	virtual void set_CPAWarn(bool) = 0;
	virtual void set_CPAWarn_NM(double) = 0;
	virtual void set_TCPA_Max(bool) = 0;
};

}

#endif
