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
	void lock () { set_locked (true); }
	void unlock () { set_locked (false); }

protected:
	void set_locked (bool locked); //TESTME with Lock links present
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

	THIEF_PROP_FIELD_CONST (bool, static_light_position);
};

class DoorMessage : public Message
{
public:
	DoorMessage (Door::State new_state, Door::State old_state);
	THIEF_MESSAGE_WRAP (DoorMessage);

	Door::State get_new_state () const;
	Door::State get_old_state () const;
};



// BasicPickable

class BasicPickable : public virtual Interactive, public virtual Lockable
{
public:
	THIEF_OBJECT_TYPE (BasicPickable)
	bool is_basic_pickable () const; //TESTME
};



// AdvPickable

class AdvPickable : public virtual Interactive, public virtual Lockable
{
public:
	THIEF_OBJECT_TYPE (AdvPickable)
	THIEF_PROP_FIELD (bool, is_advanced_pickable);

	enum class Stage { S0, S1, S2, S3, S4, S5, S6, S7, UNLOCKED };
};

class PickMessage : public Message
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

