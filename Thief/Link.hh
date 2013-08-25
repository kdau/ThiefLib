/******************************************************************************
 *  Link.hh
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

#ifndef THIEF_LINK_HH
#define THIEF_LINK_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/Message.hh>

namespace Thief {



struct Flavor
{
	typedef long Number;
	static const Number ANY;

	Number number;

	Flavor (Number);

	bool operator == (const Flavor&) const;
	bool operator != (const Flavor&) const;

	Flavor get_reverse () const;

	Flavor (const String& name);
	Flavor (const char* name);
	String get_name () const;
};



class Link
{
public:
	typedef long Number;
	static const Number NONE;

	Number number;

	Link (Number = NONE);

	static Link create (Flavor, const Object& source,
		const Object& dest, const void* data = nullptr);

	bool exists () const;
	bool destroy ();

	bool operator == (const Link&) const;
	bool operator != (const Link&) const;
	Link get_reverse () const;

	Flavor get_flavor () const;
	Object get_source () const;
	Object get_dest () const;

	const void* get_data_raw () const;
	void set_data_raw (const void* data);

	MULTI_GET_ARG (get_data_field, String, field);
	MULTI_SET_ARG (set_data_field, String, field);

	static bool any_exist (Flavor,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY);

	static Link get_one (Flavor,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY);

	enum class Inheritance { NONE, SOURCE, DEST };

	static Link get_any (Flavor = Flavor::ANY,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY,
		Inheritance = Inheritance::NONE);

	static Links get_all (Flavor = Flavor::ANY,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY,
		Inheritance = Inheritance::NONE);

	static void dump_links (Flavor = Flavor::ANY,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY,
		Inheritance = Inheritance::NONE);

	static void subscribe (const Object& source, Flavor,
		const Object& host = Object::SELF);
	static void unsubscribe (const Object& source, Flavor,
		const Object& host = Object::SELF);

private:
	void _get_data_field (const String& field, LGMultiBase& multi) const;
	void _set_data_field (const String& field, const LGMultiBase& multi);
};



class LinkChangeMessage : public Message
{
public:
	enum Event { NONE, CHANGE, ADD, REMOVE };

	LinkChangeMessage (Event, Flavor, Link::Number,
		const Object& source, const Object& dest);
	THIEF_MESSAGE_WRAP (LinkChangeMessage);

	Event get_event () const;
	Flavor get_flavor () const;
	Link::Number get_number () const;
	Object get_source () const;
	Object get_dest () const;
};



// Common declarations for flavor-specific Link subclasses

#define THIEF_FLAVORED_LINK(FlavorName) \
class FlavorName##Link; \
typedef std::vector<FlavorName##Link> FlavorName##Links; \
class FlavorName##Link : public Link

#define THIEF_FLAVORED_LINK_COMMON(FlavorName) \
private: \
	void check_valid () const; \
public: \
	static Flavor flavor (bool reverse = false); \
	FlavorName##Link (Number = NONE); \
	FlavorName##Link (const Link&); \
	static FlavorName##Links get_all \
		(const Object& source = Object::ANY, \
		const Object& dest = Object::ANY, \
		Inheritance = Inheritance::NONE); \
	static FlavorName##Links get_all_reverse \
		(const Object& source = Object::ANY, \
		const Object& dest = Object::ANY, \
		Inheritance = Inheritance::NONE);



// CorpseLink

THIEF_FLAVORED_LINK (Corpse)
{
	THIEF_FLAVORED_LINK_COMMON (Corpse)

	static CorpseLink create (const Object& source,
		const Object& dest, bool propagate_source_scale);

	bool get_propagate_source_scale () const;
	void set_propagate_source_scale (bool);
};



// FlinderizeLink

THIEF_FLAVORED_LINK (Flinderize)
{
	THIEF_FLAVORED_LINK_COMMON (Flinderize)

	static FlinderizeLink create (const Object& source, const Object& dest,
		int count, float impulse, bool scatter,
		const Vector& offset = Vector ());

	int get_count () const;
	void set_count (int);

	float get_impulse () const;
	void set_impulse (float);

	bool get_scatter () const;
	void set_scatter (bool);

	Vector get_offset () const;
	void set_offset (const Vector&);
};



// ScriptParamsLink

THIEF_FLAVORED_LINK (ScriptParams)
{
	THIEF_FLAVORED_LINK_COMMON (ScriptParams)

	static ScriptParamsLink create (const Object& source,
		const Object& dest, const String& data = String ());

	String get_data () const;
	void set_data (const String&);
};



} // namespace Thief

#include <Thief/Link.inl>

#endif // THIEF_LINK_HH

