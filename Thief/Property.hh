//! \file Property.hh Configuration and state data associated with game objects.

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

#ifndef THIEF_PROPERTY_HH
#define THIEF_PROPERTY_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>
#include <Thief/Object.hh>

namespace Thief {



/*! A reference to a type of object property.
 * Properties define almost all the attributes of a game Object, including both
 * its initial configuration and current state. This class is a reference to a
 * specific type of property. Since the various Object subclasses in ThiefLib
 * provide interpreted access to most useful properties, this class does not
 * normally need to be used directly. See ObjectProperty for more information.
 *
 * Each property type has an internal number, an internal name (such as
 * \c "PhysType"), and a DromEd name as part of the organized hierarchy (such as
 * <i>Physics&rarr;Model&rarr;Type</i>). The numbers are not stable between
 * engine versions and the DromEd names are not easily accessible to scripts, so
 * internal names should generally be used. */
class Property
{
public:
	//! Constructs a reference to no property.
	Property ();

	//! Constructs a reference to the given property.
	Property (const Property&);

	//! Destroys a property reference.
	~Property ();

	/*! Constructs a reference to the named property.
	 * \throw MissingResource if no property exists with the given \a name. */
	Property (const String& name);

	/*! Constructs a reference to the named property.
	 * If \a name is a null pointer, refers to #NONE.
	 * \throw MissingResource if no property exists with the given \a name. */
	Property (const char* name);

	//! Returns the engine-internal name of the property.
	String get_name () const;

	/*! An engine-internal property number.
	 * These numbers are not stable between engine versions, and should not
	 * be used directly. Use property names instead. */
	typedef int Number;

	//! Indicates that no property is currently referenced.
	static const Number NONE;

	/*! Constructs a reference to the property with the given number.
	 * These numbers are not stable between engine versions, and should not
	 * be used directly. The number is not checked for validity. Use
	 * property names instead. */
	explicit Property (Number);

	//! Returns the engine-internal number of the property.
	Number get_number () const;

	//! Returns whether this property is the same as the given one.
	bool operator == (const Property&) const;

	//! Returns whether this property is different from the given one.
	bool operator != (const Property&) const;

	/*! Returns whether this property should sort before the given one.
	 * Sorting is based on the properties' engine-internal numbers. */
	bool operator < (const Property&) const;

private:
	friend class ObjectProperty;
	friend class OSL;
	IGenericProperty* iface;
};



/*! A message about a change to a Property on an Object.
 * A \c PropertyChange message is sent whenver a property has been instantiated,
 * changed, or removed on a game object or an ancestor from which it inherits
 * that property. It is sent only to scripts hosted by objects that have
 * subscribed to the property, either for that specific object or for all
 * objects. This is a message created by ThiefLib, but it is available to all
 * scripts. \warning PropertyMessage instances cannot be post()ed or schedule()d
 * (including broadcast() with a delay time).
 * \note %Message name: \c PropertyChange */
class PropertyMessage : public Message
{
public:
	//! A property-related event.
	enum Event
	{
		INSTANTIATE, //!< The property was instantiated.
		CHANGE,      //!< The value of the property instance changed.
		REMOVE       //!< The property instance was removed.
	};

	//! Constructs a new property-related message for the given event.
	PropertyMessage (Event event, bool inherited, const Property& property,
		const Object& object);

	THIEF_MESSAGE_WRAP (PropertyMessage);

	//! The event that occurred on the affected property instance.
	const Event event;

	/*! Whether the event affected an instance on an ancestor of the object
	 * instead of on the object itself. \note Due to technical limitations,
	 * every message about a change to an instance on the object itself is
	 * preceded by a message about a change on an ancestor, even if no
	 * ancestor has actually changed. Treat messages having \a inherited ==
	 * \c true with caution. */
	const bool inherited;

	//! The property whose instance on the #object was affected.
	const Property property;

	//! The object whose #property instance was affected.
	const Object object;

private:
	virtual bool is_postable () const;
};



/*! A reference to a Property as it exists on a specific Object.
 * Properties define almost all the attributes of a game Object, including both
 * its initial configuration and current state. A Property is a type with a
 * defined data structure, whether a simple property (with a type such as string
 * or integer) or a complex property with multiple fields. Each field has a name
 * which is the label shown for it in the DromEd dialog for its property.
 *
 * When a property is set directly on an Object, it it said to be instantiated
 * on that object. If a property has no instance on an object, it may inherit
 * an instance from one of the object's ancestors (see Object::get_ancestors()).
 * A property is said to exist for an object if it is instantiated on the object
 * itself or inherited from an instance on an ancestor.
 *
 * Since the various Object subclasses in ThiefLib provide interpreted access to
 * most useful properties, this class does not normally need to be used directly.
 * See PropField and the member variables of the Object subclasses for more
 * information.
 *
 * However, certain properties and property fields are not wrapped specifically
 * by ThiefLib because they: are not instantiated in %Thief II's stock
 * \c dark.gam, have no documented use, work only in System Shock 2, were never
 * implemented at all, are better accessed through other methods, and/or are
 * just clearly unsuited for direct use by scripts. If you cannot locate a
 * PropField wrapper for a property or field, use this class instead. */
class ObjectProperty
{
public:
	/*! Constructs a reference to the given \a property on the given
	 * \a object. If \a instantiate_if_missing is \c true, the property will
	 * be instantiated on the object if it is not already instantiated and
	 * is not inherited from any instance on an ancestor. This allows a
	 * temporary ObjectProperty instance to be used to set fields. */
	ObjectProperty (const Property& property, const Object& object,
		bool instantiate_if_missing = false);

	//! Returns the property associated with this reference.
	Property get_property () const { return property; }

	//! Returns the object associated with this reference.
	Object get_object () const { return object; }

	/*! Returns whether this reference should sort before the given one.
	 * Sorting is based on the property and object numbers. */
	bool operator < (const ObjectProperty&) const;

	/*! Returns whether the property is instantiated on the object.
	 * \param inherited Whether to consider instances of the property on
	 * ancestors of the object. */
	bool exists (bool inherited = true) const;

	/*! Instantiates the property on the object.
	 * If the property is not directly instantiated on this object (even if
	 * it is instantiated on an ancestor), an instance will be created.
	 * \throw MissingResource if the object or property is not valid.
	 * \throw std::runtime_error if the property could not be instantiated.
	 * \return Whether the property needed to be instantiated. */
	bool instantiate ();

	/*! Copies the value of the property from the given object to this one.
	 * The property must be instantiated on or inherited by the \a source
	 * object, but need not exist on this object. \throw MissingResource
	 * if the object, property, or source object is not valid. \throw
	 * std::runtime_error if the property value could not be copied. */
	void copy_from (const Object& source);

	/*! Removes any instance of the property on the object.
	 * If the property is directly instantiated on this object, the instance
	 * will be destroyed. Any instances on ancestors will not be affected.
	 * \throw MissingResource if the object or property is not valid.
	 * \throw std::runtime_error if the property could not be removed.
	 * \return Whether the property needed to be removed. */
	bool remove ();

	/*! Returns the current value of the simple property on the object.
	 * \throw MissingResource if the object or property is not valid, the
	 * property does not exist for the object, or the property is not a
	 * simple property. \throw LGMultiTypeError if the property is of a
	 * different type than the one requested. */
	template <typename T> T get () const;

	/*! Returns the current value of the simple property on the object.
	 * If the property does not exist on the object, returns the given
	 * \a default_value. \throw MissingResource if the object or property is
	 * not valid or the property is not a simple property.
	 * \throw LGMultiTypeError if the property is of a different type than
	 * the one requested. */
	template <typename T> T get (const T& default_value) const;

	/*! Sets the simple property on the object to the given \a value.
	 * The property is instantiated on the object if needed.
	 * \throw MissingResource if the object or property is not valid.
	 * \throw std::runtime_error if the property is not a simple property or
	 * it could not be set on the object. */
	template <typename T> void set (const T& value);

	/*! Returns the current value of the given property field on the object.
	 * \throw MissingResource if the object or property is not valid, the
	 * property does not exist for the object, or the named field does
	 * not exist. \throw LGMultiTypeError if the field is of a different
	 * type than the one requested. */
	template <typename T> T get_field (const String& field) const;

	/*! Returns the current value of the given property field on the object.
	 * If the property does not exist on the object, returns the given
	 * \a default_value. \throw MissingResource if the object or property is
	 * not valid or the named field does not exist. \throw LGMultiTypeError
	 * if the field is of a different type than the one requested. */
	template <typename T>
	T get_field (const String& field, const T& default_value) const;

	/*! Sets the given property \a field on the object to the given \a value.
	 * The property must be instantiated already.
	 * \throw MissingResource if the object or property is not valid or the
	 * property is not instantiated on the object.
	 * \throw std::runtime_error if the property field does not exist or
	 * could not be set on the object. */
	template <typename T> void set_field (const String& field,
		const T& value);

	/*! Subscribes to changes to the given property on the given object.
	 * "PropertyChange" messages will be sent to all scripts on the given
	 * \a host object when such changes occur. \param property The property
	 * to monitor. \param object The object to monitor the property on,
	 * or Object::ANY to monitor the property on every object it exists for.
	 * \param host The object to notify of changes. If Object::SELF is given,
	 * the \a object will be notified. (Object::SELF is not valid if
	 * \a object is Object::ANY). \throw std::runtime_error if the
	 * subscription could not be created. */
	static void subscribe (const Property& property, const Object& object,
		const Object& host = Object::SELF);

	/*! Unsubscribes from changes to the given property on the given object.
	 * The \a property and \a object must match the original subscribe()
	 * call. Only messages to scripts on the given \a host object will stop.
	 * \return Whether a matching subscription had existed. */
	static bool unsubscribe (const Property& property, const Object& object,
		const Object& host = Object::SELF);

private:
	friend class PropFieldBase;
	friend class PropertyMessage;

	void _get (LGMultiBase& value) const;
	void _set (const LGMultiBase& value);

	void _get_field (const char* field, LGMultiBase& value) const;
	void _set_field (const char* field, const LGMultiBase& value,
		bool instantiate_if_missing);

	const void* get_raw (bool inherited = true) const;
	void set_raw (const void*);

	Property property;
	Object object;
};



/*! Base class for all PropField specializations.
 * This class cannot be used directly. See PropField for more information. */
class PropFieldBase
{
protected:
	//! \cond HIDDEN_SYMBOLS
	void get (const Object&, const char*, const char*, LGMultiBase&) const;
	void set (Object&, const char*, const char*, const LGMultiBase&);
	void set_raw (Object&, const char*, const void*);
	//! \endcond
};



/*! A field of an ObjectProperty.
 * This class serves as a proxy to a field in a property of a game object. Like
 * all field proxies, it is not intended for construction by ThiefLib users.
 * Unlike link fields, properties and property fields do not necessarily exist
 * on an object and may be inherited from its ancestors. See ObjectProperty for
 * more information. */
THIEF_FIELD_PROXY_TEMPLATE
class PropField : public PropFieldBase
{
public:
	//! Constructs a new property field proxy for the given object and index.
	PropField (Object&, size_t index);

	/*! Returns whether the underlying property is instantiated on the
	 * object. \param inherited Whether to consider instances of the
	 * underlying property on ancestors of the object. */
	bool exists (bool inherited = true) const;

	/*! Instantiates the underlying property on the object.
	 * This method affects the entire underlying property, not just this
	 * specific field. See ObjectProperty::instantiate(). */
	bool instantiate ();

	/*! Removes any instance of the underlying property on the object.
	 * This method affects the entire underlying property, not just this
	 * specific field. See ObjectProperty::remove(). */
	bool remove ();

	//! Returns the current value of the field.
	operator Type () const;

	//! Sets the field to the given value.
	PropField& operator = (const Type&);

	//! Returns whether the field currently has the given value.
	bool operator == (const Type&) const;

	//! Returns whether the field does not currently have the given value.
	bool operator != (const Type&) const;

private:
	Object& object;
	size_t index;
};

/*! Outputs the current value of the given property field to the given stream.
 * \relates PropField */
THIEF_FIELD_PROXY_TEMPLATE
std::ostream& operator << (std::ostream&,
	const THIEF_FIELD_PROXY_CLASS (PropField)&);



} // namespace Thief

#include <Thief/Property.inl>

#endif // THIEF_PROPERTY_HH

