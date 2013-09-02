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



// AIAttackMessage

class AIAttackMessage : public Message //TESTME
{
public:
	enum Event { WINDUP, START, END };

	AIAttackMessage (Event, const Object& weapon);
	THIEF_MESSAGE_WRAP (AIAttackMessage);

	Event get_event () const;
	Weapon get_weapon () const;
};



} // namespace Thief

#endif // THIEF_COMBAT_HH

