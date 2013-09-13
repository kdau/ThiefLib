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

QuestVar::QuestVar (const String& _name)
	: name (_name)
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
QuestVar::set (int value, Scope scope)
{
	SService<IQuestSrv> (LG)->Set
		(name.data (), value, eQuestDataType (scope));
}

void
QuestVar::unset ()
{
	SService<IQuestSrv> (LG)->Delete (name.data ());
}

void
QuestVar::subscribe (const Object& host, Scope scope) const
{
	SService<IQuestSrv> (LG)->SubscribeMsg
		(host.number, name.data (), eQuestDataType (scope));
}

void
QuestVar::unsubscribe (const Object& host) const
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



// Objective

const Objective::Number
Objective::NONE = UINT_MAX;

Objective::Objective (Number _number)
	: number (_number)
{}

bool
Objective::exists () const
{
	return quest_var (Field::STATE).exists ();
}

Objective::State
Objective::get_state () const
{
	return State (quest_var (Field::STATE).get ());
}

void
Objective::set_state (State state)
{
	quest_var (Field::STATE).set (int (state));
}

bool
Objective::is_visible () const
{
	return bool (quest_var (Field::VISIBLE).get ());
}

void
Objective::set_visible (bool visible)
{
	quest_var (Field::VISIBLE).set (visible);
}

bool
Objective::is_final () const
{
	return bool (quest_var (Field::FINAL).get ());
}

void
Objective::set_final (bool final)
{
	quest_var (Field::FINAL).set (final);
}

bool
Objective::is_irreversible () const
{
	return bool (quest_var (Field::IRREVERSIBLE).get ());
}

void
Objective::set_irreversible (bool irreversible)
{
	quest_var (Field::IRREVERSIBLE).set (irreversible);
}

bool
Objective::is_reverse () const
{
	return bool (quest_var (Field::REVERSE).get ());
}

void
Objective::set_reverse (bool reverse)
{
	quest_var (Field::REVERSE).set (reverse);
}

#ifdef IS_THIEF2

bool
Objective::is_optional () const
{
	return bool (quest_var (Field::OPTIONAL).get ());
}

void
Objective::set_optional (bool optional)
{
	quest_var (Field::OPTIONAL).set (optional);
}

bool
Objective::is_bonus () const
{
	return bool (quest_var (Field::BONUS).get ());
}

void
Objective::set_bonus (bool bonus)
{
	quest_var (Field::BONUS).set (bonus);
}

#endif // IS_THIEF2

Difficulty
Objective::get_difficulty () const
{
	Difficulty min_diff = Difficulty (quest_var (Field::MIN_DIFF).get (-1)),
		max_diff = Difficulty (quest_var (Field::MAX_DIFF).get (-1));

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

void
Objective::set_difficulty (Difficulty difficulty)
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
	case Difficulty::NONE: default: return;
	}

	if (min_diff != Difficulty::NONE)
		quest_var (Field::MIN_DIFF).set (int (min_diff));
	if (max_diff != Difficulty::NONE)
		quest_var (Field::MAX_DIFF).set (int (max_diff));
}

Objective::Type
Objective::get_type () const
{
	return Type (quest_var (Field::TYPE).get ());
}

void
Objective::set_type (Type type)
{
	quest_var (Field::TYPE).set (int (type));
}

Object
Objective::get_target () const
{
	return Object (quest_var (Field::TARGET).get ());
}

void
Objective::set_target (const Object& target)
{
	quest_var (Field::TARGET).set (target.number);
}

Objective::Loot
Objective::get_loot () const
{
	int special = quest_var (Field::SPECIAL).get (),
		specials = quest_var (Field::SPECIALS).get ();
	if (special > 0)
		specials |= 1 << ((special - 1) & 0xFF);

	return {
		quest_var (Field::GOLD).get (),
		quest_var (Field::GEMS).get (),
		quest_var (Field::GOODS).get (),
		quest_var (Field::LOOT).get (),
		specials
	};
}

void
Objective::set_loot (const Loot& loot)
{
	quest_var (Field::GOLD).set (loot.gold);
	quest_var (Field::GEMS).set (loot.gems);
	quest_var (Field::GOODS).set (loot.goods);
	quest_var (Field::LOOT).set (loot.total);
	quest_var (Field::SPECIAL).unset ();
	quest_var (Field::SPECIALS).set (loot.specials.to_ulong ());
}

void
Objective::subscribe (const Object& host, Field field) const
{
	quest_var (field).subscribe (host);
}

void
Objective::unsubscribe (const Object& host, Field field) const
{
	quest_var (field).unsubscribe (host);
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
		switch (objective.get_state ())
		{
		case State::INCOMPLETE: state = "- inco"; break;
		case State::COMPLETE:   state = "+ comp"; break;
		case State::CANCELLED:  state = "/ canc"; break;
		case State::FAILED:     state = "X fail"; break;
		default:                state = "? ????"; break;
		}

		Type type = objective.get_type ();
		const char* type_name = nullptr;
		switch (type)
		{
		case Type::NONE: type_name = "none"; break;
		case Type::TAKE: type_name = "take"; break;
		case Type::SLAY: type_name = "slay"; break;
		case Type::LOOT: type_name = "loot"; break;
		case Type::GOTO: type_name = "goto"; break;
		default:         type_name = "????"; break;
		}

		const char* difficulty = nullptr;
		switch (objective.get_difficulty ())
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
			<< "  " << (objective.is_visible () ? '+' : '-')
			<< "  " << difficulty
			<< "    " << (objective.is_final () ? '+' : '-')
			<< "   " << (objective.is_irreversible () ? '+' : '-')
			<< "   " << (objective.is_reverse () ? '+' : '-')
#ifdef IS_THIEF2
			<< "   " << (objective.is_optional () ? '+' : '-')
			<< "   " << (objective.is_bonus () ? '+' : '-')
#endif // IS_THIEF2
			<< "    " << type_name << " ";
		if (type == Type::LOOT)
		{
			Loot loot = objective.get_loot ();
			String specials = loot.specials.to_string ();
			std::reverse (specials.begin (), specials.end ());
			mono << boost::format ("{ %||g, %||g, %||g, %||g; %|| }")
				% loot.gold % loot.gems % loot.goods
				% loot.total % specials;
		}
		else
			mono << objective.get_target ();
		mono << '\n';
	}

	mono << std::flush << std::internal;
}

typedef std::map<Objective::Field, CIString> ObjectiveFieldPrefixes;

static const ObjectiveFieldPrefixes&
get_field_prefixes ()
{
	static const ObjectiveFieldPrefixes FIELD_PREFIXES =
	{
		{ Objective::Field::STATE, "goal_state_" },
		{ Objective::Field::VISIBLE, "goal_visible_" },
		{ Objective::Field::FINAL, "goal_final_" },
		{ Objective::Field::IRREVERSIBLE, "goal_irreversible_" },
		{ Objective::Field::REVERSE, "goal_reverse_" },
#ifdef IS_THIEF2
		{ Objective::Field::OPTIONAL, "goal_optional_" },
		{ Objective::Field::BONUS, "goal_bonus_" },
#endif
		{ Objective::Field::MIN_DIFF, "goal_min_diff_" },
		{ Objective::Field::MAX_DIFF, "goal_max_diff_" },
		{ Objective::Field::TYPE, "goal_type_" },
		{ Objective::Field::TARGET, "goal_target_" },
		{ Objective::Field::GOLD, "goal_gold_" },
		{ Objective::Field::GEMS, "goal_gems_" },
		{ Objective::Field::GOODS, "goal_goods_" },
		{ Objective::Field::LOOT, "goal_loot_" },
		{ Objective::Field::SPECIAL, "goal_special_" },
		{ Objective::Field::SPECIALS, "goal_specials_" },
	};
	return FIELD_PREFIXES;
}

Objective::QVData
Objective::parse_quest_var (const QuestVar& quest_var)
{
	static const QVData BAD_QV = { NONE, Field (-1) };

	size_t number_pos = quest_var.name.find_first_of ("0123456789");
	if (number_pos == String::npos) return BAD_QV;

	String _prefix = quest_var.name.substr (0, number_pos),
		_number = quest_var.name.substr (number_pos);

	char* end = nullptr;
	Number number = strtol (_number.data (), &end, 10);
	if (end == _number.data ()) return BAD_QV;

	for (auto& prefix : get_field_prefixes ())
		if (prefix.second == _prefix)
			return { number, prefix.first };

	return BAD_QV;
}

QuestVar
Objective::quest_var (Field field) const
{
	std::ostringstream _quest_var;
	_quest_var << get_field_prefixes ().at (field).data () << number;
	return QuestVar (_quest_var.str ());
}



} // namespace Thief

