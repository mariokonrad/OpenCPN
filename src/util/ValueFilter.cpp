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

#include "ValueFilter.h"
#include <algorithm>
#include <numeric>

namespace util {

ValueFilter::ValueFilter(size_type size)
	: values(size)
{
}

ValueFilter::~ValueFilter()
{
}

ValueFilter::size_type ValueFilter::size() const
{
	return values.size();
}

void ValueFilter::resize(size_type n)
{
	values.resize(n);
}

double ValueFilter::get() const
{
	if (size() == 0)
		return 0.0;

	return std::accumulate(values.begin(), values.end(), 0.0) / size();
}

/// Pushes a new element into the filter.
///
/// The new element will always be at the last position of the container.
/// This way the values reside within the container in chronological order.
void ValueFilter::push(double value)
{
	if (size() == 0)
		return;
	if (size() > 1)
		std::rotate(values.begin(), values.begin() + 1, values.end());
	values[size() - 1] = value;
}

}

