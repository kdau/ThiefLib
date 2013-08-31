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



/*TODO Create MeleeCombatant : AI and wrap these properties:
 * AI: Ability Settings\HtoHCombat: Audio Response = HTHAudioResp
 * AI: Ability Settings\HtoHCombat: Distances = HTHCombatDist
 * AI: Ability Settings\HtoHCombat: Grunt Always = HTHGruntAlways
 * AI: Ability Settings\HtoHCombat: Motion Response = HTHMotionResp
 */



/*TODO Create RangedCombatant : AI and wrap these properties:
 * AI: Ability Settings\RangedCombat: Audio Response = RangedAudioResp
 * AI: Ability Settings\RangedCombat: Grunt Always = RngdGruntAlways
 * AI: Ranged Combat\Ranged Combat = AIRCProp
 * AI: Ranged Combat\Ranged Combat Applicabilities = AI_RngApps
 * AI: Ranged Combat\Ranged Combat Flee = AI_RngFlee
 * AI: Ranged Combat\Ranged Combat Ranges = AIRCRanges
 * AI: Ranged Combat\Ranged Combat Shoot = AI_RngShoot
 * AI: Ranged Combat\Ranged Combat Wound Motion = AIRCWound
 * AI: Ranged Combat\Ranged Combat Wound Sound = AIRCWndSnd
 */



//TODO wrap link: AIProjectile - sAIProjectileRel



//TODO Create SuicideCombatant : AI and wrap property: AI: Ability Settings\Frog-beast: Explode range = DAI_FrogExpl



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



} // namespace Thief

