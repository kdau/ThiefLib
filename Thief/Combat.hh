/******************************************************************************
 *  Combat.hh
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

#ifndef THIEF_COMBAT_HH
#define THIEF_COMBAT_HH

#include <Thief/Base.hh>
#include <Thief/AI.hh>
#include <Thief/Rendering.hh>
#include <Thief/Types.hh>

namespace Thief {



// Blood

class Blood : public virtual Rendered, public virtual Damageable
{
public:
	THIEF_OBJECT_TYPE (Blood)
	THIEF_PROP_FIELD (bool, is_blood); //TESTME

	static void cleanse (const Vector& center, float radius = 5.0f);
};



// Weapon

class Weapon : public virtual Interactive, public virtual Combinable
{
public:
	THIEF_OBJECT_TYPE (Weapon)
	bool is_weapon () const;

	THIEF_PROP_FIELD (int, exposure_drawn);
	THIEF_PROP_FIELD (int, exposure_swung);

	THIEF_PROP_FIELD (bool, collides_with_terrain);
};



// AIAttackLink

THIEF_FLAVORED_LINK (AIAttack) //TESTME
{
	THIEF_FLAVORED_LINK_COMMON (AIAttack)

	static AIAttackLink create (const Object& source, const Object& dest,
		AI::Priority = AI::Priority::DEFAULT);

	THIEF_LINK_FIELD (AI::Priority, priority);
};



// AIAttackMessage: "StartWindup", "StartAttack", "EndAttack"

class AIAttackMessage : public Message //TESTME
{
public:
	enum Event { WINDUP, START, END };

	AIAttackMessage (Event, const Object& weapon);
	THIEF_MESSAGE_WRAP (AIAttackMessage);

	const Event event;
	const Weapon weapon;
};



// Combatant

class Combatant : public AI
{
public:
	THIEF_OBJECT_TYPE (Combatant)

	enum NonHostile
	{
		NEVER,
		PLAYER_ALWAYS, PLAYER_UNTIL_DAMAGED, PLAYER_UNTIL_THREATENED,
		UNTIL_DAMAGED, UNTIL_THREATENED, ALWAYS
	};
	THIEF_PROP_FIELD (NonHostile, non_hostile);
};



// RangedCombatant

class RangedCombatant : public Combatant
{
public:
	THIEF_OBJECT_TYPE (RangedCombatant)
	bool is_ranged_combatant () const;

	enum class RCPriority
		{ VERY_LOW, LOW, MODERATE, HIGH, VERY_HIGH };

	enum class RCFrequency
		{ NEVER, VERY_RARELY, RARELY, SOMETIMES, OFTEN, VERY_OFTEN };

	THIEF_PROP_FIELD (float, minimum_distance); //TESTME
	THIEF_PROP_FIELD (float, ideal_distance); //TESTME
	THIEF_PROP_FIELD (RCFrequency, fire_while_moving); //TESTME

	THIEF_PROP_FIELD (Time, firing_delay); //TESTME

	THIEF_PROP_FIELD (RCPriority, cover_desire); //TESTME
	THIEF_PROP_FIELD (float, decay_speed); //TESTME

	THIEF_PROP_FIELD (bool, contain_projectile); //TESTME
};



// AIProjectileLink

THIEF_FLAVORED_LINK (AIProjectile) //TESTME
{
	THIEF_FLAVORED_LINK_COMMON (AIProjectile)

	enum class Method { STRAIGHT_LINE, ARCING, REFLECTING, OVERHEAD };

	static const int INFINITE_STACK;

	static AIProjectileLink create (const Object& source, const Object& dest,
		RangedCombatant::RCPriority selection_desire =
			RangedCombatant::RCPriority::VERY_LOW,
		Method targeting_method = Method::STRAIGHT_LINE,
		RangedCombatant::RCPriority accuracy =
			RangedCombatant::RCPriority::VERY_LOW,
		AI::Joint launch_joint = AI::Joint::NONE);

	THIEF_LINK_FIELD (RangedCombatant::RCPriority, selection_desire);
	THIEF_LINK_FIELD (bool, ignore_if_enough_friends);
	THIEF_LINK_FIELD (int, min_friends_nearby);

	THIEF_LINK_FIELD (int, stack_count);
	THIEF_LINK_FIELD (int, burst_count);
	THIEF_LINK_FIELD (Time, firing_delay);

	THIEF_LINK_FIELD (Method, targeting_method);
	THIEF_LINK_FIELD (RangedCombatant::RCPriority, accuracy);
	THIEF_LINK_FIELD (bool, leads_target);
	THIEF_LINK_FIELD (AI::Joint, launch_joint);
};



// SuicideCombatant

class SuicideCombatant : public Combatant
{
public:
	THIEF_OBJECT_TYPE (SuicideCombatant)
	bool is_suicide_combatant () const; //TESTME

	THIEF_PROP_FIELD (float, detonate_range); //TESTME
};



} // namespace Thief

#endif // THIEF_COMBAT_HH

