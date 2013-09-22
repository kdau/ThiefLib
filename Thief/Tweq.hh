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

#define THIEF_TWEQ_COMMON \
	THIEF_PROP_FIELD (bool, simulate_always); \
	THIEF_PROP_FIELD (bool, simulate_far); \
	THIEF_PROP_FIELD (bool, simulate_near); \
	THIEF_PROP_FIELD (bool, simulate_onscreen); \
	\
	THIEF_PROP_FIELD (bool, jitter_low); \
	THIEF_PROP_FIELD (bool, jitter_high); \
	THIEF_PROP_FIELD (bool, jitter_multiply); \
	THIEF_PROP_FIELD (bool, random); \
	\
	THIEF_PROP_FIELD (bool, bounce); \
	THIEF_PROP_FIELD (bool, bounce_once); \
	THIEF_PROP_FIELD (bool, pendulum); \
	THIEF_PROP_FIELD (bool, unlimited); \
	THIEF_PROP_FIELD (bool, wrap); \
	THIEF_PROP_FIELD (Halt, halt_action); \
	\
	THIEF_PROP_FIELD (bool, subscribed); \
	\
	THIEF_PROP_FIELD (bool, active); \
	THIEF_PROP_FIELD (bool, reversed);

//TESTME: simulate_far, simulate_near, simulate_onscreen, jitter_low, jitter_high, jitter_multiply, random, bounce, bounce_once, pendulum, unlimited, wrap, subscribed, reversed



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

	THIEF_TWEQ_COMMON

	THIEF_PROP_FIELD (Time, duration);
	THIEF_PROP_FIELD (Time, current_time); //TESTME
};



class FlickerTweq : public virtual Tweq
{
public:
	THIEF_OBJECT_TYPE (FlickerTweq)
	bool has_flicker_tweq () const;

	THIEF_TWEQ_COMMON

	THIEF_PROP_FIELD (Time, rate);
	THIEF_PROP_FIELD (Time, current_time); //TESTME
	THIEF_PROP_FIELD (unsigned, current_frame); //TESTME
};



class LockTweq : public virtual Tweq
{
public:
	THIEF_OBJECT_TYPE (LockTweq)
	bool has_lock_tweq () const;

	THIEF_TWEQ_COMMON

	THIEF_PROP_FIELD (int, joint); //TESTME
	THIEF_PROP_FIELD (float, min_angle); //TESTME
	THIEF_PROP_FIELD (float, max_angle); //TESTME
	THIEF_PROP_FIELD (float, rate); //TESTME

	THIEF_PROP_FIELD (float, target_angle); //TESTME
	THIEF_PROP_FIELD (unsigned, current_stage); //TESTME
};



class ModelsTweq : public virtual Tweq
{
public:
	THIEF_OBJECT_TYPE (ModelsTweq)
	bool has_models_tweq () const;

	THIEF_TWEQ_COMMON

	THIEF_PROP_FIELD (Time, rate); //TESTME
	THIEF_PROP_FIELD_ARRAY (String, model, 6u); //TESTME
	THIEF_PROP_FIELD (bool, anchor_bottom); //TESTME
	THIEF_PROP_FIELD (bool, anchor_vhot); //TESTME
	THIEF_PROP_FIELD (bool, use_creature_scale); //TESTME
	THIEF_PROP_FIELD (bool, use_model_5); //TESTME

	THIEF_PROP_FIELD (Time, current_time); //TESTME
	THIEF_PROP_FIELD (unsigned, current_frame); //TESTME
};



} // namespace Thief

#endif // THIEF_TWEQ_HH

