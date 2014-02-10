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

#include "SelectItem.h"

SelectItem::SelectItem(unsigned long selection_type)
	: m_pData1(0)
	, route_point(0)
	, route(0)
	, m_seltype(selection_type)
	, user_data(0)
{}

SelectItem::~SelectItem()
{}

unsigned long SelectItem::type() const
{
	return m_seltype;
}

int SelectItem::GetUserData(void) const
{
	return user_data;
}

void SelectItem::SetUserData(int data)
{
	user_data = data;
}

