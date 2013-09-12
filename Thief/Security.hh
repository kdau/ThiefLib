/******************************************************************************
 *  Security.hh
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

#ifndef THIEF_SECURITY_HH
#define THIEF_SECURITY_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>
#include <Thief/Physics.hh>
#include <Thief/Types.hh>

namespace Thief {



// Lockable

class Lockable : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Lockable)

	THIEF_PROP_FIELD (unsigned, region_mask); //TESTME
	THIEF_PROP_FIELD (unsigned, lock_number); //TESTME

	bool is_locked () const;
	void set_locked (bool); //TESTME with Lock links present
};



// LockLink

THIEF_FLAVORED_LINK (Lock) //TESTME
{
	THIEF_FLAVORED_LINK_COMMON (Lock)

	enum class Require { ALL, ANY };

	static LockLink create (const Object& source, const Object& dest,
		Require interaction);

	THIEF_LINK_FIELD (Require, interaction);
};



// Key

class Key : public virtual SpherePhysical, public virtual Interactive,
	public virtual Combinable, public virtual Damageable
{
public:
	THIEF_OBJECT_TYPE (Key)
	bool is_key () const; //TESTME

	THIEF_PROP_FIELD (bool, master_key); //TESTME
	THIEF_PROP_FIELD (unsigned, region_mask); //TESTME
	THIEF_PROP_FIELD (unsigned, lock_number); //TESTME

	enum class Operation
	{
		DEFAULT,
		OPEN,
		CLOSE,
		CHECK
	};
	bool try_key_operation (Operation, const Lockable& lock); //TESTME
};



// Door

class Door : public virtual OBBPhysical, public virtual Interactive,
	public virtual Damageable, public virtual Lockable
{
public:
	THIEF_OBJECT_TYPE (Door)
	bool is_door () const;

	enum class State { CLOSED, OPEN, CLOSING, OPENING, HALTED, INVALID };
	State get_door_state () const;

	bool open_door ();
	bool close_door ();
	bool toggle_door ();

	bool get_blocks_sound () const;
	void set_blocks_sound (bool);

	THIEF_PROP_FIELD_CONST (bool, static_light_position); //TESTME not const?

	enum class Axis { X, Y, Z };
};

class DoorMessage : public Message // "Door{Close,Open,Closing,Opening,Halt}"
{
public:
	DoorMessage (Door::State new_state, Door::State old_state);
	THIEF_MESSAGE_WRAP (DoorMessage);

	Door::State get_new_state () const;
	Door::State get_old_state () const;
};



// RotatingDoor

class RotatingDoor : public Door
{
public:
	THIEF_OBJECT_TYPE (RotatingDoor)
	bool is_rotating_door () const;

	THIEF_PROP_FIELD (Axis, axis); //TESTME
	THIEF_PROP_FIELD_CONST (float, initial_angle); //TESTME
	THIEF_PROP_FIELD (float, open_angle); //TESTME
	THIEF_PROP_FIELD (bool, clockwise); //TESTME

	THIEF_PROP_FIELD (float, speed);
	THIEF_PROP_FIELD (float, push_mass); //TESTME

	THIEF_PROP_FIELD (bool, blocks_vision); //TESTME needs rebuild?
	THIEF_PROP_FIELD (float, blocks_sound_pct); //TESTME

	THIEF_PROP_FIELD_ARRAY_CONST (Room, room, 2u); //TESTME
};



// TranslatingDoor

class TranslatingDoor : public Door
{
public:
	THIEF_OBJECT_TYPE (TranslatingDoor)
	bool is_translating_door () const;

	THIEF_PROP_FIELD (Axis, axis); //TESTME
	THIEF_PROP_FIELD_CONST (float, initial_position); //TESTME
	THIEF_PROP_FIELD (float, open_position); //TESTME

	THIEF_PROP_FIELD (float, speed);
	THIEF_PROP_FIELD (float, push_mass); //TESTME

	THIEF_PROP_FIELD (bool, blocks_vision); //TESTME needs rebuild?
	THIEF_PROP_FIELD (float, blocks_sound_pct); //TESTME

	THIEF_PROP_FIELD_ARRAY_CONST (Room, room, 2u); //TESTME
};



// Pickable

class Pickable : public virtual Interactive, public virtual Lockable
{
public:
	THIEF_OBJECT_TYPE (Pickable)
	bool is_pickable () const; //TESTME

	THIEF_PROP_FIELD (int, current_stage); //TESTME
	THIEF_PROP_FIELD (int, current_pin); //TESTME
	THIEF_PROP_FIELD (bool, picked); //TESTME

	THIEF_PROP_FIELD (Time, random_time); //TESTME
	THIEF_PROP_FIELD (Time, total_time); //TESTME
	THIEF_PROP_FIELD (Time, stage_time); //TESTME

	THIEF_PROP_FIELD (Being, picker); //TESTME
};



// BasicPickable

class BasicPickable : public Pickable
{
public:
	THIEF_OBJECT_TYPE (BasicPickable)
	bool is_basic_pickable () const; //TESTME

	// A basic-pickable lock has three stages with independent settings.
	THIEF_PROP_FIELD_ARRAY (unsigned, pick_bits, 3u); //TESTME
	THIEF_PROP_FIELD_ARRAY (int, pin_count, 3u); //TESTME
	THIEF_PROP_FIELD_ARRAY (float, time_percent, 3u); //TESTME
	THIEF_PROP_FIELD_ARRAY (bool, reset_on_fail, 3u); //TESTME
	THIEF_PROP_FIELD_ARRAY (bool, randomize_time, 3u); //TESTME
};



// AdvPickable

class AdvPickable : public Pickable
{
public:
	THIEF_OBJECT_TYPE (AdvPickable)
	THIEF_PROP_FIELD (bool, is_advanced_pickable);

	enum class Pick { P1, P2, P3 };
	THIEF_PROP_FIELD_ARRAY (unsigned, pick_bits, 3u); //TESTME

	enum class Stage { S0, S1, S2, S3, S4, S5, S6, S7, UNLOCKED };
	THIEF_PROP_FIELD_ARRAY (Time, stage_time, 9u); //TESTME
};

class PickMessage : public Message // "PickStateChange"
{
public:
	PickMessage (AdvPickable::Stage new_stage, AdvPickable::Stage old_stage);
	THIEF_MESSAGE_WRAP (PickMessage);

	AdvPickable::Stage get_new_stage () const;
	AdvPickable::Stage get_old_stage () const;
};



// Lockpick

class Lockpick : public virtual Interactive
{
public:
	THIEF_OBJECT_TYPE (Lockpick)
	bool is_lockpick () const;

	THIEF_PROP_FIELD (unsigned, pick_bits);

	bool prepare_pick (const Object& host);
	bool release_pick (const Object& host);

	bool can_pick (const Lockable& lock) const; // not for AdvPickable locks
	bool start_picking (const Lockable& lock, const Object& host);
	bool finish_picking ();
};



} // namespace Thief

#endif // THIEF_SECURITY_HH

