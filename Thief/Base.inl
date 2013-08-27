/******************************************************************************
 *  Base.inl
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

#ifndef THIEF_BASE_HH
#error "This file should only be included from <Thief/Base.hh>."
#endif

#ifndef THIEF_BASE_INL
#define THIEF_BASE_INL



#define THIEF_IS_ENUM typename std::enable_if<std::is_enum<T>::value>::type
#define THIEF_NOT_ENUM typename std::enable_if<!std::is_enum<T>::value>::type

#define THIEF_IS_OBJECT \
typename std::enable_if<std::is_base_of<Object, T>::value>::type



// Forward declarations of LG types referenced in ThiefLib headers

struct IProperty;
struct IScript;
struct sMultiParm;
struct sScrClassDesc;
struct sScrMsg;
typedef int (__cdecl *MPrintfProc) (const char*, ...);



namespace Thief {



// Case-insensitive string support

struct CITraits : public std::char_traits<char>
{
	static bool eq (char c1, char c2)
		{ return std::toupper (c1) == std::toupper (c2); }

	static bool ne (char c1, char c2)
		{ return std::toupper (c1) != std::toupper (c2); }

	static bool lt (char c1, char c2)
		{ return std::toupper (c1) < std::toupper (c2); }

	static int compare (const char* s1, const char* s2, size_t n)
	{
		while (n-- != 0)
		{
			if (std::toupper (*s1) < std::toupper (*s2)) return -1;
			if (std::toupper (*s1) > std::toupper (*s2)) return 1;
			++s1; ++s2;
		}
		return 0;
	}

	static const char* find (const char* s, int n, char a)
	{
		while (n-- > 0 && std::toupper (*s) != std::toupper (a))
			++s;
		return s;
	}
};

inline bool
operator == (const CIString& lhs, const String& rhs)
{
	return lhs == rhs.data ();
}

inline std::ostream&
operator << (std::ostream& out, const CIString& string)
{
	out << string.data ();
	return out;
}



// CanvasPoint

inline
CanvasPoint::CanvasPoint ()
	: x (0), y (0)
{}

inline
CanvasPoint::CanvasPoint (int _x, int _y)
	: x (_x), y (_y)
{}

inline bool
CanvasPoint::operator == (const CanvasPoint& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

inline bool
CanvasPoint::operator != (const CanvasPoint& rhs) const
{
	return x != rhs.x || y != rhs.y;
}



// CanvasSize

inline
CanvasSize::CanvasSize ()
	: w (0), h (0)
{}

inline
CanvasSize::CanvasSize (int _w, int _h)
	: w (_w), h (_h)
{}

inline bool
CanvasSize::operator == (const CanvasSize& rhs) const
{
	return w == rhs.w && h == rhs.h;
}

inline bool
CanvasSize::operator != (const CanvasSize& rhs) const
{
	return w != rhs.w || h != rhs.h;
}



// CanvasRect

inline
CanvasRect::CanvasRect ()
	: CanvasPoint (), CanvasSize ()
{}

inline
CanvasRect::CanvasRect (int _x, int _y, int _w, int _h)
	: CanvasPoint (_x, _y), CanvasSize (_w, _h)
{}

inline
CanvasRect::CanvasRect (CanvasPoint position, CanvasSize size)
	: CanvasPoint (position), CanvasSize (size)
{}

inline
CanvasRect::CanvasRect (CanvasSize size)
	: CanvasPoint (), CanvasSize (size)
{}

inline bool
CanvasRect::operator == (const CanvasRect& rhs) const
{
	return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
}

inline bool
CanvasRect::operator != (const CanvasRect& rhs) const
{
	return x != rhs.x || y != rhs.y || w != rhs.w || h != rhs.h;
}

inline CanvasRect
CanvasRect::operator + (const CanvasPoint& rhs) const
{
	return CanvasRect (x + rhs.x, y + rhs.y, w, h);
}

inline CanvasRect
CanvasRect::operator - (const CanvasPoint& rhs) const
{
	return CanvasRect (x - rhs.x, y - rhs.y, w, h);
}



// Color

inline
Color::Color ()
	: red (0u), green (0u), blue (0u)
{}

inline
Color::Color (Component _red, Component _green, Component _blue)
	: red (_red), green (_green), blue (_blue)
{}

inline
Color::Color (Value value)
	: red (0u), green (0u), blue (0u)
{
	*this = value;
}

inline
Color::Color (const String& code)
	: red (0u), green (0u), blue (0u)
{
	*this = code;
}

inline std::ostream&
operator << (std::ostream& out, const Color& color)
{
	out << String (color);
	return out;
}



// Time

inline
Time::Time (Value _value)
	: value (_value)
{}

inline Time&
Time::operator = (Value _value)
{
	value = _value;
	return *this;
}

inline
Time::Time (const String& code)
	: value (0ul)
{
	*this = code;
}

inline std::ostream&
operator << (std::ostream& out, const Time& time)
{
	out << String (time);
	return out;
}



// Vector

inline
Vector::Vector ()
	: x (0.0f), y (0.0f), z (0.0f)
{}

inline
Vector::Vector (float _x, float _y, float _z)
	: x (_x), y (_y), z (_z)
{}

inline bool
Vector::operator == (const Vector& rhs) const
{
	return  std::abs (x - rhs.x) <= EPSILON &&
		std::abs (y - rhs.y) <= EPSILON &&
		std::abs (z - rhs.z) <= EPSILON;
}

inline bool
Vector::operator != (const Vector& rhs) const
{
	return  std::abs (x - rhs.x) > EPSILON ||
		std::abs (y - rhs.y) > EPSILON ||
		std::abs (z - rhs.z) > EPSILON;
}

inline Vector
Vector::operator + (const Vector& rhs) const
{
	return { x + rhs.x, y + rhs.y, z + rhs.z };
}

inline Vector
Vector::operator - (const Vector& rhs) const
{
	return { x - rhs.x, y - rhs.y, z - rhs.z };
}

inline Vector
Vector::operator * (const Vector& rhs) const
{
	return { x * rhs.x, y * rhs.y, z * rhs.z };
}

inline Vector
Vector::operator / (const Vector& rhs) const
{
	return { x / rhs.x, y / rhs.y, z / rhs.z };
}

inline Vector
Vector::operator * (double rhs) const
{
	return { float (x * rhs), float (y * rhs), float (z * rhs) };
}

inline Vector
Vector::operator / (double rhs) const
{
	return { float (x / rhs), float (y / rhs), float (z / rhs) };
}

inline Vector&
Vector::operator += (const Vector& rhs)
{
	x += rhs.x; y += rhs.y; z += rhs.z;
	return *this;
}

inline Vector&
Vector::operator -= (const Vector& rhs)
{
	x -= rhs.x; y -= rhs.y; z -= rhs.z;
	return *this;
}

inline Vector&
Vector::operator *= (const Vector& rhs)
{
	x *= rhs.x; y *= rhs.y; z *= rhs.z;
	return *this;
}

inline Vector&
Vector::operator /= (const Vector& rhs)
{
	x /= rhs.x; y /= rhs.y; z /= rhs.z;
	return *this;
}

inline Vector&
Vector::operator *= (double rhs)
{
	x *= rhs; y *= rhs; z *= rhs;
	return *this;
}

inline Vector&
Vector::operator /= (double rhs)
{
	x /= rhs; y /= rhs; z /= rhs;
	return *this;
}

inline
Vector::Vector (const String& code)
	: x (0.0f), y (0.0f), z (0.0f)
{
	*this = code;
}

inline std::ostream&
operator << (std::ostream& out, const Vector& vector)
{
	out << String (vector);
	return out;
}



// Interpolation

template <typename T>
inline THIEF_INTERPOLATE_RESULT (T)
interpolate (const T& from, const T& to, float _alpha, Curve curve)
{
	double alpha = calculate_curve (_alpha, curve);
	return from * (1.0 - alpha) + to * alpha;
}



// LGMulti: type-safe wrappers for sMultiParm

class LGMultiBase
{
public:
	LGMultiBase (const LGMultiBase&) = delete;
	~LGMultiBase ();

	operator sMultiParm& ();
	operator const sMultiParm& () const;

	LGMultiBase& operator = (const sMultiParm&);
	void clear ();

	enum Type { EMPTY, INT, FLOAT, STRING, VECTOR };
	Type get_type () const { return type; }

protected:
	LGMultiBase ();
	union { int i; float f; void* p; } data;
	Type type;

	LGMultiBase (int enum_value);
	int as_enum_value () const;
	ObjectNumber as_object () const;
};

template <typename T, typename Enable = void>
class LGMulti;

#define THIEF_LGMULTI_SPECIALIZE(Type, Default) \
template<> \
class LGMulti<Type> : public LGMultiBase \
{ \
public: \
	LGMulti (const Type& value = Default); \
	operator Type () const; \
};

class LGMultiTypeError : public std::exception
{
public:
	LGMultiTypeError (LGMultiBase::Type got, const char* expected) noexcept;
	virtual ~LGMultiTypeError () noexcept {}
	virtual const char* what () const noexcept
		{ return explanation.data (); }
private:
	String explanation;
};



// Wrappers for fundamental types

THIEF_LGMULTI_SPECIALIZE (bool, false)
THIEF_LGMULTI_SPECIALIZE (int, 0)
THIEF_LGMULTI_SPECIALIZE (unsigned int, 0u)
THIEF_LGMULTI_SPECIALIZE (long, 0l)
THIEF_LGMULTI_SPECIALIZE (unsigned long, 0ul)
THIEF_LGMULTI_SPECIALIZE (float, 0.0f)
THIEF_LGMULTI_SPECIALIZE (double, 0.0)



// Wrapper template for enumeration types

template <typename T>
class LGMulti<T, THIEF_IS_ENUM> : public LGMultiBase
{
public:
	LGMulti (const T& value = T (0)) : LGMultiBase (int (value)) {}
	operator T () const { return T (as_enum_value ()); }
};



// Wrappers for ThiefLib types

THIEF_LGMULTI_SPECIALIZE (Color, Color ())
// Object specialization is declared and defined in Object.hh
THIEF_LGMULTI_SPECIALIZE (String, String ())
THIEF_LGMULTI_SPECIALIZE (Time, Time ())
// Timer specialization is declared in Message.hh
THIEF_LGMULTI_SPECIALIZE (Vector, Vector ())



// Anti-wrapper for cMultiParm

template<>
class LGMulti<sMultiParm> : public LGMultiBase
{
public:
	LGMulti ();
	LGMulti (const sMultiParm& value);
};

typedef LGMulti<sMultiParm> LGMultiEmpty;



// Convenience macros for get/set methods using LGMulti

#define MULTI_GET_ARG(Method, ArgType, ArgName) \
template <typename T> \
inline T \
Method (const ArgType& ArgName) const \
{ \
	LGMulti<T> multi; \
	_##Method (ArgName, multi); \
	return multi; \
}

#define MULTI_GET(Method) \
template <typename T> \
inline T \
Method () const \
{ \
	LGMulti<T> multi; \
	_##Method (multi); \
	return multi; \
}

#define MULTI_SET_ARG_RET(RetType, Method, ArgType, ArgName) \
template <typename T> \
inline RetType \
Method (const ArgType& ArgName, const T& value) \
{ \
	return _##Method (ArgName, LGMulti<T> (value)); \
}

#define MULTI_SET_ARG(Method, ArgType, ArgName) \
template <typename T> \
inline void \
Method (const ArgType& ArgName, const T& value) \
{ \
	_##Method (ArgName, LGMulti<T> (value)); \
}

#define MULTI_SET(Method) \
template <typename T> \
inline void \
Method (const T& value) \
{ \
	_##Method (LGMulti<T> (value)); \
}



// FieldProxyConfig: common base for PropField and LinkField proxy objects

template <typename T>
struct FieldProxyConfig
{
	const char* major;
	const char* minor;

	typedef typename std::conditional<std::is_same<T, String>::value,
		const char*, T>::type DefaultValue;
	DefaultValue default_value;

	unsigned bitmask; // for T==bool only

	typedef T (*GetFilter) (const T&);
	GetFilter get_filter;

	typedef T (*SetFilter) (const T&);
	SetFilter set_filter;
};

#define THIEF_FIELD_PROXY(ProxyType, Type, Name, Specifiers) \
static const FieldProxyConfig<Type> F_##Name; \
Specifiers ProxyType<Type, F_##Name> Name;



} // namespace Thief

#endif // THIEF_BASE_INL

