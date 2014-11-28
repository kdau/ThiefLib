/******************************************************************************
 *  Module.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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

#include "Private.hh"
#include "OSL.hh"

#include <windef.h>
#include <winbase.h>

namespace Thief {

IScriptMan*
LG = nullptr;

extern "C" const GUID IID_IOSLService = THIEF_IOSLService_GUID;



// ScriptModule

ScriptModule::ScriptModule ()
	: name (nullptr)
{
	set_name ();
}

ScriptModule::~ScriptModule ()
{}

void
ScriptModule::set_name (const char* _name)
{
	if (name && name != real_name)
		delete[] name;
	if (_name)
	{
		name = new char[strlen (_name) + 1];
		strcpy (name, _name);
	}
	else
		name = const_cast<char*> (real_name);
}



// ScriptModuleImpl

class ScriptModuleImpl : public cInterfaceImp<IScriptModule,
	IID_Def<IScriptModule>, kInterfaceImpStatic>, public ScriptModule
{
public:
	ScriptModuleImpl () {}
	virtual ~ScriptModuleImpl () {}

	void set_name (const char* name);

	// IScriptModule
	STDMETHOD_ (const char*, GetName) ();
	STDMETHOD_ (const sScrClassDesc*, GetFirstClass) (tScrIter*);
	STDMETHOD_ (const sScrClassDesc*, GetNextClass) (tScrIter*);
	STDMETHOD_ (void, EndClassIter) (tScrIter*);
} module;

void
ScriptModuleImpl::set_name (const char* _name)
{
	ScriptModule::set_name (_name);
}

STDMETHODIMP_ (const char*)
ScriptModuleImpl::GetName ()
{
	return name;
}

STDMETHODIMP_ (const sScrClassDesc*)
ScriptModuleImpl::GetFirstClass (tScrIter* iter)
{
	*reinterpret_cast<size_t*> (iter) = 0u;
	return (script_count > 0u)
		? reinterpret_cast<const sScrClassDesc*> (&scripts [0])
		: nullptr;
}

STDMETHODIMP_ (const sScrClassDesc*)
ScriptModuleImpl::GetNextClass (tScrIter* iter)
{
	size_t& index = *reinterpret_cast<size_t*> (iter);
	return (++index < script_count)
		? reinterpret_cast<const sScrClassDesc*> (&scripts [index])
		: nullptr;
}

STDMETHODIMP_ (void)
ScriptModuleImpl::EndClassIter (tScrIter*)
{}



} // namespace Thief



// Exported functions

extern "C"
int __declspec(dllexport) __stdcall
ScriptModuleInit (const char* name, IScriptMan* manager, MPrintfProc mprintf,
	IMalloc* allocator, IScriptModule** module_ptr)
{
	if (!manager || !allocator) return false;

#ifdef DEBUG
	if (mprintf)
		mprintf ("%s module loading...\n", Thief::module.get_name ());
#endif

	// Confirm that this is NewDark by checking for the new IEngineSrv.
	try
	{
		SService<IEngineSrv> engine (manager);
	}
	catch (no_interface&)
	{
		if (mprintf)
			mprintf ("ERROR: %s cannot be used with this version of "
				"the Dark Engine. Upgrade to NewDark.\n",
				Thief::module.get_name ());
		return false;
	}

	// Attach various ThiefLib components to the engine.
	Thief::LG = manager;
	Thief::alloc.attach (allocator, name);
	Thief::mono.attach (mprintf);

	// Load the OSL.
	HMODULE osl = ::GetModuleHandleA (OSL_NAME);
	if (!osl) osl = ::LoadLibraryA (OSL_NAME);
	if (!osl)
	{
		if (mprintf)
			mprintf ("ERROR: Could not load the ThiefLib support "
				"library " OSL_NAME ".\n");
		return false;
	}

	// Initialize the OSL.
	auto osl_init = reinterpret_cast<Thief::OSLInitProc>
		(::GetProcAddress (osl, OSL_INIT_PROC));
	if (!osl_init || !osl_init (manager, mprintf, allocator))
		return false;

	// Prepare the ScriptModule.
	Thief::module.set_name (name);
	Thief::module.QueryInterface (IID_IScriptModule,
		reinterpret_cast<void**> (module_ptr));

#ifdef DEBUG
	if (mprintf) mprintf ("%s module loaded.\n", Thief::module.get_name ());
#endif

	return true;
}

extern "C"
BOOL WINAPI
DllMain (HINSTANCE dll, DWORD reason, PVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls (dll);
	(void) reserved;
	return true;
}

