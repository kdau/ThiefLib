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



// Physical
//TODO wrap property: AI: Utility\Path avoid = AI_ObjAvoid

PROXY_CONFIG (Physical, physics_type, "PhysType", "Type",
	Physical::PhysicsType, Physical::PhysicsType::NONE);
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
PROXY_CONFIG (Physical, bash_factor, "BashFactor", nullptr, float, 0.0f);
PROXY_CONFIG (Physical, bash_threshold, "BashParams", "Threshold", float, 0.0f);
PROXY_CONFIG (Physical, bash_coefficient, "BashParams", "Coefficient",
	float, 0.0f);
PROXY_CONFIG (Physical, velocity, "PhysState", "Velocity", Vector, Vector ());
PROXY_CONFIG (Physical, rotational_velocity, "PhysState", "Rot Velocity",
	Vector, Vector ());

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
	PROXY_INIT (bash_factor),
	PROXY_INIT (bash_threshold),
	PROXY_INIT (bash_coefficient),
	PROXY_INIT (velocity),
	PROXY_INIT (rotational_velocity)
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
	return Property (*this, "PhysType").remove ();
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

bool
Physical::is_velocity_locked () const
{
	return Property (*this, "PhysControl").get_field ("Controls Active", 0u)
		& PHYS_CONTROL_VELS;
}

void
Physical::lock_velocity (const Vector& _velocity)
{
#ifdef IS_THIEF2
	SService<IPhysSrv> (LG)->ControlVelocity (number, LGVector (_velocity));
#else
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) | PHYS_CONTROL_VELS);
	controls.set_field ("Velocity", _velocity);
#endif
}

void
Physical::unlock_velocity ()
{
#ifdef IS_THIEF2
	SService<IPhysSrv> (LG)->StopControlVelocity (number);
#else
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) & ~PHYS_CONTROL_VELS);
#endif
}

bool
Physical::is_rotational_velocity_locked () const
{
	return Property (*this, "PhysControl").get_field ("Controls Active", 0u)
		& PHYS_CONTROL_ROTVELS;
}

void
Physical::lock_rotational_velocity (const Vector& _rotational_velocity)
{
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) | PHYS_CONTROL_ROTVELS);
	controls.set_field ("RotationalVelocity", _rotational_velocity);
}

void
Physical::unlock_rotational_velocity ()
{
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) & ~PHYS_CONTROL_ROTVELS);
}

bool
Physical::is_location_locked () const
{
	return Property (*this, "PhysControl").get_field ("Controls Active", 0u)
		& PHYS_CONTROL_LOCATION;
}

void
Physical::lock_location ()
{
#ifdef IS_THIEF2
	SService<IPhysSrv> (LG)->ControlCurrentLocation (number);
#else
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) | PHYS_CONTROL_LOCATION);
#endif
}

void
Physical::unlock_location ()
{
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) & ~PHYS_CONTROL_LOCATION);
}

bool
Physical::is_rotation_locked () const
{
	return Property (*this, "PhysControl").get_field ("Controls Active", 0u)
		& PHYS_CONTROL_ROTATION;
}

void
Physical::lock_rotation ()
{
#ifdef IS_THIEF2
	SService<IPhysSrv> (LG)->ControlCurrentRotation (number);
#else
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) | PHYS_CONTROL_ROTATION);
#endif
}

void
Physical::unlock_rotation ()
{
	Property controls (*this, "PhysControl", true);
	controls.set_field ("Controls Active", controls.get_field
		("Controls Active", 0u) & ~PHYS_CONTROL_ROTATION);
}

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
//TODO wrap property: AI: Utility\Path Exact OBB = AI_NGOBB
//TODO wrap property: AI: Utility\Pathable object = AI_ObjPathable

PROXY_CONFIG (OBBPhysical, physics_size, "PhysDims", "Size", Vector, Vector ());
PROXY_CONFIG (OBBPhysical, physics_offset, "PhysDims", "Offset",
	Vector, Vector ());
PROXY_CONFIG (OBBPhysical, climbable_sides, "PhysAttr", "Climbable Sides",
	unsigned, Physical::ALL_AXES);
PROXY_BIT_CONFIG (OBBPhysical, edge_trigger, "PhysAttr", "Flags", 1u, false);
PROXY_BIT_CONFIG (OBBPhysical, platform_friction, "PhysAttr", "Flags", 2u,
	false);
PROXY_CONFIG (OBBPhysical, pore_size, "PhysAttr", "Pore Size", float, 0.0f);
PROXY_CONFIG (OBBPhysical, ai_fires_through, "AIFiresThrough", nullptr,
	bool, false);

OBJECT_TYPE_IMPL_ (OBBPhysical,
	PROXY_INIT (physics_size),
	PROXY_INIT (physics_offset),
	PROXY_INIT (climbable_sides),
	PROXY_INIT (edge_trigger),
	PROXY_INIT (platform_friction),
	PROXY_INIT (pore_size),
	PROXY_INIT (ai_fires_through)
)



// SpherePhysical

PROXY_CONFIG (SpherePhysical, physics_radius_1, "PhysDims", "Radius 1",
	float, 0.0f);
PROXY_CONFIG (SpherePhysical, physics_radius_2, "PhysDims", "Radius 2",
	float, 0.0f);
PROXY_CONFIG (SpherePhysical, physics_offset_1, "PhysDims", "Offset 1",
	Vector, Vector ());
PROXY_CONFIG (SpherePhysical, physics_offset_2, "PhysDims", "Offset 2",
	Vector, Vector ());
PROXY_CONFIG (SpherePhysical, rotation_axes, "PhysAttr", "Rotation Axes",
	unsigned, Physical::ALL_POS_AXES);
PROXY_CONFIG (SpherePhysical, rest_axes, "PhysAttr", "Rest Axes",
	unsigned, Physical::ALL_AXES);

OBJECT_TYPE_IMPL_ (SpherePhysical,
	PROXY_INIT (physics_radius_1),
	PROXY_INIT (physics_radius_2),
	PROXY_INIT (physics_offset_1),
	PROXY_INIT (physics_offset_2),
	PROXY_INIT (rotation_axes),
	PROXY_INIT (rest_axes)
)



//TODO wrap link: PhysAttach - sPhysAttachData



// MovingTerrain
//TODO wrap link: TPath - sTerrainPath

PROXY_CONFIG (MovingTerrain, is_moving_terrain, "MovingTerrain", nullptr, bool, false);
PROXY_BIT_CONFIG (MovingTerrain, push_vator_attachments, "PhysAttr", "Flags", 4u, false);

OBJECT_TYPE_IMPL_ (MovingTerrain,
	PROXY_INIT (is_moving_terrain),
	PROXY_INIT (push_vator_attachments)
)



// MovingTerrainMessage

MESSAGE_WRAPPER_IMPL (MovingTerrainMessage, sMovingTerrainMsg)

MovingTerrainMessage::MovingTerrainMessage (const Object& waypoint)
	: Message (new sMovingTerrainMsg ())
{
	message->message = "MovingTerrainWaypoint";
	MESSAGE_AS (sMovingTerrainMsg)->waypoint = waypoint.number;
}

MESSAGE_ACCESSOR (Object, MovingTerrainMessage, get_waypoint,
	sMovingTerrainMsg, waypoint)



// WaypointMessage

MESSAGE_WRAPPER_IMPL (WaypointMessage, sWaypointMsg)

WaypointMessage::WaypointMessage (const MovingTerrain& moving_terrain)
	: Message (new sWaypointMsg ())
{
	message->message = "WaypointReached";
	MESSAGE_AS (sWaypointMsg)->moving_terrain = moving_terrain.number;
}

MESSAGE_ACCESSOR (MovingTerrain, WaypointMessage, get_moving_terrain,
	sWaypointMsg, moving_terrain)



// Projectile
//TODO wrap property: Firer
//TODO wrap property: Gun\Projectile Description = Projectile

PROXY_CONFIG (Projectile, initial_velocity, "PhysInitVel", nullptr, Vector, Vector ());
PROXY_CONFIG (Projectile, faces_velocity, "PhysFaceVel", nullptr, bool, false);
PROXY_CONFIG (Projectile, whizzing_sound, "PrjSound", nullptr, String, "");

OBJECT_TYPE_IMPL_ (Projectile,
	PROXY_INIT (initial_velocity),
	PROXY_INIT (faces_velocity),
	PROXY_INIT (whizzing_sound)
)

bool
Projectile::is_projectile () const
{
	return initial_velocity.exists ();
}

Projectile
Projectile::launch (const Object& launcher, float velocity_mult,
	const Vector& velocity_add, unsigned flags)
{
	Projectile archetype = (get_type () == Type::ARCHETYPE)
		? *this : get_archetype ();
	LGObject projectile;
	SService<IPhysSrv> (LG)->LaunchProjectile (projectile, launcher.number,
		archetype.number, velocity_mult, flags, LGVector (velocity_add));
	return projectile.id;
}



/*TODO wrap these properties, perhaps creating {Conveyor,Explosion,PressurePlate,Rope} : Physical:
 * Physics: Misc\Pressure Plate = PhysPPlate
 * Physics: Misc\Rope = PhysRope
 * Physics: Model\ConveyorVelocity = ConveyorVel
 * Physics: Projectile\Explode Me = PhysExplode
 */



// PhysicsMessage
//TODO wrap sPhysMsg: PhysMadePhysical, PhysMadeNonPhysical, PhysFellAsleep, PhysWokeUp (and indirectly those below)



// PhysCollisionMessage : PhysicsMessage
//TODO wrap sPhysMsg: PhysCollision (and enum eCollisionType)



// PhysContactMessage : PhysicsMessage
//TODO wrap sPhysMsg: PhysContactCreate, PhysContactDestroy



// PhysTransitionMessage : PhysicsMessage
//TODO wrap sPhysMsg: PhysEnter, PhysExit



} // namespace Thief

