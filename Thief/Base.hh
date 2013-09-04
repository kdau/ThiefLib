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



// Configuration for Windows headers

#define WINVER 0x0400
#define _WIN32_WINNT 0x0400
#define WIN32_LEAN_AND_MEAN



// Dark Engine version checks

#if (_DARKGAME == 1)
#define IS_THIEF
#define IS_THIEF1
#elif (_DARKGAME == 2)
#define IS_THIEF
#define IS_THIEF2
#elif defined(_DARKGAME)
#error "ThiefLib does not support this game."
#elif !defined (IS_OSL)
#error "_DARKGAME must be defined."
#endif

#ifndef _NEWDARK
#error "ThiefLib does not support pre-NewDark versions of the Dark Engine."
#endif



// C++ standard library headers used in ThiefLib

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



namespace Thief {

class Object;
typedef std::vector<Object> Objects;

struct Flavor;
class Link;
typedef std::vector<Link> Links;




typedef std::string String;

class CITraits;
typedef std::basic_string<char, CITraits> CIString;

bool operator == (const CIString&, const String&);
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

	Value value; // milliseconds

	Time (Value value = 0ul);
	Time& operator = (Value);

	operator Value () const { return value; }
	explicit operator long () const { return value; }
	explicit operator float () const { return value; }

	Value seconds () const;
	Value minutes () const;

	explicit Time (const String&);
	Time& operator = (const String&);
	explicit operator String () const;
};

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
};

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

