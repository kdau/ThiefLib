//! \file Parameter.hh Script configuration variables for mission authors.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_PARAMETER_HH
#define THIEF_PARAMETER_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/QuestVar.hh>

namespace Thief {

class ParameterCache;
struct EnumCoding;

//! \cond HIDDEN_SYMBOLS
template <typename T, typename Enable = void>
class Parameter;
//! \endcond



/*! Base class for all ParameterConfig specializations.
 * This structure is normally not used directly. See ParameterConfig for
 * more information. */
struct ParameterConfigBase
{
	//! Constructs a parameter configuration.
	ParameterConfigBase (bool inheritable = true);

	/*! Whether the parameter can inherit its value from ancestors
	 * of the configured object. */
	bool inheritable;
};



/*! Base class for all Parameter specializations.
 * This abstract class is the base of all Parameter specializations. See those
 * specializations for more information. */
class ParameterBase
{
public:
	//! A set of instructions for interpreting a parameter.
	typedef ParameterConfigBase Config;

	//! Destroys a parameter reference.
	~ParameterBase ();

	//! Returns whether the parameter is set on the object or an ancestor.
	bool exists () const;

	/*! Sets the parameter on this object to its value on another object.
	 * Both objects must exist, and the \a source object must have the
	 * parameter set, whether direct or inherited. \return Whether the
	 * value was successfully copied. */
	bool copy_from (const Object& source);

	/*! Removes the value of the parameter on the object.
	 * The parameter may still be effectively set on the object if it is
	 * inheritable and set on an ancestor of the object.
	 * \return Whether any value was present and successfully removed. */
	bool remove ();

	/*! Returns the uninterpreted string value of the parameter.
	 * This string will have quotes and escapes processed, but will not
	 * have been interpreted into the value type. This is the value of the
	 * parameter if it were of String type. */
	const String& get_raw () const;

	/*! Sets the parameter to the given uninterpreted string.
	 * Quotes and escapes will be added as needed. This is equivalent to
	 * setting the parameter's value if it were of String type.
	 * \return Whether the value was successfully set. */
	bool set_raw (const String&);

	/*! Inteprets the parameter's value again.
	 * The parameter system automatically rereads and reinterprets the
	 * value whenever it changes, but some value types may depend on mission
	 * context for their interpretation. This method can be called to force
	 * reinterpretation when the relevant context may have changed. */
	virtual void reparse () const;

	/*! Outputs cached parameters and technical details to the monolog.
	 * ThiefLib caches the raw values of parameters on objects that scripts
	 * are currently interested in. This method prints to the monolog a list
	 * of those objects, the state of and parameters in their DesignNotes,
	 * and the values of those parameters. */
	static void dump_cache ();

protected:
	//! \cond HIDDEN_SYMBOLS

	ParameterBase (const Object& object, const CIString& name,
		const Config& config = Config ());

	ParameterBase (const ParameterBase&) = delete;
	ParameterBase& operator = (const ParameterBase&) = delete;

	void initialize () const;

	static int decode_quest_ref (const String& raw);

	mutable ParameterCache* cache;
	mutable bool does_exist;

	const Object object;
	const CIString name;
	const Config& config;

	//! \endcond

private:
	virtual bool decode (const String&) const = 0;
	virtual void set_default () const = 0;
};



/*! A set of instructions for reading and interpreting a parameter.
 * This structure contains variables which configure the parameter system's
 * interpretation of a parameter's value. It may be specialized for a parameter
 * value type that needs more context to be interpreted properly. All
 * specializations must inherit from ParameterConfigBase and include a
 * #default_value variable of the value type. Specializations should be
 * default constructible, but are allowed not to be. */
template <typename T>
struct ParameterConfig : public ParameterConfigBase
{
	//! Constructs a parameter configuration.
	ParameterConfig (const T& default_value = T (), bool inheritable = true);

	/*! The default value of the parameter.
	 * This value is used if the parameter is not set on an object. */
	T default_value;
};



/*! A script configuration variable for mission authors (with a non-enumeration
 * value type). While the original developers were able to create new properties
 * to support the %Thief games' stock scripts, fan developers do not have that
 * option for custom scripts. Instead, a standard syntax has been developed that
 * uses the <i>Editor/Design Note</i> property to store an arbitrary number of
 * parametes to configure custom scripts. As KDScript is based on ThiefLib,
 * <a href="http://kdau.github.io/KDScript/parameters.html">its documentation
 * on parameters</a> is the best description of the syntax as implemented here.
 *
 * This class accesses one parameter on one object. The syntactic details,
 * including the new individual inheritance and difficulty-index features, are
 * handled internally. */
template <typename T>
class Parameter<T, THIEF_NOT_ENUM> : public ParameterBase
{
public:
	//! A set of instructions for interpreting a parameter of this type.
	typedef ParameterConfig<T> Config;

	/*! Constructs a parameter reference.
	 * \param object The object from which the parameter will be read.
	 * \param name The name of the parameter to read.
	 * \param config Any configuration needed to read the parameter,
	 * including its default value and whether it can be inherited. */
	Parameter (const Object& object, const CIString& name,
		const Config& config = Config ());

	/*! Returns the current value of the parameter.
	 * If the parameter is not set, returns the default value. */
	operator const T& () const;

	/*! Returns a pointer to the current value of the parameter.
	 * If the parameter is not set, returns the default value.
	 * This operator is provided to allow the const methods of a compound
	 * value type to be called directly on the internally cached value. */
	const T* operator -> () const;

	/*! Sets the parameter to the given value.
	 * The parameter will be set directly on the object, with any ancestor
	 * value unaffected. */
	Parameter& operator = (const T&);

	//! Returns whether the parameter is currently set to the given value.
	bool operator == (const T&) const;

	/*! Returns whether the parameter is not currently set to the given
	 * value. */
	bool operator != (const T&) const;

private:
	virtual bool decode (const String&) const;
	virtual void set_default () const;
	String encode () const;

	const Config config;
	mutable T value;
};

/*! Outputs the current value of the given parameter to the given stream.
 * Uses the parameter value type's stream-insertion operator. */
template <typename T>
std::ostream& operator << (std::ostream&, const Parameter<T>&);

/*! Initializes a Parameter instance.
 * \param Name The name of the member variable and of the parameter itself.
 * \param ... Additional arguments, if any, are passed to the ParameterConfig
 * constructor. */
#define THIEF_PARAMETER(Name, ...) \
Name (host (), #Name, { __VA_ARGS__ })

/*! Initializes a Parameter instance with the given name.
 * \param VarName The name of the Parameter member variable.
 * \param Name The name of the parameter itself.
 * \param ... Additional arguments, if any, are passed to the ParameterConfig
 * constructor. */
#define THIEF_PARAMETER_FULL(VarName, Name, ...) \
VarName (host (), Name, { __VA_ARGS__ })



/*! Base class for specializations of Parameter with enumeration value types.
 * This class is the base of the Parameter<T, THIEF_IS_ENUM> specialization. See
 * that specialization for more information. */
class EnumParameterBase : public ParameterBase
{
public:
	//! A set of instructions for interpreting a parameter of this type.
	typedef ParameterConfig<int> Config;

protected:
	//! \cond HIDDEN_SYMBOLS

	EnumParameterBase (const Object& object, const CIString& name,
		const EnumCoding& coding, const Config& config);

	const EnumCoding& coding;
	const Config config;
	mutable int value;

	//! \endcond

private:
	virtual bool decode (const String&) const;
	virtual void set_default () const;
};

/*! A script configuration variable for mission authors (with an enumeration
 * value type). This specialization uses the EnumCoding system to support
 * enumeration value types. See the non-enum specialization for more general
 * information. */
template <typename T>
class Parameter<T, THIEF_IS_ENUM> : public EnumParameterBase
{
public:
	//! A set of instructions for interpreting a parameter of this type.
	typedef ParameterConfig<T> Config;

	/*! Constructs a parameter reference.
	 * \param object The object from which the parameter will be read.
	 * \param name The name of the parameter to read.
	 * \param config Any configuration needed to read the parameter,
	 * including its default value and whether it can be inherited. */
	Parameter (const Object& object, const CIString& name,
		const Config& config);

	/*! Returns the current value of the parameter.
	 * If the parameter is not set, returns the default value. */
	operator T () const;

	/*! Returns the current value of the parameter as an integer.
	 * If the parameter is not set, returns the default value. */
	explicit operator int () const;

	/*! Sets the parameter to the given value.
	 * The parameter will be set directly on the object, with any ancestor
	 * value unaffected. */
	Parameter& operator = (T);

	//! Returns whether the parameter is currently set to the given value.
	bool operator == (T) const;

	/*! Returns whether the parameter is not currently set to the given
	 * value. */
	bool operator != (T) const;
};



/*! A translation between enumeration values and their string representations.
 * Use the #THIEF_ENUM_CODING macro to define this structure for an enumeration
 * type . This structure and its associated macros are used by the parameter
 * system, but may also be used in any other suitable context. */
struct EnumCoding
{
	//! A form of encoding and decoding of enumerators.
	enum class Type
	{
		VALUE, /*!< Use formatted numeric values.
		        * For input, strings must be formatted numeric values.
		        * For output, the result is a formatted numeric string. */
		CODE,  /*!< Use the listed string representation(s).
		        * For input, strings must be listed in Enumerator::codes.
		        * For output, the result is the first string
		        * representation on that list, if any, else a formatted
		        * numeric string. */
		BOTH   /*!< Input strings may be in either \a VALUE or \a CODE
		        * form. This is not a valid option for output. */
	};

	//! Which form(s) input strings may take.
	Type input_type;

	//! Which form output strings will take.
	Type output_type;

	//! An acceptable value of an enumeration.
	struct Enumerator
	{
		//! The numeric value.
		int value;

		/*! The set of string representations which correspond to this
		 * value. Comparison is case-insensitive. */
		std::vector<CIString> codes;
	};

	//! The acceptable numeric values and string representations.
	std::vector<Enumerator> enumerators;

	/*! Returns the numeric value represented by the given string.
	 * \throw std::runtime_error if the string does not match anything. */
	int decode (const String& raw) const;

	/*! Returns the primary string representation of the given value.
	 * The primary representation is the first string representation on the
	 * Enumerator::codes list, if any, for Type::CODE, otherwise the
	 * string form of the numeric value. */
	String encode (int value) const;

	/*! Returns the enumeration coding structure for the given type.
	 * This method is undefined generically and must be defined for a
	 * particular enumeration type by calling the #THIEF_ENUM_CODING macro.
	 * Many ThiefLib enumeration types that are likely to be used in script
	 * configuration have their coding predefined. */
	template <typename T>
	static const EnumCoding& get (); // define for each enum
};

/*! Defines encoding/decoding information for an enumeration type.
 * \param EnumType The name of the enumeration type.
 * \param InputType The type(s) of input string allowed; see EnumCoding::Type.
 * \param OutputType The type of string output; see EnumCoding::Type.
 * \param ... Values of the enumeration, each of which should be a call to the
 * #THIEF_ENUM_VALUE macro. If any value is listed, only the listed values will
 * be accepted when decoding; if none are listed, any numeric value will be
 * accepted (and \c InputType must be \a VALUE).
 *
 * To configure a type that is input and output numerically with no checking
 * of values, call this macro as <tt>THIEF_ENUM_CODING (???, VALUE, VALUE)</tt>. */
#define THIEF_ENUM_CODING(EnumType, InputType, OutputType, ...) \
template<> const EnumCoding& \
EnumCoding::get<EnumType> () \
{ \
	typedef EnumType E; \
	static const EnumCoding CODING \
		{ Type::InputType, Type::OutputType, { __VA_ARGS__ } }; \
	return CODING; \
}

/*! Adds an enumeration value to an encoding/decoding configuration.
 * The presence of this value in the list makes it allowable for numeric
 * decoding. String decoding requires code strings to be given.
 * \param Value The enumeration value (enumerator) to add.
 * \param ... Additional arguments, if any, should be string literals that will
 * be decoded as this value if they match the input string (case-insensitive). */
#define THIEF_ENUM_VALUE(Value, ...) \
{ int (E::Value), { __VA_ARGS__ } }



} // namespace Thief

#include <Thief/Parameter.inl>

#endif // THIEF_PARAMETER_HH

