/******************************************************************************
 *  Message.inl
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
#error "This file should only be included from <Thief/Message.hh>."
#endif

#ifndef THIEF_MESSAGE_INL
#define THIEF_MESSAGE_INL

namespace Thief {

template <typename T>
inline T
Message::get_data (Datum datum, const T& default_value) const
{
	if (has_data (datum))
	{
		LGMulti<T> multi;
		_get_data (datum, multi);
		return multi;
	}
	else
		return default_value;
}

template <typename D1, typename D2, typename D3>
inline GenericMessage
GenericMessage::with_data (const char* name, const D1& data1,
	const D2& data2, const D3& data3)
{
	GenericMessage message (name);
	message.set_data (DATA1, data1);
	if (!std::is_same<D2, Empty> ())
		message.set_data (DATA2, data2);
	if (!std::is_same<D3, Empty> ())
		message.set_data (DATA3, data3);
	return message;
}

template <typename D1, typename D2, typename D3>
inline TimerMessage
TimerMessage::with_data (const char* timer_name, const D1& data1,
	const D2& data2, const D3& data3)
{
	TimerMessage message (timer_name);
	message.set_data (DATA1, data1);
	if (!std::is_same<D2, Empty> ())
		message.set_data (DATA2, data2);
	if (!std::is_same<D3, Empty> ())
		message.set_data (DATA3, data3);
	return message;
}

} // namespace Thief

#endif // THIEF_MESSAGE_INL

