/******************************************************************************
 *  Base.hh
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

#ifndef THIEF_BASE_HH
#define THIEF_BASE_HH



//! \cond HIDDEN_SYMBOLS
// Dark Engine version checks

#if (_DARKGAME == 1)
#define IS_THIEF1
#elif (_DARKGAME == 2)
#define IS_THIEF2
#elif defined(_DARKGAME)
#error "ThiefLib does not support this game."
#elif !defined (IS_OSL)
#error "_DARKGAME must be defined."
#endif

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



struct RGBColor
{
	typedef unsigned char Component;
	typedef unsigned long Value;

	Component red;
	Component green;
	Component blue;

	RGBColor ();
	RGBColor (Component red, Component green, Component blue);

	explicit RGBColor (Value);
	RGBColor& operator = (Value);
	operator Value () const;

	explicit RGBColor (const String&);
	RGBColor& operator = (const String&);
	explicit operator String () const;
};

//! \relates RGBColor
std::ostream& operator << (std::ostream&, const RGBColor&);

typedef RGBColor Color;



struct LabColor
{
	typedef double Component;
	Component L, a, b;

	LabColor ();
	LabColor (Component L, Component a, Component b);

	LabColor (const RGBColor&);
	operator RGBColor () const;
};



struct Time
{
	typedef unsigned long Value;
	static constexpr Value MILLISECONDS = 1ul,
		SECONDS = 1000ul * MILLISECONDS,
		MINUTES = 60ul * SECONDS;

	Value value; // milliseconds

	Time (Value value = 0ul, Value unit = MILLISECONDS);
	Time& operator = (Value);

	operator Value () const { return value; }
	explicit operator long () const { return value; }

	Value seconds () const;
	Value minutes () const;

	Time (float value, Value unit);
	explicit operator float () const { return value; }
	float fseconds () const;
	float fminutes () const;

	Time operator + (const Time&) const;
	Time operator - (const Time&) const;
	Time& operator += (const Time&);
	Time& operator -= (const Time&);

	explicit Time (const String&);
	Time& operator = (const String&);
	explicit operator String () const;
};

//! \relates Time
std::ostream& operator << (std::ostream&, const Time&);



struct Vector
{
	float x;
	float y;
	float z;

	Vector ();
	Vector (float x, float y, float z);

	bool operator == (const Vector&) const;
	bool operator != (const Vector&) const;

	Vector operator + (const Vector&) const;
	Vector operator - (const Vector&) const;
	Vector operator * (const Vector&) const;
	Vector operator / (const Vector&) const;
	Vector operator * (double) const;
	Vector operator / (double) const;

	Vector& operator += (const Vector&);
	Vector& operator -= (const Vector&);
	Vector& operator *= (const Vector&);
	Vector& operator /= (const Vector&);
	Vector& operator *= (double);
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

//! \relates Vector
std::ostream& operator << (std::ostream&, const Vector&);



enum class Curve
{
	LINEAR,
	QUADRATIC, RADICAL,
	LOG_10, POW_10,
	LOG_E, POW_E
};

double calculate_curve (double alpha, Curve);

#define THIEF_INTERPOLATE_RESULT(Type) typename \
std::enable_if<std::has_trivial_copy_constructor<Type>::value, Type>::type

template <typename T>
THIEF_INTERPOLATE_RESULT (T) interpolate (const T& from, const T& to,
	float alpha = 0.5f, Curve = Curve::LINEAR);

Color interpolate (const Color& from, const Color& to, float alpha = 0.5f,
	Curve = Curve::LINEAR);



} // namespace Thief

#include <Thief/Base.inl>

#endif // THIEF_BASE_HH

