/******************************************************************************
 *  Object.cc
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



// Constants

const Object::Number
Object::NONE = 0;

const Object::Number
Object::ANY = 0;

const Object::Number
Object::SELF = INT_MAX;



// Constructors and destructors

PROXY_CONFIG (Object, has_refs, "HasRefs", nullptr, bool, true);
PROXY_CONFIG (Object, stack_count, "StackCount", nullptr, int, 1);

#define OBJECT_COMMON_INIT \
	PROXY_INIT (has_refs), \
	PROXY_INIT (stack_count)

Object::Object (Number _number)
	: number (_number), OBJECT_COMMON_INIT
{}

Object::Object (const Object& copy)
	: number (copy.number), OBJECT_COMMON_INIT
{}

Object::Object (const String& name)
	: number (find (name)), OBJECT_COMMON_INIT
{}

Object&
Object::operator = (const Object& copy)
{
	number = copy.number;
	return *this;
}

Object::~Object ()
{}

#ifdef IS_THIEF2
Object
Object::find_closest (const Object& archetype, const Object& nearby)
{
	LGObject closest;
	SService<IObjectSrv> (LG)->FindClosestObjectNamed (closest,
		nearby.number, archetype.get_name ().data ());
	return closest.id;
}
#endif // IS_THIEF2



// Existence, creation and destruction

Object
Object::create (const Object& archetype)
{
	LGObject created;
	SService<IObjectSrv> (LG)->Create (created, archetype.number);
	return created.id;
}

Object
Object::start_create (const Object& archetype)
{
	LGObject created;
	SService<IObjectSrv> (LG)->BeginCreate (created, archetype.number);
	return created.id;
}

bool
Object::finish_create ()
{
	return SService<IObjectSrv> (LG)->EndCreate (number) == S_OK;
}

Object
Object::create_temp_fnord (Time lifespan)
{
	Object fnord = create (Object ("Marker"));
	fnord.set_transient (true);

	// Add a delete tweq to enforce the lifespan.
	if (lifespan > 0)
	{
		Property config (fnord, "CfgTweqDelete", true);
		config.set_field ("Halt", 0); // Destroy Obj
		config.set_field ("AnimC", 2); // Sim
		config.set_field ("Rate", lifespan);

		Property state (fnord, "StTweqDelete", true);
		state.set_field ("AnimS", 1); // On
	}

	return fnord;
}

Object
Object::create_archetype (const Object& parent, const String& name)
{
	return SInterface<ITraitManager> (LG)->CreateArchetype
		(name.data (), parent.number);
}

Object
Object::create_metaprop (const Object& parent, const String& name)
{
	return SInterface<ITraitManager> (LG)->CreateMetaProperty
		(name.data (), parent.number);
}

bool
Object::exists () const
{
	LGBool _exists;
	SService<IObjectSrv> (LG)->Exists (_exists, number);
	return _exists;
}

Object
Object::clone () const
{
	return create (*this);
}

void
Object::destroy ()
{
	SService<IObjectSrv> (LG)->Destroy (number);
}



// Identity

bool
Object::operator == (const Object& rhs) const
{
	return number == rhs.number;
}

bool
Object::operator != (const Object& rhs) const
{
	return number != rhs.number;
}

bool
Object::operator == (Number rhs) const
{
	return number == rhs;
}

bool
Object::operator != (Number rhs) const
{
	return number != rhs;
}

String
Object::get_name () const
{
	LGString name;
	SService<IObjectSrv> (LG)->GetName (name, number);
	return name;
}

void
Object::set_name (const String& name)
{
	SService<IObjectSrv> (LG)->SetName (number, name.data ());
}

String
Object::get_editor_name () const
{
	std::ostringstream fname;

	String name = exists () ? get_name ()
		: (number == NONE) ? "None" : "NONEXISTENT";
	if (!name.empty ())
		fname << name;
	else
		fname << "A " << get_archetype ().get_name ();

	fname << " (" << number << ")";
	return fname.str ();
}

//TODO wrap property: Inventory\Object Name = GameName
String
Object::get_display_name () const
{
	LGString name;
	SService<IDataSrv> (LG)->GetObjString (name, number, "objnames");
	return name;
}

//TODO wrap property: Inventory\Long Description = GameDesc
String
Object::get_description () const
{
	LGString desc;
	SService<IDataSrv> (LG)->GetObjString (desc, number, "objdescs");
	return desc;
}



// Inheritance and transience

Object::Type
Object::get_type () const
{
	if (!exists ())
		return Type::NONE;
	else if (SInterface<ITraitManager> (LG)->IsArchetype (number))
		return Type::ARCHETYPE;
	else if (SInterface<ITraitManager> (LG)->IsMetaProperty (number))
		return Type::METAPROPERTY;
	else
		return Type::CONCRETE;
}

bool
Object::inherits_from (const Object& ancestor) const
{
	LGBool inherits;
	SService<IObjectSrv> (LG)->InheritsFrom
		(inherits, number, ancestor.number);
	return inherits;
}

Objects
Object::get_ancestors () const
{
	SInterface<IObjectQuery> _ancestors =
		SInterface<ITraitManager> (LG)->Query
			(number, kTraitQueryMetaProps | kTraitQueryFull);

	Objects ancestors;

	if (_ancestors)
		for (_ancestors->Next (); // Skip this object itself.
		     !_ancestors->Done (); _ancestors->Next ())
			ancestors.push_back (_ancestors->Object ());

	return ancestors;
}

Objects
Object::get_descendants (bool include_indirect) const
{
	unsigned flags = kTraitQueryChildren;
	if (include_indirect) flags |= kTraitQueryFull;
	SInterface<IObjectQuery> _descendants =
		SInterface<ITraitManager> (LG)->Query (number, flags);

	Objects descendants;

	if (_descendants)
		for (; !_descendants->Done (); _descendants->Next ())
			descendants.push_back (_descendants->Object ());

	return descendants;
}

Object
Object::get_archetype () const
{
	return SInterface<ITraitManager> (LG)->GetArchetype (number);
}

void
Object::set_archetype (const Object& archetype)
{
	SInterface<ITraitManager> (LG)->SetArchetype (number, archetype.number);
}

bool
Object::has_metaprop (const Object& metaprop) const
{
	LGBool has;
	SService<IObjectSrv> (LG)->HasMetaProperty
		(has, number, metaprop.number);
	return has;
}

bool
Object::add_metaprop (const Object& metaprop, bool single)
{
	if (single && has_metaprop (metaprop)) return false;
	SService<IObjectSrv> (LG)->AddMetaProperty (number, metaprop.number);
	return true;
}

bool
Object::remove_metaprop (const Object& metaprop)
{
	if (!has_metaprop (metaprop)) return false;
	SService<IObjectSrv> (LG)->RemoveMetaProperty (number, metaprop.number);
	return true;
}

bool
Object::is_transient () const
{
	LGBool transient;
	SService<IObjectSrv> (LG)->IsTransient (transient, number);
	return transient;
}

void
Object::set_transient (bool transient)
{
	SService<IObjectSrv> (LG)->SetTransience (number, transient);
}



// Position

Vector
Object::get_location () const
{
	LGVector location;
	SService<IObjectSrv> (LG)->Position (location, number);
	return location;
}

void
Object::set_location (const Vector& location)
{
	set_position (location, get_rotation ());
}

Vector
Object::get_rotation () const
{
	LGVector rotation;
	SService<IObjectSrv> (LG)->Facing (rotation, number);
	return rotation;
}

void
Object::set_rotation (const Vector& rotation)
{
	set_position (get_location (), rotation);
}

void
Object::set_position (const Vector& location, const Vector& rotation,
	const Object& relative)
{
	SService<IObjectSrv> (LG)->Teleport (number,
		LGVector (location), LGVector (rotation),
		(relative.number == SELF) ? number : relative.number);
}

Vector
Object::object_to_world (const Vector& relative) const
{
	LGVector absolute;
	SService<IObjectSrv> (LG)->ObjectToWorld
		(absolute, number, LGVector (relative));
	return absolute;
}



// Containment and combination

Object
Object::get_container () const
{
	return SInterface<IContainSys> (LG)->GetContainer (number);
}

int
Object::adjust_stack_count (int by, bool destroy_if_zero)
{
	return SInterface<IContainSys> (LG)->StackAdd
		(number, by, destroy_if_zero);
}



// Private methods

Object::Number
Object::find (const String& name)
{
	LGObject named;
	SService<IObjectSrv> (LG)->Named (named, name.data ());
	if (named) return named.id;

	Number number = strtol (name.data (), nullptr, 10);
	return Object (number).exists () ? number : NONE;
}



} // namespace Thief
