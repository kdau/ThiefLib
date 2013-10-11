/******************************************************************************
 *  AI.inl
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

#ifndef THIEF_AI_HH
#error "This file should only be included from <Thief/AI.hh>."
#endif

#ifndef THIEF_AI_INL
#define THIEF_AI_INL

namespace Thief {



// AI

inline bool
AI::go_to_location (const Object& nearby, Speed speed, ActionPriority priority)
{
	return _go_to_location (nearby, speed, priority, LGMulti<Empty> ());
}

template <typename T>
inline bool
AI::go_to_location (const Object& nearby, Speed speed, ActionPriority priority,
	const T& result_data)
{
	return _go_to_location (nearby, speed, priority,
		LGMulti<T> (result_data));
}

inline bool
AI::frob_object (const Object& target, const Object& tool,
	ActionPriority priority)
{
	return _frob_object (target, tool, priority, LGMulti<Empty> ());
}

template <typename T>
bool
AI::frob_object (const Object& target, const Object& tool,
	ActionPriority priority, const T& result_data)
{
	return _frob_object (target, tool, priority, LGMulti<T> (result_data));
}



// AIActionResultMessage

template <typename T>
inline T
AIActionResultMessage::get_result_data () const
{
	LGMulti<T> multi;
	_get_result_data (multi);
	return multi;
}



} // namespace Thief

#endif // THIEF_AI_INL

