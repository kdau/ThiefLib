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

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline
LinkField<T, count, config>::LinkField (Link& _link, size_t _index)
	: link (_link), index (_index)
{
	if (index >= count)
		throw std::out_of_range ("bad field index");
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline
LinkField<T, count, config>::operator T () const
{
	LGMulti<T> raw (config.default_value);
	link._get_data_field (config.id [index].major, raw);
	return config.get_filter ? config.get_filter (raw) : T (raw);
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline LinkField<T, count, config>&
LinkField<T, count, config>::operator = (const T& value)
{
	LGMulti<T> raw (config.set_filter ? config.set_filter (value) : value);
	link._set_data_field (config.id [index].major, raw);
	return *this;
}

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
inline std::ostream&
operator << (std::ostream& out, const LinkField<T, count, config>& field)
{
	out << T (field);
	return out;
}



// LinkField<T, 1u>

template <typename T, const FieldProxyConfig<T, 1u>& config>
class LinkField<T, 1u, config> : public LinkFieldBase
{
public:
	LinkField (Link&);

	operator T () const;
	LinkField& operator = (const T&);

private:
	Link& link;
};

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline
LinkField<T, 1u, config>::LinkField (Link& _link)
	: link (_link)
{}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline
LinkField<T, 1u, config>::operator T () const
{
	LGMulti<T> raw (config.default_value);
	link._get_data_field (config.id [0u].major, raw);
	return config.get_filter ? config.get_filter (raw) : T (raw);
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline LinkField<T, 1u, config>&
LinkField<T, 1u, config>::operator = (const T& value)
{
	LGMulti<T> raw (config.set_filter ? config.set_filter (value) : value);
	link._set_data_field (config.id [0u].major, raw);
	return *this;
}

template <typename T, const FieldProxyConfig<T, 1u>& config>
inline std::ostream&
operator << (std::ostream& out, const LinkField<T, 1u, config>& field)
{
	out << T (field);
	return out;
}



// LinkField<bool, 1u>

template <const FieldProxyConfig<bool, 1u>& config>
class LinkField<bool, 1u, config> : public LinkFieldBase
{
public:
	LinkField (Link&);

	operator bool () const;
	LinkField& operator = (bool);

private:
	Link& link;
};

template <const FieldProxyConfig<bool, 1u>& config>
inline
LinkField<bool, 1u, config>::LinkField (Link& _link)
	: link (_link)
{}

template <const FieldProxyConfig<bool, 1u>& config>
inline
LinkField<bool, 1u, config>::operator bool () const
{
	LGMulti<bool> raw (config.default_value);
	if (config.bitmask)
		raw = get_bit (config, link);
	else
		link._get_data_field (config.id [0u].major, raw);
	return config.get_filter ? config.get_filter (raw) : bool (raw);
}

template <const FieldProxyConfig<bool, 1u>& config>
inline LinkField<bool, 1u, config>&
LinkField<bool, 1u, config>::operator = (bool value)
{
	LGMulti<bool> raw (config.set_filter ? config.set_filter (value) : value);
	if (config.bitmask)
		set_bit (config, link, raw);
	else
		link._set_data_field (config.id [0u].major, raw);
	return *this;
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
	FlavorName##Link (Number = NONE); \
	FlavorName##Link (const Link&); \
	FlavorName##Link (const FlavorName##Link&); \
	FlavorName##Link& operator = (const FlavorName##Link& copy); \
	static FlavorName##Links get_all \
		(const Object& source = Object::ANY, \
		const Object& dest = Object::ANY, \
		Inheritance = Inheritance::NONE, \
		bool reverse = false);

#define THIEF_LINK_FIELD(Type, Name) THIEF_FIELD_PROXY (LinkField, Type, Name, )

#define THIEF_LINK_FIELD_ARRAY(Type, Name, Count) \
THIEF_FIELD_PROXY_ARRAY (LinkField, Type, Name, Count, )

#define THIEF_LINK_FIELD_CONST(Type, Name) \
THIEF_FIELD_PROXY (LinkField, Type, Name, const)



} // namespace Thief

#endif // THIEF_LINK_INL

