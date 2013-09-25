/******************************************************************************
 *  Object.inl
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

#ifndef THIEF_OBJECT_HH
#error "This file should only be included from <Thief/Object.hh>."
#endif

#ifndef THIEF_OBJECT_INL
#define THIEF_OBJECT_INL

namespace Thief {



#define THIEF_OBJECT_TYPE(ClassName) \
	ClassName (); \
	ClassName (const Object&); \
	ClassName (const ClassName&); \
	ClassName& operator = (const ClassName&); \
	explicit ClassName (Number); \
	explicit ClassName (const String& name);



/*! \cond HIDDEN_SYMBOLS
 * LGMulti specialization for Object types */

template <typename T>
class LGMulti<T, THIEF_IS_OBJECT> : public LGMultiBase
{
public:
	LGMulti (const T& value = Object::NONE)
		: LGMultiBase (int (value.number)) {}
	operator T () const { return T (as_object ()); }
};

//! \endcond



// Locating and wrapping objects

inline
Object::Object ()
	: number (NONE.number)
{}

inline
Object::Object (const Object& copy)
	: number (copy.number)
{}

inline Object&
Object::operator = (const Object& copy)
{
	number = copy.number;
	return *this;
}

inline
Object::Object (Number _number)
	: number (_number)
{}

inline
Object::Object (const String& name)
	: number (find (name))
{}



// Object numbers and names

inline bool
Object::operator == (const Object& rhs) const
{
	return number == rhs.number;
}

inline bool
Object::operator != (const Object& rhs) const
{
	return number != rhs.number;
}

inline bool
Object::operator < (const Object& rhs) const
{
	return number < rhs.number;
}

inline bool
Object::operator == (Number rhs) const
{
	return number == rhs;
}

inline bool
Object::operator != (Number rhs) const
{
	return number != rhs;
}



} // namespace Thief

#endif // THIEF_OBJECT_INL

