/******************************************************************************
 *  Security.cc
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

namespace Thief {



// Lockable

PROXY_CONFIG (Lockable, region_mask, "KeyDst", "RegionMask", unsigned, 0u);
PROXY_CONFIG (Lockable, lock_number, "KeyDst", "LockID", unsigned, 0u);

OBJECT_TYPE_IMPL_ (Lockable,
	PROXY_INIT (region_mask),
	PROXY_INIT (lock_number)
)

bool
Lockable::is_locked () const
{
	return SService<ILockSrv> (LG)->IsLocked (number);
}

void
Lockable::set_locked (bool locked)
{
	bool have_locker = false;
	for (auto& locker : LockLink::get_all (*this))
	{
		have_locker = true;
		ObjectProperty ("Locked", locker.get_dest ()).set (locked);
	}

	ObjectProperty self_locked ("Locked", *this);
	if (!have_locker)
		self_locked.set (locked);
	else if (self_locked.exists ())
		self_locked.remove ();
}



// LockLink

PROXY_CONFIG (LockLink, interaction, nullptr, nullptr,
	LockLink::Require, Require::ALL);

LINK_FLAVOR_IMPL (Lock,
	PROXY_INIT (interaction)
)

LockLink
LockLink::create (const Object& source, const Object& dest, Require interaction)
{
	LockLink link = Link::create (flavor (), source, dest);
	if (link != Link::NONE)
		link.interaction = interaction;
	return link;
}



// Key

PROXY_CONFIG (Key, master_key, "KeySrc", "MasterBit", bool, false);
PROXY_CONFIG (Key, region_mask, "KeySrc", "RegionMask", unsigned, 0u);
PROXY_CONFIG (Key, lock_number, "KeySrc", "LockID", unsigned, 0u);

OBJECT_TYPE_IMPL_ (Key,
	Physical (), SpherePhysical (), Rendered (), SoundSource (),
	Interactive (), Combinable (), Damageable (),
	PROXY_INIT (master_key),
	PROXY_INIT (region_mask),
	PROXY_INIT (lock_number)
)

bool
Key::is_key () const
{
	return lock_number.exists ();
}

bool
Key::try_key_operation (Operation operation, const Lockable& lock)
{
	return SService<IKeySrv> (LG)->TryToUseKey
		(number, lock.number, eKeyUse (operation));
}



// Door

THIEF_ENUM_CODING (Door::State, CODE, CODE,
	THIEF_ENUM_VALUE (CLOSED, "closed"),
	THIEF_ENUM_VALUE (OPEN, "open"),
	THIEF_ENUM_VALUE (CLOSING, "closing"),
	THIEF_ENUM_VALUE (OPENING, "opening"),
	THIEF_ENUM_VALUE (HALTED, "halted"),
)

PROXY_CONFIG (Door, static_light_position, "DoorStaticLight", nullptr,
	bool, false);

OBJECT_TYPE_IMPL_ (Door,
	Physical (), OBBPhysical (), Rendered (), SoundSource (),
	Interactive (), Damageable (), Lockable (),
	PROXY_INIT (static_light_position))

bool
Door::is_door () const
{
	return get_door_state () != Door::State::INVALID;
}

Door::State
Door::get_door_state () const
{
	return State (SService<IDoorSrv> (LG)->GetDoorState (number));
}

bool
Door::open_door ()
{
	return SService<IDoorSrv> (LG)->OpenDoor (number);
}

bool
Door::close_door ()
{
	return SService<IDoorSrv> (LG)->CloseDoor (number);
}

bool
Door::toggle_door ()
{
	return SService<IDoorSrv> (LG)->ToggleDoor (number);
}

#ifndef IS_OSL

bool
Door::get_blocks_sound () const
{
	return SService<IDoorSrv> (LG)->GetSoundBlocking (number);
}

void
Door::set_blocks_sound (bool blocks_sound)
{
	SService<IDoorSrv> (LG)->SetBlocking (number, blocks_sound);
}

#endif // !IS_OSL

static Room
Door_room_getter (const FieldProxyConfig<Room>::Item& item,
	const LGMultiBase& multi)
{
	if (multi.empty ()) return item.default_value;
	Object value = reinterpret_cast<const LGMulti<Object>&> (multi);
	return (value == -1) ? Object::NONE : value;
}



// DoorMessage

inline sDoorMsg::eDoorAction
translate_door_state (Door::State state)
{
	switch (state)
	{
	case Door::State::CLOSED: return sDoorMsg::kDoorClose;
	case Door::State::OPEN: return sDoorMsg::kDoorOpen;
	case Door::State::CLOSING: return sDoorMsg::kDoorClosing;
	case Door::State::OPENING: return sDoorMsg::kDoorOpening;
	case Door::State::HALTED: default: return sDoorMsg::kDoorHalt;
	}
}

inline Door::State
translate_door_action (sDoorMsg::eDoorAction action)
{
	switch (action)
	{
	case sDoorMsg::kDoorClose: return Door::State::CLOSED;
	case sDoorMsg::kDoorOpen: return Door::State::OPEN;
	case sDoorMsg::kDoorClosing: return Door::State::CLOSING;
	case sDoorMsg::kDoorOpening: return Door::State::OPENING;
	case sDoorMsg::kDoorHalt: return Door::State::HALTED;
	default: return Door::State::INVALID;
	}
}

MESSAGE_WRAPPER_IMPL_ (DoorMessage,
		MESSAGE_NAME_TEST ("DoorClose") ||
		MESSAGE_NAME_TEST ("DoorOpen") ||
		MESSAGE_NAME_TEST ("DoorClosing") ||
		MESSAGE_NAME_TEST ("DoorOpening") ||
		MESSAGE_NAME_TEST ("DoorHalt")),
	new_state (translate_door_action (MESSAGE_AS (sDoorMsg)->ActionType)),
	old_state (translate_door_action (MESSAGE_AS (sDoorMsg)->PrevActionType))
{
	if (new_state == Door::State::INVALID ||
	    old_state == Door::State::INVALID)
		throw MessageWrapError (message, "DoorMessage", "invalid action");
}

DoorMessage::DoorMessage (Door::State _new_state, Door::State _old_state)
	: Message (new sDoorMsg ()), new_state (_new_state),
	  old_state (_old_state)
{
	switch (new_state)
	{
	case Door::State::CLOSED: message->message = "DoorClose"; break;
	case Door::State::OPEN: message->message = "DoorOpen"; break;
	case Door::State::CLOSING: message->message = "DoorClosing"; break;
	case Door::State::OPENING: message->message = "DoorOpening"; break;
	case Door::State::HALTED: default: message->message = "DoorHalt"; break;
	}

	MESSAGE_AS (sDoorMsg)->ActionType = translate_door_state (new_state);
	MESSAGE_AS (sDoorMsg)->PrevActionType = translate_door_state (old_state);
}



// RotatingDoor

PROXY_CONFIG (RotatingDoor, axis, "RotDoor", "Axis", Door::Axis, Axis::X);
PROXY_CONFIG (RotatingDoor, initial_angle, "RotDoor", "Closed Angle",
	float, 0.0f);
PROXY_CONFIG (RotatingDoor, open_angle, "RotDoor", "Open Angle",
	float, 0.0f);
PROXY_CONFIG (RotatingDoor, clockwise, "RotDoor", "Clockwise?", bool, false);
PROXY_CONFIG (RotatingDoor, speed, "RotDoor", "Base Speed", float, 0.0f);
PROXY_CONFIG (RotatingDoor, push_mass, "RotDoor", "Push Mass",
	float, 25.0f);
PROXY_CONFIG (RotatingDoor, blocks_vision, "RotDoor", "Blocks Vision?",
	bool, true);
PROXY_CONFIG (RotatingDoor, blocks_sound_pct, "RotDoor", "Blocks Sound %",
	float, 60.0f);
PROXY_ARRAY_CONFIG_ (RotatingDoor, room, 2u, Room, Door_room_getter, nullptr,
	PROXY_ARRAY_ITEM ("RotDoor", "Room ID #1", Object::NONE),
	PROXY_ARRAY_ITEM ("RotDoor", "Room ID #2", Object::NONE)
);

OBJECT_TYPE_IMPL_ (RotatingDoor,
	Physical (), OBBPhysical (), Rendered (), SoundSource (),
	Interactive (), Damageable (), Lockable (), Door (),
	PROXY_INIT (axis),
	PROXY_INIT (initial_angle),
	PROXY_INIT (open_angle),
	PROXY_INIT (clockwise),
	PROXY_INIT (speed),
	PROXY_INIT (push_mass),
	PROXY_INIT (blocks_vision),
	PROXY_INIT (blocks_sound_pct),
	PROXY_ARRAY_INIT (room, 2)
)

bool
RotatingDoor::is_rotating_door () const
{
	return axis.exists ();
}



// TranslatingDoor

PROXY_CONFIG (TranslatingDoor, axis, "TransDoor", "Axis", Door::Axis, Axis::X);
PROXY_CONFIG (TranslatingDoor, initial_position, "TransDoor", "Closed Position",
	float, 0.0f);
PROXY_CONFIG (TranslatingDoor, open_position, "TransDoor", "Open Position",
	float, 0.0f);
PROXY_CONFIG (TranslatingDoor, speed, "TransDoor", "Base Speed", float, 0.0f);
PROXY_CONFIG (TranslatingDoor, push_mass, "TransDoor", "Push Mass",
	float, 25.0f);
PROXY_CONFIG (TranslatingDoor, blocks_vision, "TransDoor", "Blocks Vision?",
	bool, true);
PROXY_CONFIG (TranslatingDoor, blocks_sound_pct, "TransDoor", "Blocks Sound %",
	float, 60.0f);
PROXY_ARRAY_CONFIG_ (TranslatingDoor, room, 2u, Room, Door_room_getter, nullptr,
	PROXY_ARRAY_ITEM ("TransDoor", "Room ID #1", Object::NONE),
	PROXY_ARRAY_ITEM ("TransDoor", "Room ID #2", Object::NONE)
);

OBJECT_TYPE_IMPL_ (TranslatingDoor,
	Physical (), OBBPhysical (), Rendered (), SoundSource (),
	Interactive (), Damageable (), Lockable (), Door (),
	PROXY_INIT (axis),
	PROXY_INIT (initial_position),
	PROXY_INIT (open_position),
	PROXY_INIT (speed),
	PROXY_INIT (push_mass),
	PROXY_INIT (blocks_vision),
	PROXY_INIT (blocks_sound_pct),
	PROXY_ARRAY_INIT (room, 2)
)

bool
TranslatingDoor::is_translating_door () const
{
	return axis.exists ();
}



// Pickable

PROXY_CONFIG (Pickable, current_stage, "PickState", "CurTumbler/State", int, 0);
PROXY_CONFIG (Pickable, current_pin, "PickState", "Pin", int, 0);
PROXY_CONFIG (Pickable, picked, "PickState", "Done", bool, false);
PROXY_CONFIG (Pickable, random_time, "PickState", "RandTime", Time, 0ul);
PROXY_CONFIG (Pickable, total_time, "PickState", "TotalTime", Time, 0ul);
PROXY_CONFIG (Pickable, stage_time, "PickState", "StageTime", Time, 0ul);
PROXY_CONFIG (Pickable, picker, "PickState", "Picker", Being, Object::NONE);

OBJECT_TYPE_IMPL_ (Pickable,
	Rendered (), SoundSource (), Interactive (), Lockable (),
	PROXY_INIT (current_stage),
	PROXY_INIT (current_pin),
	PROXY_INIT (picked),
	PROXY_INIT (random_time),
	PROXY_INIT (total_time),
	PROXY_INIT (stage_time),
	PROXY_INIT (picker)
)

bool
Pickable::is_pickable () const
{
	return BasicPickable (*this).is_basic_pickable () ||
		AdvPickable (*this).is_advanced_pickable;
}



// BasicPickable

PROXY_ARRAY_CONFIG (BasicPickable, pick_bits, 3u, unsigned,
	PROXY_ARRAY_ITEM ("PickCfg", "LockBits 1", 0u),
	PROXY_ARRAY_ITEM ("PickCfg", "LockBits 2", 0u),
	PROXY_ARRAY_ITEM ("PickCfg", "LockBits 3", 0u));
PROXY_ARRAY_CONFIG (BasicPickable, pin_count, 3u, int,
	PROXY_ARRAY_ITEM ("PickCfg", "Pins 1", 0),
	PROXY_ARRAY_ITEM ("PickCfg", "Pins 2", 0),
	PROXY_ARRAY_ITEM ("PickCfg", "Pins 3", 0));
PROXY_CONV_ARRAY_CONFIG (BasicPickable, time_percent, 3u, float, int,
	PROXY_ARRAY_ITEM ("PickCfg", "TimePct 1", 0.0f),
	PROXY_ARRAY_ITEM ("PickCfg", "TimePct 2", 0.0f),
	PROXY_ARRAY_ITEM ("PickCfg", "TimePct 3", 0.0f));
PROXY_BIT_ARRAY_CONFIG (BasicPickable, reset_on_fail, 3u, bool,
	PROXY_BIT_ARRAY_ITEM ("PickCfg", "Flags 1", false, 1u),
	PROXY_BIT_ARRAY_ITEM ("PickCfg", "Flags 2", false, 1u),
	PROXY_BIT_ARRAY_ITEM ("PickCfg", "Flags 3", false, 1u));
PROXY_BIT_ARRAY_CONFIG (BasicPickable, randomize_time, 3u, bool,
	PROXY_BIT_ARRAY_ITEM ("PickCfg", "Flags 1", false, 2u),
	PROXY_BIT_ARRAY_ITEM ("PickCfg", "Flags 2", false, 2u),
	PROXY_BIT_ARRAY_ITEM ("PickCfg", "Flags 3", false, 2u));

OBJECT_TYPE_IMPL_ (BasicPickable,
	Rendered (), SoundSource (), Interactive (), Lockable (), Pickable (),
	PROXY_ARRAY_INIT (pick_bits, 3),
	PROXY_ARRAY_INIT (pin_count, 3),
	PROXY_ARRAY_INIT (time_percent, 3),
	PROXY_ARRAY_INIT (reset_on_fail, 3),
	PROXY_ARRAY_INIT (randomize_time, 3)
)

bool
BasicPickable::is_basic_pickable () const
{
	return pick_bits [0u].exists () &&
		!AdvPickable (*this).is_advanced_pickable;
}



// AdvPickable
//TODO wrap property: Dark Gamesys\AdvPickSoundCfg = AdvPickSoundCfg (propdefs.h: sAdvPickSoundCfg)
//TODO wrap property: Dark Gamesys\AdvPickTransCfg = AdvPickTransCfg (propdefs.h: sAdvPickTransCfg)

PROXY_CONFIG (AdvPickable, is_advanced_pickable, "AdvPickStateCfg",
	"Enable Advanced System", bool, false);
PROXY_ARRAY_CONFIG (AdvPickable, pick_bits, 3u, unsigned,
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "First Pick Src", 0u),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Second Pick Src", 0u),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Third Pick Src", 0u));
PROXY_ARRAY_CONFIG (AdvPickable, stage_time, 9u, Time,
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 0", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 1", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 2", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 3", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 4", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 5", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 6", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 7", 0ul),
	PROXY_ARRAY_ITEM ("AdvPickStateCfg", "Time 8", 0ul));

OBJECT_TYPE_IMPL_ (AdvPickable,
	Rendered (), SoundSource (), Interactive (), Lockable (), Pickable (),
	PROXY_INIT (is_advanced_pickable),
	PROXY_ARRAY_INIT (pick_bits, 3),
	PROXY_ARRAY_INIT (stage_time, 9)
)



// PickStateMessage

MESSAGE_WRAPPER_IMPL (PickMessage, "PickStateChange"),
	new_stage (AdvPickable::Stage (MESSAGE_AS (sPickStateScrMsg)->NewState)),
	old_stage (AdvPickable::Stage (MESSAGE_AS (sPickStateScrMsg)->PrevState))
{}

PickMessage::PickMessage (AdvPickable::Stage _new_stage,
		AdvPickable::Stage _old_stage)
	: Message (new sPickStateScrMsg ()), new_stage (_new_stage),
	  old_stage (_old_stage)
{
	message->message = "PickStateChange";
	MESSAGE_AS (sPickStateScrMsg)->NewState = int (new_stage);
	MESSAGE_AS (sPickStateScrMsg)->PrevState = int (old_stage);
}



// Lockpick
//TODO wrap link: NowPicking - int (meaningful data??)

PROXY_CONFIG (Lockpick, pick_bits, "PickSrc", "PickBits", unsigned, 0u);

OBJECT_TYPE_IMPL_ (Lockpick, Rendered (), SoundSource (), Interactive (),
	PROXY_INIT (pick_bits)
)

bool
Lockpick::is_lockpick () const
{
	return pick_bits.exists ();
}

bool
Lockpick::prepare_pick (const Object& host)
{
	return SService<IPickLockSrv> (LG)->Ready (host.number, number);
}

bool
Lockpick::release_pick (const Object& host)
{
	return SService<IPickLockSrv> (LG)->UnReady (host.number, number);
}

bool
Lockpick::can_pick (const Lockable& lock) const
{
	// Only values 0, 1, 4, and 5 were seen in testing. Only 1 clearly
	// corresponds to a good result for a BasicPickable. This method does
	// not work (always returns 4/5) for an AdvPickable.
	// 0: (unknown)
	// 1: can pick at current stage
	// 4: locked, cannot pick
	// 5: unlocked
	return SService<IPickLockSrv> (LG)->CheckPick (number, lock.number, 0)
		== 1;
}

bool
Lockpick::start_picking (const Lockable& lock, const Object& host)
{
	return SService<IPickLockSrv> (LG)->StartPicking
		(host.number, number, lock.number);
}

bool
Lockpick::finish_picking ()
{
	return SService<IPickLockSrv> (LG)->FinishPicking (number);
}



} // namespace Thief

