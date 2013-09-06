/******************************************************************************
 *  Tweq.cc
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



// Tweq

OBJECT_TYPE_IMPL (Tweq)



// TweqMessage

MESSAGE_WRAPPER_IMPL (TweqMessage, sTweqMsg)

TweqMessage::TweqMessage (Event event, Tweq::Type type,
		Tweq::Direction direction)
	: Message (new sTweqMsg ())
{
	message->message = "TweqComplete";
	MESSAGE_AS (sTweqMsg)->Op = eTweqOperation (event);
	MESSAGE_AS (sTweqMsg)->Type = eTweqType (type);
	MESSAGE_AS (sTweqMsg)->Dir = eTweqDirection (direction);
}

MESSAGE_ACCESSOR (TweqMessage::Event, TweqMessage, get_event, sTweqMsg, Op)
MESSAGE_ACCESSOR (Tweq::Type, TweqMessage, get_tweq_type, sTweqMsg, Type)
MESSAGE_ACCESSOR (Tweq::Direction, TweqMessage, get_direction, sTweqMsg, Dir)



// DeleteTweq

PROXY_BIT_CONFIG (DeleteTweq, simulate_always, "CfgTweqDelete", "AnimC",
	2u, false);
PROXY_BIT_CONFIG (DeleteTweq, simulate_far, "CfgTweqDelete", "AnimC",
	32u, false);
PROXY_BIT_CONFIG (DeleteTweq, simulate_near, "CfgTweqDelete", "AnimC",
	16u, false);
PROXY_NEG_BIT_CONFIG (DeleteTweq, simulate_onscreen, "CfgTweqDelete", "AnimC",
	64u, false);
PROXY_CONFIG (DeleteTweq, duration, "CfgTweqDelete", "Rate", Time, 0ul);
PROXY_CONFIG (DeleteTweq, halt_action, "CfgTweqDelete", "Halt",
	Tweq::Halt, Halt::DESTROY_OBJECT);

PROXY_BIT_CONFIG (DeleteTweq, active, "StTweqDelete", "AnimS", 1u, false);
PROXY_CONFIG (DeleteTweq, current_time, "StTweqDelete", "Cur Time", Time, 0ul);

OBJECT_TYPE_IMPL_ (DeleteTweq, Tweq (),
	PROXY_INIT (simulate_always),
	PROXY_INIT (simulate_far),
	PROXY_INIT (simulate_near),
	PROXY_INIT (simulate_onscreen),
	PROXY_INIT (duration),
	PROXY_INIT (halt_action),
	PROXY_INIT (active),
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



/*TODO Create FlickerTweq and wrap the following properties:
 * Tweq\Flicker = CfgTweqBlink
 * Tweq\FlickerState = StTweqBlink
 */



/*TODO Create JointsTweq and wrap the following properties:
 * Tweq\Joints = CfgTweqJoints
 * Tweq\JointsState = StTweqJoints
 */



/*TODO Create LockTweq and wrap the following properties:
 * Tweq\Lock = CfgTweqLock
 * Tweq\LockState = StTweqLock
 */



/*TODO Create ModelsTweq and wrap the following properties:
 * Tweq\Models = CfgTweqModels
 * Tweq\ModelsState = StTweqModels
 */



/*TODO Create RotateTweq and wrap the following properties:
 * Tweq\Rotate = CfgTweqRotate
 * Tweq\RotateState = StTweqRotate
 */



/*TODO Create ScaleTweq and wrap the following properties:
 * Tweq\Scale = CfgTweqScale
 * Tweq\ScaleState = StTweqScale
 */



} // namespace Thief

