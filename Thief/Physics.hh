/******************************************************************************
 *  Physics.hh
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

#ifndef THIEF_PHYSICS_HH
#define THIEF_PHYSICS_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/Property.hh>

namespace Thief {

class Being;
class Marker;



class Physical : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Physical)
	bool is_physical () const;

	// Model type

	enum class PhysicsType { OBB, SPHERE, SPHERE_HAT, NONE };
	THIEF_PROP_FIELD (PhysicsType, physics_type);
	THIEF_PROP_FIELD (unsigned, submodel_count);

	bool remove_physics ();

	// Attributes

	THIEF_PROP_FIELD (float, gravity);
	THIEF_PROP_FIELD (float, mass);
	THIEF_PROP_FIELD (float, density);
	THIEF_PROP_FIELD (float, elasticity);
	THIEF_PROP_FIELD (float, friction);
	THIEF_PROP_FIELD (Vector, center_of_gravity);
	THIEF_PROP_FIELD (bool, mantleable);
	THIEF_PROP_FIELD (bool, remove_on_sleep);

	THIEF_PROP_FIELD (bool, collision_bounce); //TESTME
	THIEF_PROP_FIELD (bool, collision_destroy); //TESTME
	THIEF_PROP_FIELD (bool, collision_slay); //TESTME
	THIEF_PROP_FIELD (bool, collision_no_sound); //TESTME
	THIEF_PROP_FIELD (bool, collision_no_result); //TESTME
	THIEF_PROP_FIELD (bool, collision_full_sound); //TESTME

	THIEF_PROP_FIELD (bool, collides_with_ai);
	THIEF_PROP_FIELD_CONST (bool, blocks_ai); //TESTME
	THIEF_PROP_FIELD (bool, repels_ai); //TESTME

	THIEF_PROP_FIELD (float, bash_factor);
	THIEF_PROP_FIELD (float, bash_threshold);
	THIEF_PROP_FIELD (float, bash_coefficient);

	bool is_climbable () const; //TESTME

	enum Axes
	{
		X_AXIS = 1, Y_AXIS = 2, Z_AXIS = 4,
		POS_X_AXIS = 1, POS_Y_AXIS = 2, POS_Z_AXIS = 4,
		NEG_X_AXIS = 8, NEG_Y_AXIS = 16, NEG_Z_AXIS = 32,
		NO_AXES = 0, ALL_POS_AXES = 7, ALL_AXES = 65
	};

	// Current state

#ifdef IS_THIEF2
	bool is_position_valid () const;

	bool wake_up_physics (); //TESTME
#endif // IS_THIEF2

	THIEF_PROP_FIELD (Vector, velocity);
	THIEF_PROP_FIELD (bool, velocity_locked);
	void lock_velocity (const Vector&);
	void unlock_velocity ();

	THIEF_PROP_FIELD (Vector, rotational_velocity); //TESTME
	THIEF_PROP_FIELD (bool, rotational_velocity_locked); //TESTME
	void lock_rotational_velocity (const Vector&); //TESTME
	void unlock_rotational_velocity (); //TESTME

	THIEF_PROP_FIELD (bool, location_locked);
	THIEF_PROP_FIELD (bool, rotation_locked);

	// Physics messages

	enum Messages
	{
		MSG_NONE = 0,
		MSG_COLLISION = 1,
		MSG_CONTACT = 2,
		MSG_ENTER_EXIT = 4,
		MSG_FALL_ASLEEP = 8,
		MSG_WAKE_UP = 16
	};
	void subscribe_physics (Messages); //TESTME
	void unsubscribe_physics (Messages); //TESTME
};



// OBBPhysical: physical objects with OBB model type

class OBBPhysical : public virtual Physical
{
public:
	THIEF_OBJECT_TYPE (OBBPhysical)

	// Dimensions

	THIEF_PROP_FIELD (Vector, physics_size);
	THIEF_PROP_FIELD (Vector, physics_offset);

	// Attributes

	THIEF_PROP_FIELD (unsigned, climbable_sides); // positive/negative axes //TESTME

	THIEF_PROP_FIELD (bool, edge_trigger); //TESTME

	THIEF_PROP_FIELD (bool, pathable); //TESTME
	THIEF_PROP_FIELD (bool, path_exact); //TESTME

	THIEF_PROP_FIELD (bool, platform_friction); //TESTME

	THIEF_PROP_FIELD (float, pore_size); //TESTME
	THIEF_PROP_FIELD (bool, ai_fires_through); //TESTME
};

THIEF_FLAVORED_LINK (PhysAttach) //TESTME
{
	THIEF_FLAVORED_LINK_COMMON (PhysAttach)

	static PhysAttachLink create (const Object& source, const Object& dest,
		const Vector& offset);

	THIEF_LINK_FIELD (Vector, offset);
};



// MovingTerrain

class MovingTerrain : public virtual OBBPhysical
{
public:
	THIEF_OBJECT_TYPE (MovingTerrain)
	bool is_moving_terrain () const; //TESTME

	THIEF_PROP_FIELD (bool, active); //TESTME

	THIEF_PROP_FIELD (bool, push_attachments); //TESTME
};

THIEF_FLAVORED_LINK (TPath) //TESTME
{
	THIEF_FLAVORED_LINK_COMMON (TPath)

	static TPathLink create (const Object& source, const Object& dest,
		float speed, Time pause, bool path_limit);

	THIEF_LINK_FIELD (float, speed);
	THIEF_LINK_FIELD (Time, pause);
	THIEF_LINK_FIELD (bool, path_limit);
};

class MovingTerrainMessage : public Message //TESTME
{
public:
	MovingTerrainMessage (const Object& waypoint);
	THIEF_MESSAGE_WRAP (MovingTerrainMessage);

	Marker get_waypoint () const;
};

class WaypointMessage : public Message //TESTME
{
public:
	WaypointMessage (const MovingTerrain& moving_terrain);
	THIEF_MESSAGE_WRAP (WaypointMessage);

	MovingTerrain get_moving_terrain () const;
};



// SpherePhysical: physical objects with SPHERE or SPHERE_HAT model types

class SpherePhysical : public virtual Physical
{
public:
	THIEF_OBJECT_TYPE (SpherePhysical)

	// Dimensions

	THIEF_PROP_FIELD (float, physics_radius_1);
	THIEF_PROP_FIELD (float, physics_radius_2);
	THIEF_PROP_FIELD (Vector, physics_offset_1);
	THIEF_PROP_FIELD (Vector, physics_offset_2);

	// Attributes

	THIEF_PROP_FIELD (unsigned, rotation_axes); // non-polar axes //TESTME
	THIEF_PROP_FIELD (unsigned, rest_axes); // positive/negative axes //TESTME
};



// Explosion

class Explosion : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Explosion)
	bool is_explosion () const; //TESTME

	THIEF_PROP_FIELD (float, radius_squared); //TESTME
	THIEF_PROP_FIELD (int, magnitude); //TESTME

	static void explode (const Vector& center, float radius_squared,
		int magnitude); //TESTME
};



// Projectile

class Projectile : public virtual SpherePhysical
{
public:
	THIEF_OBJECT_TYPE (Projectile)
	bool is_projectile () const; //TESTME

	THIEF_PROP_FIELD (Vector, initial_velocity); //TESTME
	THIEF_PROP_FIELD (bool, faces_velocity); //TESTME

	THIEF_PROP_FIELD (String, whizzing_sound); //TESTME

	enum LaunchFlags
	{
		NONE = 0,
		ZERO_VELOCITY = 1,
		PUSH_OUT = 2,
		RELATIVE_VELOCITY = 4,
		GRAVITY = 8,
		TELL_AI = 512,
		NO_PHYSICS = 1024
	};
	Projectile launch (const Object& launcher, float velocity_mult = 1.0f,
		const Vector& velocity_add = Vector (), unsigned flags = NONE); //TESTME

	THIEF_PROP_FIELD_CONST (Being, launcher);
};



// Rope

class Rope : public virtual SpherePhysical
{
public:
	THIEF_OBJECT_TYPE (Rope)
	bool is_rope () const; //TESTME

	THIEF_PROP_FIELD (float, desired_length); //TESTME
	THIEF_PROP_FIELD (float, length); //TESTME
	THIEF_PROP_FIELD (bool, deployed); //TESTME
};



} // namespace Thief

#endif // THIEF_PHYSICS_HH

