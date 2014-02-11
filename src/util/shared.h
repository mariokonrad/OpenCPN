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

#ifndef __UTIL__SHARED__H__
#define __UTIL__SHARED__H__

namespace util {

/// This is a very lightweight implementation of a shared "resource" (pointer).
///
/// Ultimately this should be replaced by the shared pointer (std::shared_ptr or
/// boost::shared_ptr) as soon as possible. Since this software does not use
/// boost (nor want the additional dependency) and is not using C++11 (or newer),
/// this template helps to maintain code semantics.
///
/// @note This "lightweight" implementation does not do any reference counting or
///       memory management whatsoever. It just provides access to a pointer
///       and represents a shared resoruce.
template <class T>
class shared
{
public:
	shared()
		: data(0)
	{}

	shared(T* data)
		: data(data)
	{}

	T* operator->() const
	{
		return data;
	}

	T& operator*() const
	{
		return *data;
	}

	shared& operator=(T* data)
	{
		this->data = data;
		return *this;
	}

	bool operator==(T* other) const
	{
		return data == other;
	}

	void reset(T* data = 0)
	{
		this->data = data;
	}

private:
	T* data;
};

}

#endif
