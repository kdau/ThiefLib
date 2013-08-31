/******************************************************************************
 *  Private.cc
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



// Required by the lg library.

IMalloc*
g_pMalloc = nullptr;



// Replacement operators new and delete

void*
operator new (std::size_t size)
{
	void* ptr = Thief::alloc.alloc (size);
	if (!ptr) throw std::bad_alloc ();
	return ptr;
}

void*
operator new [] (std::size_t size)
{
	void* ptr = Thief::alloc.alloc (size);
	if (!ptr) throw std::bad_alloc ();
	return ptr;
}

void*
operator new (std::size_t size, const std::nothrow_t&) noexcept
{
	return Thief::alloc.alloc (size);
}

void*
operator new [] (std::size_t size, const std::nothrow_t&) noexcept
{
	return Thief::alloc.alloc (size);
}

void
operator delete (void* ptr) noexcept
{
	Thief::alloc.free (ptr);
}

void
operator delete [] (void* ptr) noexcept
{
	Thief::alloc.free (ptr);
}

void
operator delete (void* ptr, const std::nothrow_t&) noexcept
{
	Thief::alloc.free (ptr);
}

void
operator delete [] (void* ptr, const std::nothrow_t&) noexcept
{
	Thief::alloc.free (ptr);
}



namespace Thief {

bool
negate (const bool& value)
{
	return !value;
}



// Allocator

Allocator
alloc;

Allocator::Allocator()
	: malloc (nullptr)
#ifdef DEBUG
	, dbmalloc (nullptr), module_name (nullptr)
#endif
{}

Allocator::~Allocator()
{
	// module_name leaked intentionally
	if (malloc) malloc->Release ();
#ifdef DEBUG
	if (dbmalloc) dbmalloc->Release ();
#endif
}

void
Allocator::attach (IMalloc* allocator, const char* _module_name)
{
	assert (allocator != nullptr);
	g_pMalloc = allocator;

	allocator->QueryInterface (IID_IMalloc,
		reinterpret_cast<void**>(&malloc));
#ifdef DEBUG
	allocator->QueryInterface (IID_IDebugMalloc,
		reinterpret_cast<void**>(&dbmalloc));

	size_t namelen = strlen (_module_name) + sizeof ("ThiefLib allocator []");
	module_name = static_cast<char*> (allocator->Alloc (namelen));
	strcpy (module_name, "ThiefLib allocator [");
	strcat (module_name, _module_name);
	strcat (module_name, "]");
#else
	(void) _module_name;
#endif
	assert (malloc != nullptr);
}

void*
Allocator::alloc (size_t size)
{
	assert (malloc != nullptr);
#ifdef DEBUG
	if (dbmalloc)
		return dbmalloc->AllocEx (size, module_name, 0);
	else
#endif
		return malloc->Alloc (size);
}

void*
Allocator::realloc (void* ptr, size_t size)
{
	assert (malloc != nullptr);
	if (!ptr) return alloc (size);
	if (!size)
	{
		free (ptr);
		return nullptr;
	}

#ifdef DEBUG
	if (dbmalloc)
		return dbmalloc->ReallocEx (ptr, size, module_name, 0);
	else
#endif
		return malloc->Realloc (ptr, size);
}

void
Allocator::free (void* ptr)
{
	if (!ptr) return;
	assert (malloc != nullptr);

#ifdef DEBUG
	if (dbmalloc)
		dbmalloc->FreeEx (ptr, module_name, 0);
	else
#endif
		malloc->Free (ptr);
}



// XYZColor

inline double
sRGB_gamma (double c)
{
	return (c <= 0.0031308)
		? 12.92 * c
		: (1.055 * std::pow (c, 1.0 / 2.4) - 0.055);
}

inline double
sRGB_invgamma (double c)
{
	return (c <= 0.04045)
		? c / 12.92
		: std::pow ((c + 0.055) / 1.055, 2.4);
}

XYZColor::XYZColor (const Color& srgb)
{
	double  R = sRGB_invgamma (srgb.red / 255.0),
		G = sRGB_invgamma (srgb.green / 255.0),
		B = sRGB_invgamma (srgb.blue / 255.0);

	X = 0.4124564 * R + 0.3575761 * G + 0.1804375 * B;
	Y = 0.2126729 * R + 0.7151522 * G + 0.0721750 * B;
	Z = 0.0193339 * R + 0.1191920 * G + 0.9503041 * B;
}

XYZColor::operator Color () const
{
	double 	R =  3.2404542 * X - 1.5371385 * Y - 0.4985314 * Z,
		G = -0.9692660 * X + 1.8760108 * Y + 0.0415560 * Z,
		B =  0.0556434 * X - 0.2040259 * Y + 1.0572252 * Z;

	R = sRGB_gamma (std::max (0.0, std::min (1.0, R)));
	G = sRGB_gamma (std::max (0.0, std::min (1.0, G)));
	B = sRGB_gamma (std::max (0.0, std::min (1.0, B)));

	return Color (R * 255.0, G * 255.0, B * 255.0);
}



// LabColor

static const XYZColor D65_WHITE = { 0.950456, 1.0, 1.088754 };

inline double
Lab_f (double t)
{
	static const double CUTOFF = std::pow (6.0 / 29.0, 3.0);
	return (t > CUTOFF) ? std::cbrt (t)
		: (t * 841.0 / 108.0 + 4.0 / 29.0);
}

inline double
Lab_invf (double t)
{
	static const double CUTOFF = 6.0 / 29.0;
	return (t > CUTOFF) ? (t * t * t)
		: ((t - 4.0 / 29.0) * 108.0 / 841.0);
}

LabColor::LabColor (const XYZColor& xyz)
{
	double  X = Lab_f (xyz.X / D65_WHITE.X),
		Y = Lab_f (xyz.Y / D65_WHITE.Y),
		Z = Lab_f (xyz.Z / D65_WHITE.Z);

	L = 116.0 * Y - 16.0;
	a = 500.0 * (X - Y);
	b = 200.0 * (Y - Z);
}

LabColor::operator XYZColor () const
{
	double  Y = (L + 16.0) / 116.0,
		X = Y + a / 500.0,
		Z = Y - b / 200.0;

	return {
		D65_WHITE.X * Lab_invf (X),
		D65_WHITE.Y * Lab_invf (Y),
		D65_WHITE.Z * Lab_invf (Z)
	};
}



} // namespace Thief

