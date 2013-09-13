/******************************************************************************
 *  Parameter.cc
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

#include "Private.hh"
#include "ParameterCache.hh"
#include "OSL.hh"

namespace Thief {



// ParameterBase

ParameterBase::ParameterBase (const Object& _object, const CIString& _name,
		const Config& _config)
	: cache (nullptr),
	  does_exist (false),
	  object (_object),
	  name (_name),
	  config (_config)
{}

ParameterBase::~ParameterBase ()
{
	if (cache)
		cache->unwatch_object (object, *this);
}

bool
ParameterBase::exists () const
{
	initialize ();
	return does_exist;
}

bool
ParameterBase::copy_from (const Object& source)
{
	initialize ();
	return cache->copy (source, object, name);
}

bool
ParameterBase::remove ()
{
	initialize ();
	return cache->remove (object, name);
}

const String&
ParameterBase::get_raw () const
{
	initialize ();
	const String* result = cache->get (object, name, config.inheritable);
	if (result)
		return *result;
	else
		throw std::runtime_error ("parameter not set for object");
}

bool
ParameterBase::set_raw (const String& raw)
{
	initialize ();
	return cache->set (object, name, raw);
}

void
ParameterBase::reparse () const
{
	initialize ();
	does_exist = cache->exists (object, name, config.inheritable);
}

void
ParameterBase::initialize () const
{
	if (!cache)
	{
		cache = SService<IOSLService> (LG)->get_param_cache ();
		if (!cache)
			throw std::runtime_error
				("could not access parameter cache");
		cache->watch_object (object, *this);
	}
}

int
ParameterBase::decode_quest_ref (const String& raw)
{
	if (raw.empty () || raw.front () != '$') return INT_MAX;

	QuestVar quest_var (raw.substr (1).data ());
	return quest_var.exists () ? quest_var.get () : INT_MAX;
}



// Specializations of Parameter::encode/decode for fundamental and class types

template<>
bool
Parameter<bool>::decode (const String& raw) const
{
	if (raw.empty ()) return false;

	int _value = decode_quest_ref (raw);
	if (_value != INT_MAX) { value = _value; return true; }

	switch (raw.front ())
	{
	case 't': case 'T': case 'y': case 'Y':
		value = true;
		break;
	default:
		// Kept like this to conform to ScriptLib's DN syntax.
		value = strtol (raw.data (), nullptr, 0);
		break;
	}

	return true;
}

template<>
bool
Parameter<int>::decode (const String& raw) const
{
	value = decode_quest_ref (raw);
	if (value != INT_MAX) return true;

	char* end = nullptr;
	value = strtol (raw.data (), &end, 0);
	return end != raw.data ();
}

template<>
bool
Parameter<float>::decode (const String& raw) const
{
	int _value = decode_quest_ref (raw);
	if (_value != INT_MAX) { value = _value; return true; }

	char* end = nullptr;
	value = strtof (raw.data (), &end);
	return end != raw.data ();
}

template<>
bool
Parameter<Color>::decode (const String& raw) const
{
	try
	{
		value = raw;
		return true;
	}
	catch (...) { return false; }
}

template<>
bool
Parameter<Object>::decode (const String& raw) const
{
	value = Object (raw);
	return value != Object::NONE;
}

template<>
String
Parameter<Object>::encode () const
{
	String obj_name = value.get_name ();
	if (!obj_name.empty ()) return obj_name;
	std::ostringstream raw;
	raw << value.number;
	return raw.str ();
}

template<>
bool
Parameter<Objective>::decode (const String& raw) const
{
	int _value = decode_quest_ref (raw);
	if (_value != INT_MAX && _value >= 0)
		{ value.number = _value; return true; }

	char* end = nullptr;
	value.number = strtoul (raw.data (), &end, 10);
	return end != raw.data ();
}

template<>
String
Parameter<Objective>::encode () const
{
	std::ostringstream raw;
	raw << value.number;
	return raw.str ();
}

template<>
bool
Parameter<Time>::decode (const String& raw) const
{
	int _value = decode_quest_ref (raw);
	if (_value != INT_MAX) { value = _value; return true; }

	try
	{
		value = raw;
		return true;
	}
	catch (...) { return false; }
}

template<>
bool
Parameter<Vector>::decode (const String& raw) const
{
	try
	{
		value = raw;
		return true;
	}
	catch (...) { return false; }
}



// EnumParameterBase

EnumParameterBase::EnumParameterBase (const Object& _object,
		const CIString& _name, const EnumCoding& _coding,
		const Config& _config)
	: ParameterBase (_object, _name, config),
	  coding (_coding), config (_config),
	  value (config.default_value)
{}

void
EnumParameterBase::reparse () const
{
	ParameterBase::reparse ();

	if (does_exist) try
	{
		const String& raw = get_raw ();

		if (coding.input_type != EnumCoding::Input::CODE) // values OK
		{
			value = decode_quest_ref (raw);
			if (value != INT_MAX) return;
		}

		if (!raw.empty ())
		{
			value = coding.decode (raw);
			return;
		}
	}
	catch (std::exception& e)
	{
		mono.log (boost::format ("WARNING: Could not parse parameter "
			"\"%||\" on %||: %||.") % name % object % e.what ());
	}
	catch (...)
	{}

	value = config.default_value;
}



// EnumCoding

int
EnumCoding::decode (const String& raw) const
{
	// Try decoding as a code string.
	if (input_type != Input::VALUE)
		for (auto& enumerator : enumerators)
			for (auto& code : enumerator.codes)
				if (code == raw)
					return enumerator.value;

	// Try decoding as a value integer.
	if (input_type != Input::CODE)
	{
		char* end = nullptr;
		int candidate = strtol (raw.data (), &end, 0);
		if (end != raw.data ())
		{
			if (enumerators.empty ())
				return candidate;
			for (auto& enumerator : enumerators)
				if (enumerator.value == candidate)
					return candidate;
		}
	}

	throw std::runtime_error ("invalid enumerator");
}

String
EnumCoding::encode (int value) const
{
	if (output_type == Output::CODE)
		for (auto& enumerator : enumerators)
			if (enumerator.value == value)
			{
				if (enumerator.codes.empty ())
					break;
				else
					return enumerator.codes.front ().data ();
			}

	std::ostringstream raw;
	raw << value;
	return raw.str ();
}



} // namespace Thief

