/******************************************************************************
 *  Script.inl
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

#ifndef THIEF_SCRIPT_HH
#error "This file should only be included from <Thief/Script.hh>."
#endif

#ifndef THIEF_SCRIPT_INL
#define THIEF_SCRIPT_INL

namespace Thief {



// ScriptMessageHandler

template <typename _Script, typename _Message>
ScriptMessageHandler<_Script, _Message>::ScriptMessageHandler (Method method)
	: std::function<Message::Result (_Script&, _Message&)> (method)
{}

template <typename _Script, typename _Message>
Message::Result
ScriptMessageHandler<_Script, _Message>::handle (Script& script,
	sScrMsg* _message, sMultiParm* reply)
{
	_Message message (_message, reply);
	return (*this) (static_cast<_Script&> (script), message);
}



// Script

inline const String&
Script::name () const
{
	return script_name;
}

inline ScriptHost
Script::host () const
{
	return host_obj;
}

template <typename T>
inline T
Script::host_as () const
{
	return host_obj;
}

template <typename _Script, typename _Message>
inline void
Script::listen_message (const CIString& message,
	Message::Result (_Script::*handler) (_Message&))
{
	message_handlers.insert (Handlers::value_type (message,
		new ScriptMessageHandler<_Script, _Message> (handler)));
}

template <typename _Script>
inline void
Script::listen_timer (const CIString& timer,
	Message::Result (_Script::*handler) (TimerMessage&))
{
	timer_handlers.insert (Handlers::value_type (timer,
		new ScriptMessageHandler<_Script, TimerMessage> (handler)));
}

inline Timer
Script::start_timer (const char* timer, Time delay, bool repeating)
{
	return _start_timer (timer, delay, repeating, LGMultiEmpty ());
}

template <typename T>
inline Timer
Script::start_timer (const char* timer, Time delay, bool repeating,
	const T& data)
{
	return _start_timer (timer, delay, repeating, LGMulti<T> (data));
}



// Persistent

template <typename T>
Persistent<T>::Persistent (Script& _script, const String& _name)
	: script (_script), name (_name)
{}

template <typename T>
inline bool
Persistent<T>::valid () const
{
	return script.has_datum (name);
}

template <typename T>
inline
Persistent<T>::operator T () const
{
	if (!valid ())
		throw std::runtime_error ("invalid persistent data");
	LGMulti<T> value;
	script._get_datum (name, value);
	return value;
}

template <typename T>
inline bool
Persistent<T>::operator == (const T& rhs) const
{
	return operator T () == rhs;
}

template <typename T>
inline bool
Persistent<T>::operator != (const T& rhs) const
{
	return operator T () != rhs;
}

template <typename T>
inline Persistent<T>&
Persistent<T>::operator = (const T& value)
{
	if (!script._set_datum (name, LGMulti<T> (value)))
		throw std::runtime_error ("could not set persistent variable");
	return *this;
}

template <typename T>
inline void
Persistent<T>::init (const T& default_value)
{
	if (!valid ())
		*this = default_value;
}

template <typename T>
inline void
Persistent<T>::clear ()
{
	if (!script.unset_datum (name))
		throw std::runtime_error ("could not clear persistent variable");
}



} // namespace Thief

#endif // THIEF_SCRIPT_INL

