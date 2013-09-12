/******************************************************************************
 *  ParameterCache.hh
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts, Object Script Library, and Dark Hook 2
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

#ifndef PARAMETERCACHE_HH
#define PARAMETERCACHE_HH

#include "Private.hh"

namespace Thief {



// ParameterCache

class ParameterCache
{
public:
	virtual bool exists (const Object& object,
		const CIString& parameter, bool inherit) = 0;
	virtual const String* get (const Object& object,
		const CIString& parameter, bool inherit) = 0;
	virtual bool set (const Object& object,
		const CIString& parameter, const String& value) = 0;
	virtual bool copy (const Object& source, const Object& dest,
		const CIString& parameter) = 0;
	virtual bool remove (const Object& object,
		const CIString& parameter) = 0;

	virtual void watch_object (const Object&,
		const ParameterBase&) = 0;
	virtual void unwatch_object (const Object&,
		const ParameterBase&) = 0;
};



#ifdef IS_OSL



struct DesignNote
{
	DesignNote () : indirect_watchers (0), state (NONE) {}

	typedef std::set<const ParameterBase*> Watchers;
	Watchers direct_watchers;
	size_t indirect_watchers;

	enum State {
		NONE = 0,
		CACHED = 1,
		EXISTENT = 2,
		RELEVANT = 4
	};
	unsigned state;

	typedef std::vector<Object::Number> Ancestors;
	Ancestors ancestors;

	typedef std::map<CIString, String> RawValues;
	RawValues raw_values;
};



class DesignNoteReader
{
public:
	typedef DesignNote::RawValues RawValues;

	DesignNoteReader (const char* dn, RawValues& raw_values);

private:
	bool handle_character (const char& ch);
	void handle_parameter ();

	RawValues& raw_values;
	enum class State { NAME, INDEX, VALUE } state;
	bool started, escaped;
	char quoted;
	size_t spaces;
	const char *name_begin, *name_end, *index_begin, *index_end;
	String raw_value;
};



class ParameterCacheImpl : public ParameterCache
{
public:
	virtual ~ParameterCacheImpl ();

	virtual bool exists (const Object& object,
		const CIString& parameter, bool inherit);
	virtual const String* get (const Object& object,
		const CIString& parameter, bool inherit);
	virtual bool set (const Object& object,
		const CIString& parameter, const String& value);
	virtual bool copy (const Object& source, const Object& dest,
		const CIString& parameter);
	virtual bool remove (const Object& object,
		const CIString& parameter);

	virtual void watch_object (const Object&, const ParameterBase&);
	virtual void unwatch_object (const Object&, const ParameterBase&);

private:
	friend class OSL;
	ParameterCacheImpl ();
	void reset ();

	SInterface<IStringProperty> dn_prop;

	PropListenerHandle listen_handle;
	static void __stdcall on_dn_change (sPropertyListenMsg*,
		PropListenerData);

	static void __stdcall on_trait_change (const sHierarchyMsg*, void*);

	DesignNote* update_object (Object::Number number);
	void update_ancestors (const Object& object, DesignNote& dn);
	void unwatch_ancestor (Object::Number number);

	void read_dn (Object::Number number);
	bool write_dn (Object::Number number);

	typedef std::map<Object::Number, DesignNote> Data;
	Data data;

	Object::Number current;
};



#endif // IS_OSL

} // namespace Thief

#endif // PARAMETERCACHE_HH

