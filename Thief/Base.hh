/*! \file Base.hh Basic includes, macros, structures, and functions used
 * throughout ThiefLib.
 */
/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_BASE_HH
#define THIEF_BASE_HH



// Dark Engine version checks

#if (_DARKGAME == 1)
#define IS_THIEF1
#elif (_DARKGAME == 2)
#define IS_THIEF2
#elif defined (_DARKGAME)
#error "ThiefLib does not support this game."
#elif defined (IS_DOXYGEN)
/*! Defined if the current module is for %Thief: The Dark Project or %Thief Gold.
 * A module supporting both games can use this define to enable T1-specific code. */
#define IS_THIEF1
/*! Defined if the current module is for %Thief II: The Metal Age.
 * A module supporting both games can use this define to enable T2-specific code. */
#define IS_THIEF2
#elif !defined (IS_OSL)
#error "_DARKGAME must be defined."
#endif

//! \cond HIDDEN_SYMBOLS

#ifndef _NEWDARK
#error "ThiefLib does not support pre-NewDark versions of the Dark Engine."
#endif



// Configuration for Windows headers

#define WINVER 0x0400
#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN



// This odd incantation works around an overzealous workaround in the GNU C++
// library. The first macro undefined below is only supposed to disable the
// wchar and wstring overloads of sto*, getline and to_wstring, but it also
// disables the char and string overloads, which are unaffected by the bug in
// question. wchar and wstring shouldn't be necessary in a Dark Engine OSM.

#include <limits>
#ifdef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
#undef _GLIBCXX_HAVE_BROKEN_VSWPRINTF
#endif
#ifdef _GLIBCXX_USE_WCHAR_T
#undef _GLIBCXX_USE_WCHAR_T
#endif
#pragma GCC poison wchar wstring



// C++ standard library and Boost headers used in ThiefLib

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/format.hpp>

//! \endcond



namespace Thief {



// Forward declarations of ThiefLib types

class Object;



//! A string type used in case-sensitive comparison contexts.
typedef std::string String;

class CITraits;

//! A string type used in case-insensitive comparison contexts.
typedef std::basic_string<char, CITraits> CIString;

//! \cond HIDDEN_SYMBOLS
bool operator == (const CIString&, const String&);
std::ostream& operator << (std::ostream&, const CIString&);
//! \endcond



//! A color in the sRGB space, as used by the engine.
struct RGBColor
{
	//! A component dimension of the sRGB color space.
	typedef unsigned char Component;

	/*! A composite sRGB color value.
	 * This is in the Win32 COLORREF format. */
	typedef unsigned long Value;

	//! The red value of the color.
	Component red;

	//! The green value of the color.
	Component green;

	//! The blue value of the color.
	Component blue;

	//! Constructs an sRGB color that is pure black.
	RGBColor ();

	//! Constructs an sRGB color with the given component values.
	RGBColor (Component red, Component green, Component blue);

	//! Constructs an sRGB color with the given composite value.
	explicit RGBColor (Value);

	//! Sets the color color to the given composite value.
	RGBColor& operator = (Value);

	//! Returns the composite value of the color.
	operator Value () const;

	/*! Constructs an sRGB color value based on the given string.
	 * See the String assignment operator. */
	explicit RGBColor (const String&);

	/*! Sets the color value based on the given string.
	 * The string may be an HTML-style hexadecimal representation, either
	 * short or long, such as \c "#00FF00" or \c "#0F0". It may also be
	 * a comma-separated RGB value triplet, such as \c "0,255,0". It may
	 * also be one of 30 named HTML colors, such as \c "green". */
	RGBColor& operator = (const String&);

	/*! Returns a string representation of the color value.
	 * This is an HTML-style hexadecimal color, such as \c "#00FF00". */
	explicit operator String () const;
};

/*! Outputs the given sRGB color to the given stream.
 * This is the output of the color's String conversion. \relates RGBColor */
std::ostream& operator << (std::ostream&, const RGBColor&);

//! In most contexts, an sRGB value is expected for a color.
typedef RGBColor Color;



/*! A color in the CIE L*a*b* space, ideal for calculations.
 * The CIE L*a*b* color space distributes colors based on human color perception,
 * so averaging and lightness adjustment operations are best performed in this
 * space. */
struct LabColor
{
	//! A component dimension of the L*a*b* color space.
	typedef double Component;

	//! The lightness (L*) of the color.
	Component L;

	//! The hue/chroma of the color along the green-red (a*) axis.
	Component a;

	//! The hue/chroma of the color along the blue-yellow (b*) axis.
	Component b;

	//! Constructs an L*a*b* color that is pure black.
	LabColor ();

	//! Constructs an L*a*b* color with the given components.
	LabColor (Component L, Component a, Component b);

	//! Constructs an L*a*b* color corresponding to the given sRGB color.
	LabColor (const RGBColor&);

	/*! Returns an sRGB color corresponding to the L*a*b* color.
	 * The sRGB color space has a more limited gamut, so not all L*a*b*
	 * colors can be accurately converted this way. */
	operator RGBColor () const;
};



/*! A span of time or position in time on the sim clock.
 * The sim clock is the in-game clock which measures time passed in game mode
 * since the mission was started. It is not equivalent to wall clock time. It
 * is affected by pauses in game mode (interface screens and load/save),
 * explicit changes in clock rate (such as the \c scale_sim_time command in
 * DromEd), and to a small extent by hardware and performance constraints.  */
struct Time
{
	//! A time value in milliseconds.
	typedef unsigned long Value;

	//! The base unit of milliseconds.
	static constexpr Value MILLISECONDS = 1ul;

	//! The unit of seconds, as a multiple of the base millisecond unit.
	static constexpr Value SECONDS = 1000ul * MILLISECONDS;

	//! The unit of minutes, as a multiple of the base millisecond unit.
	static constexpr Value MINUTES = 60ul * SECONDS;

	//! The time value in milliseconds.
	Value value;

	//! Constructs a time value with the given number of the given unit.
	Time (Value value = 0ul, Value unit = MILLISECONDS);

	//! Sets the time value to the given value.
	Time& operator = (Value);

	//! Returns the time value in milliseconds.
	operator Value () const { return value; }

	/*! Returns the time value in milliseconds.
	 * Some contexts expect a signed long value. */
	explicit operator long () const { return value; }

	//! Returns the number of seconds represented by the value, rounded.
	Value seconds () const;

	//! Returns the number of minutes represented by the value, rounded.
	Value minutes () const;

	//! Constructs a time value with the given number of the given unit.
	Time (float value, Value unit);

	//! Returns the time value in milliseconds as a \c float.
	explicit operator float () const { return value; }

	//! Returns the number of seconds represented by the value as a \c float.
	float fseconds () const;

	//! Returns the number of minutes represented by the value as a \c float.
	float fminutes () const;

	//! Returns the sum of this time value and another.
	Time operator + (const Time&) const;

	//! Returns the difference of this time value and another.
	Time operator - (const Time&) const;

	//! Adds the given time value to this one.
	Time& operator += (const Time&);

	//! Subtracts the given time value from this one.
	Time& operator -= (const Time&);

	/*! Constructs a time value based on the given string.
	 * See the String assignment operator. */
	explicit Time (const String&);

	/*! Sets the time value based on the given string.
	 * The string may be an undecorated number of milliseconds, a number of
	 * seconds followed by 's', or a number of minutes followed by 'm'. The
	 * numeric value, which may or may not have a decimal portion, must be
	 * at the beginning of the string. */
	Time& operator = (const String&);

	/*! Returns a string representation of the time value.
	 * This is the undecorated number of milliseconds. */
	explicit operator String () const;
};

/*! Outputs the given time value to the given stream.
 * This is the output of the time value's String conversion. \relates Time */
std::ostream& operator << (std::ostream&, const Time&);



/*! A location, rotation, size, or velocity in the game world.
 * Vectors are three-dimensional coordinates in the game world. In most cases,
 * the components represent locations or lengths in DromEd units (DU), where
 * each DU is about 10 inches. For rotational vectors, the components are
 * usually angles in degrees. */
struct Vector
{
	/*! The X-axis component of the vector.
	 * For rotational vectors, this is the heading (H) component. */
	float x;

	/*! The Y-axis component of the vector.
	 * For rotational vectors, this is the pitch (P) component. */
	float y;

	/*! The Z-axis component of the vector.
	 * For rotational vectors, this is the bank (B) component. */
	float z;

	//! Constructs a vector at (0, 0, 0).
	Vector ();

	//! Constructs a vector with the given coordinates.
	Vector (float x, float y, float z);

	//! Returns whether this vector is equal to another.
	bool operator == (const Vector&) const;

	//! Returns whether this vector is not equal to another.
	bool operator != (const Vector&) const;

	//! Returns the sum of this vector and another.
	Vector operator + (const Vector&) const;

	//! Returns the difference of this vector and another.
	Vector operator - (const Vector&) const;

	//! Returns the product of this vector and another.
	Vector operator * (const Vector&) const;

	//! Returns this vector divided by another.
	Vector operator / (const Vector&) const;

	//! Returns the product of this vector and the given scalar.
	Vector operator * (double) const;

	//! Returns this vector divided by the given scalar.
	Vector operator / (double) const;

	//! Adds the given vector to this one.
	Vector& operator += (const Vector&);

	//! Subtracts the given vector from this one.
	Vector& operator -= (const Vector&);

	//! Multiplies this vector by the given vector.
	Vector& operator *= (const Vector&);

	//! Divides this vector by the given vector.
	Vector& operator /= (const Vector&);

	//! Multiplies this vector by the given scalar.
	Vector& operator *= (double);

	//! Divides this vector by the given scalar.
	Vector& operator /= (double);

	/*! Returns the normalized, or unit, vector for this vector.
	 * The normal is a unit-length (1 DU) with the same direction. */
	Vector normal () const;

	//! Sets this vector to its normalized (unit) form.
	Vector& normalize ();

	/*! Returns the magnitude of the vector.
	 * The magnitude is the length along the vector between its ends. */
	double magnitude () const;

	/*! Returns the square of the magnitude of the vector.
	 * This value is computationally simpler than the magnitude itself. */
	double mag_squared () const;

	//! Returns the dot product of this vector and another.
	double dot (const Vector&) const;

	//! Returns the cross product of this vector and another.
	Vector cross (const Vector&) const;

	/*! Returns the distance between this vector and another.
	 * The distance is the magnitude of the vectors' difference. */
	double distance (const Vector& from) const;

	/*! Constructs a vector based on the given string.
	 * See the String assignment operator. */
	explicit Vector (const String&);

	/*! Sets the vector based on the given string.
	 * The string should be a comma-separated triplet of component values,
	 * such as \c "-5.5,12,32.01". */
	Vector& operator = (const String&);

	/*! Returns a string representation of the vector.
	 * This is the three vector components at a fixed precision of three
	 * decimal places, separated by commas. */
	explicit operator String () const;

	/*! A component value below which values can be treated as zero.
	 * This epsilon value is based on the useful resolution of location and
	 * distance values in the Dark %Engine. %Vector components and related
	 * scalars whose absolute values are below EPSILON can be treated as
	 * equal to zero for most purposes. */
	static const double EPSILON;

	//! One of the three components of a vector.
	enum class Component { NONE = -1, X, Y, Z};

	/*! Returns the requested component of the vector.
	 * This method can be used to select a component variably, such as
	 * based on a Parameter. */
	float& operator [] (Component);

	/*! Returns the requested component of the vector.
	 * This method can be used to select a component variably, such as
	 * based on a Parameter. */
	const float& operator [] (Component) const;
};

/*! Outputs the given vector to the given stream.
 * This is the output of the vector's String conversion. \relates Vector */
std::ostream& operator << (std::ostream&, const Vector&);



/*! A shape of curve for interpolation calculations.
 * In the formulas, \f$t\f$ represents the input \a alpha to calculate_curve(),
 * while \f$\alpha\f$ represents that function's return value. */
enum class Curve
{
	LINEAR,    //!< A straight line: \f$\alpha = t\f$
	QUADRATIC, //!< A parabolic curve: \f$\alpha = t^2\f$
	RADICAL,   //!< A radical curve: \f$\alpha = \sqrt{t}\f$
	LOG_10,    //!< A logarithmic curve (base 10): \f$\alpha = log_{10} (0.9t + 0.1) + 1\f$
	POW_10,    //!< A power curve (base 10): \f$\alpha = (10^t - 1) \div 9\f$
	LOG_E,     //!< A logarithmic curve (base \f$e\f$): \f$\alpha = log_e ((1 - \frac{1}{e})t + \frac{1}{e}) + 1\f$
	POW_E      //!< A power curve (base \f$e\f$): \f$\alpha = \frac{e^t - 1}{e - 1}\f$
};

/*! Calculates a position along a curve of the given type.
 * This function is used by interpolate() to calculate non-linear curves, but
 * could be called by any other code that needs to make a similar calculation,
 * including overloads of interpolate(). \param alpha The equivalent position
 * along a linear curve, between 0.0 and 1.0. \param curve The type of curve to
 * calculate. \return The position along a curve of the given type, between 0.0
 * and 1.0. */
double calculate_curve (double alpha, Curve curve);

//! \cond HIDDEN_SYMBOLS
#define THIEF_INTERPOLATE_RESULT(Type) typename \
std::enable_if<std::has_trivial_copy_constructor<Type>::value, Type>::type
//! \endcond

/*! Returns a value interpolated between the \a from and \a to values.
 * The interpolation is weighted by \a alpha, where \c 0.0 returns the \a from
 * value, \c 1.0 returns the \a to value, and \c 0.5 on a linear curve returns
 * their average. The \a alpha and \a curve are passed to calculate_curve()
 * to obtain the actual weighting used. An overload for Color ensures that
 * smooth color blending is performed in the CIE L*a*b* color space. */
template <typename T>
THIEF_INTERPOLATE_RESULT (T) interpolate (const T& from, const T& to,
	float alpha = 0.5f, Curve curve = Curve::LINEAR);

//! \cond HIDDEN_SYMBOLS
Color interpolate (const Color& from, const Color& to, float alpha = 0.5f,
	Curve = Curve::LINEAR);
//! \endcond



/*! An exception thrown when a requested game resource does not exist.
 * This exception is thrown by various ThiefLib methods when they are passed
 * a null reference, are passed a reference to a nonexistent resource, or fail
 * to locate or load a requested resource. */
class MissingResource : public std::exception
{
public:
	//! A type of game resource.
	enum Type
	{
		OBJECT,    //!< A game Object.
		PROPERTY,  //!< An object Property.
		FLAVOR,    //!< A link Flavor.
		LINK,      //!< A Link between objects.
		PARAMETER, //!< A Parameter in an object's Design Note.
		BITMAP     //!< A HUDBitmap image.
	};

	/*! Constructs a new missing resource exception.
	 * \param type The type of resource that does not exist.
	 * \param name The name of the requested resource, or its number in
	 * string form.
	 * \param object An object related to the missing resource, or
	 * Object::NONE if not applicable.
	 */
	MissingResource (Type type, const String& name,
		const Object& object) noexcept;

	/*! Constructs a new missing resource exception for a missing object.
	 * \param object The object that is missing. */
	MissingResource (const Object& object) noexcept;

	//! Destroys a missing resource exception.
	virtual ~MissingResource () noexcept {}

	//! Returns a string describing the exception.
	virtual const char* what () const noexcept
		{ return explanation.data (); }

private:
	String explanation;
};



} // namespace Thief

#include <Thief/Base.inl>

#endif // THIEF_BASE_HH

