/******************************************************************************
 *  Tweq.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 *
 *****************************************************************************/

#include "Private.hh"

namespace Thief {



// Tweq

OBJECT_TYPE_IMPL (Tweq)

#define TWEQ_COMMON_CONFIG(Type, ConfigProp, StateProp) \
PROXY_BIT_CONFIG (Type, simulate_always, ConfigProp, "AnimC", 2u, false); \
PROXY_BIT_CONFIG (Type, simulate_far, ConfigProp, "AnimC", 32u, false); \
PROXY_BIT_CONFIG (Type, simulate_near, ConfigProp, "AnimC", 16u, false); \
PROXY_NEG_BIT_CONFIG (Type, simulate_onscreen, ConfigProp, "AnimC", 64u, true); \
\
PROXY_BIT_CONFIG (Type, jitter_low, ConfigProp, "CurveC", 1u, false); \
PROXY_BIT_CONFIG (Type, jitter_high, ConfigProp, "CurveC", 2u, false); \
PROXY_BIT_CONFIG (Type, jitter_multiply, ConfigProp, "CurveC", 4u, false); \
PROXY_BIT_CONFIG (Type, random, ConfigProp, "AnimC", 4u, false); \
\
PROXY_BIT_CONFIG (Type, bounce, ConfigProp, "CurveC", 16u, false); \
PROXY_BIT_CONFIG (Type, bounce_once, ConfigProp, "AnimC", 8u, false); \
PROXY_BIT_CONFIG (Type, pendulum, ConfigProp, "CurveC", 8u, false); \
PROXY_BIT_CONFIG (Type, unlimited, ConfigProp, "AnimC", 1u, false); \
PROXY_BIT_CONFIG (Type, wrap, ConfigProp, "AnimC", 4u, false); \
PROXY_CONFIG (Type, halt_action, ConfigProp, "Halt", \
	Tweq::Halt, Halt::DESTROY_OBJECT); \
\
PROXY_BIT_CONFIG (Type, subscribed, ConfigProp, "MiscC", 2u, false); \
\
PROXY_BIT_CONFIG (Type, active, StateProp, "AnimS", 1u, false); \
PROXY_BIT_CONFIG (Type, reversed, StateProp, "AnimS", 2u, false);

#define TWEQ_COMMON_INIT \
	PROXY_INIT (simulate_always), \
	PROXY_INIT (simulate_far), \
	PROXY_INIT (simulate_near), \
	PROXY_INIT (simulate_onscreen), \
	PROXY_INIT (jitter_low), \
	PROXY_INIT (jitter_high), \
	PROXY_INIT (jitter_multiply), \
	PROXY_INIT (random), \
	PROXY_INIT (bounce), \
	PROXY_INIT (bounce_once), \
	PROXY_INIT (pendulum), \
	PROXY_INIT (unlimited), \
	PROXY_INIT (wrap), \
	PROXY_INIT (halt_action), \
	PROXY_INIT (subscribed), \
	PROXY_INIT (active), \
	PROXY_INIT (reversed)



// TweqMessage

MESSAGE_WRAPPER_IMPL (TweqMessage, "TweqComplete"),
	event (Event (MESSAGE_AS (sTweqMsg)->Op)),
	tweq_type (Tweq::Type (MESSAGE_AS (sTweqMsg)->Type)),
	direction (Tweq::Direction (MESSAGE_AS (sTweqMsg)->Dir))
{}

TweqMessage::TweqMessage (Event _event, Tweq::Type _tweq_type,
		Tweq::Direction _direction)
	: Message (new sTweqMsg ()), event (_event), tweq_type (_tweq_type),
	  direction (_direction)
{
	message->message = "TweqComplete";
	MESSAGE_AS (sTweqMsg)->Op = eTweqOperation (event);
	MESSAGE_AS (sTweqMsg)->Type = eTweqType (tweq_type);
	MESSAGE_AS (sTweqMsg)->Dir = eTweqDirection (direction);
}



// DeleteTweq

TWEQ_COMMON_CONFIG (DeleteTweq, "CfgTweqDelete", "StTweqDelete");
PROXY_CONFIG (DeleteTweq, duration, "CfgTweqDelete", "Rate", Time, 0ul);
PROXY_CONFIG (DeleteTweq, current_time, "StTweqDelete", "Cur Time", Time, 0ul);

OBJECT_TYPE_IMPL_ (DeleteTweq, Tweq (), TWEQ_COMMON_INIT,
	PROXY_INIT (duration),
	PROXY_INIT (current_time)
)

bool
DeleteTweq::has_delete_tweq () const
{
	return halt_action.exists ();
}



/*TODO Create EmitTweq and wrap the following properties:
 * Tweq\Emit = CfgTweqEmit
 * Tweq\Emit2 = CfgTweq2Emit
 * Tweq\Emit3 = CfgTweq3Emit
 * Tweq\Emit4 = CfgTweq4Emit
 * Tweq\Emit5 = CfgTweq5Emit
 * Tweq\Emitter2State = StTweq2Emit
 * Tweq\Emitter3State = StTweq3Emit
 * Tweq\Emitter4State = StTweq4Emit
 * Tweq\Emitter5State = StTweq5Emit
 * Tweq\EmitterState = StTweqEmit
 */



// FlickerTweq

TWEQ_COMMON_CONFIG (FlickerTweq, "CfgTweqBlink", "StTweqBlink");
PROXY_CONFIG (FlickerTweq, rate, "CfgTweqBlink", "Rate", Time, 0ul);
PROXY_CONFIG (FlickerTweq, current_time, "StTweqBlink", "Cur Time", Time, 0ul);
PROXY_CONFIG (FlickerTweq, current_frame, "StTweqBlink", "Frame #", unsigned, 0u);

OBJECT_TYPE_IMPL_ (FlickerTweq, Tweq (), TWEQ_COMMON_INIT,
	PROXY_INIT (rate),
	PROXY_INIT (current_time),
	PROXY_INIT (current_frame)
)

bool
FlickerTweq::has_flicker_tweq () const
{
	return halt_action.exists ();
}



/*TODO Create JointsTweq and wrap the following properties:
 * Tweq\Joints = CfgTweqJoints
 * Tweq\JointsState = StTweqJoints
 */



// LockTweq

TWEQ_COMMON_CONFIG (LockTweq, "CfgTweqLock", "StTweqLock");
PROXY_CONFIG (LockTweq, joint, "CfgTweqLock", "Lock Joint", int, 0);
PROXY_COMP_CONFIG (LockTweq, min_angle, "CfgTweqLock", "    rate-low-highJoint",
	Component::Y, 0.0f);
PROXY_COMP_CONFIG (LockTweq, max_angle, "CfgTweqLock", "    rate-low-highJoint",
	Component::Z, 0.0f);
PROXY_COMP_CONFIG (LockTweq, rate, "CfgTweqLock", "    rate-low-highJoint",
	Component::X, 0.0f);
PROXY_CONFIG (LockTweq, target_angle, "StTweqLock", "Target Angle", float, 0.0f);
PROXY_CONFIG (LockTweq, current_stage, "StTweqLock", "Cur Stage", unsigned, 0u);

OBJECT_TYPE_IMPL_ (LockTweq, Tweq (), TWEQ_COMMON_INIT,
	PROXY_INIT (joint),
	PROXY_INIT (min_angle),
	PROXY_INIT (max_angle),
	PROXY_INIT (rate),
	PROXY_INIT (target_angle),
	PROXY_INIT (current_stage)
)

bool
LockTweq::has_lock_tweq () const
{
	return halt_action.exists ();
}



// ModelsTweq

TWEQ_COMMON_CONFIG (ModelsTweq, "CfgTweqModels", "StTweqModels");
PROXY_CONFIG (ModelsTweq, rate, "CfgTweqModels", "Rate", Time, 0ul);
PROXY_ARRAY_CONFIG (ModelsTweq, model, 6u, String,
        PROXY_ARRAY_ITEM ("CfgTweqModels", "Model 0", ""),
        PROXY_ARRAY_ITEM ("CfgTweqModels", "Model 1", ""),
        PROXY_ARRAY_ITEM ("CfgTweqModels", "Model 2", ""),
        PROXY_ARRAY_ITEM ("CfgTweqModels", "Model 3", ""),
        PROXY_ARRAY_ITEM ("CfgTweqModels", "Model 4", ""),
        PROXY_ARRAY_ITEM ("CfgTweqModels", "Model 5", ""));
PROXY_BIT_CONFIG (ModelsTweq, anchor_bottom, "StTweqModels", "MiscC",
	1u, false);
PROXY_BIT_CONFIG (ModelsTweq, anchor_vhot, "StTweqModels", "MiscC",
	1024u, false);
PROXY_BIT_CONFIG (ModelsTweq, use_model_5, "StTweqModels", "MiscC",
	8192u, false);
PROXY_CONFIG (ModelsTweq, current_time, "StTweqModels", "Cur Time", Time, 0ul);
PROXY_CONFIG (ModelsTweq, current_frame, "StTweqModels", "Frame #", unsigned, 0u);

OBJECT_TYPE_IMPL_ (ModelsTweq, Tweq (), TWEQ_COMMON_INIT,
	PROXY_INIT (rate),
	PROXY_ARRAY_INIT (model, 6),
	PROXY_INIT (anchor_bottom),
	PROXY_INIT (anchor_vhot),
	PROXY_INIT (use_model_5),
	PROXY_INIT (current_time),
	PROXY_INIT (current_frame)
)

bool
ModelsTweq::has_models_tweq () const
{
	return halt_action.exists ();
}



/*TODO Create RotateTweq and wrap the following properties:
 * Tweq\Rotate = CfgTweqRotate
 * Tweq\RotateState = StTweqRotate
	THIEF_PROP_FIELD (bool, link_relative); //TESTME
PROXY_BIT_CONFIG (RotateTweq, link_relative, "StTweqRotate", "MiscC",
	16384u, false);
	PROXY_INIT (link_relative),
 */



/*TODO Create ScaleTweq and wrap the following properties:
 * Tweq\Scale = CfgTweqScale
 * Tweq\ScaleState = StTweqScale
	THIEF_PROP_FIELD (bool, affect_creature_scale); //TESTME
PROXY_BIT_CONFIG (ScaleTweq, affect_creature_scale, "StTweqScale", "MiscC",
	4096u, false);
	PROXY_INIT (affect_creature_scale),
 */



} // namespace Thief

