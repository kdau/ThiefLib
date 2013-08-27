/******************************************************************************
 *  Object.hh
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

#ifndef THIEF_OBJECT_HH
#define THIEF_OBJECT_HH

#include <Thief/Base.hh>
#include <Thief/Property.hh>

namespace Thief {



#define THIEF_OBJECT_TYPE(ClassName) \
	ClassName (Number = NONE); \
	ClassName (const Object&); \
	ClassName (const ClassName&); \
	ClassName& operator = (const ClassName& copy); \
	explicit ClassName (const String& name);



class Object
{
public:
	typedef int Number;
	static const Number NONE;
	static const Number ANY;
	static const Number SELF;

	Object (Number = NONE);

	Object (const Object&);
	Object& operator = (const Object& copy);

	explicit Object (const String& name);

	virtual ~Object ();

#ifdef IS_THIEF2
	static Object find_closest (const Object& archetype,
		const Object& nearby);
#endif

	static Object create (const Object& archetype);

	static Object start_create (const Object& archetype);
	bool finish_create (); // for two_stage creation only

	static Object create_temp_fnord (Time lifespan = 1ul);

	static Object create_archetype (const Object& parent, const String& name);
	static Object create_metaprop (const Object& parent, const String& name);

	bool exists () const;
	Object clone () const;
	void destroy ();

	// Identity

	Number number;
	bool operator == (const Object&) const;
	bool operator != (const Object&) const;
	bool operator == (Number) const;
	bool operator != (Number) const;

	String get_name () const;
	void set_name (const String&);

	String get_editor_name () const;
	String get_display_name () const;
	String get_description () const;

	// Inheritance and transience

	enum class Type { NONE, CONCRETE, ARCHETYPE, METAPROPERTY };
	Type get_type () const;

	bool inherits_from (const Object& ancestor) const;
	Objects get_ancestors () const;
	Objects get_descendants (bool include_indirect) const;

	Object get_archetype () const;
	void set_archetype (const Object& archetype);

	bool has_metaprop (const Object& metaprop) const;
	bool add_metaprop (const Object& metaprop, bool single = true);
	bool remove_metaprop (const Object& metaprop);

	bool is_transient () const;
	void set_transient (bool transient);

	// Position

	Vector get_location () const;
	void set_location (const Vector&);

	Vector get_rotation () const;
	void set_rotation (const Vector&);

	void set_position (const Vector& location, const Vector& rotation,
		const Object& relative = NONE);

	Vector object_to_world (const Vector& relative) const;

	// Containment and combination

	Object get_container () const;
	THIEF_PROP_FIELD_CONST (bool, has_refs);

	THIEF_PROP_FIELD (int, stack_count);
	int adjust_stack_count (int by, bool destroy_if_zero = true);

private:
	static Number find (const String& name);
};



// LGMulti specialization

template <typename T>
class LGMulti<T, THIEF_IS_OBJECT> : public LGMultiBase
{
public:
	LGMulti (const T& value = Object::NONE)
		: LGMultiBase (int (value.number)) {}
	operator T () const { return T (as_object ()); }
};



} // namespace Thief

#endif // THIEF_OBJECT_HH

