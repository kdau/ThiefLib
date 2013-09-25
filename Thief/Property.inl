/******************************************************************************
 *  Property.inl
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

#ifndef THIEF_PROPERTY_HH
#error "This file should only be included from <Thief/Property.hh>."
#endif

#ifndef THIEF_PROPERTY_INL
#define THIEF_PROPERTY_INL

namespace Thief {



// Property

inline bool
Property::operator == (const Property& rhs) const
{
	return get_number () == rhs.get_number ();
}

inline bool
Property::operator != (const Property& rhs) const
{
	return get_number () != rhs.get_number ();
}

inline bool
Property::operator < (const Property& rhs) const
{
	return get_number () < rhs.get_number ();
}



// ObjectProperty

inline bool
ObjectProperty::operator < (const ObjectProperty& rhs) const
{
	return property < rhs.property ||
		(property == rhs.property && object < rhs.object);
}

template <typename T>
inline T
ObjectProperty::get (const T& default_value) const
{
	if (exists ())
	{
		LGMulti<T> value;
		_get (value);
		return value;
	}
	else
		return default_value;
}

template <typename T>
inline T
ObjectProperty::get_field (const String& field, const T& default_value) const
{
	if (exists ())
	{
		LGMulti<T> value;
		_get_field (field.empty () ? nullptr : field.data (), value);
		return value;
	}
	else
		return default_value;
}

inline void
ObjectProperty::_get_field (const String& field, LGMultiBase& value) const
{
	_get_field (field.empty () ? nullptr : field.data (), value);
}

inline bool
ObjectProperty::_set_field (const String& field, const LGMultiBase& value,
	bool instantiate_if_missing)
{
	return _set_field (field.empty () ? nullptr : field.data (), value,
		instantiate_if_missing);
}



// PropField

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (PropField)::PropField (Object& _object, size_t _index)
	: object (_object), index (_index)
{
	if (index >= config.count)
		throw std::out_of_range ("bad field index");
	if (!config.items [index].major)
		throw std::runtime_error ("no property specified");
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (PropField)::exists () const
{
	return ObjectProperty (config.items [index].major, object).exists ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (PropField)::instantiate ()
{
	return ObjectProperty (config.items [index].major, object).instantiate ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (PropField)::remove ()
{
	return ObjectProperty (config.items [index].major, object).remove ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (PropField)::operator Type () const
{
	LGMulti<sMultiParm> raw;
	if (exists ())
		get (object, config.items [index].major,
			config.items [index].minor, raw);
	return config.getter (config.items [index], raw);
}

THIEF_FIELD_PROXY_TEMPLATE
inline THIEF_FIELD_PROXY_CLASS (PropField)&
THIEF_FIELD_PROXY_CLASS (PropField)::operator = (const Type& value)
{
	LGMulti<sMultiParm> raw;
	if (exists ())
		get (object, config.items [index].major,
			config.items [index].minor, raw);
	config.setter (config.items [index], raw, value);
	set (object, config.items [index].major, config.items [index].minor,
		raw);
	return *this;
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (PropField)::operator == (const Type& rhs) const
{
	return operator Type () == rhs;
}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (PropField)::operator != (const Type& rhs) const
{
	return operator Type () != rhs;
}

THIEF_FIELD_PROXY_TEMPLATE
inline std::ostream&
operator << (std::ostream& out, const THIEF_FIELD_PROXY_CLASS (PropField)& field)
{
	out << Type (field);
	return out;
}



// Convenience macros for PropField members of Object subclasses

#define THIEF_PROP_FIELD(Type, Name) \
THIEF_FIELD_PROXY (PropField, Type, , Name)

#define THIEF_PROP_FIELD_ARRAY(Type, Name, Count) \
THIEF_FIELD_PROXY_ARRAY (PropField, Type, Count, , Name)

#define THIEF_PROP_FIELD_CONST(Type, Name) \
THIEF_FIELD_PROXY (PropField, Type, const, Name)

#define THIEF_PROP_FIELD_ARRAY_CONST(Type, Name, Count) \
THIEF_FIELD_PROXY_ARRAY (PropField, Type, Count, const, Name)



} // namespace Thief

#endif // THIEF_PROPERTY_INL

