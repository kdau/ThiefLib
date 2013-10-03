//! \file Module.hh Management of a custom script module as a whole.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_MODULE_HH
#define THIEF_MODULE_HH

#include <Thief/Base.hh>
#include <Thief/Script.hh>

namespace Thief {



//! \cond HIDDEN_SYMBOLS
struct ScriptInfo
{
	const char* module_name;
	const char* class_name;
	const char* parent_name;

	typedef IScript* (__cdecl *Factory) (const char*, int);
	Factory factory;
};
//! \endcond



/*! The custom script module.
 * This class and associated global functions initialize the script module when
 * it is loaded and register the module's scripts with the engine. Use the
 * #THIEF_MODULE macro to configure it. The only useful public member is
 * get_name(). */
class ScriptModule
{
public:
	//! Destroys a script module.
	virtual ~ScriptModule ();

	//! Returns the name of the script module as exposed to mission authors.
	const char* get_name () const { return name; }

protected:
	//! \cond HIDDEN_SYMBOLS

	ScriptModule ();

	void set_name (const char* name = nullptr);

	char* name;
	static const char* const real_name;

	static const ScriptInfo scripts[];
	static const size_t script_count;

	//! \endcond
};



/*! Declares the custom script module and allows the engine to instantiate it.
 * This macro should be called exactly once, and in the global namespace.
 * \param ModuleName A string literal naming the module as it will be listed in
 * the \c script_load command. \param ... The scripts in the module. Each of
 * these arguments should be a call to the #THIEF_SCRIPT macro. */
#define THIEF_MODULE(ModuleName, ...) \
namespace Thief { \
\
const char* const \
ScriptModule::real_name = ModuleName; \
\
const ScriptInfo \
ScriptModule::scripts[] = \
{ \
	__VA_ARGS__ \
}; \
\
const size_t \
ScriptModule::script_count = sizeof (scripts) / sizeof (ScriptInfo); \
\
}



} // namespace Thief

#endif // THIEF_MODULE_HH

