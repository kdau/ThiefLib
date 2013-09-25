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

inline bool
Flavor::operator < (const Flavor& rhs) const
{
	return number < rhs.number;
}

inline bool
Flavor::is_reverse () const
{
	return number < ANY;
}

inline Flavor
Flavor::get_reverse () const
{
	return -number;
}



// Link

inline
Link::Link ()
	: number (NONE.number)
{}

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
	return Link ((reverse_flavor.number << 20) | (number & ((1 << 20) - 1)));
}

inline Flavor
Link::get_flavor () const
{
	return Flavor (number >> 20);
}

inline void
Link::_get_data_field (const String& field, LGMultiBase& multi) const
{
	_get_data_field (field.empty () ? nullptr : field.data (), multi);
}

inline void
Link::_set_data_field (const String& field, const LGMultiBase& multi)
{
	_set_data_field (field.empty () ? nullptr : field.data (), multi);
}



// LinkField

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (LinkField)::LinkField (Link& _link, size_t _index)
	: link (_link), index (_index)
{
	if (index >= config.count)
		throw std::out_of_range ("bad field index");
}

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (LinkField)::operator Type () const
{
	LGMulti<sMultiParm> raw;
	link._get_data_field (config.items [index].major, raw);
	return config.getter (config.items [index], raw);
}

THIEF_FIELD_PROXY_TEMPLATE
inline THIEF_FIELD_PROXY_CLASS (LinkField)&
THIEF_FIELD_PROXY_CLASS (LinkField)::operator = (const Type& value)
{
	LGMulti<sMultiParm> raw;
	link._get_data_field (config.items [index].major, raw);
	config.setter (config.items [index], raw, value);
	link._set_data_field (config.items [index].major, raw);
	return *this;
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (LinkField)::operator == (const Type& rhs) const
{
	return operator Type () == rhs;
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (LinkField)::operator != (const Type& rhs) const
{
	return operator Type () != rhs;
}

THIEF_FIELD_PROXY_TEMPLATE
inline std::ostream&
operator << (std::ostream& out, const THIEF_FIELD_PROXY_CLASS (LinkField)& field)
{
	out << Type (field);
	return out;
}



// Convenience macros for flavor-specific Link subclasses

#define THIEF_FLAVORED_LINK(FlavorName) \
class FlavorName##Link; \
typedef std::vector<FlavorName##Link> FlavorName##Links; \
class FlavorName##Link : public Link

#define THIEF_FLAVORED_LINK_COMMON(FlavorName) \
private: \
	void check_valid () const; \
public: \
	static Flavor flavor (bool reverse = false); \
	FlavorName##Link (); \
	FlavorName##Link (const Link&); \
	FlavorName##Link (const FlavorName##Link&); \
	FlavorName##Link& operator = (const FlavorName##Link& copy); \
	explicit FlavorName##Link (Number); \
	static FlavorName##Links get_all \
		(const Object& source = Object::ANY, \
		const Object& dest = Object::ANY, \
		Inheritance = Inheritance::NONE, \
		bool reverse = false);

#define THIEF_LINK_FIELD(Type, Name) \
THIEF_FIELD_PROXY (LinkField, Type, , Name)

#define THIEF_LINK_FIELD_ARRAY(Type, Name, Count) \
THIEF_FIELD_PROXY_ARRAY (LinkField, Type, Count, , Name)

#define THIEF_LINK_FIELD_CONST(Type, Name) \
THIEF_FIELD_PROXY (LinkField, Type, const, Name)

#define THIEF_LINK_FIELD_ARRAY_CONST(Type, Name, Count) \
THIEF_FIELD_PROXY_ARRAY (LinkField, Type, Count, const, Name)



} // namespace Thief

#endif // THIEF_LINK_INL

