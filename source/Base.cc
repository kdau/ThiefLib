/******************************************************************************
 *  Base.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts and the Object Script Library
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

#include "Private.hh"

namespace Thief {



// CanvasPoint

const CanvasPoint
CanvasPoint::ORIGIN = { 0, 0 };

const CanvasPoint
CanvasPoint::OFFSCREEN = { -1, -1 };

bool
CanvasPoint::valid () const
{
	// This can't check whether the point is within the canvas size.
	return x >= 0 && y >= 0;
}

CanvasPoint
CanvasPoint::operator - () const
{
	return CanvasPoint (-x, -y);
}

CanvasPoint
CanvasPoint::operator + (const CanvasPoint& rhs) const
{
	return CanvasPoint (x + rhs.x, y + rhs.y);
}

CanvasPoint
CanvasPoint::operator - (const CanvasPoint& rhs) const
{
	return CanvasPoint (x - rhs.x, y - rhs.y);
}

CanvasPoint
CanvasPoint::operator * (int rhs) const
{
	return CanvasPoint (x * rhs, y * rhs);
}

CanvasPoint
CanvasPoint::operator / (int rhs) const
{
	if (rhs == 0) throw std::domain_error ("divide by zero");
	return CanvasPoint (x / rhs, y / rhs);
}

CanvasPoint&
CanvasPoint::operator += (const CanvasPoint& rhs)
{
	x += rhs.x; y += rhs.y;
	return *this;
}

CanvasPoint&
CanvasPoint::operator -= (const CanvasPoint& rhs)
{
	x -= rhs.x; y -= rhs.y;
	return *this;
}



// CanvasSize

bool
CanvasSize::valid () const
{
	// An empty size still counts as valid.
	return w >= 0 && h >= 0;
}



// CanvasRect

const CanvasRect
CanvasRect::NOCLIP = { 0, 0, -1, -1 };

const CanvasRect
CanvasRect::OFFSCREEN = { -1, -1, -1, -1 };

bool
CanvasRect::valid () const
{
	// A rect may legitimately extend off the screen. This can't check
	// whether at least part of the rect is onscreen.
	return (*this == NOCLIP) || (w >= 0 && h >= 0);
}



// Color

Color&
Color::operator = (Value value)
{
	red = value & 0xFFu;
	green = value >> 8 & 0xFFu;
	blue = value >> 16 & 0xFFu;
	return *this;
}

Color::operator Value () const
{
	return Value (red) | (Value (green) << 8) | (Value (blue) << 16);
}

Color&
Color::operator = (const String& code)
{
	// hexadecimal full
	if (code.length () == 7 && code.front () == '#')
	{
		red = strtoul (code.substr (1, 2).data (), nullptr, 16);
		green = strtoul (code.substr (3, 2).data (), nullptr, 16);
		blue = strtoul (code.substr (5, 2).data (), nullptr, 16);
		return *this;
	}

	// hexadecimal short
	if (code.length () == 4 && code.front () == '#')
	{
		red = strtoul (code.substr (1, 1).data (), nullptr, 16);
		green = strtoul (code.substr (2, 1).data (), nullptr, 16);
		blue = strtoul (code.substr (3, 1).data (), nullptr, 16);
		red *= 17u; green *= 17u; blue *= 17u;
		return *this;
	}

	// decimal triplet
	size_t comma1 = code.find (','), comma2 = code.rfind (',');
	if (comma1 != String::npos && comma2 != String::npos && comma1 != comma2)
	{
		red = strtoul (code.substr (0, comma1).data (), nullptr, 10);
		green = strtoul (code.substr (comma1 + 1,
			comma2 - comma1 - 1).data (), nullptr, 10);
		blue = strtoul (code.substr (comma2 + 1).data (), nullptr, 10);
		return *this;
	}

	// named color
	static const std::map<CIString, Color::Value> NAMED_COLORS =
	{
		{ "aqua",       0xFFFF00u },
		{ "black",      0x080808u },
		{ "blue",       0xFF0000u },
		{ "chartreuse", 0x80FF80u },
		{ "dodger",     0x0080FFu },
		{ "fuchsia",    0xFF00FFu },
		{ "gold",       0x80FFFFu },
		{ "gray",       0x808080u },
		{ "green",      0x008000u },
		{ "grey",       0x808080u },
		{ "lawn",       0x00FF80u },
		{ "lime",       0x00FF00u },
		{ "maroon",     0x000080u },
		{ "navy",       0x800000u },
		{ "olive",      0x008080u },
		{ "orange",     0x0080FFu },
		{ "orchid",     0xFF80FFu },
		{ "pink",       0x8000FFu },
		{ "purple",     0x800080u },
		{ "red",        0x0000FFu },
		{ "rose",       0x8080FFu },
		{ "silver",     0xC0C0C0u },
		{ "slate",      0xFF8080u },
		{ "spring",     0x80FF00u },
		{ "steel",      0xFF8000u },
		{ "teal",       0x808000u },
		{ "turquoise",  0xFFFF80u },
		{ "violet",     0xFF0080u },
		{ "white",      0xFFFFFFu },
		{ "yellow",     0x00FFFFu },
	};
	auto iter = NAMED_COLORS.find (code.data ());
	if (iter != NAMED_COLORS.end ())
	{
		return *this = iter->second;
	}

	throw std::runtime_error ("invalid color code");
}

Color::operator String () const
{
	char code[8];
	snprintf (code, sizeof code, "#%.2X%.2X%.2X", red, green, blue);
	return code;
}



// Time

Time&
Time::operator = (const String& code)
{
	char* end = nullptr;
	double units = strtod (code.data (), &end);
	if (end == code.data ())
		throw std::runtime_error ("invalid time code");

	if (end)
		switch (*end)
		{
		case 'm': case 'M':
			units *= 60000.0; // milliseconds per minute
			break;
		case 's': case 'S':
			units *= 1000.0; // milliseconds per second
			break;
		case '\0':
			break;
		default:
			if (!std::isspace (*end))
				throw std::runtime_error ("invalid time code");
		}

	value = units;
	return *this;
}

Time::operator String () const
{
	std::ostringstream code;
	code << value;
	return code.str ();
}



// Vector

const double
Vector::EPSILON = 0.00001;

Vector
Vector::normal () const
{
	return *this / (magnitude () + EPSILON);
}

Vector&
Vector::normalize ()
{
	return *this /= (magnitude () + EPSILON);
}

double
Vector::magnitude () const
{
	return std::sqrt ((x * x) + (y * y) + (z * z));
}

double
Vector::mag_squared () const
{
	return (x * x) + (y * y) + (z * z);
}

double
Vector::dot (const Vector& rhs) const
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}

Vector
Vector::cross (const Vector& rhs) const
{
	return {
		y * rhs.z - z * rhs.y,
		z * rhs.x - x * rhs.z,
		x * rhs.y - y * rhs.x
	};
}

double
Vector::distance (const Vector& from) const
{
	return (from - *this).magnitude ();
}

Vector&
Vector::operator = (const String& code)
{
	size_t comma1 = code.find (','), comma2 = code.rfind (',');
	if (comma1 == String::npos || comma2 == String::npos || comma1 == comma2)
		throw std::runtime_error ("invalid vector specification");

	x = strtof (code.substr (0, comma1).data (), nullptr);
	y = strtof (code.substr (comma1 + 1, comma2 - comma1 - 1).data (),
		nullptr);
	z = strtof (code.substr (comma2 + 1).data (), nullptr);

	return *this;
}

Vector::operator String () const
{
	std::ostringstream code;
	code << std::fixed << std::setprecision (3)
		<< '(' << x << ',' << y << ',' << z << ')';
	return code.str ();
}



// Interpolation

THIEF_ENUM_CODING (Curve, BOTH, CODE,
	THIEF_ENUM_VALUE (LINEAR, "linear"),
	THIEF_ENUM_VALUE (QUADRATIC, "quadratic", "square"),
	THIEF_ENUM_VALUE (RADICAL, "radical", "sqrt"),
	THIEF_ENUM_VALUE (LOG_10, "log_10", "log-10", "log"),
	THIEF_ENUM_VALUE (POW_10, "pow_10", "pow-10", "10^"),
	THIEF_ENUM_VALUE (LOG_E, "log_e", "log-e", "ln"),
	THIEF_ENUM_VALUE (POW_E, "pow_e", "pow-e", "e^"),
)


double
calculate_curve (double alpha, Curve curve)
{
	static const double M_1_E = 0.3678794411714423216, // 1/e
		M_M1_E = 0.6321205588285576784, // 1 - 1/e
		M_1_ME = 0.5819767068693264244; // 1 / (e - 1)

	alpha = std::max (0.0, std::min (1.0, alpha));
	switch (curve)
	{
	case Curve::QUADRATIC: return alpha * alpha;
	case Curve::RADICAL: return std::sqrt (alpha);
	case Curve::LOG_10: return 1.0 + std::log10 (alpha * 0.9 + 0.1);
	case Curve::POW_10: return (std::pow (10.0, alpha) - 1.0) / 9.0;
	case Curve::LOG_E: return 1.0 + std::log (alpha * M_M1_E + M_1_E);
	case Curve::POW_E: return M_1_ME * (std::exp (alpha) - 1.0);
	case Curve::LINEAR: default: return alpha;
	}
}

Color
interpolate (const Color& _from, const Color& _to, float _alpha, Curve curve)
{
	double alpha = calculate_curve (_alpha, curve), inv = 1.0 - alpha;
	LabColor from = XYZColor (_from), to = XYZColor (_to);
	double  L = from.L * inv + to.L * alpha,
		a = from.a * inv + to.a * alpha,
		b = from.b * inv + to.b * alpha;
	return XYZColor (LabColor (L, a, b));
}




// LGMultiBase

LGMultiBase::LGMultiBase ()
	: type (EMPTY)
{
	data.p = nullptr;
}

LGMultiBase::~LGMultiBase ()
{
	clear ();
}

LGMultiBase::operator sMultiParm& ()
{
	return *reinterpret_cast<sMultiParm*> (this);
}

LGMultiBase::operator const sMultiParm& () const
{
	return *reinterpret_cast<const sMultiParm*> (this);
}

LGMultiBase&
LGMultiBase::operator = (const sMultiParm& copy)
{
	clear ();
	switch (type = Type (copy.type))
	{
	case INT:
		data.i = copy.i;
		break;
	case FLOAT:
		data.f = copy.f;
		break;
	case STRING:
		data.p = alloc.alloc (strlen (copy.psz) + 1);
		strcpy (static_cast<char*> (data.p), copy.psz);
		break;
	case VECTOR:
		data.p = alloc.alloc (sizeof (mxs_vector));
		memcpy (data.p, copy.pVector, sizeof (mxs_vector));
		break;
	default:
		break;
	}
	return *this;
}

void
LGMultiBase::clear ()
{
	if (data.p && (type == STRING || type == VECTOR))
		alloc.free (data.p);
	data.p = nullptr;
	type = EMPTY;
}

LGMultiBase::LGMultiBase (int enum_value)
	: type (INT)
{
	data.i = enum_value;
}

int
LGMultiBase::as_enum_value () const
{
	if (type != INT)
		throw LGMultiTypeError (type, "[enumeration]");
	return data.i;
}

ObjectNumber
LGMultiBase::as_object () const \
{
	switch (type)
	{
	case INT:
		return data.i;
	case STRING:
		return Object (static_cast<const char*> (data.p)).number;
	default:
		throw LGMultiTypeError (type, "[Object or subclass]");
	}
}

LGMultiTypeError::LGMultiTypeError (LGMultiBase::Type got, const char* expected)
	noexcept
{
	std::ostringstream explain;
	explain << "Cannot translate an LGMulti of type ";
	switch (got)
	{
	case LGMultiBase::Type::EMPTY: explain << "EMPTY"; break;
	case LGMultiBase::Type::INT: explain << "INT"; break;
	case LGMultiBase::Type::FLOAT: explain << "FLOAT"; break;
	case LGMultiBase::Type::STRING: explain << "STRING"; break;
	case LGMultiBase::Type::VECTOR: explain << "VECTOR"; break;
	}
	explain << " to a value of type " << expected << ".";
	explanation = explain.str ();
}



// LGMulti specializations

#define LGMULTI_SPECIALIZE_SET(Type, MultiType, Member) \
LGMulti<Type>::LGMulti (const Type& value) \
{ \
	data.Member = value; \
	type = MultiType; \
}

#define LGMULTI_SPECIALIZE_GET(Type, MultiType, Member) \
LGMulti<Type>::operator Type () const \
{ \
	if (type != MultiType) \
		throw LGMultiTypeError (type, typeid (Type).name ()); \
	return data.Member; \
}

#define LGMULTI_SPECIALIZE_IMPL(Type, MultiType, Member) \
LGMULTI_SPECIALIZE_SET (Type, MultiType, Member) \
LGMULTI_SPECIALIZE_GET (Type, MultiType, Member)



LGMULTI_SPECIALIZE_IMPL (bool, INT, i)
LGMULTI_SPECIALIZE_IMPL (int, INT, i)
LGMULTI_SPECIALIZE_IMPL (unsigned int, INT, i)
LGMULTI_SPECIALIZE_IMPL (long, INT, i)
LGMULTI_SPECIALIZE_IMPL (unsigned long, INT, i)
LGMULTI_SPECIALIZE_IMPL (float, FLOAT, f)
LGMULTI_SPECIALIZE_IMPL (double, FLOAT, f)
LGMULTI_SPECIALIZE_IMPL (Time, INT, i)



LGMULTI_SPECIALIZE_SET (Color, INT, i)

LGMulti<Color>::operator Color () const \
{
	switch (type)
	{
	case INT: return Color (data.i);
	case STRING: return Color (static_cast<const char*> (data.p));
	default: throw LGMultiTypeError (type, "Color");
	}
}



LGMulti<String>::LGMulti (const String& value)
{
	data.p = alloc.alloc (value.size () + 1);
	strcpy (static_cast<char*> (data.p), value.data ());
	type = STRING;
}

LGMulti<String>::operator String () const \
{
	if (type != STRING)
		throw LGMultiTypeError (type, "String");
	return static_cast<const char*> (data.p);
}



LGMulti<Timer>::LGMulti (const Timer& value)
{
	data.p = value.id;
	type = INT;
}

LGMulti<Timer>::operator Timer () const \
{
	if (type != INT)
		throw LGMultiTypeError (type, "Timer");
	return data.p;
}



LGMulti<Vector>::LGMulti (const Vector& value)
{
	data.p = alloc.alloc (sizeof (mxs_vector));
	memcpy (data.p, &value, sizeof (mxs_vector));
	type = VECTOR;
}

LGMulti<Vector>::operator Vector () const \
{
	if (type != VECTOR)
		throw LGMultiTypeError (type, "Vector");
	return *static_cast<const Vector*> (data.p);
}



LGMulti<sMultiParm>::LGMulti ()
{}

LGMulti<sMultiParm>::LGMulti (const sMultiParm& value)
{
	*this = value;
}



} // namespace Thief

