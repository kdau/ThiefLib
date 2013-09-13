/******************************************************************************
 *  Tweq.hh
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

#ifndef THIEF_TWEQ_HH
#define THIEF_TWEQ_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>
#include <Thief/Object.hh>

namespace Thief {



class Tweq : public virtual Object
{
public:
	enum class Type { NONE = -1,
		SCALE, ROTATE, JOINTS, MODELS, DELETE, EMIT, FLICKER, LOCK };

	enum class Direction { FORWARD, REVERSE };

	enum class Halt
		{ DESTROY_OBJECT, REMOVE, HALT, CONTINUE, SLAY_OBJECT };

protected:
	THIEF_OBJECT_TYPE (Tweq)
};



class TweqMessage : public Message // "TweqComplete" //TESTME
{
public:
	enum Event { DESTROY, REMOVE, HALT, CONTINUE, SLAY, FRAME };

	TweqMessage (Event, Tweq::Type, Tweq::Direction);
	THIEF_MESSAGE_WRAP (TweqMessage);

	const Event event;
	const Tweq::Type tweq_type;
	const Tweq::Direction direction;
};



class DeleteTweq : public virtual Tweq
{
public:
	THIEF_OBJECT_TYPE (DeleteTweq)
	bool has_delete_tweq () const;

	THIEF_PROP_FIELD (bool, simulate_always);
	THIEF_PROP_FIELD (bool, simulate_far); //TESTME
	THIEF_PROP_FIELD (bool, simulate_near); //TESTME
	THIEF_PROP_FIELD (bool, simulate_onscreen); //TESTME

	THIEF_PROP_FIELD (Time, duration);
	THIEF_PROP_FIELD (Halt, halt_action);

	THIEF_PROP_FIELD (bool, active);
	THIEF_PROP_FIELD (Time, current_time); //TESTME
};



} // namespace Thief

#endif // THIEF_TWEQ_HH

