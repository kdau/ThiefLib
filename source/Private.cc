/******************************************************************************
 *  Private.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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
	if (malloc) malloc->Release ();
#ifdef DEBUG
	if (dbmalloc) dbmalloc->Release ();
	// module_name leaked intentionally
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

	size_t namelen = (_module_name ? strlen (_module_name) : 3) +
		sizeof ("ThiefLib allocator []");
	module_name = static_cast<char*> (allocator->Alloc (namelen));
	strcpy (module_name, "ThiefLib allocator [");
	strcat (module_name, _module_name ? _module_name : "???");
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

const XYZColor
XYZColor::D65_WHITE = { 0.950456, 1.0, 1.088754 };

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

XYZColor::XYZColor (const RGBColor& srgb)
{
	double  R = sRGB_invgamma (srgb.red / 255.0),
		G = sRGB_invgamma (srgb.green / 255.0),
		B = sRGB_invgamma (srgb.blue / 255.0);

	X = 0.4124564 * R + 0.3575761 * G + 0.1804375 * B;
	Y = 0.2126729 * R + 0.7151522 * G + 0.0721750 * B;
	Z = 0.0193339 * R + 0.1191920 * G + 0.9503041 * B;
}

XYZColor::operator RGBColor () const
{
	double 	R =  3.2404542 * X - 1.5371385 * Y - 0.4985314 * Z,
		G = -0.9692660 * X + 1.8760108 * Y + 0.0415560 * Z,
		B =  0.0556434 * X - 0.2040259 * Y + 1.0572252 * Z;

	R = 255.0 * std::max (0.0, std::min (1.0, sRGB_gamma (R)));
	G = 255.0 * std::max (0.0, std::min (1.0, sRGB_gamma (G)));
	B = 255.0 * std::max (0.0, std::min (1.0, sRGB_gamma (B)));
	return RGBColor (R, G, B);
}

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

XYZColor::operator LabColor () const
{
	double  _X = Lab_f (X / D65_WHITE.X),
		_Y = Lab_f (Y / D65_WHITE.Y),
		_Z = Lab_f (Z / D65_WHITE.Z);

	return {
		std::max (0.0, 116.0 * _Y - 16.0),
		500.0 * (_X - _Y),
		200.0 * (_Y - _Z)
	};
}

XYZColor::XYZColor (const LabColor& lab)
{
	double  _Y = (std::max (0.0, std::min (100.0, lab.L)) + 16.0) / 116.0,
		_X = _Y + std::max (-110.0, std::min (110.0, lab.a)) / 500.0,
		_Z = _Y - std::max (-110.0, std::min (110.0, lab.b)) / 200.0;

	X = std::max (0.0, std::min (1.0, D65_WHITE.X * Lab_invf (_X)));
	Y = std::max (0.0, std::min (1.0, D65_WHITE.Y * Lab_invf (_Y)));
	Z = std::max (0.0, std::min (1.0, D65_WHITE.Z * Lab_invf (_Z)));
}



// LinkMessageImpl

LinkMessageImpl::LinkMessageImpl ()
	: sScrMsg (),
	  event (Event::CHANGE),
	  flavor (Flavor::ANY),
	  link (Link::NONE.number),
	  source (),
	  dest ()
{}

LinkMessageImpl::~LinkMessageImpl ()
{}

const char* __thiscall
LinkMessageImpl::GetName () const
{
	return "LinkMessageImpl";
}



// PropertyMessageImpl

PropertyMessageImpl::PropertyMessageImpl ()
	: sScrMsg (),
	  event (Event::CHANGE),
	  inherited (false),
	  property (Property::NONE),
	  object ()
{}

PropertyMessageImpl::~PropertyMessageImpl ()
{}

const char* __thiscall
PropertyMessageImpl::GetName () const
{
	return "PropertyMessageImpl";
}



// ConversationMessageImpl

ConversationMessageImpl::ConversationMessageImpl ()
	: sScrMsg (),
	  conversation ()
{}

ConversationMessageImpl::~ConversationMessageImpl ()
{}

const char* __thiscall
ConversationMessageImpl::GetName () const
{
	return "ConversationMessageImpl";
}



} // namespace Thief

