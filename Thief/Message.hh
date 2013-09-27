//! \file Message.hh Event notifications sent to and between scripts.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_MESSAGE_HH
#define THIEF_MESSAGE_HH

#include <Thief/Base.hh>

namespace Thief {



/*! Reference to a scheduled message's timer.
 * When a Message is scheduled for later posting with Message::schedule(), a
 * Timer is returned that represents the countdown (whether one-time or
 * repeating) to the posting of that message. The Timer, which can be persisted,
 * can be used to cancel all future postings of the message. */
struct Timer
{
	/*! The engine-internal ID number of the referenced timer.
	 * This is not actually a pointer, and can be persisted. */
	void* id;

	/*! Constructs a new timer reference.
	 * New references should generally only be created by ThiefLib. */
	Timer (void* _id = nullptr) : id (_id) {}

	//! Returns whether a timer is currently referenced.
	operator bool () const { return id; }

	/*! Cancels the referenced timer.
	 * No future copies of the timer's message will be posted. If this
	 * method is called from within a propagation cycle for the message, the
	 * cycle itself will not be affected. After this method is called, there
	 * will be no referenced timer (#id will be null). */
	void cancel ();
};

//! \cond HIDDEN_SYMBOLS
THIEF_LGMULTI_SPECIALIZE (Timer, Timer ())
// ! \endcond



/*! Base class for event notifications sent to and between scripts.
 * Messages are used to notify Dark %Engine scripts of game and interface events.
 * They are sent both between scripts and from the engine itself. Virtually all
 * script activity originates from message-handling methods. This class is a
 * wrapper for engine-internal message objects, and in some cases the engine-
 * internal object may survive the lifetime of the wrapper instance.
 *
 * Messages may be conveyed in three ways: send()ing propagates a message
 * synchronously from the sending method; post()ing propagates a message
 * asynchronously after the previous propagation cycle has finished; and
 * schedule()ing propagates a message after a given interval of time or at
 * repeated intervals.
 *
 * Once a message is being conveyed, it has a timestamp indicating the time it
 * was last propagated to a script. It also has "from" and "to" object
 * references. Only scripts hosted on the "to" object will receive the message.
 * The "from" object is typically the host object of the script conveying the
 * message, or Object::NONE if the message is conveyed by the engine, but it may
 * be set to some other object in some cases.
 *
 * Beyond their names, messages may carry data in three ways. All messages have
 * three data slots, #DATA1, #DATA2, and #DATA3, which can hold a variety of
 * basic data types (FIXME LGMulti). Messages that are sent (instead of posted
 * or scheduled) have a fourth data slot, the #REPLY, which can be set by a
 * script handling the message to return data to the sender. Many messages have
 * additional data fields of fixed types which are relevant to their specific
 * meanings; those are implemented in classes descending from this one.
 *
 * \note Users of ThiefLib should not create classes inheriting from Message, as
 * the proper implementation of custom message types requires access to engine
 * internals not exposed by this library.
 *
 * \nosubgrouping */
class Message
{
public:
	//! Returned by message handlers to suggest a disposition for a message.
	enum Result
	{
		CONTINUE,	//!< The message should be passed to any other relevant handlers.
		HALT,		/*!< The message should not be passed to any other handlers for this script.
				 * Other scripts on the host object will still
				 * receive and handle the message. */
		ERROR		/*!< An error occurred while handling the message.
				 * A generic error will be logged, but the
				 * message will still be passed to any other
				 * relevant handlers. */
	};

	/*! Constructs a new wrapper referencing the same message as another.
	 * The two wrappers will still reference the same underlying message, so
	 * for example changes to one will affect the other. */
	Message (const Message&);

	/*! Destroys a message wrapper.
	 * The referenced message will be destroyed only if it is not currently
	 * post()ed or schedule()d and no other wrapper references it. */
	virtual ~Message ();

	/*! Returns the name of the referenced message.
	 * This is the name that scripts can use to listen for the message. */
	const char* get_name () const;

	/*! Returns the object from which the message was last conveyed, if any.
	 * This object is set by the conveying method, and may or may not be the
	 * actual host object of a script which conveyed the message. For
	 * messages originating from the engine itself, the "from" object may be
	 * Object::NONE. */
	Object get_from () const;

	/*! Returns the object to which the message was last propagated, if any.
	 * Only scripts hosted by this object received the message in the last
	 * propagation. (Note that broadcast()ed messages will have many
	 * different "to" objects in succession.) */
	Object get_to () const;

	/*! Returns the time when the message was last propagated to a script.
	 * This time is measured on the sim clock; see Time for more information.
	 * For post()ed or schedule()d messages, this value may not be equal to
	 * the original conveyance time. The return value is undefined if the
	 * message has not been conveyed. */
	Time get_time () const;

	//! \name Conveyance
	//@{

	/*! Conveys a message immediately and synchronously.
	 * \param from The object that notionally conveyed the message, if any.
	 * \param to Scripts hosted on this object will receive the message. */
	void send (const Object& from, const Object& to);

	/*! Conveys a message asynchronously in the next cycle.
	 * \param from The object that notionally conveyed the message, if any.
	 * \param to Scripts hosted on this object will receive the message. */
	void post (const Object& from, const Object& to);

	/*! Schedules a message for future asynchronous delivery.
	 * \param from The object that notionally conveyed the message, if any.
	 * \param to Scripts hosted on this object will receive the message.
	 * \param delay How much time (on the sim clock) should pass before the
	 * message is delivered.
	 * \param repeating Whether the message should be repeated indefinitely
	 * at \a delay intervals (\c true) or only delivered once (\c false).
	 * \return A reference to the timer that will deliver the message. */
	Timer schedule (const Object& from, const Object& to,
		Time delay, bool repeating);

	/*! Sends or schedules a message along a given list of links.
	 * For each link in the \a links list, the message will be conveyed
	 * from the source of the link to the destination of the link. If \a
	 * delay is greater than zero, the message will be schedule()d to each
	 * recipient, otherwise it will be sent. */
	void broadcast (const Links& links, Time delay = 0ul);

	/*! Sends or schedules a message along links from a given object.
	 * This is a convenience for calling broadcast() on a list of links
	 * with a specific source (\a from) and flavor (\a link_flavor). See the
	 * Links overload of broadcast() for more information. */
	void broadcast (const Object& from, const Flavor& link_flavor,
		Time delay = 0ul);

	//@}
	//! \name Generic data
	//@{

	//! Identifier of a generic message data slot.
	enum Slot
	{
		DATA1, //!< The first of three slots for generic data.
		DATA2, //!< The second of three slots for generic data.
		DATA3, //!< The third of three slots for generic data.
		REPLY  /*!< A special data slot for replies from scripts.
		        * The reply slot is only valid for messages that will be
		        * or have been sent (not posted, scheduled, or
		        * broadcasted). For other messages, it is always empty.
		        * Scripts handling a sent message may use this slot to
		        * return data to the sending script. */
	};

	//! Returns whether the given data slot is currently non-empty.
	bool has_data (Slot) const;

	/*! Returns the data held in the given data slot.
	 * \throw LGMultiTypeError if the slot is empty or holds data of a
	 * different, incompatible type. */
	template <typename T> T get_data (Slot) const;

	/*! Returns the data held in the given slot, or a default if it is empty.
	 * \throw LGMultiTypeError if the slot holds data of a different,
	 * incompatible type. */
	template <typename T> T get_data (Slot, const T& default_value) const;

	//! Puts the given data in the given data slot.
	template <typename T> void set_data (Slot, const T& value);

	//@}

protected:
	//! \cond HIDDEN_SYMBOLS

	Message (sScrMsg* message, sMultiParm* reply = nullptr,
		bool valid = true, const char* wrap_type = nullptr);

	sScrMsg* const message;

	const char* get_lg_typename () const;

	template <typename, typename> friend struct ScriptMessageHandler;

	//! \endcond

private:
	void _get_data (Slot, LGMultiBase& value) const;
	void _set_data (Slot, const LGMultiBase& value);

	void _get_reply (LGMultiBase& value) const;
	void _set_reply (const LGMultiBase& value);

	LGMulti<sMultiParm> reply_local;
	sMultiParm* const reply_remote;
};



#ifdef IS_DOXYGEN
#define THIEF_MESSAGE_WRAP(Type)
#else
#define THIEF_MESSAGE_WRAP(Type) Type (sScrMsg*, sMultiParm*);
#endif



/*! Exception thrown when a message is not of the expected type.
 * When a script listens for a certain message with Script::listen_message()
 * or Script::listen_timer(), the argument type of the handler function passed
 * to one of those methods determine the expected type of message objects. If a
 * message is received that matches the expected name but not the expected type,
 * or some detail of the message is inconsistent with that type, this exception
 * is thrown. Individual scripts should not normally need to be handle it, as
 * the Script class will catch it and log it appropriately. */
class MessageWrapError : public std::exception
{
public:
	/*! Constructs a new message-wrapping exception for the given message.
	 * \param message The LG message object that could not be wrapped.
	 * \param wrap_type The name of the expected ThiefLib message type.
	 * \param problem A brief description of the discrepancy.
	 */
	MessageWrapError (const sScrMsg* message, const char* wrap_type,
		const char* problem) noexcept;

	//! Destroys a message-wrapping exception.
	virtual ~MessageWrapError () noexcept {}

	//! Returns a string describing the exception.
	virtual const char* what () const noexcept
		{ return explanation.data (); }

private:
	String explanation;
};



/*! Non-specific messages that can be created by scripts.
 * The GenericMessage class can be used by scripts to create messages that do
 * not have any specific type (that calls for fields other than #DATA1/2/3).
 * In message handler methods, however, these non-specific messages should be
 * received as <tt>Message&</tt> instead. \note %Message name: (any) */
class GenericMessage : public Message
{
public:
	/*! Constructs a new message with the given name.
	 * \warning Due to engine limitations, it is strongly recommended that
	 * the \a name string be a constant with static storage duration. If a
	 * temporary or non-constant string must be used, ensure that it will
	 * remain allocated and unchanged for the lifetime of the message. This
	 * is straightforward with the send() method but not with the post() and
	 * schedule() methods. */
	GenericMessage (const char* name);

	THIEF_MESSAGE_WRAP (GenericMessage);

	/*! Constructs a new message with the given name and data slot values.
	 * This is a convience template which allows a message with data to be
	 * assembled and conveyed from a single line of code. If omitted, #DATA2
	 * and #DATA3 will be empty. See the main constructor for other notes. */
	template <typename D1, typename D2 = Empty, typename D3 = Empty>
	static GenericMessage with_data (const char* name, const D1& data1,
		const D2& data2 = D2 (), const D3& data3 = D3 ());
};



/*! %Message generated after a delay or at regular intervals.
 * Although any message type can be posted after a delay or repeated at regular
 * intervals, it is most common to use a \c %Timer message for this purpose. The
 * \c %Timer message has a timer name, separate from the message name, which
 * identifies it uniquely. Use the schedule() method to convey this message.
 * \note %Message name: \c %Timer */
class TimerMessage : public Message
{
public:
	//! Constructs a new \c %Timer message with the given \a timer_name.
	TimerMessage (const String& timer_name);

	THIEF_MESSAGE_WRAP (TimerMessage);

	//! The name of the specific timer that has elapsed.
	const String timer_name;

	/*! Constructs a new \c %Timer message with the given timer name and data slot values.
	 * This is a convience template which allows a message with data to be
	 * assembled and conveyed from a single line of code. If omitted, #DATA2
	 * and #DATA3 will be empty. See the main constructor for other notes. */
	template <typename D1, typename D2 = Empty, typename D3 = Empty>
	static TimerMessage with_data (const String& timer_name, const D1& data1,
		const D2& data2 = D2 (), const D3& data3 = D3 ());
};



} // namespace Thief

#include <Thief/Message.inl>

#endif // THIEF_MESSAGE_HH

