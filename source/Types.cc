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
MESSAGE_WRAPPER_IMPL_ (CombineMessage, MESSAGE_NAME_TEST ("Combine")),
	stack (MESSAGE_AS (sCombineScrMsg)->combiner)
{}

CombineMessage::CombineMessage (const Object& _stack)
	: Message (new sCombineScrMsg ()),
	  stack (_stack)
{
	message->message = "Combine";
	MESSAGE_AS (sCombineScrMsg)->combiner = stack.number;
}



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
Damageable::damage (const Object& stimulus, int intensity, const Object& culprit)
{
	SService<IDamageSrv> (LG)->Damage
		(number, culprit.number, intensity, stimulus.number);
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

MESSAGE_WRAPPER_IMPL (DamageMessage, sDamageScrMsg),
	culprit (MESSAGE_AS (sDamageScrMsg)->culprit),
	stimulus (MESSAGE_AS (sDamageScrMsg)->kind),
	hit_points (MESSAGE_AS (sDamageScrMsg)->damage)
{}

DamageMessage::DamageMessage (const Object& _culprit, const Object& _stimulus,
		int _hit_points)
	: Message (new sDamageScrMsg ()),
	  culprit (_culprit),
	  stimulus (_stimulus),
	  hit_points (_hit_points)
{
	message->message = "Damage";
	MESSAGE_AS (sDamageScrMsg)->culprit = culprit.number;
	MESSAGE_AS (sDamageScrMsg)->kind = stimulus.number;
	MESSAGE_AS (sDamageScrMsg)->damage = hit_points;
}



// SlayMessage

MESSAGE_WRAPPER_IMPL (SlayMessage, sSlayMsg),
	culprit (MESSAGE_AS (sSlayMsg)->culprit),
	stimulus (MESSAGE_AS (sSlayMsg)->kind)
{}

SlayMessage::SlayMessage (const Object& _culprit, const Object& _stimulus)
	: Message (new sSlayMsg ()),
	  culprit (_culprit),
	  stimulus (_stimulus)
{
	message->message = "Slain";
	MESSAGE_AS (sSlayMsg)->culprit = culprit.number;
	MESSAGE_AS (sSlayMsg)->kind = stimulus.number;
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



// Interactive
//TODO wrap link: FrobProxy - FrobProxyInfo

PROXY_CONFIG (Interactive, frob_world_action, "FrobInfo", "World Action",
	unsigned, FrobAction::INERT);
PROXY_CONFIG (Interactive, frob_inventory_action, "FrobInfo", "Inventory Action",
	unsigned, FrobAction::INERT);
PROXY_CONFIG (Interactive, frob_tool_action, "FrobInfo", "Tool Action",
	unsigned, FrobAction::INERT);
PROXY_CONFIG (Interactive, pick_distance, "PickDist", nullptr, float, 0.0f);
PROXY_CONFIG (Interactive, pick_bias, "PickBias", nullptr, float, 0.0f);
PROXY_CONFIG (Interactive, tool_reach, "ToolReach", nullptr, float, 0.0f);
PROXY_CONFIG (Interactive, inventory_type, "InvType", nullptr,
	Interactive::InventoryType, InventoryType::JUNK);
PROXY_NEG_CONFIG (Interactive, droppable, "NoDrop", nullptr, bool, true);
PROXY_CONFIG (Interactive, cycle_order, "InvCycleOrder", nullptr, String, "");
PROXY_CONFIG (Interactive, inventory_display, "InvRendType", "Type",
	Interactive::InventoryDisplay, InventoryDisplay::DEFAULT);
PROXY_CONFIG (Interactive, alt_resource, "InvRendType", "Resource", String, "");
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
	PROXY_INIT (inventory_type),
	PROXY_INIT (droppable),
	PROXY_INIT (cycle_order),
	PROXY_INIT (inventory_display),
	PROXY_INIT (alt_resource),
	PROXY_INIT (limb_model),
	PROXY_INIT (loot_value_gold),
	PROXY_INIT (loot_value_gems),
	PROXY_INIT (loot_value_goods),
	PROXY_INIT (loot_value_special),
	PROXY_INIT (store_price)
)



// FrobMessage

FrobMessage::Event
FrobMessage::parse (const char* _name)
{
	CIString name = _name ? _name : "";
	size_t length = name.length ();
	if (length > 5 && name.compare (length - 5, 5, "Begin") == 0)
		return Event::BEGIN;
	else if (length > 3 && name.compare (length - 3, 3, "End") == 0)
		return Event::END;
	else
		return Event (-1);
}

MESSAGE_WRAPPER_IMPL (FrobMessage, sFrobMsg),
	event (parse (message->message)),
	frobber (MESSAGE_AS (sFrobMsg)->Frobber),
	tool (MESSAGE_AS (sFrobMsg)->SrcObjId),
	frobbed (MESSAGE_AS (sFrobMsg)->DstObjId),
	frob_loc (Location (MESSAGE_AS (sFrobMsg)->SrcLoc)),
	obj_loc (Location (MESSAGE_AS (sFrobMsg)->DstLoc)),
	duration (MESSAGE_AS (sFrobMsg)->Sec * 1000ul),
	was_aborted (MESSAGE_AS (sFrobMsg)->Abort)
{
	if (int (event) == -1)
		throw MessageWrapError (message, "FrobMessage", "invalid event");
}

FrobMessage::FrobMessage (Event _event, const Object& _frobber,
		const Object& _tool, const Object& _frobbed, Location _frob_loc,
		Location _obj_loc, Time _duration, bool _was_aborted)
	: Message (new sFrobMsg ()), event (_event), frobber (_frobber),
	  tool (_tool), frobbed (_frobbed), frob_loc (_frob_loc),
	  obj_loc (_obj_loc), duration (_duration), was_aborted (_was_aborted)
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
	MESSAGE_AS (sFrobMsg)->Abort = was_aborted;
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
Container::contains (const Object& maybe_contained, bool inherit)
{
	return inherit
		? SInterface<IContainSys> (LG)->Contains
			(number, maybe_contained.number)
		: get_contain_type (maybe_contained) != Type::NONE;
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
	MESSAGE_NAME_TEST ("Container") || MESSAGE_NAME_TEST ("Contained")),
	subject (MESSAGE_NAME_TEST ("Container") ? CONTAINER : CONTENT),
	// The event member will work for either structure.
	event (Event (MESSAGE_AS (sContainedScrMsg)->event)),
	container ((subject == CONTAINER) ? message->to
		: MESSAGE_AS (sContainedScrMsg)->container),
	content ((subject == CONTENT) ? message->to
		: MESSAGE_AS (sContainerScrMsg)->containee)
{}

ContainmentMessage::ContainmentMessage (Subject _subject, Event _event,
		const Object& _container, const Object& _content)
	: Message ((_subject == CONTAINER)
		? static_cast<sScrMsg*> (new sContainerScrMsg ())
		: static_cast<sScrMsg*> (new sContainedScrMsg ())),
	  subject (_subject), event (_event),
	  container (_container), content (_content)
{
	if (subject == CONTAINER)
	{
		message->message = "Container";
		MESSAGE_AS (sContainerScrMsg)->event = event;
		MESSAGE_AS (sContainerScrMsg)->containee = content.number;
		// Caller may send/post the message only to the container.
	}
	else // CONTENT
	{
		message->message = "Contained";
		MESSAGE_AS (sContainedScrMsg)->event = event;
		MESSAGE_AS (sContainedScrMsg)->container = container.number;
		// Caller may send/post the message only to the contents.
	}
}



// Marker

PROXY_CONFIG (Marker, flee_value, "AI_FleePoint", nullptr, int, 0);
PROXY_CONFIG (Marker, cover_value, "AICoverPt", "Value", int, 0);
PROXY_CONFIG (Marker, cover_decay_speed, "AICoverPt", "Decay Speed",
	float, 0.8f); //TODO Should this be a Time?
PROXY_CONFIG (Marker, cover_can_duck, "AICoverPt", "Can Duck", bool, false);
PROXY_CONFIG (Marker, vantage_value, "AIVantagePt", "Value", int, 0);
PROXY_CONFIG (Marker, vantage_decay_speed, "AIVantagePt", "Decay Speed",
	float, 0.8f); //TODO Should this be a Time?

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
Readable::show_book (bool use_art, Time duration, bool reload)
{
	String book = book_name, art = book_art;
	if (book.empty ()) return false;

	if (use_art && !art.empty ())
		Mission::show_book (book, art, reload);
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

static Fog::Zone
Room_fog_zone_getter (const FieldProxyConfig<Fog::Zone>::Item& item,
	const LGMultiBase& multi)
{
	if (multi.empty ()) return item.default_value;
	int value = reinterpret_cast<const LGMulti<int>&> (multi);
	return Fog::Zone (value - 1);
}

static void
Room_fog_zone_setter (const FieldProxyConfig<Fog::Zone>::Item&,
	LGMultiBase& multi, const Fog::Zone& value)
{
	reinterpret_cast<LGMulti<int>&> (multi) = int (value) + 1;
}

PROXY_CONFIG (Room, ambient_light_zone, "Weather", "ambient lighting",
	AmbientLightZone, AmbientLightZone::GLOBAL);
PROXY_CONFIG (Room, automap_page, "Automap", "Page", int, 0);
PROXY_CONFIG (Room, automap_location, "Automap", "Location", int, 0);
PROXY_CONFIG (Room, environment_map_zone, "RoomRend", "Env Zone",
	EnvironmentMapZone, EnvironmentMapZone::GLOBAL);
PROXY_CONFIG_ (Room, fog_zone, "Weather", "fog", Fog::Zone, Fog::Zone::DISABLED,
	0, Room_fog_zone_getter, Room_fog_zone_setter);
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

MESSAGE_WRAPPER_IMPL (RoomMessage, sRoomMsg),
	event (Event (MESSAGE_AS (sRoomMsg)->TransitionType)),
	object_type (ObjectType (MESSAGE_AS (sRoomMsg)->ObjType)),
	object (MESSAGE_AS (sRoomMsg)->MoveObjId),
	from_room (MESSAGE_AS (sRoomMsg)->FromObjId),
	to_room (MESSAGE_AS (sRoomMsg)->ToObjId)
{}

RoomMessage::RoomMessage (Event _event, ObjectType _object_type,
		const Object& _object, const Object& _from_room,
		const Object& _to_room)
	: Message (new sRoomMsg ()), event (_event), object_type (_object_type),
	  object (_object), from_room (_from_room), to_room (_to_room)
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

