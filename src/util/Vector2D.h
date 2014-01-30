/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __UTIL__VECTOR2D_H__
#define __UTIL__VECTOR2D_H__

namespace util {

class Vector2D
{
public:
	Vector2D();
	Vector2D(double a, double b);

	double dot(const Vector2D& other) const;
	double sqr() const;
	double length() const;

	friend bool operator==(const Vector2D& a, const Vector2D& b);
	friend bool operator!=(const Vector2D& a, const Vector2D& b);
	friend Vector2D operator-(const Vector2D& a, const Vector2D& b);
	friend Vector2D operator+(const Vector2D& a, const Vector2D& b);
	friend Vector2D operator*(double t, const Vector2D& a);
	friend Vector2D operator*(const Vector2D& a, double t);
	friend double operator*(const Vector2D& a, const Vector2D& b);

	union
	{
		double x;
		double lon;
	};
	union
	{
		double y;
		double lat;
	};
};

double lengthOfNormal(const Vector2D& a, const Vector2D& b, Vector2D& n);

}

#endif
