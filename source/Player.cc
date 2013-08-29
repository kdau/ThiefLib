/******************************************************************************
 *  Player.cc
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



PROXY_CONFIG (Player, visibility, "AI_Visibility", "Level", int, 0);
PROXY_CONFIG (Player, vis_light_rating, "AI_Visibility", "Light rating",
	int, 0);
PROXY_CONFIG (Player, vis_movement_rating, "AI_Visibility", "Movement rating",
	int, 0);
PROXY_CONFIG (Player, vis_exposure_rating, "AI_Visibility", "Exposure rating",
	int, 0);
PROXY_CONFIG (Player, vis_last_update, "AI_Visibility", "Last update time",
	Time, 0ul);

Player::Player ()
	: Object ("Player"),
	  PROXY_INIT (visibility),
	  PROXY_INIT (vis_light_rating),
	  PROXY_INIT (vis_movement_rating),
	  PROXY_INIT (vis_exposure_rating),
	  PROXY_INIT (vis_last_update)
{}



// Inventory

bool
Player::is_in_inventory (const Object& object) const
{
	return SInterface<IContainSys> (LG)->Contains (number, object.number);
}

Container::Contents
Player::get_inventory () const
{
	return Container (*this).get_contents ();
}

void
Player::add_to_inventory (const Object& object)
{
	SInterface<IInventory> (LG)->Add (object.number);
}

void
Player::remove_from_inventory (const Object& object)
{
	SInterface<IInventory> (LG)->Remove (object.number);
}

Interactive
Player::get_selected_item () const
{
	return SInterface<IInventory> (LG)->Selection (kInvItem);
}

bool
Player::is_wielding_junk () const
{
	return SInterface<IInventory> (LG)->WieldingJunk ();
}

void
Player::select_item (const Object& item)
{
	SInterface<IInventory> (LG)->Select (item.number);
}

void
Player::select_loot ()
{
	Engine::run_command ("loot_select");
}

void
Player::select_newest_item ()
{
	Engine::run_command ("select_newest_item");
}

void
Player::cycle_item_selection (Cycle direction)
{
	SInterface<IInventory> (LG)->CycleSelection (kInvItem,
		eCycleDirection (direction));
}

void
Player::clear_item ()
{
	SInterface<IInventory> (LG)->ClearSelection (kInvItem);
}

void
Player::start_tool_use ()
{
	Engine::run_command ("use_item 0");
}

void
Player::finish_tool_use ()
{
	Engine::run_command ("use_item 1");
}

void
Player::drop_item ()
{
	// There does not appear to be any script access to this.
	Engine::run_command ("drop_item");
}

bool
Player::has_touched (const Object& object) const
{
	// Consider the starting point as well.
	Object player = exists () ? number
		: Link::get_one ("PlayerFactory").get_source ();
	if (!object.exists () || !player.exists ()) return false;

	// Is the player holding the object?
	if (Link::any_exist ("Contains", player, object))
		return true;

	// Has the player held but dropped the object (still culpable for it)?
	if (Link::any_exist ("CulpableFor", player, object))
		return true;

	// Is the object currently attached to the player arm or bow arm?
	AI attachment = Link::get_one ("~CreatureAttachment", object).get_dest ();
	switch (attachment.creature_type)
	{
	case AI::CreatureType::PLAYER_ARM:
	case AI::CreatureType::PLAYER_BOW_ARM:
		return true;
	default:
		break;
	}

	return false;
}



// Combat

Weapon
Player::get_selected_weapon () const
{
	return SInterface<IInventory> (LG)->Selection (kInvWeapon);
}

bool
Player::is_bow_selected () const
{
	return SService<IBowSrv> (LG)->IsEquipped ();
}

void
Player::select_weapon (const Weapon& weapon)
{
	SInterface<IInventory> (LG)->Select (weapon.number);
}

void
Player::cycle_weapon_selection (Cycle direction)
{
	SInterface<IInventory> (LG)->CycleSelection (kInvWeapon,
		eCycleDirection (direction));
}

void
Player::clear_weapon ()
{
	SInterface<IInventory> (LG)->ClearSelection (kInvWeapon);
}

bool
Player::start_attack ()
{
	Weapon weapon = get_selected_weapon ();
	if (SService<IBowSrv> (LG)->IsEquipped ())
		return SService<IBowSrv> (LG)->StartAttack ();
	else
		return SService<IWeaponSrv> (LG)->StartAttack
			(number, weapon.number);
}

bool
Player::finish_attack ()
{
	Weapon weapon = get_selected_weapon ();
	if (SService<IBowSrv> (LG)->IsEquipped ())
		return SService<IBowSrv> (LG)->FinishAttack ();
	else
		return SService<IWeaponSrv> (LG)->FinishAttack
			(number, weapon.number);
}

bool
Player::abort_attack ()
{
	// "Weapon" (sword/blackjack) attacks cannot be aborted.
	return SService<IBowSrv> (LG)->IsEquipped () &&
		SService<IBowSrv> (LG)->AbortAttack ();
}



// Physics and movement

#ifdef IS_THIEF2

Physical
Player::get_climbing_object () const
{
	LGObject object;
	SService<IPhysSrv> (LG)->GetClimbingObject (number, object);
	return object.id;
}

void
Player::nudge_physics (int submodel, const Vector& _by)
{
	LGVector by (_by);
	SService<IPhysSrv> (LG)->PlayerMotionSetOffset (submodel, by);
}

#endif // IS_THIEF2

void
Player::unstick ()
{
	Engine::run_command ("unstick_player");
}

void
Player::add_speed_control (const String& name, float factor)
{
	SService<IDarkInvSrv> (LG)->AddSpeedControl
		(name.data (), factor, factor);
}

void
Player::remove_speed_control (const String& name)
{
	SService<IDarkInvSrv> (LG)->RemoveSpeedControl (name.data ());
}



// Limb model (player arm)
//TODO wrap property: Renderer\Invisible = INVISIBLE (as render_arm where true == 0 and false == -1)

bool
Player::show_arm ()
{
	return SService<IPlayerLimbsSrv> (LG)->Equip
		(get_selected_item ().number);
}

bool
Player::start_arm_use ()
{
	return SService<IPlayerLimbsSrv> (LG)->StartUse
		(get_selected_item ().number);
}

bool
Player::finish_arm_use ()
{
	return SService<IPlayerLimbsSrv> (LG)->FinishUse
		(get_selected_item ().number);
}

bool
Player::hide_arm ()
{
	return SService<IPlayerLimbsSrv> (LG)->UnEquip
		(get_selected_item ().number);
}



// Miscellaneous

void
Player::attach_camera (const Object& camera, bool freelook)
{
	if (freelook)
		SService<ICameraSrv> (LG)->DynamicAttach (camera.number);
	else
		SService<ICameraSrv> (LG)->StaticAttach (camera.number);
}

bool
Player::detach_camera (const Object& camera)
{
	if (camera == Object::ANY)
		return SService<ICameraSrv> (LG)->ForceCameraReturn () == S_OK;
	else
		return SService<ICameraSrv> (LG)->CameraReturn
			(camera.number) == S_OK;
}

bool
Player::drop_dead ()
{
	return SService<IDarkGameSrv> (LG)->KillPlayer () == S_OK;
}

void
Player::enable_world_focus ()
{
	// Controlling the other three capabilities does not have any effect.
	SService<IDarkInvSrv> (LG)->CapabilityControl
		(kDrkInvCapWorldFocus, kDrkInvControlOn);
}

void
Player::disable_world_focus ()
{
	SService<IDarkInvSrv> (LG)->CapabilityControl
		(kDrkInvCapWorldFocus, kDrkInvControlOff);
}



} // namespace Thief

