/******************************************************************************
 *  Security.cc
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
		Property (locker.get_dest (), "Locked").set (locked);
	}

	Property self_locked (*this, "Locked");
	if (!have_locker)
		self_locked.set (locked);
	else if (self_locked.exists ())
		self_locked.remove ();
}



// LockLink

PROXY_CONFIG (LockLink, interaction, nullptr, nullptr,
	LockLink::Require, Require::ALL);

FLAVORED_LINK_IMPL_ (Lock,
	PROXY_INIT (interaction)
)

LockLink
LockLink::create (const Object& source, const Object& dest, Require interaction)
{
	LockLink link = Link::create (flavor (), source, dest);
	link.interaction = interaction;
	return link;
}



// Key

PROXY_CONFIG (Key, master_key, "KeySrc", "MasterBit", bool, false);
PROXY_CONFIG (Key, region_mask, "KeySrc", "RegionMask", unsigned, 0u);
PROXY_CONFIG (Key, lock_number, "KeySrc", "LockID", unsigned, 0u);

OBJECT_TYPE_IMPL_ (Key, Physical (), SpherePhysical (), Rendered (),
		Interactive (), Damageable (),
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
//TODO wrap property: Door\Rotating = RotDoor
//TODO wrap property: Door\Static Light Pos = DoorStaticLight
//TODO wrap property: Door\Translating = TransDoor

THIEF_ENUM_CODING (Door::State, CODE, CODE,
	THIEF_ENUM_VALUE (CLOSED, "closed"),
	THIEF_ENUM_VALUE (OPEN, "open"),
	THIEF_ENUM_VALUE (CLOSING, "closing"),
	THIEF_ENUM_VALUE (OPENING, "opening"),
	THIEF_ENUM_VALUE (HALTED, "halted"),
)

OBJECT_TYPE_IMPL_ (Door, Physical (), OBBPhysical (), Rendered (),
	Interactive (), Damageable (), Lockable ())

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



// DoorMessage

MESSAGE_WRAPPER_IMPL (DoorMessage, sDoorMsg)

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
translate_door_action (const DoorMessage& message, const sScrMsg* lgmessage,
	sDoorMsg::eDoorAction action)
{
	switch (action)
	{
	case sDoorMsg::kDoorClose: return Door::State::CLOSED;
	case sDoorMsg::kDoorOpen: return Door::State::OPEN;
	case sDoorMsg::kDoorClosing: return Door::State::CLOSING;
	case sDoorMsg::kDoorOpening: return Door::State::OPENING;
	case sDoorMsg::kDoorHalt: return Door::State::HALTED;
	default: throw MessageWrapError (lgmessage, typeid (message),
		"invalid action");
	}
}

DoorMessage::DoorMessage (Door::State new_state, Door::State old_state)
	: Message (new sDoorMsg ())
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

Door::State
DoorMessage::get_new_state () const
{
	return translate_door_action (*this, message,
		MESSAGE_AS (sDoorMsg)->ActionType);
}

Door::State
DoorMessage::get_old_state () const
{
	return translate_door_action (*this, message,
		MESSAGE_AS (sDoorMsg)->PrevActionType);
}



// BasicPickable
//TODO wrap property: Dark GameSys\PickCfg = PickCfg
//TODO wrap property: Dark GameSys\PickState = PickState (does it apply below too?)

OBJECT_TYPE_IMPL_ (BasicPickable, Rendered (), Interactive (), Lockable ())

bool
BasicPickable::is_basic_pickable () const
{
	return Property (*this, "PickCfg").exists () && //TODO Check one of the PropFields, once created.
		!AdvPickable (*this).is_advanced_pickable;
}



// AdvPickable
//TODO wrap property: Dark Gamesys\AdvPickSoundCfg = AdvPickSoundCfg (propdefs.h: sAdvPickSoundCfg)
//TODO wrap property: Dark Gamesys\AdvPickStateCfg = AdvPickStateCfg (propdefs.h: sAdvPickStateCfg)
//TODO wrap property: Dark Gamesys\AdvPickTransCfg = AdvPickTransCfg (propdefs.h: sAdvPickTransCfg)

PROXY_CONFIG (AdvPickable, is_advanced_pickable, "AdvPickStateCfg",
	"Enable Advanced System", bool, false);

OBJECT_TYPE_IMPL_ (AdvPickable, Rendered (), Interactive (), Lockable (),
	PROXY_INIT (is_advanced_pickable)
)



// PickStateMessage

// "PickStateChange" reports as "sScrMsg", so it can't be tested by type.
MESSAGE_WRAPPER_IMPL_ (PickMessage, MESSAGE_NAME_TEST ("PickStateChange"))

PickMessage::PickMessage (AdvPickable::Stage new_stage,
		AdvPickable::Stage old_stage)
	: Message (new sPickStateScrMsg ())
{
	message->message = "PickStateChange";
	MESSAGE_AS (sPickStateScrMsg)->NewState = int (new_stage);
	MESSAGE_AS (sPickStateScrMsg)->PrevState = int (old_stage);
}

MESSAGE_ACCESSOR (AdvPickable::Stage, PickMessage, get_new_stage,
	sPickStateScrMsg, NewState);
MESSAGE_ACCESSOR (AdvPickable::Stage, PickMessage, get_old_stage,
	sPickStateScrMsg, PrevState);




// Lockpick
//TODO wrap link: NowPicking - int (meaningful data??)

PROXY_CONFIG (Lockpick, pick_bits, "PickSrc", "PickBits", unsigned, 0u);

OBJECT_TYPE_IMPL_ (Lockpick, Rendered (), Interactive (),
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

