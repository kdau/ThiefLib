/******************************************************************************
 *  module.hh
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts and the Object Script Library
 *  Copyright (C) 2005-2013 Tom N Harris <telliamed@whoopdedo.org>
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

#ifndef THIEF_MODULE_HH
#define THIEF_MODULE_HH

#include <Thief/Script.hh>



// LG interface expects this format

struct sScrClassDesc
{
	const char* module_name;
	const char* class_name;
	const char* parent_name;

	typedef IScript* (__cdecl *Factory) (const char*, int);
	Factory factory;
};



// Begin the module definition: open namespace Thief and
//	define ScriptModule::real_name and ScriptModule::scripts

#define THIEF_MODULE_BEGIN(ModuleName) \
namespace Thief { \
\
const char* const \
ScriptModule::real_name = ModuleName; \
\
const sScrClassDesc \
ScriptModule::scripts[] = \
{



// Declare a script: list it and its factory lambda in ScriptModule::scripts

#define THIEF_SCRIPT(Name, Parent, Class) \
	{ ScriptModule::real_name, Name, Parent, \
	  [] (const char* name, int host) __cdecl \
		{ \
			if (_stricmp (name, Name) != 0) \
				return static_cast<IScript*> (nullptr); \
			Class* script = new (std::nothrow) Class (Name, host); \
			return script->get_interface (); \
		} \
	},



// End the module definition: close ScriptModule::scripts,
//	define ScriptModule::script_count, and close namespace Thief

#define THIEF_MODULE_END \
}; \
\
const size_t \
ScriptModule::script_count = sizeof (scripts) / sizeof (sScrClassDesc); \
\
}



#endif // THIEF_MODULE_HH

