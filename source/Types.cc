/******************************************************************************
 *  Types.cc
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



// Combinable

PROXY_CONFIG (Combinable, stack_count, "StackCount", nullptr, int, 1);

OBJECT_TYPE_IMPL_ (Combinable,
	PROXY_INIT (stack_count)
)

int
Combinable::adjust_stack_count (int by, bool destroy_if_zero)
{
	return SInterface<IContainSys> (LG)->StackAdd
		(number, by, destroy_if_zero);
}

//TODO wrap void IContainSys::CombineTry (int, int, int)
//TODO wrap int IContainSys::CombineCount (int, int)
//TODO wrap void IContainSys::CombineAdd (int, int, int, ulong)
//TODO wrap bool IContainSys::CanCombine (int, int, int)
//TODO wrap bool IContainSys::CanCombineContainer (int, int, int)
//TODO wrap int IContainSys::RemoveFromStack (int, ulong, int)



// CombineMessage

// "Combine" reports as "sScrMsg", so it can't be tested by type.
MESSAGE_WRAPPER_IMPL_ (CombineMessage, MESSAGE_NAME_TEST ("Combine"))

CombineMessage::CombineMessage (const Object& stack)
	: Message (new sCombineScrMsg ())
{
	message->message = "Combine";
	MESSAGE_AS (sCombineScrMsg)->combiner = stack.number;
}

MESSAGE_ACCESSOR (Combinable, CombineMessage, get_stack,
	sCombineScrMsg, combiner)



// Damageable

PROXY_CONFIG (Damageable, hit_points, "HitPoints", nullptr, int, 0);
PROXY_CONFIG (Damageable, max_hit_points, "MAX_HP", nullptr, int, 0);
PROXY_CONFIG (Damageable, slay_result, "SlayResult", nullptr,
	Damageable::SlayResult, SlayResult::NORMAL);
PROXY_CONFIG (Damageable, death_stage, "DeathStage", nullptr, int, 0);

OBJECT_TYPE_IMPL_ (Damageable,
	PROXY_INIT (hit_points),
	PROXY_INIT (max_hit_points),
	PROXY_INIT (slay_result),
	PROXY_INIT (death_stage)
)

void
Damageable::damage (const Object& type, int intensity, const Object& culprit)
{
	SService<IDamageSrv> (LG)->Damage
		(number, culprit.number, intensity, type.number);
}

void
Damageable::slay (const Object& culprit)
{
	SService<IDamageSrv> (LG)->Slay (number, culprit.number);
}

void
Damageable::resurrect (const Object& culprit)
{
	SService<IDamageSrv> (LG)->Resurrect (number, culprit.number);
}



// DamageMessage

MESSAGE_WRAPPER_IMPL (DamageMessage, sDamageScrMsg)

DamageMessage::DamageMessage (const Object& culprit, const Object& stimulus,
		int hit_points)
	: Message (new sDamageScrMsg ())
{
	message->message = "Damage";
	MESSAGE_AS (sDamageScrMsg)->culprit = culprit.number;
	MESSAGE_AS (sDamageScrMsg)->kind = stimulus.number;
	MESSAGE_AS (sDamageScrMsg)->damage = hit_points;
}

MESSAGE_ACCESSOR (Being, DamageMessage, get_culprit, sDamageScrMsg, culprit)

MESSAGE_ACCESSOR (Object, DamageMessage, get_stimulus, sDamageScrMsg, kind) //TODO Fix return type once ActReact.hh is ready.

MESSAGE_ACCESSOR (int, DamageMessage, get_hit_points, sDamageScrMsg, damage)



// SlayMessage

MESSAGE_WRAPPER_IMPL (SlayMessage, sSlayMsg)

SlayMessage::SlayMessage (const Object& culprit, const Object& stimulus)
	: Message (new sSlayMsg ())
{
	message->message = "Slain";
	MESSAGE_AS (sSlayMsg)->culprit = culprit.number;
	MESSAGE_AS (sSlayMsg)->kind = stimulus.number;
}

MESSAGE_ACCESSOR (Being, SlayMessage, get_culprit, sSlayMsg, culprit)

MESSAGE_ACCESSOR (Object, SlayMessage, get_stimulus, sSlayMsg, kind) //TODO Fix return type once ActReact.hh is ready.



// Interactive
//TODO wrap property: Inventory\Cycle Order = InvCycleOrder
//TODO wrap property: Inventory\Render Type = InvRendType
//TODO wrap property: Inventory\Type = InvType
//TODO wrap link: FrobProxy - FrobProxyInfo

PROXY_CONFIG (Interactive, frob_world_action, "FrobInfo", "World Action",
	Interactive::FrobAction, FrobAction::INERT);
PROXY_CONFIG (Interactive, frob_inventory_action, "FrobInfo", "Inventory Action",
	Interactive::FrobAction, FrobAction::INERT);
PROXY_CONFIG (Interactive, frob_tool_action, "FrobInfo", "Tool Action",
	Interactive::FrobAction, FrobAction::INERT);
PROXY_CONFIG (Interactive, pick_distance, "PickDist", nullptr, float, 0.0f);
PROXY_CONFIG (Interactive, pick_bias, "PickBias", nullptr, float, 0.0f);
PROXY_CONFIG (Interactive, tool_reach, "ToolReach", nullptr, float, 0.0f);
PROXY_NEG_CONFIG (Interactive, droppable, "NoDrop", nullptr, bool, false);
PROXY_CONFIG (Interactive, limb_model, "InvLimbModel", nullptr, String, "");
PROXY_CONFIG (Interactive, loot_value_gold, "Loot", "Gold", int, 0);
PROXY_CONFIG (Interactive, loot_value_gems, "Loot", "Gems", int, 0);
PROXY_CONFIG (Interactive, loot_value_goods, "Loot", "Art", int, 0);
PROXY_CONFIG (Interactive, loot_value_special, "Loot", "Specials", unsigned, 0u);
PROXY_CONFIG (Interactive, store_price, "SalePrice", nullptr, int, 0);

OBJECT_TYPE_IMPL_ (Interactive, Rendered (),
	PROXY_INIT (frob_world_action),
	PROXY_INIT (frob_inventory_action),
	PROXY_INIT (frob_tool_action),
	PROXY_INIT (pick_distance),
	PROXY_INIT (pick_bias),
	PROXY_INIT (tool_reach),
	PROXY_INIT (droppable),
	PROXY_INIT (limb_model),
	PROXY_INIT (loot_value_gold),
	PROXY_INIT (loot_value_gems),
	PROXY_INIT (loot_value_goods),
	PROXY_INIT (loot_value_special),
	PROXY_INIT (store_price)
)

Interactive::InventoryType
Interactive::get_inventory_type () const
{
	return InventoryType (SInterface<IInventory> (LG)->GetType (number));
}



// FrobMessage

MESSAGE_WRAPPER_IMPL (FrobMessage, sFrobMsg)

FrobMessage::FrobMessage (Event event, const Object& frobber, const Object& tool,
		const Object& frobbed, Location frob_loc, Location obj_loc,
		Time duration, bool aborted)
	: Message (new sFrobMsg ())
{
	switch (event)
	{
	case BEGIN:
		switch (frob_loc)
		{
		case INVENTORY: message->message = "FrobInvBegin"; break;
		case TOOL: message->message = "FrobToolBegin"; break;
		case WORLD: default: message->message = "FrobWorldBegin"; break;
		}
		break;
	case END:
	default:
		switch (frob_loc)
		{
		case INVENTORY: message->message = "FrobInvEnd"; break;
		case TOOL: message->message = "FrobToolEnd"; break;
		case WORLD: default: message->message = "FrobWorldEnd"; break;
		}
		break;
	}

	MESSAGE_AS (sFrobMsg)->Frobber = frobber.number;
	MESSAGE_AS (sFrobMsg)->SrcObjId = tool.number;
	MESSAGE_AS (sFrobMsg)->DstObjId = frobbed.number;
	MESSAGE_AS (sFrobMsg)->SrcLoc = eFrobLoc (frob_loc);
	MESSAGE_AS (sFrobMsg)->DstLoc = eFrobLoc (obj_loc);
	MESSAGE_AS (sFrobMsg)->Sec = duration / 1000.0f;
	MESSAGE_AS (sFrobMsg)->Abort = aborted;
}


MESSAGE_ACCESSOR (Being, FrobMessage, get_frobber, sFrobMsg, Frobber);
MESSAGE_ACCESSOR (Interactive, FrobMessage, get_tool, sFrobMsg, SrcObjId);
MESSAGE_ACCESSOR (Interactive, FrobMessage, get_frobbed, sFrobMsg, DstObjId);
MESSAGE_ACCESSOR (FrobMessage::Location, FrobMessage, get_frob_loc,
	sFrobMsg, SrcLoc);
MESSAGE_ACCESSOR (FrobMessage::Location, FrobMessage, get_obj_loc,
	sFrobMsg, DstLoc);
MESSAGE_ACCESSOR (bool, FrobMessage, was_aborted, sFrobMsg, Abort);

FrobMessage::Event
FrobMessage::get_event () const
{
	CIString name = get_name ();
	size_t length = name.length ();
	if (length > 5 && name.compare (length - 5, 5, "Begin") == 0)
		return BEGIN;
	else if (length > 3 && name.compare (length - 3, 3, "End") == 0)
		return END;
	else
		throw MessageWrapError (message, typeid (*this), "invalid event");
}

Time
FrobMessage::get_duration () const
{
	return MESSAGE_AS (sFrobMsg)->Sec * 1000ul;
}



// Being
//TODO wrap property: AI: Utility\Visibility control = AI_VisCtrl

THIEF_ENUM_CODING (Being::Team, CODE, CODE,
	THIEF_ENUM_VALUE (GOOD, "good"),
	THIEF_ENUM_VALUE (NEUTRAL, "neutral"),
	THIEF_ENUM_VALUE (BAD_1, "bad1", "bad_1", "bad 1", "bad-1", "evil"),
	THIEF_ENUM_VALUE (BAD_2, "bad2", "bad_2", "bad 2", "bad-2"),
	THIEF_ENUM_VALUE (BAD_3, "bad3", "bad_3", "bad 3", "bad-3"),
	THIEF_ENUM_VALUE (BAD_4, "bad4", "bad_4", "bad 4", "bad-4"),
	THIEF_ENUM_VALUE (BAD_5, "bad5", "bad_5", "bad 5", "bad-5", "undead"),
)

PROXY_CONFIG (Being, team, "AI_Team", nullptr, Being::Team, Team::GOOD);
PROXY_CONFIG (Being, culpable, "Culpable", nullptr, bool, false);
PROXY_CONFIG (Being, blood_type, "BloodType", nullptr, String, "");
PROXY_CONFIG (Being, current_breath, "AirSupply", nullptr, Time, 0ul);
PROXY_CONFIG (Being, maximum_breath, "BreathConfig", "Max Air (ms)", Time, 0ul);
PROXY_CONFIG (Being, breath_recovery_rate, "BreathConfig", "Recover rate",
	float, 0.0f);
PROXY_CONFIG (Being, drowning_damage, "BreathConfig", "Drown Damage", int, 0);
PROXY_CONFIG (Being, drowning_frequency, "BreathConfig", "Drown Freq (ms)",
	Time, 0ul);

OBJECT_TYPE_IMPL_ (Being, Physical (), SpherePhysical (), Damageable (),
	PROXY_INIT (team),
	PROXY_INIT (culpable),
	PROXY_INIT (blood_type),
	PROXY_INIT (current_breath),
	PROXY_INIT (maximum_breath),
	PROXY_INIT (breath_recovery_rate),
	PROXY_INIT (drowning_damage),
	PROXY_INIT (drowning_frequency)
)

bool
Being::is_being () const
{
	return culpable.exists ();
}



// Container

THIEF_ENUM_CODING (Container::Type, CODE, CODE,
	THIEF_ENUM_VALUE (ALTERNATE, "alternate", "alt"),
	THIEF_ENUM_VALUE (HAND, "hand"),
	THIEF_ENUM_VALUE (BELT, "belt"),
	THIEF_ENUM_VALUE (GENERIC, "generic"),
)

PROXY_CONFIG (Container, inherit_contains, "ContainInherit", nullptr,
	bool, false);
PROXY_CONFIG (Container, is_loadout_store, "ItemStore", nullptr, bool, false);

OBJECT_TYPE_IMPL_ (Container,
	PROXY_INIT (inherit_contains),
	PROXY_INIT (is_loadout_store)
)

bool
Container::contains (const Object& maybe_contained, bool direct_only)
{
	return direct_only
		? get_contain_type (maybe_contained) != Type::NONE
		: SInterface<IContainSys> (LG)->Contains
			(number, maybe_contained.number);
}

Container::Type
Container::get_contain_type (const Object& maybe_contained)
{
	return Type (SInterface<IContainSys> (LG)->IsHeld
		(number, maybe_contained.number));
}

Container::Contents
Container::get_contents () const
{
	Contents contents;
	sContainIter* iter = SInterface<IContainSys> (LG)->IterStart (number);
	do
		contents.push_back
			({ iter->Object, Type (iter->ContainType), iter->Link });
	while (SInterface<IContainSys> (LG)->IterNext (iter));
	SInterface<IContainSys> (LG)->IterEnd (iter);
	return contents;
}

bool
Container::add_contents (const Object& contained, Type type, bool combine)
{
	return SService<IContainSrv> (LG)->Add
		(contained.number, number, int (type), combine) == S_OK;
}

void
Container::remove_contents (const Object& contained)
{
	SService<IContainSrv> (LG)->Remove (contained.number, number);
}

void
Container::move_contents (const Object& new_container, bool combine)
{
	SService<IContainSrv> (LG)->MoveAllContents
		(number, new_container.number, combine);
}



// ContainsLink

PROXY_CONFIG (ContainsLink, type, nullptr, nullptr,
	Container::Type, Container::Type::NONE);

FLAVORED_LINK_IMPL_ (Contains,
	PROXY_INIT (type)
)

ContainsLink
ContainsLink::create (const Object& source, const Object& dest,
	Container::Type type)
{
	ContainsLink link = Link::create (flavor (), source, dest);
	link.type = type;
	return link;
}



// ContainmentMessage

// "Contained" and "Container" both report as "sScrMsg", so neither can be
// tested by type.
MESSAGE_WRAPPER_IMPL_ (ContainmentMessage,
	MESSAGE_NAME_TEST ("Container") || MESSAGE_NAME_TEST ("Contained"))

ContainmentMessage::ContainmentMessage (Subject subject, Event event,
		const Object& container, const Object& contents)
	: Message ((subject == CONTAINER)
		? static_cast<sScrMsg*> (new sContainerScrMsg ())
		: static_cast<sScrMsg*> (new sContainedScrMsg ()))
{
	if (subject == CONTAINER)
	{
		message->message = "Container";
		MESSAGE_AS (sContainerScrMsg)->event = event;
		MESSAGE_AS (sContainerScrMsg)->containee = contents.number;
		// Caller may send/post the message only to the container.
	}
	else // CONTENTS
	{
		message->message = "Contained";
		MESSAGE_AS (sContainedScrMsg)->event = event;
		MESSAGE_AS (sContainedScrMsg)->container = container.number;
		// Caller may send/post the message only to the contents.
	}
}

ContainmentMessage::Subject
ContainmentMessage::get_subject () const
{
	return (_stricmp (message->message, "Container") == 0)
		? CONTAINER : CONTENTS;
}

MESSAGE_ACCESSOR (ContainmentMessage::Event, ContainmentMessage, get_event,
	sContainedScrMsg, event) // the cast will work either way

Container
ContainmentMessage::get_container () const
{
	return (get_subject () == CONTAINER) ? message->to
		: MESSAGE_AS (sContainedScrMsg)->container;
}

Object
ContainmentMessage::get_contents () const
{
	return (get_subject () == CONTENTS) ? message->to
		: MESSAGE_AS (sContainerScrMsg)->containee;
}



// Marker

PROXY_CONFIG (Marker, flee_value, "AI_FleePoint", nullptr, int, 0);
PROXY_CONFIG (Marker, cover_value, "AICoverPt", "Value", int, 0);
PROXY_CONFIG (Marker, cover_decay_speed, "AICoverPt", "Decay Speed",
	float, 0.8f);
PROXY_CONFIG (Marker, cover_can_duck, "AICoverPt", "Can Duck", bool, false);
PROXY_CONFIG (Marker, vantage_value, "AIVantagePt", "Value", int, 0);
PROXY_CONFIG (Marker, vantage_decay_speed, "AIVantagePt", "Decay Speed",
	float, 0.8f);

OBJECT_TYPE_IMPL_ (Marker,
	PROXY_INIT (flee_value),
	PROXY_INIT (cover_value),
	PROXY_INIT (cover_decay_speed),
	PROXY_INIT (cover_can_duck),
	PROXY_INIT (vantage_value),
	PROXY_INIT (vantage_decay_speed)
)



// Readable

PROXY_CONFIG (Readable, book_name, "Book", nullptr, String, "");
PROXY_CONFIG (Readable, book_art, "BookArt", nullptr, String, "");

OBJECT_TYPE_IMPL_ (Readable, Rendered (), Interactive (),
	PROXY_INIT (book_name),
	PROXY_INIT (book_art)
)

String
Readable::get_book_text (int page)
{
	return Mission::get_book_text (book_name, page);
}

bool
Readable::show_book (bool use_art, Time duration)
{
	String book = book_name, art = book_art;
	if (book.empty ()) return false;

	if (use_art && !art.empty ())
		Mission::show_book (book, art);
	else
	{
		String text = Mission::get_book_text (book);
		if (text.empty ()) return false;
		Mission::show_text (text, duration);
	}

	return true;
}



// Room
//TODO wrap property (if useful): Room\Acoustics = Acoustics
//TODO wrap property (if useful): Room\Ambient = Ambient
//TODO wrap property (if useful): Room\Loud Room = LoudRoom

PROXY_CONFIG (Room, ambient_light_zone, "Weather", "ambient lighting",
	AmbientLightZone, AmbientLightZone::GLOBAL);
PROXY_CONFIG (Room, automap_page, "Automap", "Page", int, 0);
PROXY_CONFIG (Room, automap_location, "Automap", "Location", int, 0);
PROXY_CONFIG (Room, environment_map_zone, "RoomRend", "Env Zone",
	EnvironmentMapZone, EnvironmentMapZone::GLOBAL);
PROXY_CONFIG_ (Room, fog_zone, "Weather", "fog", Fog::Zone, Fog::Zone (0), 0u,
	[] (const Fog::Zone& zone) { return Fog::Zone (zone - 1); },
	[] (const Fog::Zone& zone) { return Fog::Zone (zone + 1); });
PROXY_CONFIG (Room, gravity, "RoomGrav", nullptr, int, 100);
PROXY_CONFIG (Room, precipitation, "Weather", "precipitation", bool, false);
PROXY_BIT_CONFIG (Room, see_through, "RoomRend", "Flags", 1u, false);

OBJECT_TYPE_IMPL_ (Room,
	PROXY_INIT (ambient_light_zone),
	PROXY_INIT (automap_page),
	PROXY_INIT (automap_location),
	PROXY_INIT (environment_map_zone),
	PROXY_INIT (fog_zone),
	PROXY_INIT (gravity),
	PROXY_INIT (precipitation),
	PROXY_INIT (see_through)
)

bool
Room::is_room () const
{
	return inherits_from (Object ("Base Room")); // hopefully not renamed
}



// RoomMessage

MESSAGE_WRAPPER_IMPL (RoomMessage, sRoomMsg)

RoomMessage::RoomMessage (Event event, ObjectType object_type,
		const Object& object, const Object& from_room,
		const Object& to_room)
	: Message (new sRoomMsg ())
{
	switch (event)
	{
	case ENTER:
		switch (object_type)
		{
		case PLAYER: message->message = "PlayerRoomEnter"; break;
		case CREATURE: message->message = "CreatureRoomEnter"; break;
		case OBJECT: default: message->message = "ObjectRoomEnter"; break;
		// REMOTE_PLAYER is skipped since we don't support multiplayer.
		}
		break;
	case EXIT:
		switch (object_type)
		{
		case PLAYER: message->message = "PlayerRoomExit"; break;
		case CREATURE: message->message = "CreatureRoomExit"; break;
		case OBJECT: default: message->message = "ObjectRoomExit"; break;
		// REMOTE_PLAYER is skipped since we don't support multiplayer.
		}
		break;
	case TRANSIT:
	default:
		message->message = "ObjRoomTransit";
		break;
	}

	MESSAGE_AS (sRoomMsg)->FromObjId = from_room.number;
	MESSAGE_AS (sRoomMsg)->ToObjId = to_room.number;
	MESSAGE_AS (sRoomMsg)->MoveObjId = object.number;
	MESSAGE_AS (sRoomMsg)->ObjType = sRoomMsg::eObjType (object_type);
	MESSAGE_AS (sRoomMsg)->TransitionType = sRoomMsg::eTransType (event);
}

MESSAGE_ACCESSOR (RoomMessage::Event, RoomMessage, get_event,
	sRoomMsg, TransitionType);
MESSAGE_ACCESSOR (RoomMessage::ObjectType, RoomMessage, get_object_type,
	sRoomMsg, ObjType);
MESSAGE_ACCESSOR (Object, RoomMessage, get_object, sRoomMsg, MoveObjId);
MESSAGE_ACCESSOR (Room, RoomMessage, get_from_room, sRoomMsg, FromObjId);
MESSAGE_ACCESSOR (Room, RoomMessage, get_to_room, sRoomMsg, ToObjId);



// Secret

#ifdef IS_THIEF2

PROXY_BIT_CONFIG (Secret, is_secret, "DarkStat", nullptr, 4u, false); //TODO Adjust secret count when this is changed.

OBJECT_TYPE_IMPL_ (Secret,
	PROXY_INIT (is_secret)
)

void
Secret::find_secret ()
{
	SService<IDarkGameSrv> (LG)->FoundObject (number);
}

#endif // IS_THIEF2




/*TODO Create Texture and wrap these properties:
 * Physics: Terrain\Can Attach = CanAttach
 * Physics: Terrain\Climbability = Climbability
 * Physics: Terrain\Friction = Friction
 * Renderer\Texture Anim Data = AnimTex
 */



} // namespace Thief

