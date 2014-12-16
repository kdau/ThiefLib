/******************************************************************************
 *  ActReact.inl
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2014 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef THIEF_ACTREACT_HH
#error "This file should only be included from <Thief/ActReact.hh>."
#endif

#ifndef THIEF_ACTREACT_INL
#define THIEF_ACTREACT_INL

namespace Thief {



// Reaction

inline Reaction::Reaction ()
	: number (NONE.number)
{}

inline Reaction::Reaction (Number _number)
	: number (_number)
{}

inline bool
Reaction::operator == (const Reaction& rhs) const
{
	return number == rhs.number;
}

inline bool
Reaction::operator != (const Reaction& rhs) const
{
	return number != rhs.number;
}

inline bool
Reaction::operator < (const Reaction& rhs) const
{
	return number < rhs.number;
}



} // namespace Thief

#endif // THIEF_ACTREACT_INL

