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

#undef OBJECT_TYPE
#define OBJECT_TYPE Physical

PF_CONFIG (physics_type, "PhysType", "Type", Physical::PhysicsType,
	Physical::PhysicsType::NONE);
PF_CONFIG (submodel_count, "PhysType", "# Submodels", unsigned, 0u);
PF_CONFIG (gravity, "PhysAttr", "Gravity %", float, 100.0f);
PF_CONFIG (mass, "PhysAttr", "Mass", float, 30.0f);
PF_CONFIG (density, "PhysAttr", "Density", float, 1.0f);
PF_CONFIG (elasticity, "PhysAttr", "Elasticity", float, 1.0f);
PF_CONFIG (friction, "PhysAttr", "Base Friction", float, 0.0f);
PF_CONFIG (center_of_gravity, "PhysAttr", "COG Offset", Vector, Vector ());
PF_CONFIG (mantleable, "PhysCanMant", nullptr, bool, true);
PF_CONFIG (remove_on_sleep, "PhysType", "Remove on Sleep", bool, false);
PFB_CONFIG (collision_bounce, "CollisionType", nullptr, 1u, false);
PFB_CONFIG (collision_destroy, "CollisionType", nullptr, 2u, false);
PFB_CONFIG (collision_slay, "CollisionType", nullptr, 4u, false);
PFB_CONFIG (collision_no_sound, "CollisionType", nullptr, 8u, false);
PFB_CONFIG (collision_no_result, "CollisionType", nullptr, 16u, false);
PFB_CONFIG (collision_full_sound, "CollisionType", nullptr, 32u, false);
PF_CONFIG (collides_with_ai, "PhysAIColl", nullptr, bool, false);
PF_CONFIG (bash_factor, "BashFactor", nullptr, float, 0.0f);
PF_CONFIG (bash_threshold, "BashParams", "Threshold", float, 0.0f);
PF_CONFIG (bash_coefficient, "BashParams", "Coefficient", float, 0.0f);
PF_CONFIG (velocity, "PhysState", "Velocity", Vector, Vector ());
PF_CONFIG (rotational_velocity, "PhysState", "Rot Velocity", Vector, Vector ());

OBJECT_TYPE_IMPL_ (Physical,
	PF_INIT (physics_type),
	PF_INIT (submodel_count),
	PF_INIT (gravity),
	PF_INIT (mass),
	PF_INIT (density),
	PF_INIT (elasticity),
	PF_INIT (friction),
	PF_INIT (center_of_gravity),
	PF_INIT (mantleable),
	PF_INIT (remove_on_sleep),
	PF_INIT (collision_bounce),
	PF_INIT (collision_destroy),
	PF_INIT (collision_slay),
	PF_INIT (collision_no_sound),
	PF_INIT (collision_no_result),
	PF_INIT (collision_full_sound),
	PF_INIT (collides_with_ai),
	PF_INIT (bash_factor),
	PF_INIT (bash_threshold),
	PF_INIT (bash_coefficient),
	PF_INIT (velocity),
	PF_INIT (rotational_velocity)
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

#undef OBJECT_TYPE
#define OBJECT_TYPE OBBPhysical

PF_CONFIG (physics_size, "PhysDims", "Size", Vector, Vector ());
PF_CONFIG (physics_offset, "PhysDims", "Offset", Vector, Vector ());
PF_CONFIG (climbable_sides, "PhysAttr", "Climbable Sides", unsigned,
	Physical::ALL_AXES);
PFB_CONFIG (edge_trigger, "PhysAttr", "Flags", 1u, false);
PFB_CONFIG (platform_friction, "PhysAttr", "Flags", 2u, false);
PF_CONFIG (pore_size, "PhysAttr", "Pore Size", float, 0.0f);
PF_CONFIG (ai_fires_through, "AIFiresThrough", nullptr, bool, false);

OBJECT_TYPE_IMPL_ (OBBPhysical,
	PF_INIT (physics_size),
	PF_INIT (physics_offset),
	PF_INIT (climbable_sides),
	PF_INIT (edge_trigger),
	PF_INIT (platform_friction),
	PF_INIT (pore_size),
	PF_INIT (ai_fires_through)
)



// SpherePhysical

#undef OBJECT_TYPE
#define OBJECT_TYPE SpherePhysical

PF_CONFIG (physics_radius_1, "PhysDims", "Radius 1", float, 0.0f);
PF_CONFIG (physics_radius_2, "PhysDims", "Radius 2", float, 0.0f);
PF_CONFIG (physics_offset_1, "PhysDims", "Offset 1", Vector, Vector ());
PF_CONFIG (physics_offset_2, "PhysDims", "Offset 2", Vector, Vector ());
PF_CONFIG (rotation_axes, "PhysAttr", "Rotation Axes", unsigned,
	Physical::ALL_POS_AXES);
PF_CONFIG (rest_axes, "PhysAttr", "Rest Axes", unsigned, Physical::ALL_AXES);

OBJECT_TYPE_IMPL_ (SpherePhysical,
	PF_INIT (physics_radius_1),
	PF_INIT (physics_radius_2),
	PF_INIT (physics_offset_1),
	PF_INIT (physics_offset_2),
	PF_INIT (rotation_axes),
	PF_INIT (rest_axes)
)



//TODO wrap link: PhysAttach - sPhysAttachData



// MovingTerrain
//TODO wrap link: TPath - sTerrainPath

#undef OBJECT_TYPE
#define OBJECT_TYPE MovingTerrain

PF_CONFIG (is_moving_terrain, "MovingTerrain", nullptr, bool, false);
PFB_CONFIG (push_vator_attachments, "PhysAttr", "Flags", 4u, false);

OBJECT_TYPE_IMPL_ (MovingTerrain,
	PF_INIT (is_moving_terrain),
	PF_INIT (push_vator_attachments)
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

#undef OBJECT_TYPE
#define OBJECT_TYPE Projectile

PF_CONFIG (initial_velocity, "PhysInitVel", nullptr, Vector, Vector ());
PF_CONFIG (faces_velocity, "PhysFaceVel", nullptr, bool, false);
PF_CONFIG (whizzing_sound, "PrjSound", nullptr, String, "");

OBJECT_TYPE_IMPL_ (Projectile,
	PF_INIT (initial_velocity),
	PF_INIT (faces_velocity),
	PF_INIT (whizzing_sound)
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

