/******************************************************************************
 *  Parameter.hh
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
#define THIEF_PARAMETER_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/QuestVar.hh>

namespace Thief {

struct IParameterCache;



// ParameterBase (common functions for all Parameter specializations)

class ParameterBase
{
public:
	struct Config
	{
		Config (bool inheritable = true);
		bool inheritable;
	};

	ParameterBase (const Object& object, const CIString& name,
		const Config& config = Config ());
	~ParameterBase ();

	ParameterBase (const ParameterBase&) = delete;
	ParameterBase& operator = (const ParameterBase&) = delete;

	bool exists () const;
	bool copy_from (const Object& source);
	bool remove ();

	const String& get_raw () const;
	bool set_raw (const String&);
	virtual void reparse () const;

protected:
	void initialize () const;

	static int decode_quest_ref (const String& raw);

	mutable IParameterCache* cache;
	mutable bool does_exist;

	const Object object;
	const CIString name;
	const Config& config;
};



// ParameterConfig: configuration for parameter encoding/decoding
//	Specialize this class to pass data from script classes to encode/decode.

template <typename T>
struct ParameterConfig : public ParameterBase::Config
{
	ParameterConfig (const T& default_value = T (), bool inheritable = true);
	T default_value;
};



// Parameter: specialization for parameters of fundamental and class types

template <typename T, typename Enable = void>
class Parameter;

template <typename T>
class Parameter<T, THIEF_NOT_ENUM> : public ParameterBase
{
public:
	typedef ParameterConfig<T> Config;

	Parameter (const Object& object, const CIString& name,
		const Config& config = Config ());

	operator const T& () const;
	const T* operator -> () const;

	Parameter& operator = (const T&);

	bool operator == (const T&) const;
	bool operator != (const T&) const;

	virtual void reparse () const;

private:
	bool decode (const String&) const;
	String encode () const;

	const Config config;
	mutable T value;
};

template <typename T>
std::ostream& operator << (std::ostream&, const Parameter<T>&);



// Specializations of Parameter::encode/decode for fundamental and class types

template<> bool Parameter<bool>::decode (const String& raw) const;

template<> bool Parameter<int>::decode (const String& raw) const;

template<> bool Parameter<float>::decode (const String& raw) const;

template<> bool Parameter<Color>::decode (const String& raw) const;

template<> bool Parameter<Object>::decode (const String& raw) const;
template<> String Parameter<Object>::encode () const;

template<> bool Parameter<Objective>::decode (const String& raw) const;
template<> String Parameter<Objective>::encode () const;

template<> bool Parameter<String>::decode (const String& raw) const;
template<> String Parameter<String>::encode () const;

template<> bool Parameter<Time>::decode (const String& raw) const;

template<> bool Parameter<Vector>::decode (const String& raw) const;



// Parameter: specialization for parameters of enumeration types

struct EnumCoding;

class EnumParameterBase : public ParameterBase
{
public:
	typedef ParameterConfig<int> Config;

	virtual void reparse () const;

protected:
	EnumParameterBase (const Object& object, const CIString& name,
		const EnumCoding& coding, const Config& config);

	const EnumCoding& coding;
	const Config config;
	mutable int value;
};

template <typename T>
class Parameter<T, THIEF_IS_ENUM> : public EnumParameterBase
{
public:
	typedef ParameterConfig<T> Config;

	Parameter (const Object& object, const CIString& name,
		const Config& config);

	operator T () const;
	Parameter& operator = (T);
};



// EnumCoding: encoding/decoding information for enumerations

struct EnumCoding
{
	enum class Input { VALUE, CODE, BOTH } input_type;
	enum class Output { VALUE, CODE } output_type;

	struct Enumerator
	{
		int value;
		std::vector<CIString> codes;
	};
	std::vector<Enumerator> enumerators;

	int decode (const String& raw) const;
	String encode (int value) const;

	template <typename T>
	static const EnumCoding& get (); // define for each enum
};

// Use these macros with a new enum type that will have full encoding/decoding.

#define THIEF_ENUM_CODING(Type, InputType, OutputType, ...) \
template<> const EnumCoding& \
EnumCoding::get<Type> () \
{ \
	typedef Type E; \
	static const EnumCoding CODING \
		{ Input::InputType, Output::OutputType, { __VA_ARGS__ } }; \
	return CODING; \
}

#define THIEF_ENUM_VALUE(Value, ...) { int (E::Value), { __VA_ARGS__ } }

// Use this macro with a new enum type that does not need name encoding/decoding.
// No checking of integer values will be performed by EnumParameter, either.

#define THIEF_ENUM_SIMPLE(Type) THIEF_ENUM_CODING (Type, VALUE, VALUE)



// Initializer macros for script classes

#define PARAMETER_(VarName, Name, ...) VarName (host (), Name, { __VA_ARGS__ })
#define PARAMETER(Name, ...) PARAMETER_ (Name, #Name, __VA_ARGS__)



} // namespace Thief

#include <Thief/Parameter.inl>

#endif // THIEF_PARAMETER_HH

