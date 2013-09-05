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

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline
PropField<T, count, config>::PropField (Object& _object, size_t _index)
	: object (_object), index (_index)
{
	if (index >= count)
		throw std::out_of_range ("bad field index");
	if (!config.id [index].major)
		throw std::runtime_error ("no property specified");
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline bool
PropField<T, count, config>::exists () const
{
	return ObjectProperty (config.id [index].major, object).exists ();
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline bool
PropField<T, count, config>::instantiate ()
{
	return ObjectProperty (config.id [index].major, object).instantiate ();
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline bool
PropField<T, count, config>::remove ()
{
	return ObjectProperty (config.id [index].major, object).remove ();
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline
PropField<T, count, config>::operator T () const
{
	LGMulti<T> raw (config.default_value);
	if (exists ())
		get (object, config.id [index].major, config.id [index].minor,
			raw);
	return config.get_filter ? config.get_filter (raw) : T (raw);
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline PropField<T, count, config>&
PropField<T, count, config>::operator = (const T& value)
{
	set (object, config.id [index].major, config.id [index].minor,
		LGMulti<T> (config.set_filter ? config.set_filter (value)
			: T (value)));
	return *this;
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline std::ostream&
operator << (std::ostream& out, const PropField<T, count, config>& field)
{
	out << T (field);
	return out;
}



// PropField<T, 1u>


template <typename T, const FieldProxyConfig<T, 1u>& config>
class PropField<T, 1u, config> : public PropFieldBase
{
public:
	PropField (Object&);

	bool exists () const;
	bool instantiate ();
	bool remove ();

	operator T () const;
	PropField& operator = (const T&);

private:
	Object& object;
};

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline
PropField<T, 1u, config>::PropField (Object& _object)
	: object (_object)
{
	if (!config.id [0u].major)
		throw std::runtime_error ("no property specified");
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline bool
PropField<T, 1u, config>::exists () const
{
	return ObjectProperty (config.id [0u].major, object).exists ();
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline bool
PropField<T, 1u, config>::instantiate ()
{
	return ObjectProperty (config.id [0u].major, object).instantiate ();
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline bool
PropField<T, 1u, config>::remove ()
{
	return ObjectProperty (config.id [0u].major, object).remove ();
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline
PropField<T, 1u, config>::operator T () const
{
	LGMulti<T> raw (config.default_value);
	if (exists ())
		get (object, config.id [0u].major, config.id [0u].minor, raw);
	return config.get_filter ? config.get_filter (raw) : T (raw);
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline PropField<T, 1u, config>&
PropField<T, 1u, config>::operator = (const T& value)
{
	set (object, config.id [0u].major, config.id [0u].minor,
		LGMulti<T> (config.set_filter ? config.set_filter (value)
			: T (value)));
	return *this;
}



// PropField<bool, 1u>

template <const FieldProxyConfig<bool, 1u>& config>
class PropField<bool, 1u, config> : public PropFieldBase
{
public:
	PropField (Object&);

	bool exists () const;
	bool instantiate ();
	bool remove ();

	operator bool () const;
	PropField& operator = (bool);

private:
	Object& object;
};

template <const FieldProxyConfig<bool, 1u>& config>
inline
PropField<bool, 1u, config>::PropField (Object& _object)
	: object (_object)
{
	if (!config.id [0u].major)
		throw std::runtime_error ("no property specified");
}

template <const FieldProxyConfig<bool, 1u>& config>
inline bool
PropField<bool, 1u, config>::exists () const
{
	return ObjectProperty (config.id [0u].major, object).exists ();
}

template <const FieldProxyConfig<bool, 1u>& config>
inline bool
PropField<bool, 1u, config>::instantiate ()
{
	return ObjectProperty (config.id [0u].major, object).instantiate ();
}

template <const FieldProxyConfig<bool, 1u>& config>
inline bool
PropField<bool, 1u, config>::remove ()
{
	return ObjectProperty (config.id [0u].major, object).remove ();
}

template <const FieldProxyConfig<bool, 1u>& config>
inline
PropField<bool, 1u, config>::operator bool () const
{
	LGMulti<bool> raw (config.default_value);
	if (config.bitmask)
		raw = get_bit (config, object);
	else if (exists ())
		get (object, config.id [0u].major, config.id [0u].minor, raw);
	return config.get_filter ? config.get_filter (raw) : bool (raw);
}

template <const FieldProxyConfig<bool, 1u>& config>
inline PropField<bool, 1u, config>&
PropField<bool, 1u, config>::operator = (bool value)
{
	bool raw = config.set_filter ? config.set_filter (value) : value;

	if (config.bitmask)
		set_bit (config, object, raw);
	else
		set (object, config.id [0u].major, config.id [0u].minor,
			LGMulti<bool> (raw));

	return *this;
}



// Convenience macro for PropField members of Object subclasses

#define THIEF_PROP_FIELD(Type, Name) THIEF_FIELD_PROXY (PropField, Type, Name, )

#define THIEF_PROP_FIELD_ARRAY(Type, Name, Count) \
THIEF_FIELD_PROXY_ARRAY (PropField, Type, Name, Count, )

#define THIEF_PROP_FIELD_CONST(Type, Name) \
THIEF_FIELD_PROXY (PropField, Type, Name, const)



} // namespace Thief

#endif // THIEF_PROPERTY_INL

