/******************************************************************************
 *  Message.hh
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

#ifndef THIEF_MESSAGE_HH
#define THIEF_MESSAGE_HH

#include <Thief/Base.hh>

namespace Thief {



struct Timer
{
	void* id;

	Timer (void* _id = nullptr) : id (_id) {}
	operator bool () const { return id; }
	void cancel ();
};

THIEF_LGMULTI_SPECIALIZE (Timer, Timer ())




class Message
{
public:
	enum Result
	{
		CONTINUE,
		HALT,
		ERROR
	};

	Message (const Message&) = delete;
	virtual ~Message ();

	const char* get_name () const;

	void send (const Object& from, const Object& to);
	void post (const Object& from, const Object& to);
	Timer schedule (const Object& from, const Object& to,
		Time delay, bool repeating);

	void broadcast (const Links& links, Time delay = 0ul);
	void broadcast (const Object& from, const Flavor& link_flavor,
		Time delay = 0ul);

	Object get_from () const;
	Object get_to () const;
	Time get_time () const;

	enum Datum { DATA1, DATA2, DATA3 };
	bool has_data (Datum datum) const;
	MULTI_GET_ARG (get_data, Datum, datum);
	MULTI_SET_ARG (set_data, Datum, datum);

	MULTI_GET (get_reply);
	MULTI_SET (set_reply);

protected:
	Message (sScrMsg* message, sMultiParm* reply = nullptr,
		bool own_message = false);

	sScrMsg* const message;
	sMultiParm* const reply;

	const char* get_lg_typename () const;

private:
	void _get_data (Datum datum, LGMultiBase& value) const;
	void _set_data (Datum datum, const LGMultiBase& value);

	void _get_reply (LGMultiBase& value) const;
	void _set_reply (const LGMultiBase& value);

	bool own_reply;
};



// Message wrapping

#define THIEF_MESSAGE_WRAP(Type) Type (sScrMsg*, sMultiParm*);

class MessageWrapError : public std::exception
{
public:
	MessageWrapError (const sScrMsg* message,
		const std::type_info& wrap_type, const char* problem) noexcept;
	virtual ~MessageWrapError () noexcept {}
	virtual const char* what () const noexcept
		{ return explanation.data (); }
private:
	String explanation;
};



// General and object messages

class GenericMessage : public Message
{
public:
	GenericMessage (const char* name);
	THIEF_MESSAGE_WRAP (GenericMessage);
};

class CombineMessage : public Message //TESTME
{
public:
	CombineMessage (const Object& stack);
	THIEF_MESSAGE_WRAP (CombineMessage);

	Object get_stack () const;
};

class TimerMessage : public Message
{
public:
	TimerMessage (const char* timer_name);
	THIEF_MESSAGE_WRAP (TimerMessage);

	String get_timer_name () const;
};



} // namespace Thief

#endif // THIEF_MESSAGE_HH

