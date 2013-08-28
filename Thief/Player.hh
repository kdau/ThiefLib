/******************************************************************************
 *  Player.hh
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

#ifndef THIEF_PLAYER_HH
#define THIEF_PLAYER_HH

#include <Thief/Base.hh>
#include <Thief/Combat.hh>
#include <Thief/Types.hh>

namespace Thief {

class Player : public Being
{
public:
	Player ();
	Player (const Player&) = delete;

	// Inventory

	enum class Cycle { BACKWARD = -1, FORWARD = 1 };

	bool is_in_inventory (const Object&) const;
	Container::Contents get_inventory () const;

	void add_to_inventory (const Object&);
	void remove_from_inventory (const Object&);

	Interactive get_selected_item () const;
	bool is_wielding_junk () const;

	void select_item (const Object& item);
	void select_loot ();
	void select_newest_item ();
	void cycle_item_selection (Cycle direction);
	void clear_item ();

	void start_tool_use ();
	void finish_tool_use ();
	void drop_item ();

	bool has_touched (const Object&) const;

	// Combat

	Weapon get_selected_weapon () const;
	bool is_bow_selected () const;
	void select_weapon (const Weapon&);
	void cycle_weapon_selection (Cycle direction);
	void clear_weapon ();

	bool start_attack ();
	bool finish_attack ();
	bool abort_attack ();

	// Miscellaneous

	bool show_arm ();
	bool start_arm_use (); //TESTME
	bool finish_arm_use (); //TESTME
	bool hide_arm ();

	void attach_camera (const Object& camera, bool freelook);
	bool detach_camera (const Object& camera = Object::ANY);

#ifdef IS_THIEF2
	Physical get_climbing_object () const;
#endif

	bool drop_dead ();

#ifdef IS_THIEF2
	void nudge_physics (int submodel, const Vector& by);
#endif

	void add_speed_control (const String& name, float factor);
	void remove_speed_control (const String& name);

	void unstick ();

	void enable_world_focus ();
	void disable_world_focus ();
};

} // namespace Thief

#endif // THIEF_PLAYER_HH

