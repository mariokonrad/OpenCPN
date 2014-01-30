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

#include "EmbossData.h"

EmbossData::EmbossData()
	: map_width(0)
	, map_height(0)
	, pmap(NULL)
	, gltexind(0)
{}

EmbossData::EmbossData(int width, int height)
	: map_width(width)
	, map_height(height)
	, pmap(NULL)
	, gltexind(0)
{
	pmap = new int[map_width * map_height];
}

EmbossData::~EmbossData()
{
	if (pmap) {
		delete [] pmap;
		pmap = NULL;
	}
}

int EmbossData::size() const
{
	return map_width * map_height;
}

int EmbossData::width() const
{
	return map_width;
}

int EmbossData::height() const
{
	return map_height;
}

GLuint EmbossData::gltex_index() const
{
	return gltexind;
}

void EmbossData::set_gltex_index(GLuint tex_index)
{
	gltexind = tex_index;
}

int EmbossData::at(int index) const
{
	return pmap[index];
}

int& EmbossData::at(int index)
{
	return pmap[index];
}

