/**
 * This file is part of SDLTerminal.
 * Copyright (C) 2011 Vincent Ho <www.whimsicalvee.com>
 *
 * SDLTerminal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDLTerminal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with SDLTerminal.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "point.hpp"

Point::Point()
{
	setLocation(0, 0);
}

Point::Point(int nX, int nY)
{
	setLocation(nX, nY);
}

void Point::setX(int nX)
{
	m_nX = nX;
}

void Point::setY(int nY)
{
	m_nY = nY;
}

int Point::getX() const
{
	return m_nX;
}

int Point::getY() const
{
	return m_nY;
}


void Point::setLocation(int nX, int nY)
{
	m_nX = nX;
	m_nY = nY;
}
