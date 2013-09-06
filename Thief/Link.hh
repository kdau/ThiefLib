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
	bool operator < (const Flavor&) const;

	bool is_reverse () const;
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

	MULTI_GET_ARG (get_data_field, String, field);
	MULTI_SET_ARG (set_data_field, String, field);

	const void* get_data_raw () const;
	void set_data_raw (const void*);

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

	static bool subscribe (Flavor, const Object& source,
		const Object& host = Object::SELF);
	static bool unsubscribe (Flavor, const Object& source,
		const Object& host = Object::SELF);

private:
	friend class LinkFieldBase;

	template <typename T, size_t count,
		const FieldProxyConfig<T, count>& config>
	friend class LinkField;

	void _get_data_field (const String& field, LGMultiBase& multi) const;
	void _get_data_field (const char* field, LGMultiBase& multi) const;
	void _set_data_field (const String& field, const LGMultiBase& multi);
	void _set_data_field (const char* field, const LGMultiBase& multi);
};



// LinkField: interpreted access to fields of flavor-specific Link subclasses

class LinkFieldBase
{
protected:
	bool get_bit (const FieldProxyConfig<bool, 1u>&, const Link&) const;
	void set_bit (const FieldProxyConfig<bool, 1u>&, Link&, bool);
};

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
class LinkField : public LinkFieldBase
{
public:
	LinkField (Link&, size_t index);

	operator T () const;
	LinkField& operator = (const T&);

private:
	Link& link;
	size_t index;
};

template <typename T, const FieldProxyConfig<T, 1u>& config>
class LinkField<T, 1u, config>;

template <const FieldProxyConfig<bool, 1u>& config>
class LinkField<bool, 1u, config>;

template <typename T, size_t count, const FieldProxyConfig<T, count>& config>
std::ostream& operator << (std::ostream&, const LinkField<T, count, config>&);



// LinkChangeMessage

class LinkChangeMessage : public Message //TESTME
{
public:
	enum Event { CHANGE, ADD, REMOVE };

	LinkChangeMessage (Event, Flavor, Link::Number,
		const Object& source, const Object& dest);
	THIEF_MESSAGE_WRAP (LinkChangeMessage);

	Event get_event () const;
	Flavor get_flavor () const;
	Link::Number get_link () const;
	Object get_source () const;
	Object get_dest () const;
};



} // namespace Thief

#include <Thief/Link.inl>

namespace Thief {



// CorpseLink

THIEF_FLAVORED_LINK (Corpse)
{
	THIEF_FLAVORED_LINK_COMMON (Corpse)

	static CorpseLink create (const Object& source,
		const Object& dest, bool propagate_source_scale);

	THIEF_LINK_FIELD (bool, propagate_source_scale);
};



// FlinderizeLink

THIEF_FLAVORED_LINK (Flinderize)
{
	THIEF_FLAVORED_LINK_COMMON (Flinderize)

	static FlinderizeLink create (const Object& source, const Object& dest,
		int count, float impulse, bool scatter,
		const Vector& offset = Vector ());

	THIEF_LINK_FIELD (int, count);
	THIEF_LINK_FIELD (float, impulse);
	THIEF_LINK_FIELD (bool, scatter);
	THIEF_LINK_FIELD (Vector, offset);
};



// ScriptParamsLink

THIEF_FLAVORED_LINK (ScriptParams)
{
	THIEF_FLAVORED_LINK_COMMON (ScriptParams)

	static ScriptParamsLinks get_all_by_data (const Object& source,
		const CIString& data, Inheritance = Inheritance::NONE,
		bool reverse = false);

	static ScriptParamsLink get_one_by_data (const Object& source,
		const CIString& data, bool reverse = false);

	static ScriptParamsLink create (const Object& source,
		const Object& dest, const String& data = String ());

	THIEF_LINK_FIELD (String, data);
};



} // namespace Thief

#endif // THIEF_LINK_HH

