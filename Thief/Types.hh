/******************************************************************************
 *  Types.hh
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

#ifndef THIEF_TYPES_HH
#define THIEF_TYPES_HH

#include <Thief/Base.hh>
#include <Thief/Link.hh>
#include <Thief/Message.hh>
#include <Thief/Mission.hh>
#include <Thief/Physics.hh>
#include <Thief/Property.hh>
#include <Thief/Rendering.hh>

namespace Thief {

class Being;



// Combinable

class Combinable : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Combinable)

	THIEF_PROP_FIELD (int, stack_count);
	int adjust_stack_count (int by, bool destroy_if_zero = true);
};

class CombineMessage : public Message // "Combine" //TESTME
{
public:
	CombineMessage (const Object& stack);
	THIEF_MESSAGE_WRAP (CombineMessage);

	Combinable get_stack () const;
};



// Damageable

class Damageable : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Damageable)

	THIEF_PROP_FIELD (int, hit_points);
	THIEF_PROP_FIELD (int, max_hit_points);

	enum class SlayResult { NORMAL, NONE, TERMINATE, DESTROY };
	THIEF_PROP_FIELD (SlayResult, slay_result);

	THIEF_PROP_FIELD_CONST (int, death_stage);

	void damage (const Object& type, int intensity, const Object& culprit);
	void slay (const Object& culprit);
	void resurrect (const Object& culprit);
};

class DamageMessage : public Message // "Damage" //TESTME
{
public:
	DamageMessage (const Object& culprit, const Object& stimulus,
		int hit_points);
	THIEF_MESSAGE_WRAP (DamageMessage);

	Being get_culprit () const;
	Object get_stimulus () const;
	int get_hit_points () const;
};

class SlayMessage : public Message // "Slain"
{
public:
	SlayMessage (const Object& culprit, const Object& stimulus);
	THIEF_MESSAGE_WRAP (SlayMessage);

	Being get_culprit () const;
	Object get_stimulus () const;
};



// Interactive

class Interactive : public virtual Rendered
{
public:
	THIEF_OBJECT_TYPE (Interactive)

	enum class InventoryType { JUNK, ITEM, WEAPON };
	InventoryType get_inventory_type () const;

	enum class FrobAction
	{
		INERT = 0,
		ADD_TO_INVENTORY = 1,
		FROB_SCRIPTS = 2,
		DESTROY = 4,
		IGNORE = 8,
		FOCUS_SCRIPTS = 16,
		USE_AS_TOOL = 32,
		REDUCE_STACK = 64,
		STIMULATE = 128,
		DESELECT = 256
	};
	THIEF_PROP_FIELD (FrobAction, frob_world_action); //TESTME
	THIEF_PROP_FIELD (FrobAction, frob_inventory_action); //TESTME
	THIEF_PROP_FIELD (FrobAction, frob_tool_action); //TESTME

	THIEF_PROP_FIELD (float, pick_distance); //TESTME
	THIEF_PROP_FIELD (float, pick_bias); //TESTME
	THIEF_PROP_FIELD (float, tool_reach); //TESTME

	THIEF_PROP_FIELD (bool, droppable); //TESTME
	THIEF_PROP_FIELD (String, limb_model); //TESTME

	THIEF_PROP_FIELD (int, loot_value_gold); //TESTME
	THIEF_PROP_FIELD (int, loot_value_gems); //TESTME
	THIEF_PROP_FIELD (int, loot_value_goods); //TESTME
	THIEF_PROP_FIELD (unsigned, loot_value_special); //TESTME
	THIEF_PROP_FIELD (int, store_price); //TESTME
};

class FrobMessage : public Message // "Frob{World,Inv,Tool}{Begin,End}" //TESTME details only
{
public:
	enum Event { BEGIN, END };
	enum Location { WORLD, INVENTORY, TOOL, NONE };

	FrobMessage (Event, const Object& frobber, const Object& tool,
		const Object& frobbed, Location frob_loc, Location obj_loc,
		Time duration, bool aborted);
	THIEF_MESSAGE_WRAP (FrobMessage);

	Event get_event () const;
	Being get_frobber () const;
	Interactive get_tool () const;
	Interactive get_frobbed () const;
	Location get_frob_loc () const;
	Location get_obj_loc () const;
	Time get_duration () const;
	bool was_aborted () const;
};



// Being: base class for shared features of AIs and avatars

class Being : public virtual SpherePhysical, public virtual Damageable
{
public:
	THIEF_OBJECT_TYPE (Being)
	bool is_being () const;

	enum class Team { GOOD, NEUTRAL, BAD_1, BAD_2, BAD_3, BAD_4, BAD_5 };
	THIEF_PROP_FIELD (Team, team);
	THIEF_PROP_FIELD (bool, culpable);

	THIEF_PROP_FIELD (String, blood_type);

	THIEF_PROP_FIELD (Time, current_breath);
	THIEF_PROP_FIELD (Time, maximum_breath);
	THIEF_PROP_FIELD (float, breath_recovery_rate);
	THIEF_PROP_FIELD (int, drowning_damage);
	THIEF_PROP_FIELD (Time, drowning_frequency);
};



// Container

class Container : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Container)

	enum class Type
	{
		ALTERNATE = -3,
		HAND = -2,
		BELT = -1,
		GENERIC = 0,
		NONE = INT_MAX
	};

	bool contains (const Object& maybe_contained, bool direct_only = false); //TESTME +direct_only
	Type get_contain_type (const Object& maybe_contained);

	struct Content
	{
		Object object;
		Type type;
		Link link;
	};
	typedef std::vector<Content> Contents;
	Contents get_contents () const;

	bool add_contents (const Object& contained, Type type, bool combine);
	void remove_contents (const Object& contained);
	void move_contents (const Object& new_container, bool combine);

	THIEF_PROP_FIELD (bool, inherit_contains); //TESTME
	THIEF_PROP_FIELD (bool, is_loadout_store); //TESTME
};

THIEF_FLAVORED_LINK (Contains)
{
	THIEF_FLAVORED_LINK_COMMON (Contains)

	static ContainsLink create (const Object& source,
		const Object& dest, Container::Type = Container::Type::GENERIC);

	THIEF_LINK_FIELD (Container::Type, type);
};

class ContainmentMessage : public Message // "Contained", "Container"
{
public:
	enum Subject { CONTAINER, CONTENTS };
	enum Event { QUERY_ADD, QUERY_COMBINE, ADD, REMOVE, COMBINE };

	ContainmentMessage (Subject, Event,
		const Object& container, const Object& contents);
	THIEF_MESSAGE_WRAP (ContainmentMessage);

	Subject get_subject () const;
	Event get_event () const;
	Container get_container () const;
	Object get_contents () const;
};



// Marker

class Marker : public Object
{
public:
	THIEF_OBJECT_TYPE (Marker)

	THIEF_PROP_FIELD (int, flee_value);

	THIEF_PROP_FIELD (int, cover_value);
	THIEF_PROP_FIELD (float, cover_decay_speed);
	THIEF_PROP_FIELD (bool, cover_can_duck);

	THIEF_PROP_FIELD (int, vantage_value);
	THIEF_PROP_FIELD (float, vantage_decay_speed);
};



// Readable

class Readable : public virtual Interactive
{
public:
	THIEF_OBJECT_TYPE (Readable)

	THIEF_PROP_FIELD (String, book_name);
	THIEF_PROP_FIELD (String, book_art);

	String get_book_text (int page = 0); //TESTME

	bool show_book (bool use_art, Time duration = 0ul, bool reload = false); //TESTME
};



// Room

class Room : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Room)
	bool is_room () const; //TESTME

	THIEF_PROP_FIELD_CONST (AmbientLightZone, ambient_light_zone); //TESTME

	THIEF_PROP_FIELD_CONST (int, automap_page); //TESTME
	THIEF_PROP_FIELD_CONST (int, automap_location); //TESTME

	THIEF_PROP_FIELD_CONST (EnvironmentMapZone, environment_map_zone); //TESTME

	THIEF_PROP_FIELD_CONST (Fog::Zone, fog_zone);

	THIEF_PROP_FIELD_CONST (int, gravity); //TESTME

	THIEF_PROP_FIELD_CONST (bool, precipitation); //TESTME

	THIEF_PROP_FIELD_CONST (bool, see_through); //TESTME
};

class RoomMessage : public Message
// "{Player,Creature,Object}Room{Enter,Exit}", "ObjRoomTransit"
{
public:
	enum Event { ENTER, EXIT, TRANSIT };
	enum ObjectType { PLAYER, REMOTE_PLAYER, CREATURE, OBJECT, NONE };

	RoomMessage (Event, ObjectType, const Object& object,
		const Object& from_room, const Object& to_room);
	THIEF_MESSAGE_WRAP (RoomMessage);

	Event get_event () const;
	ObjectType get_object_type () const;
	Object get_object () const;
	Room get_from_room () const;
	Room get_to_room () const;
};



// Secret

#ifdef IS_THIEF2
class Secret : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Secret)
	THIEF_PROP_FIELD (bool, is_secret); //TESTME

	void find_secret (); //TESTME
};
#endif // IS_THIEF2



} // namespace Thief

#endif // THIEF_TYPES_HH

