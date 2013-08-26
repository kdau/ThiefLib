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

#ifndef THIEF_PARAMETER_HH
#error "This file should only be included from <Thief/Parameter.hh>."
#endif

#ifndef THIEF_PARAMETER_INL
#define THIEF_PARAMETER_INL

namespace Thief {



// ParameterBase

inline
ParameterBase::Config::Config (bool _inheritable)
	: inheritable (_inheritable)
{}

inline
ParameterBase::ParameterBase (const Object& _object, const CIString& _name,
		const Config& _config)
	: cache (),
	  does_exist (false),
	  object (_object),
	  name (_name),
	  config (_config)
{}

inline bool
ParameterBase::exists () const
{
	initialize ();
	return does_exist;
}



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
	set_raw (encode ());
	return *this;
}

template <typename T>
inline bool
Parameter<T, THIEF_NOT_ENUM>::operator == (const T& rhs) const
{
	initialize ();
	return value == rhs;
}

template <typename T>
inline bool
Parameter<T, THIEF_NOT_ENUM>::operator != (const T& rhs) const
{
	initialize ();
	return value != rhs;
}

template <typename T>
void
Parameter<T, THIEF_NOT_ENUM>::reparse () const
{
	ParameterBase::reparse ();

	try
	{
		if (does_exist && decode (get_raw ()))
			return;
	}
	catch (std::exception& e)
	{
		mono << "Could not parse parameter \"" << name << "\" on "
			<< object.get_editor_name () << ": " << e.what ()
			<< std::endl;
	}
	catch (...)
	{}

	value = config.default_value;
}

template <typename T>
inline String
Parameter<T, THIEF_NOT_ENUM>::encode () const
{
	std::ostringstream raw;
	raw << std::boolalpha << value;
	return raw.str ();
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
Parameter<T, THIEF_IS_ENUM>::operator T () const
{
	initialize ();
	return T (value);
}

template <typename T>
inline Parameter<T, THIEF_IS_ENUM>&
Parameter<T, THIEF_IS_ENUM>::operator = (T _value)
{
	initialize ();
	value = int (_value);
	set_raw (coding.encode (value));
	return *this;
}



} // namespace Thief

#endif // THIEF_PARAMETER_INL
