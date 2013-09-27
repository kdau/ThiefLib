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

	QuestVar (const String& name, Scope = Scope::MISSION);

	String name;
	Scope scope;

	bool exists () const;

	operator int () const;
	int get (int default_value = 0) const;

	QuestVar& operator = (int);
	void set (int);

	void clear ();

	static void subscribe (const String& name, const Object& host,
		Scope = Scope::ANY);
	static void unsubscribe (const String& name, const Object& host);
};



struct Numbered
{
	typedef unsigned int Number;
	Number number;

	Numbered (Number);
	Numbered& operator ++ ();
	Numbered& operator -- ();
};

THIEF_FIELD_PROXY_TEMPLATE
class QuestField
{
public:
	QuestField (Numbered&, size_t index);

	bool exists () const;
	void clear ();

	operator Type () const;
	QuestField& operator = (const Type&);

	void subscribe (const Object& host) const;
	void unsubscribe (const Object& host) const;

protected:
	QuestVar qvar (bool major = true) const;

	Numbered& numbered;
	size_t index;
};



class QuestMessage : public Message // "QuestChange"
{
public:
	QuestMessage (const char* quest_var, int new_value, int old_value);
	THIEF_MESSAGE_WRAP (QuestMessage);

	const QuestVar quest_var;
	const int new_value;
	const int old_value;
};



} // namespace Thief

#include <Thief/QuestVar.inl>

namespace Thief {



class Objective : public Numbered
{
public:
	static const Number NONE;

	Objective (Number = NONE);
	Objective (const Objective&);
	Objective& operator = (const Objective&);

	bool exists () const;

	enum class State { INCOMPLETE, COMPLETE, CANCELLED, FAILED };
	THIEF_QUEST_FIELD (State, state);
	THIEF_QUEST_FIELD (bool, visible);

	THIEF_QUEST_FIELD (bool, final);
	THIEF_QUEST_FIELD (bool, irreversible);
	THIEF_QUEST_FIELD (bool, reverse);
#ifdef IS_THIEF2
	THIEF_QUEST_FIELD (bool, optional);
	THIEF_QUEST_FIELD (bool, bonus);
#endif // IS_THIEF2

	DifficultyQuestField difficulty;

	enum class Type { NONE, TAKE, SLAY, LOOT, GOTO };
	THIEF_QUEST_FIELD (Type, type);

	THIEF_QUEST_FIELD (Object, target);

	THIEF_QUEST_FIELD (int, loot_gold);
	THIEF_QUEST_FIELD (int, loot_gems);
	THIEF_QUEST_FIELD (int, loot_goods);
	THIEF_QUEST_FIELD (int, loot_total);

	typedef std::bitset<8> Specials;
	SpecialsQuestField loot_specials;

	static size_t count_objectives ();
	static void dump_objectives ();
};



class ObjectiveMessage : public Message // "ObjectiveChange"
{
public:
	enum Field
	{
		STATE, VISIBLE,
		FINAL, IRREVERSIBLE, REVERSE,
		MIN_DIFF, MAX_DIFF,
		TYPE, TARGET,
		GOLD, GEMS, GOODS, LOOT, SPECIAL,
#ifdef IS_THIEF2
		SPECIALS, OPTIONAL, BONUS
#endif
	};

	ObjectiveMessage (const Objective&, Field, int new_raw_value,
		int old_raw_value);
	THIEF_MESSAGE_WRAP (ObjectiveMessage);

	const Objective objective;
	const Field field;
	const int new_raw_value;
	const int old_raw_value;

private:
	typedef std::pair<Objective::Number, Field> ParseResult;
	static ParseResult parse (const char* quest_var);
};



} // namespace Thief

#endif // THIEF_QUESTVAR_HH

