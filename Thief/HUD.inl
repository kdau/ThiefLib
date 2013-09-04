/******************************************************************************
 *  HUD.inl
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts and the Object Script Library
 *  Copyright (C) 2005-2013 Tom N Harris <telliamed@whoopdedo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#ifndef THIEF_HUD_HH
#error "This file should only be included from <Thief/HUD.hh>."
#endif

#ifndef THIEF_HUD_INL
#define THIEF_HUD_INL

namespace Thief {



// CanvasPoint

inline
CanvasPoint::CanvasPoint ()
	: x (0), y (0)
{}

inline
CanvasPoint::CanvasPoint (int _x, int _y)
	: x (_x), y (_y)
{}

inline bool
CanvasPoint::operator == (const CanvasPoint& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

inline bool
CanvasPoint::operator != (const CanvasPoint& rhs) const
{
	return x != rhs.x || y != rhs.y;
}



// CanvasRect

inline
CanvasRect::CanvasRect ()
	: CanvasPoint (), CanvasSize ()
{}

inline
CanvasRect::CanvasRect (int _x, int _y, int _w, int _h)
	: CanvasPoint (_x, _y), CanvasSize (_w, _h)
{}

inline
CanvasRect::CanvasRect (CanvasPoint position, CanvasSize size)
	: CanvasPoint (position), CanvasSize (size)
{}

inline
CanvasRect::CanvasRect (CanvasSize size)
	: CanvasPoint (), CanvasSize (size)
{}

inline bool
CanvasRect::operator == (const CanvasRect& rhs) const
{
	return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
}

inline bool
CanvasRect::operator != (const CanvasRect& rhs) const
{
	return x != rhs.x || y != rhs.y || w != rhs.w || h != rhs.h;
}

inline CanvasRect
CanvasRect::operator + (const CanvasPoint& rhs) const
{
	return CanvasRect (x + rhs.x, y + rhs.y, w, h);
}

inline CanvasRect
CanvasRect::operator - (const CanvasPoint& rhs) const
{
	return CanvasRect (x - rhs.x, y - rhs.y, w, h);
}



} // namespace Thief

#endif // THIEF_HUD_INL

