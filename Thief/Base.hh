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
#elif defined(_DARKGAME)
#error "ThiefLib does not support this game."
#elif IS_DOXYGEN
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

class Object;
typedef std::vector<Object> Objects;

struct Flavor;
class Link;
typedef std::vector<Link> Links;




typedef std::string String;

class CITraits;
typedef std::basic_string<char, CITraits> CIString;

//! \relates CIString
bool operator == (const CIString&, const String&);
//! \relates CIString
std::ostream& operator << (std::ostream&, const CIString&);



//! Represents a color in the sRGB space, as used by the engine.
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

typedef RGBColor Color;



/*! Represents a color in the CIE L*a*b* space, ideal for calculations.
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



/*! Represents a span of time or position in time on the sim clock.
 * The sim clock is the in-game clock which measures time passed in game mode
 * since the mission was started. It is not equivalent to wall clock time. It
 * is affected by pauses in game mode (interface screens and load/save),
 * explicit changes in clock rate (such as the \c scale_sim_time command in
 * DromEd), and to a small extent by hardware and performance constraints.  */
struct Time
{
	typedef unsigned long Value;

	//! The base unit of milliseconds.
	static constexpr Value MILLISECONDS = 1ul;

	//! The unit of seconds, as a multiple of the base millisecond unit.
	static constexpr Value SECONDS = 1000ul * MILLISECONDS;

	//! The unit of minutes, as a multiple of the base millisecond unit.
	static constexpr Value MINUTES = 60ul * SECONDS;

	//! The time value, in milliseconds.
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



/*! Represents a location, rotation, size, or velocity in the game world.
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

	Vector normal () const;
	Vector& normalize ();

	double magnitude () const;
	double mag_squared () const;

	double dot (const Vector&) const;
	Vector cross (const Vector&) const;
	double distance (const Vector& from) const;

	explicit Vector (const String&);
	Vector& operator = (const String&);
	explicit operator String () const;

	static const double EPSILON;

	enum class Component { NONE = -1, X, Y, Z};
	float& operator [] (Component);
	const float& operator [] (Component) const;
};

/*! Outputs the given vector to the given stream.
 * This is the output of the vector's String conversion. \relates Vector */
std::ostream& operator << (std::ostream&, const Vector&);



enum class Curve
{
	LINEAR,
	QUADRATIC, RADICAL,
	LOG_10, POW_10,
	LOG_E, POW_E
};

double calculate_curve (double alpha, Curve);

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



} // namespace Thief

#include <Thief/Base.inl>

#endif // THIEF_BASE_HH

