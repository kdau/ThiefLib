/******************************************************************************
 *  Message.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts and the Object Script Library
 *  Copyright (C) 2005-2013 Tom N Harris <telliamed@whoopdedo.org>
 *  Adapted in part from TWScript
 *  Copyright (C) 2012-2013 Chris Page <chris@starforge.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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



// Timer

void
Timer::cancel ()
{
	if (id)
	{
		LG->KillTimedMessage (tScrTimer (id));
		id = nullptr;
	}
}



// Message

Message::Message (sScrMsg* _message, sMultiParm* _reply, bool)
	: message (_message),
	  reply (_reply ? _reply : new cMultiParm ()),
	  own_reply (!_reply)
{
	if (message)
		message->AddRef ();
	else
	{
		if (own_reply) delete reply;
		throw std::invalid_argument ("null message");
	}
}

Message::~Message ()
{
	if (message) message->Release ();
	if (own_reply) delete reply;
}

const char*
Message::get_name () const
{
	return message->message;
}

void
Message::send (const Object& from, const Object& to)
{
	message->from = from.number;
	message->to = to.number;
	LG->SendMessage (message, reply);
}

void
Message::post (const Object& from, const Object& to)
{
	message->from = from.number;
	message->to = to.number;
	LG->PostMessage (message);
}

Timer
Message::schedule (const Object& from, const Object& to,
	Time delay, bool repeating)
{
	message->from = from.number;
	message->to = to.number;
	return Timer (LG->SetTimedMessage (message, delay,
		repeating ? kSTM_Periodic : kSTM_OneShot));
}

void
Message::broadcast (const Links& links, Time delay)
{
	for (auto& link : links)
		if (delay > 0ul)
			schedule (link.get_source (), link.get_dest (),
				delay, false);
		else
			send (link.get_source (), link.get_dest ());
}

void
Message::broadcast (const Object& from, const Flavor& link_flavor, Time delay)
{
	broadcast (Link::get_all (link_flavor, from), delay);
}

Object
Message::get_from () const
{
	return message->from;
}

Object
Message::get_to () const
{
	return message->to;
}

Time
Message::get_time () const
{
	return message->time;
}

bool
Message::has_data (Datum datum) const
{
	switch (datum)
	{
	case DATA1: return message->data.type != kMT_Undef;
	case DATA2: return message->data2.type != kMT_Undef;
	case DATA3: return message->data3.type != kMT_Undef;
	default: throw std::invalid_argument ("bad message datum");
	}
}

void
Message::_get_data (Datum datum, LGMultiBase& value) const
{
	switch (datum)
	{
	case DATA1: value = message->data; break;
	case DATA2: value = message->data2; break;
	case DATA3: value = message->data3; break;
	default: throw std::invalid_argument ("bad message datum");
	}
}

void
Message::_set_data (Datum datum, const LGMultiBase& value)
{
	switch (datum)
	{
	case DATA1: message->data = value; break;
	case DATA2: message->data2 = value; break;
	case DATA3: message->data3 = value; break;
	default: throw std::invalid_argument ("bad message datum");
	}
}

void
Message::_get_reply (LGMultiBase& value) const
{
	value = *reply;
}

void
Message::_set_reply (const LGMultiBase& value)
{
	*reply = value;
}

const char*
Message::get_lg_typename () const
{
	return message->Persistent_GetName ();
}



// GenericMessage

MESSAGE_WRAPPER_IMPL_ (GenericMessage, true) // allow any message type

GenericMessage::GenericMessage (const char* name)
	: Message (new sGenericScrMsg ())
{
	message->message = name;
}



// CombineMessage

// "Combine" reports as "sScrMsg", so it can't be tested by type.
MESSAGE_WRAPPER_IMPL_ (CombineMessage, MESSAGE_NAME_TEST ("Combine"))

CombineMessage::CombineMessage (const Object& stack)
	: Message (new sCombineScrMsg ())
{
	message->message = "Combine";
	MESSAGE_AS (sCombineScrMsg)->combiner = stack.number;
}

MESSAGE_ACCESSOR (Object, CombineMessage, get_stack, sCombineScrMsg, combiner)



// DamageMessage

MESSAGE_WRAPPER_IMPL (DamageMessage, sDamageScrMsg)

DamageMessage::DamageMessage (const Object& culprit, const Object& stimulus,
		int hit_points)
	: Message (new sDamageScrMsg ())
{
	message->message = "Damage";
	MESSAGE_AS (sDamageScrMsg)->culprit = culprit.number;
	MESSAGE_AS (sDamageScrMsg)->kind = stimulus.number;
	MESSAGE_AS (sDamageScrMsg)->damage = hit_points;
}

MESSAGE_ACCESSOR (Object, DamageMessage, get_culprit, sDamageScrMsg, culprit)

MESSAGE_ACCESSOR (Object, DamageMessage, get_stimulus, sDamageScrMsg, kind)

MESSAGE_ACCESSOR (int, DamageMessage, get_hit_points, sDamageScrMsg, damage)



// FrobMessage

MESSAGE_WRAPPER_IMPL (FrobMessage, sFrobMsg)

FrobMessage::FrobMessage (Event event, const Object& frobber, const Object& tool,
		const Object& frobbed, Location frob_loc, Location obj_loc,
		Time duration, bool aborted)
	: Message (new sFrobMsg ())
{
	switch (event)
	{
	case BEGIN:
		switch (frob_loc)
		{
		case WORLD: message->message = "FrobWorldBegin"; break;
		case INVENTORY: message->message = "FrobInvBegin"; break;
		case TOOL: message->message = "FrobToolBegin"; break;
		default: throw std::invalid_argument ("bad FrobMessage location");
		}
		break;
	case END:
		switch (frob_loc)
		{
		case WORLD: message->message = "FrobWorldEnd"; break;
		case INVENTORY: message->message = "FrobInvEnd"; break;
		case TOOL: message->message = "FrobToolEnd"; break;
		default: throw std::invalid_argument ("bad FrobMessage location");
		}
		break;
	default:
		throw std::invalid_argument ("bad FrobMessage event");
	}

	MESSAGE_AS (sFrobMsg)->Frobber = frobber.number;
	MESSAGE_AS (sFrobMsg)->SrcObjId = tool.number;
	MESSAGE_AS (sFrobMsg)->DstObjId = frobbed.number;
	MESSAGE_AS (sFrobMsg)->SrcLoc = eFrobLoc (frob_loc);
	MESSAGE_AS (sFrobMsg)->DstLoc = eFrobLoc (obj_loc);
	MESSAGE_AS (sFrobMsg)->Sec = duration / 1000.0f;
	MESSAGE_AS (sFrobMsg)->Abort = aborted;
}


MESSAGE_ACCESSOR (Object, FrobMessage, get_frobber, sFrobMsg, Frobber);
MESSAGE_ACCESSOR (Object, FrobMessage, get_tool, sFrobMsg, SrcObjId);
MESSAGE_ACCESSOR (Object, FrobMessage, get_frobbed, sFrobMsg, DstObjId);
MESSAGE_ACCESSOR (FrobMessage::Location, FrobMessage, get_frob_loc,
	sFrobMsg, SrcLoc);
MESSAGE_ACCESSOR (FrobMessage::Location, FrobMessage, get_obj_loc,
	sFrobMsg, DstLoc);
MESSAGE_ACCESSOR (bool, FrobMessage, was_aborted, sFrobMsg, Abort);

FrobMessage::Event
FrobMessage::get_event () const
{
	CIString name = get_name ();
	size_t length = name.length ();
	if (length > 5 && name.compare (length - 5, 5, "Begin") == 0)
		return BEGIN;
	else if (length > 3 && name.compare (length - 3, 3, "End") == 0)
		return END;
	else
		throw std::runtime_error ("invalid FrobMessage");
}

Time
FrobMessage::get_duration () const
{
	return MESSAGE_AS (sFrobMsg)->Sec * 1000ul;
}



// SlayMessage

MESSAGE_WRAPPER_IMPL (SlayMessage, sSlayMsg)

SlayMessage::SlayMessage (const Object& culprit, const Object& stimulus)
	: Message (new sSlayMsg ())
{
	message->message = "Slain";
	MESSAGE_AS (sSlayMsg)->culprit = culprit.number;
	MESSAGE_AS (sSlayMsg)->kind = stimulus.number;
}

MESSAGE_ACCESSOR (Object, SlayMessage, get_culprit, sSlayMsg, culprit)

MESSAGE_ACCESSOR (Object, SlayMessage, get_stimulus, sSlayMsg, kind)



// TimerMessage

MESSAGE_WRAPPER_IMPL (TimerMessage, sScrTimerMsg)

TimerMessage::TimerMessage (const char* timer_name)
	: Message (new sScrTimerMsg ())
{
	message->message = "Timer";
	MESSAGE_AS (sScrTimerMsg)->name = timer_name;
}

MESSAGE_ACCESSOR (String, TimerMessage, get_timer_name, sScrTimerMsg, name)



} // namespace Thief

