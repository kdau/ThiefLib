/******************************************************************************
 *  Script.hh
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

#ifndef THIEF_SCRIPT_HH
#define THIEF_SCRIPT_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>

namespace Thief {

class Script;



// Message handler template magic

struct MessageHandler
{
	typedef std::unique_ptr<MessageHandler> Ptr;
	virtual Message::Result handle (Script&, sScrMsg*, sMultiParm*) = 0;
	virtual ~MessageHandler ();
};

template <typename _Script, typename _Message>
struct ScriptMessageHandler : public MessageHandler,
	private std::function<Message::Result (_Script&, _Message&)>
{
	typedef Message::Result (_Script::*Method) (_Message&);

	ScriptMessageHandler (Method method);

	virtual Message::Result handle (Script&, sScrMsg*, sMultiParm*);
};



// Object subclass for objects hosting scripts

class ScriptHost : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (ScriptHost)

	THIEF_PROP_FIELD (Time, script_timing);
};



// Base class for all scripts

class Script
{
public:
	virtual ~Script ();

	const String& name () const;

	ScriptHost host () const;
	template <typename T> T host_as () const;

	IScript* get_interface ();

protected:
	Script (const String& name, const Object& host);

	Monolog& mono () const;

	bool is_sim () const { return sim; }
	Time get_sim_time () const { return sim_time; }

	template <typename _Script, typename _Message>
	void listen_message (const CIString& message,
		Message::Result (_Script::*handler) (_Message&));

	template <typename _Script>
	void listen_timer (const CIString& timer,
		Message::Result (_Script::*handler) (TimerMessage&));

	Timer start_timer (const char* timer, Time delay, bool repeating);

	template <typename T>
	Timer start_timer (const char* timer, Time delay, bool repeating,
		const T& data);

	bool has_datum (const String& datum) const;
	MULTI_GET_ARG (get_datum, String, datum);
	MULTI_SET_ARG_RET (bool, set_datum, String, datum);
	bool unset_datum (const String& datum);

private:
	virtual void initialize ();
	void fix_player_links (); //TESTME

	typedef std::multimap<CIString, MessageHandler::Ptr> Handlers;
	Handlers message_handlers;
	Handlers timer_handlers;

	bool dispatch (sScrMsg& message, sMultiParm* reply, unsigned trace);
	bool dispatch_cycle (Handlers& candidates, const CIString& key,
		sScrMsg& message, sMultiParm* reply);

	friend class Transition;
	Timer _start_timer (const char* timer, Time delay, bool repeating,
		const LGMultiBase& data);

	template <typename T> friend class Persistent;
	void _get_datum (const String& datum, LGMultiBase& value) const;
	bool _set_datum (const String& datum, const LGMultiBase& value);

	class Impl;
	friend class Impl;
	Impl& impl;

	const String script_name;
	Object::Number host_obj;
	bool initialized, sim, post_sim;
	Time sim_time;
};



// Persistent script variables

template <typename T>
class Persistent
{
public:
	Persistent (Script& script, const String& name);
	Persistent (Script& script, const String& name, const T& default_value);

	Persistent (const Persistent&) = delete;
	Persistent& operator = (const Persistent&) = delete;

	bool exists () const;
	operator const T& () const;

	T* operator -> (); // Do not use any data-altering methods.
	const T* operator -> () const;

	bool operator == (const T&) const;
	bool operator != (const T&) const;

	Persistent& operator = (const T&);
	bool remove ();

	void set_default_value (const T&);

private:
	void get_value () const;

	Script& script;
	String name;

	mutable T value;
	T default_value;
	bool has_default_value;
};

// initializer convenience macros
#define PERSISTENT(Name, Default) Name (*this, #Name, Default)
#define PERSISTENT_(Name) Name (*this, #Name)



// Trap and trigger scripts

class TrapTrigger : public Script
{
public:
	virtual ~TrapTrigger ();

protected:
	TrapTrigger (const String& name, const Object& host);

	enum Flags
	{
		FLAGS_NONE = 0,
		FLAG_ONCE = 1,
		FLAG_INVERT = 2,
		FLAG_NO_ON = 4,
		FLAG_NO_OFF = 8
	};
	unsigned get_flags () const; //TESTME

	void trigger (bool on, bool unconditional = false); //TESTME

private:
	virtual Message::Result on_trap (bool on, Message&);

	Message::Result on_turn_on (Message&);
	Message::Result on_turn_off (Message&);
	Message::Result on_revert (TimerMessage&);

#ifdef IS_THIEF1
	Parameter<String> tcf;
#endif
};



// Helper for timed transitions with value interpolation

class Transition : public MessageHandler
{
public:
	template <typename _Script>
	Transition (_Script& host, bool (_Script::*step_method) (),
		const String& name, Time resolution = 50ul,
		Time default_length = 0ul, Curve default_curve = Curve::LINEAR,
		const CIString& length_param = "transition",
		const CIString& curve_param = "curve");
	~Transition ();

	void start ();

	bool is_finished () const;
	float get_progress () const;

	Parameter<Time> length;
	Parameter<Curve> curve;

	template <typename T> THIEF_INTERPOLATE_RESULT (T)
	interpolate (const T& from, const T& to) const;

	template <typename T>
	T interpolate (const Persistent<T>& from, const Persistent<T>& to) const;

private:
	void initialize ();
	virtual Message::Result handle (Script&, sScrMsg*, sMultiParm*);

	Script& host;
	std::function<bool ()> step_method;

	const String name;
	const Time resolution;

	Persistent<Timer> timer;
	Persistent<Time> remaining;
};



// The script module

class ScriptModule
{
public:
	virtual ~ScriptModule ();

	const char* get_name () const { return name; }

protected:
	ScriptModule ();

	void set_name (const char* name = nullptr);

	char* name;
	static const char* const real_name;

	static const sScrClassDesc scripts[];
	static const size_t script_count;
};



} // namespace Thief

#include <Thief/Script.inl>

#endif // THIEF_SCRIPT_HH

