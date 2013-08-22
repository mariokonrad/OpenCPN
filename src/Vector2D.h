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

#ifndef __VECTOR2D_H__
#define __VECTOR2D_H__

class Vector2D
{
	public:
		Vector2D();
		Vector2D(double a, double b);

		friend bool operator==(const Vector2D & a, const Vector2D & b);
		friend bool operator!=(const Vector2D & a, const Vector2D & b);
		friend Vector2D operator-(const Vector2D & a, const Vector2D & b);
		friend Vector2D operator+(const Vector2D & a, const Vector2D & b);
		friend Vector2D operator*(double t, const Vector2D & a);
		friend Vector2D operator*(const Vector2D & a, double t);

		union { double x; double lon; };
		union { double y; double lat; };
};

double vGetLengthOfNormal(Vector2D * a, Vector2D * b, Vector2D * n);
double vDotProduct(Vector2D * v0, Vector2D * v1);
Vector2D * vAddVectors(Vector2D * v0, Vector2D * v1, Vector2D * v);
Vector2D * vSubtractVectors(Vector2D * v0, Vector2D * v1, Vector2D * v);
double vVectorMagnitude(Vector2D * v0);
double vVectorSquared(Vector2D * v0);

#endif
