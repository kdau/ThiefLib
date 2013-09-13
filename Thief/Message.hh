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

	Message (const Message&);
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

	template <typename T>
	T get_data (Datum, const T& default_value) const;

	MULTI_GET (get_reply);
	MULTI_SET (set_reply);

protected:
	Message (sScrMsg* message, sMultiParm* reply = nullptr,
		bool valid = true);

	sScrMsg* const message;

	const char* get_lg_typename () const;

	template <typename, typename> friend struct ScriptMessageHandler;

private:
	void _get_data (Datum datum, LGMultiBase& value) const;
	void _set_data (Datum datum, const LGMultiBase& value);

	void _get_reply (LGMultiBase& value) const;
	void _set_reply (const LGMultiBase& value);

	LGMulti<sMultiParm> reply_local;
	sMultiParm* const reply_remote;
};



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



class GenericMessage : public Message
{
public:
	GenericMessage (const char* name);
	THIEF_MESSAGE_WRAP (GenericMessage);

	template <typename D1, typename D2 = Empty, typename D3 = Empty>
	static GenericMessage with_data (const char* name, const D1& data1,
		const D2& data2 = D2 (), const D3& data3 = D3 ());
};



class TimerMessage : public Message // "Timer"
{
public:
	TimerMessage (const String& timer_name);
	THIEF_MESSAGE_WRAP (TimerMessage);

	const String timer_name;

	template <typename D1, typename D2 = Empty, typename D3 = Empty>
	static TimerMessage with_data (const String& timer_name, const D1& data1,
		const D2& data2 = D2 (), const D3& data3 = D3 ());
};



} // namespace Thief

#include <Thief/Message.inl>

#endif // THIEF_MESSAGE_HH

