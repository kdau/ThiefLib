//! \file QuestVar.inl.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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
#error "This file should only be included from <Thief/QuestVar.hh>."
#endif

#ifndef THIEF_QUESTVAR_INL
#define THIEF_QUESTVAR_INL

namespace Thief {



// QuestVar

inline
QuestVar::operator int () const
{
	return get ();
}

inline QuestVar&
QuestVar::operator = (int value)
{
	set (value);
	return *this;
}



// Numbered

inline
Numbered::Numbered (Number _number)
	: number (_number)
{}

inline Numbered&
Numbered::operator ++ ()
{
	++number;
	return *this;
}

inline Numbered&
Numbered::operator -- ()
{
	++number;
	return *this;
}



// QuestField

#define THIEF_QUEST_FIELD(Type, Name) \
THIEF_FIELD_PROXY (QuestField, Type, , Name)

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (QuestField)::QuestField (Numbered& _numbered,
		size_t _index)
	: numbered (_numbered), index (_index)
{}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (QuestField)::exists () const
{
	return qvar ().exists ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline void
THIEF_FIELD_PROXY_CLASS (QuestField)::clear ()
{
	qvar ().clear ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (QuestField)::operator Type () const
{
	QuestVar _qvar (qvar ());
	return _qvar.exists () ? Type (_qvar.get ())
		: config->items [index].default_value;
}

THIEF_FIELD_PROXY_TEMPLATE
inline THIEF_FIELD_PROXY_CLASS (QuestField)&
THIEF_FIELD_PROXY_CLASS (QuestField)::operator = (const Type& value)
{
	qvar ().set (int (value));
	return *this;
}

THIEF_FIELD_PROXY_TEMPLATE
inline void
THIEF_FIELD_PROXY_CLASS (QuestField)::subscribe (const Object& host) const
{
	QuestVar::subscribe (qvar ().name, host);
}

THIEF_FIELD_PROXY_TEMPLATE
inline void
THIEF_FIELD_PROXY_CLASS (QuestField)::unsubscribe (const Object& host) const
{
	QuestVar::unsubscribe (qvar ().name, host);
}

THIEF_FIELD_PROXY_TEMPLATE
inline QuestVar
THIEF_FIELD_PROXY_CLASS (QuestField)::qvar (bool major) const
{
	return QuestVar ((boost::format (major ? config->items [index].major
		: config->items [index].minor) % numbered.number).str (),
		QuestVar::Scope (config->items [index].detail));
}



// DoubleQuestField

THIEF_FIELD_PROXY_TEMPLATE
class DoubleQuestField : protected THIEF_FIELD_PROXY_CLASS (QuestField)
{
public:
	using THIEF_FIELD_PROXY_CLASS (QuestField)::qvar;

	DoubleQuestField (Numbered&, size_t index);

	bool exists () const;
	void clear ();

	void subscribe (const Object& host) const;
	void unsubscribe (const Object& host) const;
};

THIEF_FIELD_PROXY_TEMPLATE
inline
THIEF_FIELD_PROXY_CLASS (DoubleQuestField)::DoubleQuestField
		(Numbered& _numbered, size_t _index)
	: THIEF_FIELD_PROXY_CLASS (QuestField) (_numbered, _index)
{}

THIEF_FIELD_PROXY_TEMPLATE
inline bool
THIEF_FIELD_PROXY_CLASS (DoubleQuestField)::exists () const
{
	return qvar (true).exists () || qvar (false).exists ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline void
THIEF_FIELD_PROXY_CLASS (DoubleQuestField)::clear ()
{
	qvar (true).clear ();
	qvar (false).clear ();
}

THIEF_FIELD_PROXY_TEMPLATE
inline void
THIEF_FIELD_PROXY_CLASS (DoubleQuestField)::subscribe (const Object& host) const
{
	QuestVar::subscribe (qvar (true).name, host);
	QuestVar::subscribe (qvar (false).name, host);
}

THIEF_FIELD_PROXY_TEMPLATE
inline void
THIEF_FIELD_PROXY_CLASS (DoubleQuestField)::unsubscribe (const Object& host)
	const
{
	QuestVar::unsubscribe (qvar (true).name, host);
	QuestVar::unsubscribe (qvar (false).name, host);
}



// DifficultyQuestField

#ifdef IS_DOXYGEN
#define DifficultyQuestField QuestField<Difficulty>
#else

extern const FieldProxyConfig<Difficulty> F_objective_difficulty;

class DifficultyQuestField
	: public DoubleQuestField<Difficulty, &F_objective_difficulty>
{
public:
	DifficultyQuestField (Numbered&, size_t index = 0u);

	operator Difficulty () const;
	DifficultyQuestField& operator = (Difficulty);
};

inline
DifficultyQuestField::DifficultyQuestField (Numbered& _numbered, size_t _index)
	: DoubleQuestField<Difficulty, &F_objective_difficulty> (_numbered,
		_index)
{}

#endif // !IS_DOXYGEN



// SpecialsQuestField

#ifdef IS_DOXYGEN
#define SpecialsQuestField QuestField<Objective::Specials>
#else

extern const FieldProxyConfig<std::bitset<8>> F_objective_specials;

class SpecialsQuestField
	: public DoubleQuestField<std::bitset<8>, &F_objective_specials>
{
public:
	SpecialsQuestField (Numbered&, size_t index = 0u);

	operator std::bitset<8> () const;
	SpecialsQuestField& operator = (const std::bitset<8>&);
};

inline
SpecialsQuestField::SpecialsQuestField (Numbered& _numbered, size_t _index)
	: DoubleQuestField<std::bitset<8>, &F_objective_specials> (_numbered,
		_index)
{}

#endif // !IS_DOXYGEN



} // namespace Thief

#endif // THIEF_QUESTVAR_INL

