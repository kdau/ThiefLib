//! \file Script.hh Base classes and utilities for custom scripts.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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
 */

#ifndef THIEF_SCRIPT_HH
#define THIEF_SCRIPT_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>
#include <Thief/Parameter.hh>
#include <Thief/Security.hh>

namespace Thief {

class Script;

#ifdef IS_DOXYGEN
/*! The default level of logging detail for scripts.
 * This is Script::Log::NORMAL if debugging is enabled at compile time, or
 * Script::Log::INFO otherwise. */
#define THIEF_DEFAULT_LOG_LEVEL THIEF_DEFAULT_LOG_LEVEL
#elif defined (DEBUG)
#define THIEF_DEFAULT_LOG_LEVEL Script::Log::NORMAL
#else
#define THIEF_DEFAULT_LOG_LEVEL Script::Log::INFO
#endif




/*! Base for classes that handle low-level message structures.
 * A message handler wraps the underlying engine message type as a ThiefLib
 * message type and presents it appropriately to a script. Scripts do not need
 * to (and should not) handle this directly; the Script::listen_message() and
 * Script::listen_timer() methods create handler structures automatically. */
struct MessageHandler
{
	/*! A pointer to a message handler structure.
	 * These structures are created internally and destroyed automatically. */
	typedef std::shared_ptr<MessageHandler> Ptr;

	/*! Handles a message by wrapping it and passing it to the given script.
	 * Derived classes must implement this method and use a message wrapper
	 * constructor to produce a ThiefLib message type for the script. */
	virtual Message::Result handle (Script&, sScrMsg*, sMultiParm*) = 0;

	//! Destroys a message handler structure.
	virtual ~MessageHandler ();
};



/*! An object that may host one or more scripts.
 * This is the object type used by Script::host(). It includes fields that are
 * relevant to the behavior of scripts.
 *
 * The trap control flags (#trap_on, #trap_off, #trap_invert, and #trap_once)
 * were not supported by pre-NewDark %Thief: The Dark Project and %Thief Gold
 * (\c IS_THIEF1). While they are supported with NewDark, some existing missions
 * have used a \c tcf parameter to store these values instead. The TrapTrigger
 * base script supports that parameter on its host object for TDP/TG missions,
 * but other TDP/TG users of this class would need to add their own support.
 *
 * Since the \c Scripts property itself has an unusual inheritance pattern and
 * can result in odd behaviors if modified by scripts themselves, it is not
 * exposed here. */
class ScriptHost : public virtual Lockable
{
public:
	THIEF_OBJECT_TYPE (ScriptHost)

	/*! A span of time associated with behaviors of scripts on the object.
	 * The script timing is used by stock scripts in a number of different
	 * ways. Depending on the context, it may be a number of milliseconds,
	 * seconds, or minutes, or none of these. Scripts are responsible for
	 * rescaling this value accordingly. */
	THIEF_PROP_FIELD (Time, script_timing);

	/*! Accepts \c TurnOn messages to and from trap and trigger scripts.
	 * Trap scripts hosted on the object will only receive \c TurnOn
	 * messages if this flag is set. Certain trigger scripts will only send
	 * \c TurnOn messages if it is set. */
	THIEF_PROP_FIELD (bool, trap_on);

	/*! Accepts \c TurnOff messages to and from trap and trigger scripts.
	 * Trap scripts hosted on the object will only receive \c TurnOff
	 * messages if this flag is set. Certain trigger scripts will only send
	 * \c TurnOff messages if it is set. */
	THIEF_PROP_FIELD (bool, trap_off);

	/*! Inverts the sense of \c TurnOn and \c TurnOff messages.
	 * Trap scripts on the object will handle the opposite of the message
	 * received. Some trigger scripts will send the opposite of the message
	 * intended, including button and lever scripts. Inversion is considered
	 * after #trap_on and #trap_off, so those fields deal with the
	 * uninverted message. */
	THIEF_PROP_FIELD (bool, trap_invert);

	/*! Limits trap scripts on the object to being activated only once.
	 * After the first activation (\c TurnOn or \c TurnOff), the host object
	 * is locked and scripts hosted on it cannot be activated again. */
	THIEF_PROP_FIELD (bool, trap_once);
};



/*! Base class for all custom scripts.
 * A Dark %Engine object script is a C++ class implementing the engine's IScript
 * interface, included in a DLL bundled with the original game or a fan mission.
 * Each script class is registered under a name that mission authors can use to
 * include it on object archetypes and concrete objects. For every script on a
 * concrete object, a script class is instantiated each time the mission is
 * started or loaded from a saved game. (Script classes may also be
 * reinstantiated due to metaproperty changes and other mid-mission events.)
 *
 * A script instance is always associated with an object known as its host
 * object (ScriptHost). It works by handling a series of messages (Message) from
 * the engine and other scripts. Scripts have access to objects (Object), their
 * properties (as exposed in various field on Object subclasses), and the links
 * between them (Link), as well as a number of other aspects of the game world
 * (Mission) and of the engine itself (Engine).
 *
 * Scripts interact with the player through changes to the game world, as well
 * as by displaying on-screen messages (Mission::show_text ()) and other HUD
 * elements (HUDElement). They can provide logging for mission authors by
 * writing to the Monolog (see also mono() and log() in this class). Mission
 * authors can configure custom scripts using the fan-created parameter system.
 * For scripts' internal use, a persistent data store (Persistent) is available. */
class Script
{
public:
	//! A level of verbosity for a monolog message.
	enum class Log
	{
		VERBOSE, /*!< Additional details that are very long or would be
		          * logged very frequently, to the extent that they
		          * might interfere with reading other messages. */
		NORMAL,  /*!< Additional details that are not likely to be
		          * needed except when debugging or in special cases. */
		INFO,    /*!< %Key information that mission authors are likely
		          * to consult in the development process. */
		WARNING, //!< A problem that may reduce the script's function.
		ERROR    //!< A problem that blocks the script from proceeding.
	};

	//! Destroys a custom script.
	virtual ~Script ();

	//! Returns the name of the script as exposed to mission authors.
	const String& name () const;

	//! Returns the object hosting the script.
	ScriptHost host () const;

	/*! Returns the object hosting the script as the requested object type.
	 * This syntactic convenience allows access to other fields on the host
	 * object without a local variable. */
	template <typename T, typename = THIEF_IS_OBJECT> T host_as () const;

	//! \cond HIDDEN_SYMBOLS
	IScript* get_interface ();
	//! \endcond

protected:
	/*! Constructs a script of the given name on the given host object.
	 * All derived classes must have a public constructor that requires only
	 * the \a name and \a host arguments. This constructor and those of
	 * derived classes must not call any method or access any data that
	 * relies on a mission being properly loaded, as script classes are
	 * temporarily instantiated before and after that time. Instead override
	 * the initialize() method to perform such tasks.
	 * \param name The name of the script as it was listed by the author.
	 * \param host The object on which this script is hosted.
	 * \param min_level The minimum level of logging detail for the script.
	 * Messages below this level will not be output. This level may be
	 * adjusted by a \c debug parameter on the host object or a \c debug
	 * quest variable. */
	Script (const String& name, const Object& host,
		Log min_level = THIEF_DEFAULT_LOG_LEVEL);

	/*! Prepares the script to handle messages.
	 * This method is called before the first message received by the script
	 * is handled. Since a script instance may be replaced during a single
	 * game mode session, this method may be called multiple times even
	 * without an intervening reload. Scripts may extend this method to
	 * perform preparation that requires access to elements of a loaded
	 * mission, such as objects, links, or persistent variables. */
	virtual void initialize ();

	/*! Releases resources used by the script.
	 * This method is called if the script receives an \c EndScript message
	 * or is destroyed, but only if it has been initialized previously.
	 * Since a script instance may be replaced during a single game mode
	 * session, this method may be called multiple times even without an
	 * intervening reload. Scripts may extend this method to unsubscribe
	 * from subscribed messages, deregister HUD elements, and release any
	 * other resources that are not managed automatically. */
	virtual void deinitialize ();

	/*! Outputs an appropriate prefix to the monolog and returns it.
	 * This method is used to output monolog messages with a traditional
	 * ostream. The message will be prefixed with the log level, the current
	 * sim time, the script name, and the host object number. The script
	 * must append the trailing newline.
	 * \param level The log level of this message. If it is below the log
	 * level for this script, nothing will be logged (the #null_mono will
	 * be used to discard the output). */
	Monolog& mono (Log level = Log::NORMAL) const;

	/*! Outputs a formatted log message to the monolog.
	 * This method is used to output monolog messages using Boost's format
	 * library. The message will be prefixed with the log level, the current
	 * sim time, the script name, and the host object number. The trailing
	 * newline will be supplied, and need not be included in the format.
	 * \param level The log level of this message. If it is below the log
	 * level for this script, nothing will be logged (the #null_mono will
	 * be used to discard the output).
	 * \param format A Boost-style format string. See the documentation of
	 * that library's <a href="http://www.boost.org/doc/libs/release/libs/format/doc/format.html#syntax">
	 * format string syntax</a>.
	 * \param ... Any additional arguments are substituted into the format
	 * string in sequence. */
	template <typename... Args>
	void log (Log level, const String& format, const Args&...) const;

	//! Returns whether the simulation (mission) is currently running.
	bool is_sim () const { return sim; }

	/*! Returns the current time on the sim clock.
	 * This time is updated with every message received. Since script code
	 * almost exclusively runs from message handlers, this value should be
	 * up to date. In certain contexts, such as drawing handlers of scripts
	 * that are also HUD elements, it may be stale. */
	Time get_sim_time () const { return sim_time; }

	/*! Listens for messages of the given name to be handled by the given
	 * method. This method is usually called in a script's constructor,
	 * since listeners are not persistent. Calling this method does not
	 * guarantee that messages of the given name will be received by the
	 * object; see the various "subscribe" methods throughout ThiefLib.
	 * \param message The name of the message to listen for.
	 * \param handler A pointer to a member function of the script class. */
	template <typename _Script, typename _Message>
	void listen_message (const CIString& message,
		Message::Result (_Script::*handler) (_Message&));

	/*! Listens for \c %Timer messages with the given timer name.
	 * Use this overload for handler methods expecting a generic Message.
	 * This method is usually called in a script's constructor, since
	 * listeners are not persistent.
	 * \param timer The name of the timer to listen for.
	 * \param handler A pointer to a member function of the script class. */
	template <typename _Script>
	void listen_timer (const CIString& timer,
		Message::Result (_Script::*handler) (Message&));

	/*! Listens for \c %Timer messages with the given timer name.
	 * Use this overload for handler methods expecting a TimerMessage.
	 * This method is usually called in a script's constructor, since
	 * listeners are not persistent.
	 * \param timer The name of the timer to listen for.
	 * \param handler A pointer to a member function of the script class. */
	template <typename _Script>
	void listen_timer (const CIString& timer,
		Message::Result (_Script::*handler) (TimerMessage&));

	/*! Schedules a timer message with the given timer name.
	 * This is effectively a call to Message::schedule() on a newly
	 * constructed TimerMessage; see those for more information. */
	Timer start_timer (const char* timer, Time delay, bool repeating);

	/*! Schedules a timer message with the given timer name and data.
	 * This is effectively a call to Message::schedule() on a message
	 * constructed with TimerMessage::with_data(); see those methods for
	 * more information. */
	template <typename T>
	Timer start_timer (const char* timer, Time delay, bool repeating,
		const T& data);

private:
	void fix_player_links ();

	template <typename T, typename... Args>
	void log_step (Log level, boost::format& format, const T& arg,
		const Args&...) const;
	void log_step (Log level, boost::format& format) const;

	typedef std::multimap<CIString, MessageHandler::Ptr> Handlers;
	Handlers message_handlers;
	Handlers timer_handlers;

	bool dispatch (sScrMsg& message, sMultiParm* reply, unsigned trace);
	bool dispatch_cycle (Handlers& candidates, const CIString& key,
		sScrMsg& message, sMultiParm* reply);

	friend class Transition;
	Timer _start_timer (const char* timer, Time delay, bool repeating,
		const LGMultiBase& data);

	class Impl;
	friend class Impl;
	Impl& impl;

	const String script_name;
	Object::Number host_obj;
	Log min_level;

	bool initialized, sim, post_sim;
	Time sim_time;
};

/*! Declares a script and allows the engine to instantiate it.
 * This macro should be called for each script in a module. It may only be
 * called as an argument to the THIEF_MODULE macro.
 * \param Name A string literal naming the script as it will be listed in the
 * \c Scripts property.
 * \param Parent A string literal naming a script from which this script
 * directly inherits. (This does not appear to be used anywhere.)
 * \param Class The name of the class implementing the script. */
#define THIEF_SCRIPT(Name, Parent, Class) \
{ \
	ScriptModule::real_name, Name, Parent, \
	[] (const char* name, int host) __cdecl \
	{ \
		if (_stricmp (name, Name) == 0) \
			try \
			{ \
				auto script = new Class (Name, Object (host)); \
				return script->get_interface (); \
			} \
			catch (...) {} \
		return static_cast<IScript*> (nullptr); \
	} \
}



/*! Base class for all Persistent specializations.
 * This class cannot be used directly. See Persistent for more information. */
class PersistentBase
{
public:
	//! Returns whether the persistent variable is set.
	bool exists () const;

	/*! Removes any value set for the persistent variable.
	 * \return Whether the variable had been set and was unset. */
	bool remove ();

protected:
	//! \cond HIDDEN_SYMBOLS
	PersistentBase (Script& script, const String& name);
	void get (LGMultiBase& value) const;
	void set (const LGMultiBase& value);
	//! \endcond

private:
	Script& script;
	String name;
};



/*! A script-associated variable that persists in saved games.
 * Since script classes are necessarily reinstantiated on every load of a saved
 * game, normal member variables cannot be relied upon to store most data. In
 * fact, script classes may be reinstantiated even during a single play session
 * for several reasons.
 *
 * Instead, the engine offers a persistent script data store which accepts a
 * variety of basic data types (FIXME LGMulti). Variables in the persistent
 * store are identified by a triplet of script name, host object number, and
 * variable name. (This persistence is within one mission only; the engine does
 * not offer any campaign-level persistence other than quest variables.)
 *
 * An instance of this class is a reference to one persistent variable of a
 * particular script. Wherever possible, it can be treated just like a normal
 * member variable. The key exception is that persistent variables can be
 * nonexistent (undefined). Unless a default value is passed to the constructor,
 * retrieving the value of a nonexistent persistent variable will throw an
 * exception. */
template <typename T>
class Persistent : public PersistentBase
{
public:
	/*! Constructs a reference to the named persistent variable on the given
	 * script. No default value will be set, so an exception will be thrown
	 * if the variable's value is retrieved while it is not set. */
	Persistent (Script& script, const String& name);

	/*! Constructs a reference to the named persistent variable on the given
	 * script. The given \a default_value will be returned if the variable's
	 * value is retrieved while it is not set. */
	Persistent (Script& script, const String& name, const T& default_value);

	/*! Returns the current value of the persistent variable.
	 * If the variable is not set, returns the default value.
	 * \throw std::runtime_error if the variable is not set and there is no
	 * default value. \throw LGMultiTypeError if the variable is of the
	 * wrong type. */
	operator const T& () const;

	/*! Returns a pointer to the current value of the persistent variable.
	 * This operator is provided to allow the const methods of a compound
	 * value type to be called directly on the internally cached value.
	 * See operator const T& () for more information on return behavior.
	 * \warning This non-const version of the operator is provided to allow
	 * access to non-const methods of compound value types. If a non-const
	 * method is called that alters the value data, however, no update will
	 * be made to the persistent variable. Use with caution. */
	T* operator -> ();

	/*! Returns a pointer to the current value of the persistent variable.
	 * This operator is provided to allow the const methods of a compound
	 * value type to be called directly on the internally cached value.
	 * See operator const T& () for more information on return behavior. */
	const T* operator -> () const;

	//! Returns whether the persistent variable is set to the given value.
	bool operator == (const T&) const;

	/*! Returns whether the persistent variable is not set to the given
	 * value. */
	bool operator != (const T&) const;

	//! Sets the persistent variable to the given value.
	Persistent& operator = (const T&);

	/*! Changes the default value returned by this reference.
	 * The persistent variable itself is unaffected. */
	void set_default_value (const T&);

private:
	Persistent (const Persistent&) = delete;
	Persistent& operator = (const Persistent&) = delete;

	void get_value () const;

	mutable T value;
	T default_value;
	bool has_default_value;
};

/*! Initializes a Persistent variable instance.
 * \param Name The name of the member and of the persistent variable itself. */
#define THIEF_PERSISTENT(Name) Name (*this, #Name)

/*! Initializes a Persistent variable instance with a default value.
 * \param Name The name of the member and of the persistent variable itself.
 * \param Default A default value for the persistent variable. */
#define THIEF_PERSISTENT_FULL(Name, Default) Name (*this, #Name, Default)



/*! Base class for custom scripts with trap and/or trigger behavior.
 * Traps are scripts that perform actions in response to the \c TurnOn and/or
 * \c TurnOff messages. This class provides behaviors common to all standard
 * traps (among the stock scripts, those traps that inherit from \c StdTrap).
 * Specifically, it handles locking, filtering, and inversion as specified by
 * the trap control flags and timed delay or reversion as specified by the
 * script timing (see ScriptHost).
 *
 * Trigger behavior is provided by the trigger() method; see it for more
 * information. */
class TrapTrigger : public Script
{
public:
	//! Destoys a trap/trigger script.
	virtual ~TrapTrigger ();

	//! A behavior related to the ScriptHost::script_timing field.
	enum class Timing
	{
		NONE,	//!< The field is ignored.
		DELAY,	/*!< \c TurnOn and \c TurnOff will not be processed
			 * until the specified time has elapsed. */
		REVERT	/*! Each time the trap is turned on, it will be turned
			 * off again after the specified time has elapsed. (No
			 * reversion will occur for \c TurnOff.) */
	};

protected:
	/*! Constructs a trap/trigger script of the given name on the given host
	 * object. A trap will with ScriptHost::script_timing set will behave
	 * according to the specified \c timing_behavior.
	 *
	 * See Script::Script() for more general information. */
	TrapTrigger (const String& name, const Object& host,
		Log min_level = THIEF_DEFAULT_LOG_LEVEL,
		Timing timing_behavior = Timing::NONE);

	/*! Prepares the script to handle messages.
	 * In %Thief: The Dark Project and %Thief Gold (\c IS_THIEF1) only,
	 * TrapTrigger checks for the existence of a \c tcf parameter on the
	 * host object. If it is present and the \c TrapFlags property is not,
	 * the parameter's value is read and used to update the property. The
	 * parameter is then removed from the host object. */
	virtual void initialize ();

	/*! Sends a trigger message along \a ControlDevice links.
	 * This method replicates the basic behavior of most standard trigger
	 * scripts. Each object that is the destination of a \a ControlDevice
	 * link from the script's host object will receive the message.
	 * \param on Whether to send \c "TurnOn" (\c true) or \c "TurnOff"
	 * (\c false). \param conditional Whether to \a not send the message if
	 * the host object is locked. If \a conditional is \c true and the
	 * ScriptHost::trap_once flag is set, the host object will be locked
	 * after sending the message. \param filtered Whether to heed the
	 * ScriptHost::trap_on, ScriptHost::trap_off, and
	 * ScriptHost::trap_invert flags. If \c true, the message will only be
	 * sent if allowed by the first two flags, and its sense will be
	 * inverted if required by the third flag. This is the behavior of the
	 * standard button and lever scripts. */
	void trigger (bool on, bool conditional = true, bool filtered = false);

	/*! Handles a processed \c TurnOn or \c TurnOff message on a trap.
	 * Derived scripts with trap behavior should override this method to
	 * implement a trap response. The base version does nothing.
	 * \param on Whether the processed message is \c TurnOn or \c TurnOff.
	 * \param message The underlying message that resulted in this trap
	 * activation. Its name and detailed type are not guaranteed. */
	virtual Message::Result on_trap (bool on, Message& message);

private:
	Message::Result on_turn_on (Message&);
	Message::Result on_turn_off (Message&);
	Message::Result on_timer (TimerMessage&);

	Timing timing_behavior;
	Persistent<Timer> timer;
};



/*! A change over time between two states.
 * This class assists scripts in implementing transitions between states of
 * gradient values (such as color, opacity, or speed). It uses \c %Timer
 * messages to trigger a series of steps between the starting and finishing
 * states. A script using this class passes it a pointer to a member function
 * that updates the state at each step, using interpolate()d values. */
class Transition : public MessageHandler
{
public:
	/*! Constructs a transition.
	 * \param host The script that will handle the transition steps.
	 * \param step_method The member method of the script class that will
	 * be called at each step. \param name The name of the transition, used
	 * to distinguish it from others in timer and persistent variable names.
	 * \param resolution How much time should pass between steps. Higher
	 * resolutions (lower time values) provide higher transition quality but
	 * are more resource-intensive. \param default_length The duration of
	 * the transition, if no parameter specifies it. \param default_curve
	 * The shape of the transition curve, if no parameter specifies it.
	 * \param length_param The name of a parameter on the script's host
	 * object giving the duration of the transition. If empty, no parameter
	 * will be read. \param curve_param The name of a parameter on the
	 * script's host object giving the shape of the transition curve. If
	 * empty, no parameter will be read. */
	template <typename _Script>
	Transition (_Script& host, bool (_Script::*step_method) (),
		const String& name, Time resolution = 50ul,
		Time default_length = 0ul, Curve default_curve = Curve::LINEAR,
		const CIString& length_param = CIString (),
		const CIString& curve_param = CIString ());

	//! Destroys a transition.
	~Transition ();

	/*! Starts the transition.
	 * Any previous cycle will be aborted. */
	void start ();

	/*! Returns whether the transition is \a not currently in progress.
	 * This could be during the last step (progress is \c 1.0) or outside
	 * the transition cycle. */
	bool is_finished () const;

	/*! Returns the current point along the transition, between 0.0 and 1.0.
	 * This is the linear point, not yet affected by #curve. If the
	 * transition is not in progress, returns \c 0.0. */
	float get_progress () const;

	//! The duration of the transition.
	Parameter<Time> length;

	//! The shape of the transition curve.
	Parameter<Curve> curve;

	/*! Returns a value interpolated between the \a from and \a to values.
	 * This is equivalent to Thief::interpolate() using the transition's
	 * curve shape and current progress. A specialization supports the use
	 * of persistent variables as arguments. */
	template <typename T> THIEF_INTERPOLATE_RESULT (T)
	interpolate (const T& from, const T& to) const;

	//! \cond HIDDEN_SYMBOLS
	template <typename T>
	T interpolate (const Persistent<T>& from, const Persistent<T>& to) const;
	//! \endcond HIDDEN_SYMBOLS

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



} // namespace Thief

#include <Thief/Script.inl>

#endif // THIEF_SCRIPT_HH

