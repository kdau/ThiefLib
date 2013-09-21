/******************************************************************************
 *  Parameter.inl
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts, Object Script Library, and Dark Hook 2
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

#ifndef THIEF_PARAMETER_HH
#error "This file should only be included from <Thief/Parameter.hh>."
#endif

#ifndef THIEF_PARAMETER_INL
#define THIEF_PARAMETER_INL

namespace Thief {



// ParameterBase::Config

inline
ParameterBase::Config::Config (bool _inheritable)
	: inheritable (_inheritable)
{}



// ParameterConfig

template <typename T>
ParameterConfig<T>::ParameterConfig (const T& _default_value, bool _inheritable)
	: ParameterBase::Config (_inheritable),
	  default_value (_default_value)
{}



// Parameter (for fundamental and class types)

template <typename T>
Parameter<T, THIEF_NOT_ENUM>::Parameter (const Object& _object,
		const CIString& _name, const Config& _config)
	: ParameterBase (_object, _name, config),
	  config (_config),
	  value (config.default_value)
{}

template <typename T>
inline
Parameter<T, THIEF_NOT_ENUM>::operator const T& () const
{
	initialize ();
	return value;
}

template <typename T>
inline const T*
Parameter<T, THIEF_NOT_ENUM>::operator -> () const
{
	initialize ();
	return &value;
}

template <typename T>
inline Parameter<T, THIEF_NOT_ENUM>&
Parameter<T, THIEF_NOT_ENUM>::operator = (const T& _value)
{
	initialize ();
	value = _value;
	if (!set_raw (encode ()))
		throw std::runtime_error ("could not set parameter");
	return *this;
}

template <typename T>
inline bool
Parameter<T, THIEF_NOT_ENUM>::operator == (const T& rhs) const
{
	return operator const T& () == rhs;
}

template <typename T>
inline bool
Parameter<T, THIEF_NOT_ENUM>::operator != (const T& rhs) const
{
	return operator const T& () != rhs;
}

template <typename T>
inline void
Parameter<T, THIEF_NOT_ENUM>::set_default () const
{
	value = config.default_value;
}

template <typename T>
inline bool
Parameter<T, THIEF_NOT_ENUM>::decode (const String& raw) const
{
	value = T (raw);
	return true;
}

template <typename T>
inline String
Parameter<T, THIEF_NOT_ENUM>::encode () const
{
	return std::to_string (value);
}

template <typename T>
inline std::ostream&
operator << (std::ostream& out, const Parameter<T>& param)
{
	out << (const T&) param;
	return out;
}



// Specializations of Parameter::encode/decode for String

template<>
inline bool
Parameter<String>::decode (const String& raw) const
{
	value = raw;
	return true;
}

template<>
inline String
Parameter<String>::encode () const
{
	return value;
}



// Parameter (for enumeration types)

template <typename T>
inline
Parameter<T, THIEF_IS_ENUM>::Parameter (const Object& _object,
		const CIString& _name, const Config& _config)
	: EnumParameterBase (_object, _name, EnumCoding::get<T> (),
		{ int (_config.default_value), _config.inheritable })
{}

template <typename T>
inline
Parameter<T, THIEF_IS_ENUM>::operator T () const
{
	initialize ();
	return T (value);
}

template <typename T>
inline
Parameter<T, THIEF_IS_ENUM>::operator int () const
{
	initialize ();
	return value;
}

template <typename T>
inline Parameter<T, THIEF_IS_ENUM>&
Parameter<T, THIEF_IS_ENUM>::operator = (T _value)
{
	initialize ();
	value = int (_value);
	if (!set_raw (coding.encode (value)))
		throw std::runtime_error ("could not set parameter");
	return *this;
}

template <typename T>
inline bool
Parameter<T, THIEF_IS_ENUM>::operator == (T rhs) const
{
	return operator T () == rhs;
}

template <typename T>
inline bool
Parameter<T, THIEF_IS_ENUM>::operator != (T rhs) const
{
	return operator T () != rhs;
}



} // namespace Thief

#endif // THIEF_PARAMETER_INL

