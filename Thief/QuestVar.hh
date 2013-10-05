//! \file QuestVar.hh Access to quest variables, including objectives.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_QUESTVAR_HH
#define THIEF_QUESTVAR_HH

#include <Thief/Base.hh>
#include <Thief/Mission.hh>

namespace Thief {



/*! A numeric variable in the quest database.
 * The quest variable database is used to store objective configuration, mission
 * and campaign statistics, and other numeric values that pertain to the mission
 * or campaign as a whole. Though it supports only integers, it is the only
 * freeform data store that is directly available to mission authors.
 *
 * The Objective class provides organized access to objective-related quest
 * variables. */
class QuestVar
{
public:
	//! A storage duration for a quest variable.
	enum class Scope
	{
		MISSION,  /*!< The variable will be stored for the current
		           * mission only. */
		CAMPAIGN, /*!< The variable will be stored through all the
		           * missions in the current campaign. */
		ANY       /*!< Consider both mission and campaign quest
		           * variables. This value is not valid as a variable's
		           * scope, but is for use with the subscribe() method. */
	};

	/*! Constructs a new quest variable reference.
	 * The named variable will not be created if absent, but if and when it
	 * is created by this QuestVar instance, it will have the given scope. */
	QuestVar (const String& name, Scope = Scope::MISSION);

	//! The name of the referenced quest variable.
	String name;

	//! The scope that will be assigned to the referenced quest variable.
	Scope scope;

	/*! Returns whether the referenced variable is present in the quest
	 * database. */
	bool exists () const;

	/*! Returns the current value of the referenced quest variable.
	 * Returns \c 0 if the variable does not exist. */
	operator int () const;

	/*! Returns the current value of the referenced quest variable.
	 * Returns \a default_value if the variable does not exist. */
	int get (int default_value = 0) const;

	//! Sets the referenced quest variable to the given value.
	QuestVar& operator = (int);

	//! Sets the referenced quest variable to the given value.
	void set (int);

	//! Removes the referenced variable from the quest database.
	void clear ();

	/*! Subscribes to any changes in the named quest variable's value.
	 * A "QuestChange" message will be sent to all scripts on the given
	 * \a host object if the variable \a name with scope \a scope is set or
	 * cleared. */
	static void subscribe (const String& name, const Object& host,
		Scope scope = Scope::ANY);

	/*! Unsubscribes from any changes in the named quest variable's value.
	 * Only messages to scripts on the given \a host object will stop. */
	static void unsubscribe (const String& name, const Object& host);
};



/*! A message about a change to a quest variable.
 * A quest message is sent whenever a quest variable is set or cleared. It is
 * sent only to scripts hosted by objects that are subscribed to the variable
 * (see QuestVariable::subscribe and QuestVariable::unsubscribe). \note
 * %Message name: \c QuestChange */
class QuestMessage : public Message
{
public:
	/*! Constructs a new \c QuestChange message for the named variable.
	 * \warning Due to engine limitations, it is strongly recommended that
	 * the \a quest_var string be a constant with static storage duration.
	 * If a temporary or non-constant string must be used, ensure that it
	 * will remain allocated and unchanged for the lifetime of the message.
	 * This is straightforward with the send() method but not with the
	 * post() and schedule() methods. */
	QuestMessage (const char* quest_var, int new_value, int old_value);

	THIEF_MESSAGE_WRAP (QuestMessage);

	//! The quest variable that has changed.
	const QuestVar quest_var;

	//! The new value of the quest variable, or 0 if it has been cleared.
	const int new_value;

	//! The old value of the quest variable, or 0 if it did not exist yet.
	const int old_value;
};



/*! Base class for classes whose references are array-index-like.
 * Objective is derived from this class to allow it to use QuestField. */
struct Numbered
{
	//! A numeric reference.
	typedef unsigned int Number;

	//! The numeric reference.
	Number number;

	//! Constructs a new numeric reference with the given number.
	Numbered (Number);

	//! Increments the numeric reference.
	Numbered& operator ++ ();

	//! Decrements the numeric reference.
	Numbered& operator -- ();
};



/*! An Objective field that is stored in the quest database.
 * This class serves as a proxy to the QuestVar (one or more) that underlies
 * a field on a class such as Objective. Like all field proxies, it is not
 * intended for construction by ThiefLib users. */
THIEF_FIELD_PROXY_TEMPLATE
class QuestField
{
public:
	//! Constructs a new quest field proxy for the given indices.
	QuestField (Numbered&, size_t index);

	//! Returns whether the field is currently set.
	bool exists () const;

	//! Removes any value set on the field.
	void clear ();

	//! Returns the current value of the field.
	operator Type () const;

	//! Sets the field to the given value.
	QuestField& operator = (const Type&);

	/*! Subscribes to the quest variable that underlies this field.
	 * A "QuestChange" message will be sent to all scripts on the given
	 * \a host object if the underlying quest variable is set or cleared.
	 * If more than one variable is involved, all will be subscribed.
	 * Changes to Objective fields will also result in "ObjectiveChange"
	 * messages. */
	void subscribe (const Object& host) const;

	//! Unsubscribes from the quest variable that underlies this field.
	void unsubscribe (const Object& host) const;

protected:
	//! \cond HIDDEN_SYMBOLS
	QuestVar qvar (bool major = true) const;
	Numbered& numbered;
	size_t index;
	//! \endcond
};



} // namespace Thief

#include <Thief/QuestVar.inl>

namespace Thief {



/*! A task to be completed or requirement to be satisfied in a mission.
 * The objective system shapes the progress of a %Thief mission, defining who or
 * what the player must, must not, or may steal, kill, reach, or otherwise deal
 * with. This class offers structured access to all the quest variables which
 * define an objective's configuration and status. The \c convict.osm module
 * controls the behavior of objectives; see the \c convict.doc reference and
 * various fan-compiled references for more information. */
class Objective : public Numbered
{
public:
	//! Indicates that no valid objective is referenced.
	static const Number NONE;

	//! Constructs a reference to the objective with the given number.
	Objective (Number = NONE);

	//! Constructs a reference to the same objective as the given reference.
	Objective (const Objective&);

	//! Sets this instance to refer to the same objective as another.
	Objective& operator = (const Objective&);

	/*! Returns whether the referenced objective currently exists.
	 * An objective exists if its state variable exists. */
	bool exists () const;

	//! A level of satisfaction of an objective's conditions.
	enum class State
	{
		INCOMPLETE, //!< The objective's conditions are yet to be met.
		COMPLETE,   //!< The objective's conditions have been met.
		CANCELLED,  /*!< The objective will no longer be considered.
		             * Optional objectives that would be failed if
		             * mandatory are cancelled instead. */
		FAILED      /*!< The mandatory objective cannot be met.
		             * The mission will immediately end in failure. */
	};

	//! The current state of the referenced objective.
	THIEF_QUEST_FIELD (State, state);

	/*! Whether the referenced objective is currently displayed to the
	 * player. Invisible objectives are not considered when checking for
	 * mission completion. */
	THIEF_QUEST_FIELD (bool, visible);

	/*! Whether the referenced objective is one of the last to be completed.
	 * Final objectives will not be marked as complete until all applicable
	 * non-final objectives have been completed. */
	THIEF_QUEST_FIELD (bool, final);

	/*! Whether the referenced objective cannot return to state #INCOMPLETE.
	 * Irreversible objectives will remain complete even if their conditions
	 * are no longer met. */
	THIEF_QUEST_FIELD (bool, irreversible);

	/*! Whether the referenced objective should be avoided instead of met.
	 * Reverse objectives will be failed if their conditions are met, and
	 * completed if they are incomplete at mission end. */
	THIEF_QUEST_FIELD (bool, reverse);

#ifdef IS_THIEF2

	/*! Whether the referenced objective may be left incomplete.
	 * Optional objectives do not have to be completed for the mission to
	 * succeed, and their breach will not cause mission failure. Note that
	 * objectives cannot be both optional and reverse without special
	 * adjustments. \t2only */
	THIEF_QUEST_FIELD (bool, optional);

	/*! Whether the referenced objective is a hidden bonus.
	 * Bonus objectives are invisible (and thus not considered) until and
	 * unless they are completed, at which time they are revealed. They
	 * allow the player to be rewarded for performing tasks not explicitly
	 * requested in e.g. an optional objective. \t2only */
	THIEF_QUEST_FIELD (bool, bonus);

#endif // IS_THIEF2

	/*! The range of difficulties to which the referenced objective applies.
	 * At other difficulties, the objective will be invisible and inert. */
	DifficultyQuestField difficulty;

	//! A type of condition required by an objective.
	enum class Type
	{
		NONE, /*!< The objective will not be automatically completed
		       * in any conditions. A script must instead set the
		       * objective's state to #COMPLETE when appropriate. */
		TAKE, /*!< The player must put the #target object in inventory.
		       * The conditions will no longer be met if the object is
		       * dropped. */
		SLAY, /*!< The player must kill the #target object.
		       * If the #target is an archetype, the player must kill
		       * at least one instance of that archetype or a descendant
		       * archetype. The #target must have the \c JAccuse script
		       * for this type to work automatically. */
		LOOT, //!< The player must steal a certain amount of loot.
		GOTO  /*!< The player must enter the #target room.
		       * The conditions will no longer be met if the player
		       * exits the room. */
	};

	/*! The type of condition which must be satisfied to complete the
	 * referenced objective. */
	THIEF_QUEST_FIELD (Type, type);

	/*! The object relevant to the referenced objective's conditions.
	 * See the #TAKE, #SLAY, and #GOTO types. */
	THIEF_QUEST_FIELD (Object, target);

	/*! The amount of gold that the player must steal.
	 * This field only applies to #LOOT objectives. */
	THIEF_QUEST_FIELD (int, loot_gold);

	/*! The amount of gems that the player must steal.
	 * This field only applies to #LOOT objectives. */
	THIEF_QUEST_FIELD (int, loot_gems);

	/*! The amount of goods that the player must steal.
	 * This field only applies to #LOOT objectives. */
	THIEF_QUEST_FIELD (int, loot_goods);

	/*! The total loot amount that the player must steal.
	 * This is independent of any gold, gems, or goods requirement. This
	 * field only applies to #LOOT objectives. */
	THIEF_QUEST_FIELD (int, loot_total);

	/*! A selection of special loot items.
	 * An object with a loot value cannot be targeted with a #TAKE-type
	 * objective. Instead, it must carry one (or more) of eight special loot
	 * item flags. This bitset defines which special item or items apply to
	 * an objective. */
	typedef std::bitset<8> Specials;

	/*! The special loot item or items that the player must steal.
	 * This is independent of any numeric loot requirement. This field only
	 * applies to #LOOT objectives. */
	SpecialsQuestField loot_specials;

	/*! Returns the number of objectives that are currently defined.
	 * Objectives are checked based on their state variables, starting at 0.
	 * A break in the number sequence will end the count. */
	static size_t count_objectives ();

	/*! Outputs a table of objectives to the monolog.
	 * The table includes the objectives' configuration and current state. */
	static void dump_objectives ();
};



/*! A message about a change to an objective.
 * An objective message is sent whenever an objective-related quest variable is
 * set or cleared. It is sent only to scripts hosted by objects that are
 * subscribed to the variable (through the QuestField::subscribe method on the
 * desired objective number and field). \note This is a virtual message created
 * internally by ThiefLib. It is not available to non-ThiefLib scripts.
 * \note %Message name: \c ObjectiveChange */
class ObjectiveMessage : public Message
{
public:
	/*! A type of objective-related quest variable.
	 * The related Objective field is listed for each. */
	enum Field
	{
		STATE,        //!< Objective::state
		VISIBLE,      //!< Objective::visible
		FINAL,        //!< Objective::final
		IRREVERSIBLE, //!< Objective::irreversible
		REVERSE,      //!< Objective::reverse
		MIN_DIFF,     //!< Objective::difficulty (partial)
		MAX_DIFF,     //!< Objective::difficulty (partial)
		TYPE,         //!< Objective::type
		TARGET,       //!< Objective::target
		GOLD,         //!< Objective::loot_gold
		GEMS,         //!< Objective::loot_gems
		GOODS,        //!< Objective::loot_goods
		LOOT,         //!< Objective::loot_total
		SPECIAL,      //!< Objective::loot_specials (partial)
#ifdef IS_THIEF2
		SPECIALS,     //!< Objective::loot_specials (partial) \t2only
		OPTIONAL,     //!< Objective::optional \t2only
		BONUS         //!< Objective::bonus \t2only
#endif
	};

	/*! Constructs a new \c ObjectiveChange message for the given objective
	 * and field. */
	ObjectiveMessage (const Objective&, Field, int new_raw_value,
		int old_raw_value);

	THIEF_MESSAGE_WRAP (ObjectiveMessage);

	//! The objective whose field has changed.
	const Objective objective;

	//! The field that has changed.
	const Field field;

	//! The new value of the quest variable underlying the field.
	const int new_raw_value;

	//! The old value of the quest variable underlying the field.
	const int old_raw_value;

private:
	typedef std::pair<Objective::Number, Field> ParseResult;
	static ParseResult parse (const char* quest_var);
};



} // namespace Thief

#endif // THIEF_QUESTVAR_HH

