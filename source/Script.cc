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

#include <windef.h>
#include <winbase.h>

namespace Thief {

IScriptMan*
LG = nullptr;



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
	script.impl = nullptr;
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
			script.mono () << "Error: " << e.what () << std::endl;
		}
		catch (...) {}
		return S_FALSE;
	}
	catch (...)
	{
		try
		{
			script.mono () << "Error: an unknown error occurred."
				<< std::endl;
		}
		catch (...) {}
		return S_FALSE;
	}
}



// Script

Script::Script (const String& _name, const Object& _host)
	: impl (new Impl (*this)), script_name (_name), host_obj (_host.number),
	  initialized (false), sim (Engine::is_sim ()), post_sim (false),
	  sim_time (0ul)
{}

Script::~Script ()
{
	for (auto& handler : message_handlers)
		delete handler.second;
	for (auto& handler : timer_handlers)
		delete handler.second;
}

IScript*
Script::get_interface ()
{
	return impl;
}

Monolog&
Script::mono () const
{
	Thief::mono << '[' << std::setw (4) << (sim_time / 1000);
	char fill = Thief::mono.fill ('0');
	Thief::mono << '.' << std::setw (3) << (sim_time % 1000) << "] ";
	Thief::mono.fill (fill);
	Thief::mono << script_name << " [" << host_obj << "]: ";
	return Thief::mono;
}

bool
Script::has_datum (const String& datum) const
{
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	return LG->IsScriptDataSet (&tag);
}

void
Script::_get_datum (const String& datum, LGMultiBase& value) const
{
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	if (LG->GetScriptData (&tag, &(sMultiParm&)value) != S_OK)
		throw std::runtime_error ("could not get script datum");
}

bool
Script::_set_datum (const String& datum, const LGMultiBase& value)
{
	sScrDatumTag tag { host_obj, script_name.data (), datum.data () };
	return LG->SetScriptData (&tag, &(const sMultiParm&)value) == S_OK;
}

bool
Script::unset_datum (const String& datum)
{
	LGMultiEmpty junk;
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

#ifndef DEBUG
	if (trace != kNoAction)
#endif
		mono () << "Got message \"" << message.message << '\"'
#ifdef DEBUG
			<< " of type " << message.Persistent_GetName ()
#endif
			<< (trace == kBreak ? ". Breaking." : ".") << std::endl;

	if (trace == kBreak)
		asm ("int 3");

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

	if (_stricmp (message.message, "DHNotify") == 0)
	{
		const char* translated = nullptr;
		switch (static_cast<sDHNotifyMsg*> (&message)->typeDH)
		{
		case kDH_Property: translated = "PropertyChange"; break;
		case kDH_Relation: translated = "LinkChange"; break;
		default: break;
		}

		if (translated)
			result &= dispatch_cycle (message_handlers, translated,
				message, reply);
	}

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
		{
			mono () << "Error: " << e.what () << std::endl;
		}
		catch (...)
		{
			mono () << "Error: an unknown error occurred."
				<< std::endl;
		}

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
		host_as<Lockable> ().lock ();
}

Message::Result
TrapTrigger::on_trap (bool, Message&)
{
	return Message::CONTINUE;
}

Message::Result
TrapTrigger::on_turn_on (GenericMessage& message)
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
		host_as<Lockable> ().lock ();

	return result;
}

Message::Result
TrapTrigger::on_turn_off (GenericMessage& message)
{
	unsigned flags = get_flags ();

	if (flags & FLAG_NO_OFF) return Message::HALT;
	if (host_as<Lockable> ().is_locked ()) return Message::HALT;

	bool on = (flags & FLAG_INVERT) ? true : false;
	Message::Result result = on_trap (on, message);

	if (result != Message::ERROR && (flags & FLAG_ONCE))
		host_as<Lockable> ().lock ();

	return result;
}

Message::Result
TrapTrigger::on_revert (TimerMessage& message)
{
	bool on = message.get_data<bool> (Message::DATA1);
	return on_trap (on, message);
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
			mprintf ("%s cannot be used with this version of the "
				"Dark Engine. Upgrade to NewDark.\n",
				Thief::module.get_name ());
		return false;
	}

	// Attach various ThiefLib components to the engine.
	Thief::LG = manager;
	Thief::alloc.attach (allocator, name);
	Thief::mono.attach (mprintf);

	// Load and initialize the OSL.
	DarkHookInitializeService (manager, allocator);

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

