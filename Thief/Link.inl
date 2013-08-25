/******************************************************************************
 *  Link.inl
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts and the Object Script Library
 *  Copyright (C) 2005-2013 Tom N Harris <telliamed@whoopdedo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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

#ifndef THIEF_LINK_HH
#error "This file should only be included from <Thief/Link.hh>."
#endif

#ifndef THIEF_LINK_INL
#define THIEF_LINK_INL

namespace Thief {



// Flavor

inline
Flavor::Flavor (Number _number)
	: number (_number)
{}

inline bool
Flavor::operator == (const Flavor& rhs) const
{
	return number == rhs.number;
}

inline bool
Flavor::operator != (const Flavor& rhs) const
{
	return number != rhs.number;
}

inline Flavor
Flavor::get_reverse () const
{
	return -number;
}



// Link

inline
Link::Link (Number _number)
	: number (_number)
{}

inline bool
Link::operator == (const Link& rhs) const
{
	return number == rhs.number;
}

inline bool
Link::operator != (const Link& rhs) const
{
	return number != rhs.number;
}

inline Link
Link::get_reverse () const
{
	Flavor reverse_flavor = get_flavor ().get_reverse ();
	return (reverse_flavor.number << 20) | (number & ((1 << 20) - 1));
}

inline Flavor
Link::get_flavor () const
{
	return Flavor (number >> 20);
}



} // namespace Thief

#endif // THIEF_LINK_INL

