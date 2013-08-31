/******************************************************************************
 *  Property.hh
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

#ifndef THIEF_PROPERTY_HH
#define THIEF_PROPERTY_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>
#include <Thief/Object.hh>

namespace Thief {



// Property: pointer to a property type

class Property
{
public:
	typedef int Number;
	static const Number NONE;

	Property (Number);
	Property (const String& name);
	Property (const char* name);
	Property (const Property&);
	~Property ();

	bool operator == (const Property&) const;
	bool operator != (const Property&) const;

	Number get_number () const;
	String get_name () const;

private:
	friend class ObjectProperty;
	friend class PropertyChangeMessage;
	IProperty* iface;
};


// ObjectProperty: direct access to properties and property fields

class ObjectProperty
{
public:
	ObjectProperty (const Property&, const Object&,
		bool add_if_missing = false);

	Property get_property () const;
	Object get_object () const;

	bool exists (bool inherited = true) const;
	bool add ();
	bool copy_from (const Object& source);
	bool remove ();

	MULTI_GET (get);
	MULTI_SET (set);
	template <typename T> T get (const T& default_value) const;

	MULTI_GET_ARG (get_field, String, field);
	MULTI_SET_ARG (set_field, String, field);
	template <typename T>
	T get_field (const String& field, const T& default_value) const;

	static bool subscribe (const Property&, const Object&,
		const Object& host = Object::SELF);
	static bool unsubscribe (const Property&, const Object&,
		const Object& host = Object::SELF);

private:
	friend class PropFieldBase;
	friend class PropertyChangeMessage;

	void _get (LGMultiBase& value) const;
	bool _set (const LGMultiBase& value);
	void _get_field (const String& field, LGMultiBase& value) const;
	void _get_field (const char* field, LGMultiBase& value) const;
	bool _set_field (const String& field, const LGMultiBase& value,
		bool add_if_missing = false);
	bool _set_field (const char* field, const LGMultiBase& value,
		bool add_if_missing = false);

	Property property;
	Object object;
};



// PropField: interpreted access to property fields for Object subclasses

class PropFieldBase
{
protected:
	void get (const Object&, const char*, const char*, LGMultiBase&) const;
	void set (Object&, const char*, const char*, const LGMultiBase&);

	bool get_bit (const FieldProxyConfig<bool>&, const Object&) const;
	void set_bit (const FieldProxyConfig<bool>&, Object&, bool);
};

template <typename T, const FieldProxyConfig<T>& config>
class PropField : public PropFieldBase
{
public:
	PropField (Object&);

	bool exists () const;
	operator T () const;
	PropField& operator = (const T&);

private:
	Object& object;
};

template <const FieldProxyConfig<bool>& config>
class PropField<bool, config> : public PropFieldBase
{
public:
	PropField (Object&);

	bool exists () const;
	operator bool () const;
	PropField& operator = (bool);

private:
	Object& object;
};

template <typename T, const FieldProxyConfig<T>& config>
std::ostream& operator << (std::ostream&, const PropField<T, config>&);



// PropertyChangeMessage

class PropertyChangeMessage : public Message //TESTME
{
public:
	enum Event { CHANGE, ADD, REMOVE };

	PropertyChangeMessage (Event, bool inherited, const Property&,
		const Object&);
	THIEF_MESSAGE_WRAP (PropertyChangeMessage);

	Event get_event () const;
	bool is_inherited () const;

	Property get_property () const;
	Object get_object () const;
};



} // namespace Thief

#include <Thief/Property.inl>

#endif // THIEF_PROPERTY_HH

