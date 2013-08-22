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

bool operator==(const Vector2D & a, const Vector2D & b)
{
	return a.x == b.x && a.y == b.y;
}

bool operator!=(const Vector2D & a, const Vector2D & b)
{
	return a.x != b.x || a.y != b.y;
}

Vector2D operator-(const Vector2D & a, const Vector2D & b)
{
	return Vector2D(a.x - b.x, a.y - b.y);
}

Vector2D operator+(const Vector2D & a, const Vector2D & b)
{
	return Vector2D(a.x + b.x, a.y + b.y);
}

Vector2D operator*(double t, const Vector2D & a)
{
	return Vector2D(a.x * t, a.y * t);
}

Vector2D operator*(const Vector2D & a, double t)
{
	return t * a;
}



double vGetLengthOfNormal(Vector2D * a, Vector2D * b, Vector2D * n)
{
    Vector2D c, vNormal;
    vNormal.x = 0;
    vNormal.y = 0;
    //
    //Obtain projection vector.
    //
    //c = ((a * b)/(|b|^2))*b
    //
    c.x = b->x * ( vDotProduct( a, b ) / vDotProduct( b, b ) );
    c.y = b->y * ( vDotProduct( a, b ) / vDotProduct( b, b ) );
//
    //Obtain perpendicular projection : e = a - c
    //
    vSubtractVectors( a, &c, &vNormal );
    //
    //Fill PROJECTION structure with appropriate values.
    //
    *n = vNormal;

    return ( vVectorMagnitude( &vNormal ) );
}

double vDotProduct(Vector2D * v0, Vector2D * v1) // FIXME: move to Vector2D
{
    return (!v0 || !v1) ? 0.0 : ( v0->x * v1->x ) + ( v0->y * v1->y );
}

Vector2D * vAddVectors(Vector2D * v0, Vector2D * v1, Vector2D * v)
{
    if (!v0 || !v1)
		v = 0;
    else {
        v->x = v0->x + v1->x;
        v->y = v0->y + v1->y;
    }
    return v;
}

Vector2D * vSubtractVectors(Vector2D * v0, Vector2D * v1, Vector2D * v)
{
    if (!v0 || !v1)
		v = 0;
    else {
        v->x = v0->x - v1->x;
        v->y = v0->y - v1->y;
    }
    return v;
}

double vVectorSquared(Vector2D * v0)
{
    double dS;

    if (!v0)
		dS = 0.0;
    else
        dS = ( ( v0->x * v0->x ) + ( v0->y * v0->y ) );
    return dS;
}

double vVectorMagnitude(Vector2D * v0)
{
    double dMagnitude;

    if (!v0)
		dMagnitude = 0.0;
    else
        dMagnitude = sqrt( vVectorSquared( v0 ) );
    return dMagnitude;
}

