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

#include "Vector2D.h"
#include <cmath>

Vector2D::Vector2D()
{
	x = 0.0;
	y = 0.0;
}

Vector2D::Vector2D(double a, double b)
{
	x = a;
	y = b;
}

double Vector2D::dot(const Vector2D& other) const
{
	return (x * other.x) + (y * other.y);
}

bool operator==(const Vector2D& a, const Vector2D& b)
{
	return a.x == b.x && a.y == b.y;
}

bool operator!=(const Vector2D& a, const Vector2D& b)
{
	return !(a == b);
}

Vector2D operator-(const Vector2D& a, const Vector2D& b)
{
	return Vector2D(a.x - b.x, a.y - b.y);
}

Vector2D operator+(const Vector2D& a, const Vector2D& b)
{
	return Vector2D(a.x + b.x, a.y + b.y);
}

Vector2D operator*(double t, const Vector2D& a)
{
	return Vector2D(a.x * t, a.y * t);
}

Vector2D operator*(const Vector2D& a, double t)
{
	return t * a;
}

double operator*(const Vector2D& a, const Vector2D& b)
{
	return a.dot(b);
}

double Vector2D::sqr() const
{
	return (x * x) + (y * y);
}

double Vector2D::length() const
{
	return ::sqrt(sqr());
}

double lengthOfNormal(const Vector2D& a, const Vector2D& b, Vector2D& n)
{
	// projection vector: c = b * ((a * b) / ||b||^2)
	const double d = (a * b) / b.sqr();
	Vector2D c = d * b;

	// obtain perpendicular projection: n = a - c
	n = a - c;

	return n.length();
}

