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

#ifndef __UTIL__VALUEFILTER__H__
#define __UTIL__VALUEFILTER__H__

#include <vector>

namespace util {

class ValueFilter
{
public:
	typedef std::vector<double>::size_type size_type;

	ValueFilter(size_type size = 0);
	virtual ~ValueFilter();

	size_type size() const;
	void resize(size_type);
	virtual void fill(double);
	virtual double get() const;
	virtual void push(double);

private:
	std::vector<double> values;
};

}

#endif
