/******************************************************************************
 *  Combat.cc
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



// Blood

PROXY_CONFIG (Blood, is_blood, "Blood", nullptr, bool, false);

OBJECT_TYPE_IMPL_ (Blood, Rendered (), Damageable (),
	PROXY_INIT (is_blood)
)

void
Blood::cleanse (const Vector& _center, float radius)
{
	Object center = Object::create_temp_fnord ();
	center.set_location (_center);
	SService<IDarkPowerupsSrv> (LG)->CleanseBlood (center.number, radius);
}



// Weapon

PROXY_CONFIG (Weapon, exposure_drawn, "WpnExposure", nullptr, int, 0);
PROXY_CONFIG (Weapon, exposure_swung, "SwingExpose", nullptr, int, 0);
PROXY_CONFIG (Weapon, collides_with_terrain, "WpnTerrColl", nullptr,
	bool, false);

OBJECT_TYPE_IMPL_ (Weapon, Rendered (), Interactive (), Combinable (),
	PROXY_INIT (exposure_drawn),
	PROXY_INIT (exposure_swung),
	PROXY_INIT (collides_with_terrain)
)

bool
Weapon::is_weapon () const
{
	return get_inventory_type () == InventoryType::WEAPON;
}



// AIAttackLink

PROXY_CONFIG (AIAttackLink, priority, nullptr, nullptr,
	AI::Priority, AI::Priority::DEFAULT);

FLAVORED_LINK_IMPL_ (AIAttack,
	PROXY_INIT (priority)
)

AIAttackLink
AIAttackLink::create (const Object& source, const Object& dest,
	AI::Priority priority)
{
	AIAttackLink link = Link::create (flavor (), source, dest);
	link.priority = priority;
	return link;
}



// AIAttackMessage

MESSAGE_WRAPPER_IMPL (AIAttackMessage, sAttackMsg)

AIAttackMessage::AIAttackMessage (Event event, const Object& weapon)
	: Message (new sAttackMsg ())
{
	switch (event)
	{
	case WINDUP: message->message = "StartWindup"; break;
	case START:  message->message = "StartAttack"; break;
	case END:
	default:     message->message = "EndAttack"; break;
	}
	MESSAGE_AS (sAttackMsg)->weapon = weapon.number;
}

AIAttackMessage::Event
AIAttackMessage::get_event () const
{
	CIString name = get_name ();
	if (name == "StartWindup") return WINDUP;
	if (name == "StartAttack") return START;
	if (name == "EndAttack") return END;
	throw MessageWrapError (message, typeid (*this), "invalid event");
}

MESSAGE_ACCESSOR (Weapon, AIAttackMessage, get_weapon, sAttackMsg, weapon)



// Combatant
//TODO wrap property: AI: Ability Settings\Combat: Timing = AI_CbtTiming
//TODO wrap property: AI: Ability Settings\Non-combat: Dmg Response Params = AI_NCDmgRsp
//TODO wrap property: AI: Ability Settings\Non-combat: Respond to damage = AI_NCDmRsp
//TODO wrap property: AI: Ability Settings\Flee: Condition for flee = AI_FleeConds
//TODO wrap property: AI: Ability Settings\Flee: If Aware of AI/Player = AI_FleeAwr

PROXY_CONFIG (Combatant, non_hostile, "AI_NonHst", nullptr,
	Combatant::NonHostile, NonHostile::NEVER);

OBJECT_TYPE_IMPL_ (Combatant, Rendered (), Interactive (), Physical (),
	SpherePhysical (), Damageable (), AI (),
	PROXY_INIT (non_hostile)
)



//TODO wrap link: AIFleeDest - sAIFleeDest



//TODO wrap link: AINoFlee - sAINoFleeLink



/*TODO Create MeleeCombatant : Combatant and wrap these properties:
 * AI: Ability Settings\HtoHCombat: Audio Response = HTHAudioResp
 * AI: Ability Settings\HtoHCombat: Distances = HTHCombatDist
 * AI: Ability Settings\HtoHCombat: Grunt Always = HTHGruntAlways
 * AI: Ability Settings\HtoHCombat: Motion Response = HTHMotionResp
 */



// RangedCombatant
//TODO wrap property: AI: Ability Settings\RangedCombat: Audio Response = RangedAudioResp
//TODO wrap property: AI: Ability Settings\RangedCombat: Grunt Always = RngdGruntAlways
//TODO wrap property: AI: Ranged Combat\Ranged Combat Applicabilities = AI_RngApps
//TODO wrap property: AI: Ranged Combat\Ranged Combat Flee = AI_RngFlee
//TODO wrap property: AI: Ranged Combat\Ranged Combat Ranges = AIRCRanges
//TODO wrap property: AI: Ranged Combat\Ranged Combat Shoot = AI_RngShoot
//TODO wrap property: AI: Ranged Combat\Ranged Combat Wound Motion = AIRCWound
//TODO wrap property: AI: Ranged Combat\Ranged Combat Wound Sound = AIRCWndSnd

PROXY_CONFIG (RangedCombatant, minimum_distance, "AIRCProp", "Minimum Distance",
	int, 10); //TODO Should be a float.
PROXY_CONFIG (RangedCombatant, ideal_distance, "AIRCProp", "Ideal Distance",
	int, 40); //TODO Should be a float.
PROXY_CONFIG (RangedCombatant, fire_while_moving, "AIRCProp", "Fire While Moving",
	RangedCombatant::RCFrequency, RCFrequency::NEVER);
PROXY_CONFIG (RangedCombatant, firing_delay, "AIRCProp", "Firing Delay",
	float, 0.0f); //TODO Should be a Time.
PROXY_CONFIG (RangedCombatant, cover_desire, "AIRCProp", "Cover Desire",
	RangedCombatant::RCPriority, RCPriority::MODERATE);
PROXY_CONFIG (RangedCombatant, decay_speed, "AIRCProp", "Decay Speed",
	float, 0.8f); //TODO Should be a Time?
PROXY_CONFIG (RangedCombatant, contain_projectile,
	"AIRCProp", "Contain Projectile", bool, false);

OBJECT_TYPE_IMPL_ (RangedCombatant, Rendered (), Interactive (), Physical (),
	SpherePhysical (), Damageable (), Combatant (),
	PROXY_INIT (minimum_distance),
	PROXY_INIT (ideal_distance),
	PROXY_INIT (fire_while_moving),
	PROXY_INIT (firing_delay),
	PROXY_INIT (cover_desire),
	PROXY_INIT (decay_speed),
	PROXY_INIT (contain_projectile)
)

bool
RangedCombatant::is_ranged_combatant () const
{
	return minimum_distance.exists ();
}



// AIProjectileLink

const int
AIProjectileLink::INFINITE_STACK = INT_MAX;

PROXY_CONFIG (AIProjectileLink, selection_desire, "Selection Desire", nullptr,
	RangedCombatant::RCPriority, RangedCombatant::RCPriority::VERY_LOW);
PROXY_BIT_CONFIG (AIProjectileLink, ignore_if_enough_friends,
	"Constraint Type", nullptr, 1u, false);
PROXY_CONFIG (AIProjectileLink, min_friends_nearby, "Constraint Data", nullptr,
	int, 0);
PROXY_CONFIG_ (AIProjectileLink, stack_count, "Ammo", nullptr, int, 0, 0u,
	[] (const int& ammo) { return ammo ? (ammo - 1) : INFINITE_STACK; },
	[] (const int& stack) { return (stack == INFINITE_STACK)
		? 0 : (stack + 1); });
PROXY_CONFIG (AIProjectileLink, burst_count, "Burst Count", nullptr, int, 0);
PROXY_CONFIG (AIProjectileLink, firing_delay, "Firing Delay", nullptr,
	float, 0.0f); //TODO Should be a Time.
PROXY_CONFIG (AIProjectileLink, targeting_method, "Targeting Method", nullptr,
	AIProjectileLink::Method, Method::STRAIGHT_LINE);
PROXY_CONFIG (AIProjectileLink, accuracy, "Accuracy", nullptr,
	RangedCombatant::RCPriority, RangedCombatant::RCPriority::VERY_LOW);
PROXY_CONFIG (AIProjectileLink, leads_target, "Leads Target", nullptr,
	bool, false);
PROXY_CONFIG (AIProjectileLink, launch_joint, "Launch Joint", nullptr,
	AI::Joint, AI::Joint::NONE);


FLAVORED_LINK_IMPL_ (AIProjectile,
	PROXY_INIT (selection_desire),
	PROXY_INIT (ignore_if_enough_friends),
	PROXY_INIT (min_friends_nearby),
	PROXY_INIT (stack_count),
	PROXY_INIT (burst_count),
	PROXY_INIT (firing_delay),
	PROXY_INIT (targeting_method),
	PROXY_INIT (accuracy),
	PROXY_INIT (leads_target),
	PROXY_INIT (launch_joint)
)

AIProjectileLink
AIProjectileLink::create (const Object& source, const Object& dest,
	RangedCombatant::RCPriority selection_desire, Method targeting_method,
	RangedCombatant::RCPriority accuracy, AI::Joint launch_joint)
{
	AIProjectileLink link = Link::create (flavor (), source, dest);
	link.selection_desire = selection_desire;
	link.targeting_method = targeting_method;
	link.accuracy = accuracy;
	link.launch_joint = launch_joint;
	return link;
}



// SuicideCombatant

PROXY_CONFIG (SuicideCombatant, detonate_range, "DAI_FrogExpl", nullptr,
	float, 0.0f);

OBJECT_TYPE_IMPL_ (SuicideCombatant, Rendered (), Interactive (), Physical (),
	SpherePhysical (), Damageable (), Combatant (),
	PROXY_INIT (detonate_range)
)

bool
SuicideCombatant::is_suicide_combatant () const
{
	return behavior_set == "Frog" && detonate_range.exists ();
}



} // namespace Thief

