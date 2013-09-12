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
#include "OSL.hh"

#include <windef.h>
#include <winbase.h>

namespace Thief {

IScriptMan*
LG = nullptr;

extern "C" const GUID IID_IHUD = THIEF_IHUD_GUID;
extern "C" const GUID IID_IOSLService = THIEF_IOSLService_GUID;



// MessageHandler

MessageHandler::~MessageHandler ()
{}



// ScriptHost

PROXY_CONFIG (ScriptHost, script_timing, "ScriptTiming", nullptr, Time, 0ul);

OBJECT_TYPE_IMPL_ (ScriptHost,
	PROXY_INIT (script_timing)
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
			throw MessageWrapError (message, typeid (Message),
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

bool
Script::has_persistent (const String& datum) const
{
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	return LG->IsScriptDataSet (&tag);
}

void
Script::_get_persistent (const String& datum, LGMultiBase& value) const
{
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	if (LG->GetScriptData (&tag, &(sMultiParm&)value) != S_OK)
		throw std::runtime_error ("could not get persistent variable");
}

bool
Script::_set_persistent (const String& datum, const LGMultiBase& value)
{
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	return LG->SetScriptData (&tag, &(const sMultiParm&)value) == S_OK;
}

bool
Script::unset_persistent (const String& datum)
{
	LGMulti<Empty> junk;
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	return LG->ClearScriptData (&tag, &(sMultiParm&)junk) == S_OK;
}

void
Script::initialize ()
{}

void
Script::fix_player_links ()
{
	Object start = Link::get_one ("PlayerFactory").get_source ();
	Player player;
	if (start == Object::NONE || player == Object::NONE) return;

	for (auto& link : Link::get_all (Flavor::ANY, host (), start))
	{
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

	if (!initialized)
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



// TrapTrigger

TrapTrigger::TrapTrigger (const String& _name, const Object& _host)
	: Script (_name, _host)
#ifdef IS_THIEF1
	  , PARAMETER (tcf)
#endif
{
	listen_message ("TurnOn", &TrapTrigger::on_turn_on);
	listen_message ("TurnOff", &TrapTrigger::on_turn_off);
	listen_timer ("Revert", &TrapTrigger::on_revert);
}

TrapTrigger::~TrapTrigger ()
{}

unsigned
TrapTrigger::get_flags () const
{
	unsigned flags = FLAGS_NONE;
#if defined (IS_THIEF2)
	ObjectProperty _flags ("TrapFlags", host ());
	if (_flags.exists ())
		flags = _flags.get<unsigned> ();
#elif defined (IS_THIEF1)
	if (tcf.exists ())
	{
		if (tcf->find ("01") != String::npos) flags |= FLAG_ONCE;
		if (tcf->find ("<>") != String::npos) flags |= FLAG_INVERT;
		if (tcf->find ("!+") != String::npos) flags |= FLAG_NO_ON;
		if (tcf->find ("!-") != String::npos) flags |= FLAG_NO_OFF;
	}
#endif // IS_THIEF2 : IS_THIEF1
	return flags;
}

void
TrapTrigger::trigger (bool on, bool unconditional)
{
	if (!unconditional && host_as<Lockable> ().is_locked ())
		return;

	GenericMessage (on ? "TurnOn" : "TurnOff").broadcast
		(host (), "ControlDevice");

	if (!unconditional && (get_flags () & FLAG_ONCE))
		host_as<Lockable> ().set_locked (true);
}

Message::Result
TrapTrigger::on_trap (bool, Message&)
{
	return Message::CONTINUE;
}

Message::Result
TrapTrigger::on_turn_on (Message& message)
{
	unsigned flags = get_flags ();

	if (flags & FLAG_NO_ON) return Message::HALT;
	if (host_as<Lockable> ().is_locked ()) return Message::HALT;

	bool on = (flags & FLAG_INVERT) ? false : true;
	Message::Result result = on_trap (on, message);

	Time revert = host ().script_timing;
	if (result == Message::CONTINUE && revert != 0ul)
		start_timer ("Revert", revert, false, !on);

	if (result != Message::ERROR && (flags & FLAG_ONCE))
		host_as<Lockable> ().set_locked (true);

	return result;
}

Message::Result
TrapTrigger::on_turn_off (Message& message)
{
	unsigned flags = get_flags ();

	if (flags & FLAG_NO_OFF) return Message::HALT;
	if (host_as<Lockable> ().is_locked ()) return Message::HALT;

	bool on = (flags & FLAG_INVERT) ? true : false;
	Message::Result result = on_trap (on, message);

	if (result != Message::ERROR && (flags & FLAG_ONCE))
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
	// Creating a unique_ptr to @this is horrible. This only works
	// because Transition's destructor removes and releases the pointer
	// before Script's destructor can get to it. The alternative is a
	// unique_ptr wrapper with conditional deletion, which seems excessive.
	host.timer_handlers.insert (std::make_pair ("TransitionStep", this));
}

void
Transition::start ()
{
	if (timer.exists ()) // Stop any previous transition.
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



// ScriptModule

ScriptModule::ScriptModule ()
	: name (nullptr)
{
	set_name ();
}

ScriptModule::~ScriptModule ()
{}

void
ScriptModule::set_name (const char* _name)
{
	if (name && name != real_name)
		delete[] name;
	if (_name)
	{
		name = new char[strlen (_name) + 1];
		strcpy (name, _name);
	}
	else
		name = const_cast<char*> (real_name);
}



// ScriptModuleImpl

class ScriptModuleImpl : public cInterfaceImp<IScriptModule,
	IID_Def<IScriptModule>, kInterfaceImpStatic>, public ScriptModule
{
public:
	ScriptModuleImpl () {}
	virtual ~ScriptModuleImpl () {}

	void set_name (const char* name);

	// IScriptModule
	STDMETHOD_ (const char*, GetName) ();
	STDMETHOD_ (const sScrClassDesc*, GetFirstClass) (tScrIter*);
	STDMETHOD_ (const sScrClassDesc*, GetNextClass) (tScrIter*);
	STDMETHOD_ (void, EndClassIter) (tScrIter*);
} module;

void
ScriptModuleImpl::set_name (const char* _name)
{
	ScriptModule::set_name (_name);
}

STDMETHODIMP_ (const char*)
ScriptModuleImpl::GetName ()
{
	return name;
}

STDMETHODIMP_ (const sScrClassDesc*)
ScriptModuleImpl::GetFirstClass (tScrIter* iter)
{
	*reinterpret_cast<size_t*> (iter) = 0u;
	return (script_count > 0u) ? &scripts [0] : nullptr;
}

STDMETHODIMP_ (const sScrClassDesc*)
ScriptModuleImpl::GetNextClass (tScrIter* iter)
{
	size_t& index = *reinterpret_cast<size_t*> (iter);
	return (++index < script_count) ? &scripts [index] : nullptr;
}

STDMETHODIMP_ (void)
ScriptModuleImpl::EndClassIter (tScrIter*)
{}



} // namespace Thief



// Exported functions

extern "C"
int __declspec(dllexport) __stdcall
ScriptModuleInit (const char* name, IScriptMan* manager, MPrintfProc mprintf,
	IMalloc* allocator, IScriptModule** module_ptr)
{
	if (!manager || !allocator) return false;

	// Confirm that this is NewDark by checking for the new IEngineSrv.
	try
	{
		SService<IEngineSrv> engine (manager);
	}
	catch (no_interface&)
	{
		if (mprintf)
			mprintf ("ERROR: %s cannot be used with this version of "
				"the Dark Engine. Upgrade to NewDark.\n",
				Thief::module.get_name ());
		return false;
	}

	// Attach various ThiefLib components to the engine.
	Thief::LG = manager;
	Thief::alloc.attach (allocator, name);
	Thief::mono.attach (mprintf);

	// Load the OSL.
	HMODULE osl = ::GetModuleHandleA (OSL_NAME);
	if (!osl) osl = ::LoadLibraryA (OSL_NAME);
	if (!osl)
	{
		if (mprintf)
			mprintf ("ERROR: Could not load the ThiefLib support "
				"library " OSL_NAME ".\n");
		return false;
	}

	// Initialize the OSL.
	auto osl_init = reinterpret_cast<Thief::OSLInitProc>
		(::GetProcAddress (osl, OSL_INIT_PROC));
	if (!osl_init || !osl_init (manager, mprintf, allocator))
		return false;

	// Prepare the ScriptModule.
	Thief::module.set_name (name);
	Thief::module.QueryInterface (IID_IScriptModule,
		reinterpret_cast<void**> (module_ptr));

	return true;
}

extern "C"
BOOL WINAPI
DllMain (HINSTANCE dll, DWORD reason, PVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls (dll);
	(void) reserved;
	return true;
}

