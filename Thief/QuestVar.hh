/******************************************************************************
 *  QuestVar.hh
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

#ifndef THIEF_QUESTVAR_HH
#define THIEF_QUESTVAR_HH

#include <Thief/Base.hh>
#include <Thief/Mission.hh>

namespace Thief {



class QuestVar
{
public:
	enum class Scope { MISSION, CAMPAIGN, ANY };

	QuestVar (const String& name = String ());
	String name;

	bool exists () const;
	int get (int default_value = 0) const;
	void set (int value, Scope scope = Scope::MISSION);
	void unset ();

	void subscribe (const Object& host, Scope scope = Scope::MISSION) const;
	void unsubscribe (const Object& host) const;
};



class QuestChangeMessage : public Message
{
public:
	QuestChangeMessage (const char* quest_var, int new_value, int old_value);
	THIEF_MESSAGE_WRAP (QuestChangeMessage);

	QuestVar get_quest_var () const;
	int get_new_value () const;
	int get_old_value () const;
};



class Objective
{
public:
	typedef unsigned int Number;
	static const Number NONE;

	Objective (Number = NONE);

	Number number;

	bool exists () const;

	enum class State { INCOMPLETE, COMPLETE, CANCELLED, FAILED };
	State get_state () const;
	void set_state (State state);

	bool is_visible () const;
	void set_visible (bool visible);

	bool is_final () const;
	void set_final (bool final);

	bool is_irreversible () const;
	void set_irreversible (bool irreversible);

	bool is_reverse () const;
	void set_reverse (bool reverse);

#ifdef IS_THIEF2

	bool is_optional () const;
	void set_optional (bool optional);

	bool is_bonus () const;
	void set_bonus (bool bonus);

#endif // IS_THIEF2

	Difficulty get_difficulty () const;
	void set_difficulty (Difficulty difficulty);

	enum class Type { NONE, TAKE, SLAY, LOOT, GOTO };
	Type get_type () const;
	void set_type (Type type);

	Object get_target () const;
	void set_target (const Object& target);

	struct Loot
	{
		int gold, gems, goods, total;
		std::bitset<8> specials;
	};
	Loot get_loot () const;
	void set_loot (const Loot& loot);

	enum class Field
	{
		STATE, VISIBLE,
		FINAL, IRREVERSIBLE, REVERSE,
#ifdef IS_THIEF2
		OPTIONAL, BONUS,
#else
		X_INVALID1, X_INVALID2,
#endif
		MIN_DIFF, MAX_DIFF,
		TYPE, TARGET,
		GOLD, GEMS, GOODS, LOOT,
		SPECIAL, SPECIALS
	};
	void subscribe (const Object& host, Field field) const;
	void unsubscribe (const Object& host, Field field) const;

	static size_t count_objectives ();
	static void dump_objectives ();

	struct QVData { Number number; Field field; };
	static QVData parse_quest_var (const QuestVar& quest_var);

private:
	QuestVar quest_var (Field field) const;
};



} // namespace Thief

#endif // THIEF_QUESTVAR_HH

