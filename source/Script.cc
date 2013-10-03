/******************************************************************************
 *  Script.cc
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



// MessageHandler

MessageHandler::~MessageHandler ()
{}



// ScriptHost

PROXY_CONFIG (ScriptHost, script_timing, "ScriptTiming", nullptr, Time, 0ul);
PROXY_NEG_BIT_CONFIG (ScriptHost, trap_on, "TrapFlags", nullptr, 4u, true);
PROXY_NEG_BIT_CONFIG (ScriptHost, trap_off, "TrapFlags", nullptr, 8u, true);
PROXY_BIT_CONFIG (ScriptHost, trap_invert, "TrapFlags", nullptr, 2u, false);
PROXY_BIT_CONFIG (ScriptHost, trap_once, "TrapFlags", nullptr, 1u, false);

OBJECT_TYPE_IMPL_ (ScriptHost, Lockable (),
	PROXY_INIT (script_timing),
	PROXY_INIT (trap_on),
	PROXY_INIT (trap_off),
	PROXY_INIT (trap_invert),
	PROXY_INIT (trap_once)
)



// Script::Impl

class Script::Impl : public cInterfaceImp<IScript>
{
public:
	Impl (Script& script);
	virtual ~Impl ();

	Script& script;

	// IScript
	STDMETHOD_ (const char*, GetClassName) ();
	STDMETHOD (ReceiveMessage) (sScrMsg*, sMultiParm*, eScrTraceAction);
};

Script::Impl::Impl (Script& _script)
	: script (_script)
{}

Script::Impl::~Impl ()
{
	// To provide COM lifetime management to derived script classes without
	// exposing the IUnknown interface in the ThiefLib API, the Script::Impl
	// has to be the one to delete its owner.
	delete &script;
}

STDMETHODIMP_ (const char*)
Script::Impl::GetClassName ()
{
	return script.script_name.data ();
}

STDMETHODIMP
Script::Impl::ReceiveMessage (sScrMsg* message, sMultiParm* reply,
	eScrTraceAction trace)
{
	try
	{
		if (!message)
			throw MessageWrapError (message, "Message",
				"message is null");
		return script.dispatch (*message, reply, trace) ? S_OK : S_FALSE;
	}
	catch (std::exception& e)
	{
		try
		{
			script.log (Log::ERROR, e.what ());
		}
		catch (...) {}
		return S_FALSE;
	}
	catch (...)
	{
		try
		{
			script.log (Log::ERROR, "An unknown error occurred.");
		}
		catch (...) {}
		return S_FALSE;
	}
}



// Script

THIEF_ENUM_CODING (Script::Log, CODE, CODE,
	THIEF_ENUM_VALUE (VERBOSE, "verbose", "verb"),
	THIEF_ENUM_VALUE (NORMAL, "normal", "norm"),
	THIEF_ENUM_VALUE (INFO, "info"),
	THIEF_ENUM_VALUE (WARNING, "warning", "warn"),
	THIEF_ENUM_VALUE (ERROR, "error", "err"),
)

Script::Script (const String& _name, const Object& _host, Log _min_level)
	: impl (*new Impl (*this)),
	  script_name (_name),
	  host_obj (_host.number),
	  min_level (_min_level),
	  initialized (false),
	  sim (Engine::is_sim ()),
	  post_sim (false)
{}

Script::~Script ()
{}

void
Script::initialize ()
{
	// Adjust minimum logging level based on "debug" parameter or "debug"
	// quest variable, if any.
	Parameter<Log> min_level_param (host (), "debug", {});
	if (min_level_param.exists ())
		min_level = min_level_param;
	else
		switch (QuestVar ("debug"))
		{
		case 2: // always VERBOSE
			min_level = Log::VERBOSE;
			break;
		case 1: // at least NORMAL
			if (min_level != Log::VERBOSE)
				min_level = Log::NORMAL;
			break;
		case -1: // at most INFO
			if (min_level != Log::ERROR &&
			    min_level != Log::WARNING)
				min_level = Log::INFO;
			break;
		case -2: // at most WARNING
			if (min_level != Log::ERROR)
				min_level = Log::WARNING;
			break;
		default: break;
		}
}

IScript*
Script::get_interface ()
{
	return &impl;
}

Monolog&
Script::mono (Log level) const
{
	// If the level is too low, send the output to the circular file.
	if (int (level) < int (min_level))
		return null_mono;

	const char* prefix = "";
	switch (level)
	{
	case Log::INFO: prefix = "INFO"; break;
	case Log::WARNING: prefix = "WARNING"; break;
	case Log::ERROR: prefix = "ERROR"; break;
	default: break;
	}

	static const boost::format format ("%|-7| [%|4|.%|03|] %|| [%||]: ");
	Thief::mono << boost::format (format) % prefix % (sim_time / 1000ul)
		% (sim_time % 1000ul) % script_name % host_obj;

	return Thief::mono;
}

void
Script::fix_player_links ()
{
	Object start = Link::get_one ("PlayerFactory").get_source ();
	Player player;
	if (start == Object::NONE || player == Object::NONE) return;

	for (auto& link : Link::get_all (Flavor::ANY, host (), start))
	{
		log (Log::NORMAL, "Transferring a %|| link with source %|| from "
			"destination %|| to %||.", link.get_flavor (), host (),
			start, player);
		Link::create (link.get_flavor (), host (), player,
			link.get_data_raw ());
		link.destroy ();
	}
}

bool
Script::dispatch (sScrMsg& message, sMultiParm* reply, unsigned trace)
{
	sim_time = message.time;

	if (!sim && _stricmp (message.message, "PhysMadeNonPhysical") == 0)
		return true; // Silently ignore these to avoid extra work.

	mono ((trace != kNoAction) ? Log::NORMAL : Log::VERBOSE)
		<< "Got message \"" << message.message << '\"'
#ifdef DEBUG
		<< " of type " << message.Persistent_GetName ()
#endif
		<< (trace == kBreak ? ". Breaking." : ".") << std::endl;

	if (trace == kBreak)
		asm ("int $0x3");

	// The EndScript message can potentially be sent to scripts immediately
	// *after* a load from a saved game, immediately followed by BeginScript.
	// Since certain initialization tasks would fail in that environment,
	// and others might be immediately undone by EndScript message handlers,
	// EndScript cannot be used as an initialization trigger.
	if (!initialized && _stricmp (message.message, "EndScript") != 0)
	{
		initialize ();
		initialized = true;
	}

	if (_stricmp (message.message, "Sim") == 0)
	{
		sim = static_cast<sSimMsg*> (&message)->fStarting;
		if (sim)
			GenericMessage ("PostSim").post (host (), host ());
	}

	if (_stricmp (message.message, "PostSim") == 0)
	{
		if (post_sim)
			return true; // Only handle one instance of the message.
		else
		{
			post_sim = true;
			fix_player_links ();
		}
	}

	bool result = dispatch_cycle
		(message_handlers, message.message, message, reply);

	if (_stricmp (message.message, "QuestChange") == 0)
		try
		{
			ObjectiveMessage check_type (&message, reply);
			result &= dispatch_cycle (message_handlers,
				"ObjectiveChange", message, reply);
		}
		catch (...) {} // The quest variable is not objective-related.

	if (_stricmp (message.message, "Timer") == 0)
		result &= dispatch_cycle (timer_handlers, (const char*)
			static_cast<sScrTimerMsg*> (&message)->name,
			message, reply);

	return result;
}

bool
Script::dispatch_cycle (Handlers& candidates, const CIString& key,
	sScrMsg& message, sMultiParm* reply)
{
	bool cycle_result = true;

	auto matches = candidates.equal_range (key);
	for (auto match = matches.first; match != matches.second; ++match)
	{
		Message::Result result = Message::ERROR;
		try
		{
			result = match->second->handle (*this, &message, reply);
		}
		catch (std::exception& e)
			{ log (Log::ERROR, e.what ()); }
		catch (...)
			{ log (Log::ERROR, "An unknown error occurred."); }

		switch (result)
		{
		case Message::CONTINUE:
			break;
		case Message::HALT:
			return cycle_result;
		case Message::ERROR: // If this was returned without exception,
		default:             // the script should have already logged.
			cycle_result = false;
			break; // This doesn't match the TWScript behavior.
		}
	}

	return cycle_result;
}

Timer
Script::_start_timer (const char* timer, Time delay, bool repeating,
	const LGMultiBase& data)
{
	return Timer (LG->SetTimedMessage2 (host ().number, timer, delay,
		repeating ? kSTM_Periodic : kSTM_OneShot, data));
}



// PersistentBase

PersistentBase::PersistentBase (Script& _script, const String& _name)
	: script (_script), name (_name)
{}

bool
PersistentBase::exists () const
{
	sScrDatumTag tag
		{ script.host ().number, script.name ().data (), name.data () };
	return LG->IsScriptDataSet (&tag);
}

bool
PersistentBase::remove ()
{
	LGMulti<Empty> junk;
	sScrDatumTag tag
		{ script.host ().number, script.name ().data (), name.data () };
	return LG->ClearScriptData (&tag, &(sMultiParm&)junk) == S_OK;
}

void
PersistentBase::get (LGMultiBase& value) const
{
	sScrDatumTag tag
		{ script.host ().number, script.name ().data (), name.data () };
	if (LG->GetScriptData (&tag, &(sMultiParm&)value) != S_OK)
		throw std::runtime_error ("could not get persistent variable");
}

void
PersistentBase::set (const LGMultiBase& value)
{
	sScrDatumTag tag
		{ script.host ().number, script.name ().data (), name.data () };
	if (LG->SetScriptData (&tag, &(const sMultiParm&)value) != S_OK)
		throw std::runtime_error ("could not set persistent variable");
}



// TrapTrigger

TrapTrigger::TrapTrigger (const String& _name, const Object& _host, Log _level,
		bool _delayed_revert)
	: Script (_name, _host, _level), delayed_revert (_delayed_revert)
{
	listen_message ("TurnOn", &TrapTrigger::on_turn_on);
	listen_timer ("TurnOn", &TrapTrigger::on_turn_on);

	listen_message ("TurnOff", &TrapTrigger::on_turn_off);
	listen_timer ("TurnOff", &TrapTrigger::on_turn_off);

	if (delayed_revert)
		listen_timer ("Revert", &TrapTrigger::on_revert);
}

TrapTrigger::~TrapTrigger ()
{}

void
TrapTrigger::initialize ()
{
	Script::initialize ();

#ifdef IS_THIEF1
	Parameter<String> tcf (host (), "tcf", {});
	if (tcf.exists () && !host ().trap_on.exists ())
	{
		log (Log::NORMAL, "Translating tcf parameter \"%||\" to "
			"TrapFlags property.", tcf);
		host ().trap_on = tcf->find ("!+") == String::npos;
		host ().trap_off = tcf->find ("!-") == String::npos;
		host ().trap_invert = tcf->find ("<>") != String::npos;
		host ().trap_once = tcf->find ("01") != String::npos;
		tcf.remove ();
	}
#endif // IS_THIEF1
}

void
TrapTrigger::trigger (bool on, bool conditional, bool filtered)
{
	if ((conditional && host ().is_locked ()) ||
	    (filtered && on && !host ().trap_on) ||
	    (filtered && !on && !host ().trap_off))
		return;

	if (host ().trap_invert)
		on = !on;

	GenericMessage (on ? "TurnOn" : "TurnOff").broadcast
		(host (), "ControlDevice");

	if (conditional && host ().trap_once)
		host ().set_locked (true);
}

Message::Result
TrapTrigger::on_trap (bool, Message&)
{
	return Message::CONTINUE;
}

Message::Result
TrapTrigger::on_turn_on (Message& message)
{
	if (!host ().trap_on) return Message::HALT;
	if (host ().is_locked ()) return Message::HALT;

	bool on = host ().trap_invert ? false : true;
	Message::Result result = on_trap (on, message);

	if (delayed_revert && result == Message::CONTINUE &&
	    host ().script_timing != 0ul)
		start_timer ("Revert", host ().script_timing, false, !on);

	if (result != Message::ERROR && host ().trap_once)
		host ().set_locked (true);

	return result;
}

Message::Result
TrapTrigger::on_turn_off (Message& message)
{
	if (!host ().trap_off) return Message::HALT;
	if (host ().is_locked ()) return Message::HALT;

	bool on = host ().trap_invert ? true : false;
	Message::Result result = on_trap (on, message);

	if (result != Message::ERROR && host ().trap_once)
		host_as<Lockable> ().set_locked (true);

	return result;
}

Message::Result
TrapTrigger::on_revert (TimerMessage& message)
{
	bool on = message.get_data (Message::DATA1, false);
	return on_trap (on, message);
}



// Transition

Transition::~Transition ()
{
	try
	{
		for (auto iter = host.timer_handlers.begin ();
		     iter != host.timer_handlers.end (); ++iter)
			if (iter->second.get () == this)
			{
				iter->second.release ();
				host.timer_handlers.erase (iter);
				break;
			}
	}
	catch (...) {}
}

void
Transition::initialize ()
{
	/* Creating a unique_ptr to this is horrible. It only works because
	 * Transition's destructor removes and releases the pointer before
	 * Script's destructor can get to it. The alternative is a unique_ptr
	 * wrapper with conditional deletion, which seems excessive. */
	host.timer_handlers.insert (std::make_pair ("TransitionStep", this));
}

void
Transition::start ()
{
	if (timer.exists ()) // Stop any previous cycle.
	{
		timer->cancel ();
		timer.remove ();
	}

	remaining = length;
	TimerMessage::with_data ("TransitionStep", name)
		.send (host.host (), host.host ());
}

bool
Transition::is_finished () const
{
	return !remaining.exists () || remaining == 0ul;
}

float
Transition::get_progress () const
{
	if (!remaining.exists ())
		return 0.0f;
	else if (length == 0ul || remaining == 0ul)
		return 1.0f;
	else
	{
		float _length = float (Time (length)),
			_remaining = float (Time (remaining));
		return (_length - _remaining) / _length;
	}
}

Message::Result
Transition::handle (Script&, sScrMsg* _message, sMultiParm* reply)
{
	TimerMessage message (_message, reply);

	if (message.get_data (Message::DATA1, String ()) != name)
		return Message::CONTINUE;

	if (step_method () && remaining.exists () && Time (remaining) > 0ul)
	{
		remaining = std::max (0l, long (Time (remaining)) -
			long (resolution));
		timer = host.start_timer ("TransitionStep", resolution, false,
			name);
	}
	else
	{
		timer.remove ();
		remaining.remove ();
	}

	return Message::HALT;
}



} // namespace Thief

