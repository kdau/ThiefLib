/******************************************************************************
 *  QuestVar.cc
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



// QuestVar

QuestVar::QuestVar (const String& _name, Scope _scope)
	: name (_name), scope (_scope)
{}

bool
QuestVar::exists () const
{
	return SService<IQuestSrv> (LG)->Exists (name.data ());
}

int
QuestVar::get (int default_value) const
{
	if (default_value != 0 && !exists ())
		return default_value;
	else
		return SService<IQuestSrv> (LG)->Get (name.data ());
}

void
QuestVar::set (int value)
{
	SService<IQuestSrv> (LG)->Set
		(name.data (), value, eQuestDataType (scope));
}

void
QuestVar::clear ()
{
	SService<IQuestSrv> (LG)->Delete (name.data ());
}

void
QuestVar::subscribe (const String& name, const Object& host, Scope scope)
{
	SService<IQuestSrv> (LG)->SubscribeMsg
		(host.number, name.data (), eQuestDataType (scope));
}

void
QuestVar::unsubscribe (const String& name, const Object& host)
{
	SService<IQuestSrv> (LG)->UnsubscribeMsg (host.number, name.data ());
}



// QuestMessage

MESSAGE_WRAPPER_IMPL (QuestMessage, sQuestMsg),
	quest_var (MESSAGE_AS (sQuestMsg)->m_pName
		? MESSAGE_AS (sQuestMsg)->m_pName : ""),
	new_value (MESSAGE_AS (sQuestMsg)->m_newValue),
	old_value (MESSAGE_AS (sQuestMsg)->m_oldValue)
{}

QuestMessage::QuestMessage (const char* _quest_var, int _new_value,
		int _old_value)
	: Message (new sQuestMsg ()), quest_var (_quest_var ? _quest_var : ""),
	  new_value (_new_value), old_value (_old_value)
{
	message->message = "QuestChange";
	MESSAGE_AS (sQuestMsg)->m_pName = _quest_var;
	MESSAGE_AS (sQuestMsg)->m_newValue = new_value;
	MESSAGE_AS (sQuestMsg)->m_oldValue = old_value;
}



// DifficultyQuestField

const FieldProxyConfig<Difficulty>::Item
I_objective_difficulty { "goal_min_diff_%||", "goal_max_diff_%||",
	int (QuestVar::Scope::MISSION), Difficulty::NONE }; \

const FieldProxyConfig<Difficulty>
F_objective_difficulty { &I_objective_difficulty, 1u, nullptr, nullptr };

DifficultyQuestField::operator Difficulty () const
{
	Difficulty min_diff = Difficulty (qvar (true).get (-1)),
		max_diff = Difficulty (qvar (false).get (-1));

	switch (min_diff)
	{
	case Difficulty::NONE:
	case Difficulty::NORMAL:
		switch (max_diff)
		{
		case Difficulty::NONE: case Difficulty::EXPERT:
			return Difficulty::ANY;
		case Difficulty::NORMAL:
			return Difficulty::NORMAL;
		case Difficulty::HARD:
			return Difficulty::NOT_EXPERT;
		default:
			return Difficulty::NONE;
		}
	case Difficulty::HARD:
		switch (max_diff)
		{
		case Difficulty::NONE: case Difficulty::EXPERT:
			return Difficulty::NOT_NORMAL;
		case Difficulty::HARD:
			return Difficulty::HARD;
		case Difficulty::NORMAL: default:
			return Difficulty::NONE;
		}
	case Difficulty::EXPERT:
		switch (max_diff)
		{
		case Difficulty::NONE: case Difficulty::EXPERT:
			return Difficulty::EXPERT;
		case Difficulty::NORMAL: case Difficulty::HARD: default:
			return Difficulty::NONE;
		}
	default:
		return Difficulty::NONE;
	}
}

DifficultyQuestField&
DifficultyQuestField::operator = (Difficulty difficulty)
{
	Difficulty min_diff = Difficulty::NONE,
		max_diff = Difficulty::NONE;

	switch (difficulty)
	{
	case Difficulty::NORMAL:     max_diff = Difficulty::NORMAL; break;
	case Difficulty::HARD:       min_diff = max_diff = Difficulty::HARD; break;
	case Difficulty::EXPERT:     min_diff = Difficulty::EXPERT; break;
	case Difficulty::NOT_EXPERT: max_diff = Difficulty::HARD; break;
	case Difficulty::NOT_NORMAL: min_diff = Difficulty::HARD; break;
	case Difficulty::ANY:        break;
	case Difficulty::NONE:
	default:
		throw std::runtime_error ("invalid difficulty");
	}

	if (min_diff != Difficulty::NONE)
		qvar (true) = int (min_diff);
	else
		qvar (true).clear ();

	if (max_diff != Difficulty::NONE)
		qvar (false) = int (max_diff);
	else
		qvar (false).clear ();

	return *this;
}



// SpecialsQuestField

const FieldProxyConfig<Objective::Specials>::Item
I_objective_specials { "goal_special_%||", "goal_specials_%||",
	int (QuestVar::Scope::MISSION), 0ull }; \

const FieldProxyConfig<Objective::Specials>
F_objective_specials { &I_objective_specials, 1u, nullptr, nullptr };

SpecialsQuestField::operator Objective::Specials () const
{
#ifdef IS_THIEF2
	int special = qvar (true), specials = qvar (false);
	if (special > 0)
		specials |= 1 << ((special - 1) & 0xFF);
	return specials;
#else
	return int (qvar (true));
#endif
}

SpecialsQuestField&
SpecialsQuestField::operator = (const Objective::Specials& specials)
{
#ifdef IS_THIEF2
	qvar (true).clear ();
	qvar (false) = specials.to_ulong ();
#else
	qvar (true) = specials.to_ulong ();
	qvar (false).clear ();
#endif
	return *this;
}



// Objective

const Objective::Number
Objective::NONE = UINT_MAX;

#define QUEST_PROXY_CONFIG_(Class, Member, Major, Type, Default, Scope) \
PROXY_CONFIG_ (Class, Member, Major, nullptr, Type, Default, Scope, nullptr, \
	nullptr)

#define QUEST_PROXY_CONFIG(Class, Member, Major, Type, Default) \
QUEST_PROXY_CONFIG_ (Class, Member, Major, Type, Default, \
	QuestVar::Scope::MISSION)

QUEST_PROXY_CONFIG (Objective, state, "goal_state_%||",
	Objective::State, State::INCOMPLETE);
QUEST_PROXY_CONFIG (Objective, visible, "goal_visible_%||", bool, false);
QUEST_PROXY_CONFIG (Objective, final, "goal_final_%||", bool, false);
QUEST_PROXY_CONFIG (Objective, irreversible, "goal_irreversible_%||",
	bool, false);
QUEST_PROXY_CONFIG (Objective, reverse, "goal_reverse_%||", bool, false);
#ifdef IS_THIEF2
QUEST_PROXY_CONFIG (Objective, optional, "goal_optional_%||", bool, false);
QUEST_PROXY_CONFIG (Objective, bonus, "goal_bonus_%||", bool, false);
#endif
QUEST_PROXY_CONFIG (Objective, type, "goal_type_%||",
	Objective::Type, Type::NONE);
QUEST_PROXY_CONFIG (Objective, target, "goal_target_%||", Object, Object::NONE);
QUEST_PROXY_CONFIG (Objective, loot_gold, "goal_gold_%||", int, 0);
QUEST_PROXY_CONFIG (Objective, loot_gems, "goal_gems_%||", int, 0);
QUEST_PROXY_CONFIG (Objective, loot_goods, "goal_goods_%||", int, 0);
QUEST_PROXY_CONFIG (Objective, loot_total, "goal_loot_%||", int, 0);

#define OBJECTIVE_IMPL(...) \
Objective::Objective (Number _number) : Numbered (_number), __VA_ARGS__ {} \
Objective::Objective (const Objective& copy) : Numbered (copy), __VA_ARGS__ {} \
Objective& Objective::operator = (const Objective& copy) \
	{ number = copy.number; return *this; }

OBJECTIVE_IMPL (
	  PROXY_INIT (state),
	  PROXY_INIT (visible),
	  PROXY_INIT (final),
	  PROXY_INIT (irreversible),
	  PROXY_INIT (reverse),
#ifdef IS_THIEF2
	  PROXY_INIT (optional),
	  PROXY_INIT (bonus),
#endif
	  PROXY_INIT (difficulty),
	  PROXY_INIT (type),
	  PROXY_INIT (target),
	  PROXY_INIT (loot_gold),
	  PROXY_INIT (loot_gems),
	  PROXY_INIT (loot_goods),
	  PROXY_INIT (loot_total),
	  PROXY_INIT (loot_specials)
)

bool
Objective::exists () const
{
	return state.exists ();
}

size_t
Objective::count_objectives ()
{
	size_t count = 0;
	for (Objective objective = 0; objective.exists (); ++objective.number)
		++count;
	return count;
}

void
Objective::dump_objectives ()
{
	if (!Engine::is_editor ()) return; // Monolog is a no-op in the game.

	mono << "Dumping objectives...\n";
#ifdef IS_THIEF2
	mono << "###   State  Vis Dif   Fin Irr Rev Opt Bon   Type Target\n";
	mono << "===   ====== === ===   === === === === ===   ==== ====================\n";
#else // !IS_THIEF2
	mono << "###   State  Vis Dif   Fin Irr Rev   Type Target\n";
	mono << "===   ====== === ===   === === ===   ==== ====================\n";
#endif // IS_THIEF2

	mono << std::right;
	for (Objective objective = 0; objective.exists (); ++objective.number)
	{
		const char* state = nullptr;
		switch (objective.state)
		{
		case State::INCOMPLETE: state = "- inco"; break;
		case State::COMPLETE:   state = "+ comp"; break;
		case State::CANCELLED:  state = "/ canc"; break;
		case State::FAILED:     state = "X fail"; break;
		default:                state = "? ????"; break;
		}

		const char* type = nullptr;
		switch (objective.type)
		{
		case Type::NONE: type = "none"; break;
		case Type::TAKE: type = "take"; break;
		case Type::SLAY: type = "slay"; break;
		case Type::LOOT: type = "loot"; break;
		case Type::GOTO: type = "goto"; break;
		default:         type = "????"; break;
		}

		const char* difficulty = nullptr;
		switch (objective.difficulty)
		{
		case Difficulty::NOT_EXPERT: difficulty = "NH-"; break;
		case Difficulty::NOT_NORMAL: difficulty = "-HE"; break;
		case Difficulty::ANY:        difficulty = "NHE"; break;
		case Difficulty::NORMAL:     difficulty = "N--"; break;
		case Difficulty::HARD:       difficulty = "-H-"; break;
		case Difficulty::EXPERT:     difficulty = "--E"; break;
		default:                     difficulty = "???"; break;
		}

		mono << std::setw (3) << objective.number
			<< "   " << state
			<< "  " << (objective.visible ? '+' : '-')
			<< "  " << difficulty
			<< "    " << (objective.final ? '+' : '-')
			<< "   " << (objective.irreversible ? '+' : '-')
			<< "   " << (objective.reverse ? '+' : '-')
#ifdef IS_THIEF2
			<< "   " << (objective.optional ? '+' : '-')
			<< "   " << (objective.bonus ? '+' : '-')
#endif // IS_THIEF2
			<< "    " << type << " ";
		if (objective.type == Type::LOOT)
		{
			String specials =
				Specials (objective.loot_specials).to_string ();
			std::reverse (specials.begin (), specials.end ());
			mono << boost::format ("{ %||g, %||g, %||g, %||g; %|| }")
				% objective.loot_gold % objective.loot_gems
				% objective.loot_goods % objective.loot_total
				% specials;
		}
		else
			mono << objective.target;
		mono << '\n';
	}

	mono << std::flush << std::internal;
}



// ObjectiveMessage

THIEF_ENUM_CODING (ObjectiveMessage::Field, CODE, CODE,
	THIEF_ENUM_VALUE (STATE, "state"),
	THIEF_ENUM_VALUE (VISIBLE, "visible"),
	THIEF_ENUM_VALUE (FINAL, "final"),
	THIEF_ENUM_VALUE (IRREVERSIBLE, "irreversible"),
	THIEF_ENUM_VALUE (REVERSE, "reverse"),
	THIEF_ENUM_VALUE (MIN_DIFF, "min_diff"),
	THIEF_ENUM_VALUE (MAX_DIFF, "max_diff"),
	THIEF_ENUM_VALUE (TYPE, "type"),
	THIEF_ENUM_VALUE (TARGET, "target"),
	THIEF_ENUM_VALUE (GOLD, "gold"),
	THIEF_ENUM_VALUE (GEMS, "gems"),
	THIEF_ENUM_VALUE (GOODS, "goods"),
	THIEF_ENUM_VALUE (LOOT, "loot"),
	THIEF_ENUM_VALUE (SPECIAL, "special"),
#ifdef IS_THIEF2
	THIEF_ENUM_VALUE (SPECIALS, "specials"),
	THIEF_ENUM_VALUE (OPTIONAL, "optional"),
	THIEF_ENUM_VALUE (BONUS, "bonus"),
#endif
)

MESSAGE_WRAPPER_IMPL_ (ObjectiveMessage,
	MESSAGE_TYPENAME_TEST ("sQuestMsg") &&
		parse (static_cast<sQuestMsg*> (_message)->m_pName).first
			!= Objective::NONE),
	objective (parse (MESSAGE_AS (sQuestMsg)->m_pName).first),
	field (parse (MESSAGE_AS (sQuestMsg)->m_pName).second),
	new_raw_value (MESSAGE_AS (sQuestMsg)->m_newValue),
	old_raw_value (MESSAGE_AS (sQuestMsg)->m_oldValue)
{}

ObjectiveMessage::ObjectiveMessage (const Objective& _objective, Field _field,
		int _new_raw_value, int _old_raw_value)
	: Message (new sQuestMsg ()), objective (_objective), field (_field),
	  new_raw_value (_new_raw_value), old_raw_value (_old_raw_value)
{
	message->message = "QuestChange";
	MESSAGE_AS (sQuestMsg)->m_newValue = new_raw_value;
	MESSAGE_AS (sQuestMsg)->m_oldValue = old_raw_value;

	boost::format qvar_name ("goal_%||_%||");
	qvar_name % EnumCoding::get<Field> ().encode (field) % objective.number;
	MESSAGE_AS (sQuestMsg)->m_pName = qvar_name.str ().data (); //FIXME This data will be lost.
}

ObjectiveMessage::ParseResult
ObjectiveMessage::parse (const char* _quest_var)
{
	static const ParseResult BAD_QV { Objective::NONE, Field (-1) };

	if (!_quest_var) return BAD_QV;
	String quest_var { _quest_var };

	if (quest_var.size () < 8 || quest_var.substr (0, 5) != "goal_")
		return BAD_QV;

	size_t number_pos = quest_var.find_first_of ("0123456789");
	if (number_pos == String::npos || quest_var [number_pos - 1] != '_')
		return BAD_QV;

	String _field = quest_var.substr (5, number_pos - 6),
		_number = quest_var.substr (number_pos);

	Field field = Field (-1);
	try
	{
		field = Field (EnumCoding::get<Field> ().decode (_field));
	}
	catch (...) { return BAD_QV; }

	Objective::Number number = Objective::NONE;
	try
	{
		number = std::stoul (_number, nullptr, 10);
	}
	catch (...) { return BAD_QV; }

	return { number, field };
}



} // namespace Thief

