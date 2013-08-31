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



// ObjectProperty

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
	bool add_if_missing)
{
	return _set_field (field.empty () ? nullptr : field.data (), value,
		add_if_missing);
}



// PropField

template <typename T, const FieldProxyConfig<T>& config>
inline
PropField<T, config>::PropField (Object& _object)
	: object (_object)
{
	if (!config.major)
		throw std::runtime_error ("no property specified");
}

template <typename T, const FieldProxyConfig<T>& config>
inline bool
PropField<T, config>::exists () const
{
	return ObjectProperty (config.major, object).exists ();
}

template <typename T, const FieldProxyConfig<T>& config>
inline
PropField<T, config>::operator T () const
{
	LGMulti<T> raw (config.default_value);
	if (exists ()) get (object, config.major, config.minor, raw);
	return config.get_filter ? config.get_filter (raw) : T (raw);
}

template <typename T, const FieldProxyConfig<T>& config>
inline PropField<T, config>&
PropField<T, config>::operator = (const T& value)
{
	set (object, config.major, config.minor, LGMulti<T>
		(config.set_filter ? config.set_filter (value) : T (value)));
	return *this;
}

template <typename T, const FieldProxyConfig<T>& config>
inline std::ostream&
operator << (std::ostream& out, const PropField<T, config>& field)
{
	out << T (field);
	return out;
}



// PropField<bool>

template <const FieldProxyConfig<bool>& config>
inline
PropField<bool, config>::PropField (Object& _object)
	: object (_object)
{
	if (!config.major)
		throw std::runtime_error ("no property specified");
}

template <const FieldProxyConfig<bool>& config>
inline bool
PropField<bool, config>::exists () const
{
	return ObjectProperty (config.major, object).exists ();
}

template <const FieldProxyConfig<bool>& config>
inline
PropField<bool, config>::operator bool () const
{
	LGMulti<bool> raw (config.default_value);
	if (config.bitmask)
		raw = get_bit (config, object);
	else if (exists ())
		get (object, config.major, config.minor, raw);
	return config.get_filter ? config.get_filter (raw) : bool (raw);
}

template <const FieldProxyConfig<bool>& config>
inline PropField<bool, config>&
PropField<bool, config>::operator = (bool value)
{
	bool raw = config.set_filter ? config.set_filter (value) : value;

	if (config.bitmask)
		set_bit (config, object, raw);
	else
		set (object, config.major, config.minor, LGMulti<bool> (raw));

	return *this;
}



// Convenience macro for PropField members of Object subclasses

#define THIEF_PROP_FIELD(Type, Name) THIEF_FIELD_PROXY (PropField, Type, Name, )

#define THIEF_PROP_FIELD_CONST(Type, Name) \
THIEF_FIELD_PROXY (PropField, Type, Name, const)



} // namespace Thief

#endif // THIEF_PROPERTY_INL

