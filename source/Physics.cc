/******************************************************************************
 *  Physics.cc
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



// Physical

PROXY_CONFIG (Physical, physics_type, "PhysType", "Type",
	Physical::PhysicsType, PhysicsType::NONE);
PROXY_CONFIG (Physical, submodel_count, "PhysType", "# Submodels", unsigned, 0u);
PROXY_CONFIG (Physical, gravity, "PhysAttr", "Gravity %", float, 100.0f);
PROXY_CONFIG (Physical, mass, "PhysAttr", "Mass", float, 30.0f);
PROXY_CONFIG (Physical, density, "PhysAttr", "Density", float, 1.0f);
PROXY_CONFIG (Physical, elasticity, "PhysAttr", "Elasticity", float, 1.0f);
PROXY_CONFIG (Physical, friction, "PhysAttr", "Base Friction", float, 0.0f);
PROXY_CONFIG (Physical, center_of_gravity, "PhysAttr", "COG Offset", Vector,
	Vector ());
PROXY_CONFIG (Physical, mantleable, "PhysCanMant", nullptr, bool, true);
PROXY_CONFIG (Physical, remove_on_sleep, "PhysType", "Remove on Sleep",
	bool, false);
PROXY_BIT_CONFIG (Physical, collision_bounce, "CollisionType", nullptr, 1u,
	false);
PROXY_BIT_CONFIG (Physical, collision_destroy, "CollisionType", nullptr, 2u,
	false);
PROXY_BIT_CONFIG (Physical, collision_slay, "CollisionType", nullptr, 4u,
	false);
PROXY_BIT_CONFIG (Physical, collision_no_sound, "CollisionType", nullptr, 8u,
	false);
PROXY_BIT_CONFIG (Physical, collision_no_result, "CollisionType", nullptr, 16u,
	false);
PROXY_BIT_CONFIG (Physical, collision_full_sound, "CollisionType", nullptr, 32u,
	false);
PROXY_CONFIG (Physical, collides_with_ai, "PhysAIColl", nullptr, bool, false);
PROXY_BIT_CONFIG (Physical, blocks_ai, "AI_ObjAvoid", "Flags", 1u, false);
PROXY_BIT_CONFIG (Physical, repels_ai, "AI_ObjAvoid", "Flags", 2u, false);
PROXY_CONFIG (Physical, bash_factor, "BashFactor", nullptr, float, 0.0f);
PROXY_CONFIG (Physical, bash_threshold, "BashParams", "Threshold", float, 0.0f);
PROXY_CONFIG (Physical, bash_coefficient, "BashParams", "Coefficient",
	float, 0.0f);
PROXY_CONFIG (Physical, velocity, "PhysState", "Velocity", Vector, Vector ());
PROXY_BIT_CONFIG (Physical, velocity_locked, "PhysControl", "Controls Active",
	PHYS_CONTROL_VELS, false);
PROXY_CONFIG (Physical, velocity_lock_to, "PhysControl", "Velocity",
	Vector, Vector ());
PROXY_CONFIG (Physical, rotational_velocity, "PhysState", "Rot Velocity",
	Vector, Vector ());
PROXY_BIT_CONFIG (Physical, rotational_velocity_locked, "PhysControl",
	"Controls Active", PHYS_CONTROL_ROTVELS, false);
PROXY_CONFIG (Physical, rotational_velocity_lock_to, "PhysControl",
	"RotationalVelocity", Vector, Vector ());
PROXY_BIT_CONFIG (Physical, location_locked, "PhysControl", "Controls Active",
	PHYS_CONTROL_LOCATION, false);
PROXY_BIT_CONFIG (Physical, rotation_locked, "PhysControl", "Controls Active",
	PHYS_CONTROL_ROTATION, false);

OBJECT_TYPE_IMPL_ (Physical,
	PROXY_INIT (physics_type),
	PROXY_INIT (submodel_count),
	PROXY_INIT (gravity),
	PROXY_INIT (mass),
	PROXY_INIT (density),
	PROXY_INIT (elasticity),
	PROXY_INIT (friction),
	PROXY_INIT (center_of_gravity),
	PROXY_INIT (mantleable),
	PROXY_INIT (remove_on_sleep),
	PROXY_INIT (collision_bounce),
	PROXY_INIT (collision_destroy),
	PROXY_INIT (collision_slay),
	PROXY_INIT (collision_no_sound),
	PROXY_INIT (collision_no_result),
	PROXY_INIT (collision_full_sound),
	PROXY_INIT (collides_with_ai),
	PROXY_INIT (blocks_ai),
	PROXY_INIT (repels_ai),
	PROXY_INIT (bash_factor),
	PROXY_INIT (bash_threshold),
	PROXY_INIT (bash_coefficient),
	PROXY_INIT (velocity),
	PROXY_INIT (velocity_locked),
	PROXY_INIT (velocity_lock_to),
	PROXY_INIT (rotational_velocity),
	PROXY_INIT (rotational_velocity_locked),
	PROXY_INIT (rotational_velocity_lock_to),
	PROXY_INIT (location_locked),
	PROXY_INIT (rotation_locked)
)

bool
Physical::is_physical () const
{
#ifdef IS_THIEF2
	return SService<IPhysSrv> (LG)->HasPhysics (number);
#else
	return physics_type.exists ();
#endif
}

bool
Physical::remove_physics ()
{
#ifdef IS_THIEF2
	return SService<IPhysSrv> (LG)->DeregisterModel (number) == S_OK;
#else
	return ObjectProperty ("PhysType", *this).remove ();
#endif
}

bool
Physical::is_climbable () const
{
	return SService<IPhysSrv> (LG)->IsRope (number);
}

#ifdef IS_THIEF2

bool
Physical::is_position_valid () const
{
	return SService<IPhysSrv> (LG)->ValidPos (number);
}

bool
Physical::wake_up_physics ()
{
	return SService<IPhysSrv> (LG)->Activate (number) == S_OK;
}

#endif // IS_THIEF2

void
Physical::subscribe_physics (Messages messages)
{
	SService<IPhysSrv> (LG)->SubscribeMsg (number, messages);
}

void
Physical::unsubscribe_physics (Messages messages)
{
	SService<IPhysSrv> (LG)->UnsubscribeMsg (number, messages);
}



// OBBPhysical

PROXY_CONFIG (OBBPhysical, physics_size, "PhysDims", "Size", Vector, Vector ());
PROXY_CONFIG (OBBPhysical, physics_offset, "PhysDims", "Offset",
	Vector, Vector ());
PROXY_CONFIG (OBBPhysical, climbable_sides, "PhysAttr", "Climbable Sides",
	unsigned, Physical::ALL_AXES);
PROXY_BIT_CONFIG (OBBPhysical, edge_trigger, "PhysAttr", "Flags", 1u, false);
PROXY_CONFIG (OBBPhysical, pathable, "AI_ObjPathable", nullptr, bool, false);
PROXY_CONFIG (OBBPhysical, path_exact, "AI_NGOBB", nullptr, bool, false);
PROXY_BIT_CONFIG (OBBPhysical, platform_friction, "PhysAttr", "Flags", 2u,
	false);
PROXY_CONFIG (OBBPhysical, pore_size, "PhysAttr", "Pore Size", float, 0.0f);
PROXY_CONFIG (OBBPhysical, ai_fires_through, "AIFiresThrough", nullptr,
	bool, false);
PROXY_CONFIG_ (OBBPhysical, conveyor_velocity, "ConveyorVel", nullptr, float,
	0.0f, Vector::Component::X, FieldProxyConfig<float>::component_getter,
	FieldProxyConfig<float>::component_setter);

OBJECT_TYPE_IMPL_ (OBBPhysical, Physical (),
	PROXY_INIT (physics_size),
	PROXY_INIT (physics_offset),
	PROXY_INIT (climbable_sides),
	PROXY_INIT (edge_trigger),
	PROXY_INIT (pathable),
	PROXY_INIT (path_exact),
	PROXY_INIT (platform_friction),
	PROXY_INIT (pore_size),
	PROXY_INIT (ai_fires_through),
	PROXY_INIT (conveyor_velocity)
)



// PhysAttachLink

PROXY_CONFIG (PhysAttachLink, offset, "Offset", nullptr, Vector, Vector ());

FLAVORED_LINK_IMPL_ (PhysAttach,
	PROXY_INIT (offset)
)

PhysAttachLink
PhysAttachLink::create (const Object& source, const Object& dest,
	const Vector& offset)
{
	PhysAttachLink link = Link::create (flavor (), source, dest);
	link.offset = offset;
	return link;
}



// MovingTerrain

PROXY_CONFIG (MovingTerrain, active, "MovingTerrain", "Active", bool, false);
PROXY_BIT_CONFIG (MovingTerrain, push_attachments, "PhysAttr", "Flags",
	4u, false);

OBJECT_TYPE_IMPL_ (MovingTerrain, Physical (), OBBPhysical (),
	PROXY_INIT (active),
	PROXY_INIT (push_attachments)
)

bool
MovingTerrain::is_moving_terrain () const
{
	return active.exists ();
}



// TPathLink

PROXY_CONFIG (TPathLink, speed, "Speed", nullptr, float, 0.0f);
PROXY_CONFIG (TPathLink, pause, "Pause (ms)", nullptr, Time, 0ul);
PROXY_CONFIG (TPathLink, path_limit, "Path Limit?", nullptr, bool, false);

FLAVORED_LINK_IMPL_ (TPath,
	PROXY_INIT (speed),
	PROXY_INIT (pause),
	PROXY_INIT (path_limit)
)

TPathLink
TPathLink::create (const Object& source, const Object& dest,
	float speed, Time pause, bool path_limit)
{
	TPathLink link = Link::create (flavor (), source, dest);
	link.speed = speed;
	link.pause = pause;
	link.path_limit = path_limit;
	return link;
}



// MovingTerrainMessage

MESSAGE_WRAPPER_IMPL (MovingTerrainMessage, sMovingTerrainMsg),
	waypoint (MESSAGE_AS (sMovingTerrainMsg)->waypoint)
{}

MovingTerrainMessage::MovingTerrainMessage (const Object& _waypoint)
	: Message (new sMovingTerrainMsg ()), waypoint (_waypoint)
{
	message->message = "MovingTerrainWaypoint";
	MESSAGE_AS (sMovingTerrainMsg)->waypoint = waypoint.number;
}



// WaypointMessage

MESSAGE_WRAPPER_IMPL (WaypointMessage, sWaypointMsg),
	moving_terrain (MESSAGE_AS (sWaypointMsg)->moving_terrain)
{}

WaypointMessage::WaypointMessage (const Object& _moving_terrain)
	: Message (new sWaypointMsg ()), moving_terrain (_moving_terrain)
{
	message->message = "WaypointReached";
	MESSAGE_AS (sWaypointMsg)->moving_terrain = moving_terrain.number;
}



//TODO Create PressurePlate : OBBPhysical and wrap property: Physics: Misc\Pressure Plate = PhysPPlate



// SpherePhysical

PROXY_ARRAY_CONFIG (SpherePhysical, physics_radius, 2u, float, //TODO Support more than two submodels.
	PROXY_ARRAY_ITEM ("PhysDims", "Radius 1", 0.0f),
	PROXY_ARRAY_ITEM ("PhysDims", "Radius 2", 0.0f));
PROXY_ARRAY_CONFIG (SpherePhysical, physics_offset, 2u, Vector, //TODO Support more than two submodels.
	PROXY_ARRAY_ITEM ("PhysDims", "Offset 1", Vector ()),
	PROXY_ARRAY_ITEM ("PhysDims", "Offset 2", Vector ()));
PROXY_CONFIG (SpherePhysical, rotation_axes, "PhysAttr", "Rotation Axes",
	unsigned, Physical::ALL_POS_AXES);
PROXY_CONFIG (SpherePhysical, rest_axes, "PhysAttr", "Rest Axes",
	unsigned, Physical::ALL_AXES);

OBJECT_TYPE_IMPL_ (SpherePhysical, Physical (),
	PROXY_ARRAY_INIT (physics_radius, 2),
	PROXY_ARRAY_INIT (physics_offset, 2),
	PROXY_INIT (rotation_axes),
	PROXY_INIT (rest_axes)
)



// Explosion

static float
Explosion_radius_getter (const FieldProxyConfig<float>::Item& item,
	const LGMultiBase& multi)
{
	return multi.empty () ? item.default_value
		: std::sqrt (reinterpret_cast<const LGMulti<float>&> (multi));
}

static void
Explosion_radius_setter (const FieldProxyConfig<float>::Item&,
	LGMultiBase& multi, const float& value)
{
	reinterpret_cast<LGMulti<float>&> (multi) = value * value;
}

PROXY_CONFIG_ (Explosion, radius, "PhysExplode", "Radius (squared)",
	float, 0.0f, 0, Explosion_radius_getter, Explosion_radius_setter);
PROXY_CONFIG (Explosion, magnitude, "PhysExplode", "Magnitude", int, 0);

OBJECT_TYPE_IMPL_ (Explosion,
	PROXY_INIT (radius),
	PROXY_INIT (magnitude)
)

bool
Explosion::is_explosion () const
{
	return radius.exists ();
}

void
Explosion::explode (const Vector& center, float radius, int magnitude)
{
	Explosion explosion = Object::create_temp_fnord ();
	explosion.set_location (center);
	explosion.radius = radius;
	explosion.magnitude = magnitude;
}



// Projectile
//TODO wrap property: Gun\Projectile Description = Projectile

PROXY_CONFIG (Projectile, initial_velocity, "PhysInitVel", nullptr,
	Vector, Vector ());
PROXY_CONFIG (Projectile, faces_velocity, "PhysFaceVel", nullptr, bool, false);
PROXY_CONFIG (Projectile, whizzing_sound, "PrjSound", nullptr, String, "");
PROXY_CONFIG (Projectile, launcher, "Firer", nullptr, Object, Object::NONE);

OBJECT_TYPE_IMPL_ (Projectile, Physical (), SpherePhysical (),
	PROXY_INIT (initial_velocity),
	PROXY_INIT (faces_velocity),
	PROXY_INIT (whizzing_sound),
	PROXY_INIT (launcher)
)

bool
Projectile::is_projectile () const
{
	return initial_velocity.exists ();
}

Projectile
Projectile::launch (const Object& archetype, const Object& launcher,
	float velocity_mult, const Vector& velocity_add, unsigned flags)
{
	LGObject projectile;
	SService<IPhysSrv> (LG)->LaunchProjectile (projectile, launcher.number,
		archetype.number, velocity_mult, flags, LGVector (velocity_add));
	return projectile.id;
}



// Rope

PROXY_CONFIG (Rope, desired_length, "PhysRope", "Desired Length", float, 0.0f);
PROXY_CONFIG (Rope, length, "PhysRope", "Length", float, 0.0f);
PROXY_CONFIG (Rope, deployed, "PhysRope", "Deployed", bool, false);

OBJECT_TYPE_IMPL_ (Rope, Physical (), SpherePhysical (),
	PROXY_INIT (desired_length),
	PROXY_INIT (length),
	PROXY_INIT (deployed)
)

bool
Rope::is_rope () const
{
	return desired_length.exists ();
}



// PhysicsMessage
//TODO wrap sPhysMsg: PhysMadePhysical, PhysMadeNonPhysical, PhysFellAsleep, PhysWokeUp (and indirectly those below)



// PhysCollisionMessage : PhysicsMessage
//TODO wrap sPhysMsg: PhysCollision (and enum eCollisionType)



// PhysContactMessage : PhysicsMessage
//TODO wrap sPhysMsg: PhysContactCreate, PhysContactDestroy



// PhysTransitionMessage : PhysicsMessage
//TODO wrap sPhysMsg: PhysEnter, PhysExit



} // namespace Thief

