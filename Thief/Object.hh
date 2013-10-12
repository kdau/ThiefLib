//! \file Object.hh Base class and macros for game object wrappers.

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

#ifndef THIEF_OBJECT_HH
#define THIEF_OBJECT_HH

#include <Thief/Base.hh>

namespace Thief {

/*! A reference to a manipulable object in the game world.
 * Virtually all dynamic, interactive, and scriptable elements of the Dark
 * %Engine game world exist as objects. The Object class and its many
 * descendants are wrappers for game objects, referring to them and providing
 * access to their many properties and behaviors.
 *
 * An object wrapper refers to an object by its #number, and may or may not
 * reference a valid object at any time. (This can be checked with the exists()
 * method.) If a wrapper does not currently reference a valid object, all \b but
 * the following members, and all members of descendant classes, have undefined
 * behavior:
 *   - #number
 *   - exists()
 *   - get_type()
 *   - comparison and assignment operators
 *   - get_editor_name()
 *   - operator <<()
 *
 * The game world is made up of concrete objects. Each concrete object inherits
 * from one direct, abstract ancestor, an archetype which identifies its type.
 * That archetype in turn inherits from other archetypes, up to one of several
 * root archetypes. In Thief, the root archetypes are \c Object, \c Stimulus,
 * <tt>Flow Group</tt>, \c MetaProperty, \link Room <tt>Base %Room</tt>\endlink,
 * and \c Texture.
 *
 * In addition to direct and indirect archetypes, both concrete and abstract
 * objects may hold any number of metaproperties. Metaproperties are abstract
 * objects which allow bundles of links and properties to be applied in various
 * places across the main hierarchy of archetypes.
 *
 * All concrete objects have a specific position in the game world, consisting
 * of a location vector (X-Y-Z coordinates) and a rotation vector (heading
 * pitch-bank directions in degrees).
 *
 * Concrete objects have positive object numbers and are stored in the mission
 * file (<tt>*.mis</tt>). Abstract objects (both archetypes and metaproperties)
 * have negative object numbers and are stored in the gamesys file
 * (<tt>*.gam</tt>). Both types of object are stored in a saved game file
 * (<tt>*.sav</tt>) or a DromEd COW file (<tt>*.cow</tt>). Transient objects are
 * not saved in any file.
 *
 * \nosubgrouping */
class Object
{
public:
	//! A list of references to game objects.
	typedef std::vector<Object> List;

	//! A set of references to game objects.
	typedef std::set<Object> Set;

	/*! %Object numbers uniquely identify objects, both concrete (positive)
	 * and abstract (negative), within the mission and gamesys. */
	typedef int Number;

	//! \name Locating and wrapping objects
	//@{

	//! Constructs an object wrapper not referencing any object.
	Object ();

	//! Constructs an object wrapper referencing the same object as another.
	Object (const Object&);

	//! Copies the reference of the given object wrapper to this wrapper.
	Object& operator = (const Object&);

	/*! Constructs an object wrapper referencing an object with the given
	 * number. The number will not be checked for validity. */
	explicit Object (Number);

	/*! Constructs an object wrapper referencing the named object.
	 * If no object with the given \a name exists, the wrapper will
	 * reference #NONE. */
	explicit Object (const String& name);

	//! Returns whether the wrapper currently references a valid object.
	bool exists () const;

#ifdef IS_THIEF2
	/*! Returns the object of the given archetype nearest the given point.
	 * Objects that inherit directly or indirectly from the \a archetype are
	 * considered, and the inheriting object whose center is closest in
	 * location to the center of the reference object \a nearby is returned.
	 * Returns #NONE if no concrete objects inheriting from \a archetype
	 * exist. \t2only */
	static Object find_closest (const Object& archetype,
		const Object& nearby);
#endif

	//@}
	//! \name Creating and destroying objects
	//@{

	//! Creates a concrete instance of the given \a archetype.
	static Object create (const Object& archetype);

	/*! Starts creating a concrete instance of the given \a archetype.
	 * This is two-stage object creation, which must be finished by a call
	 * to finish_create(). The feature is intended to allow certain key
	 * properties and links to be established before the object is examined
	 * by the engine, but certain aspects may be examined even before a call
	 * to finish_create(). YMMV. */
	static Object start_create (const Object& archetype);

	/*! Finishes creating an object that was started by start_create().
	 * \throw std::runtime_error if the object could not finish being
	 * created or was not created with start_create(). */
	void finish_create ();

	/*! Creates a fnord object with a limited lifespan.
	 * The object will inherit directly from the \c Marker archetype. It
	 * will not be persisted in saved games. It will be destroyed after the
	 * given \a lifespan has passed, implemented by a delete tweq. */
	static Object create_temp_fnord (Time lifespan = 1ul);

	/*! Creates an archetype.
	 * The archetype will be named \a name and will directly inherit from
	 * \a parent, which must also be an archetype. */
	static Object create_archetype (const Object& parent, const String& name);

	/*! Creates a metaproperty.
	 * The metaproperty will be named \a name and will directly inherit from
	 * \a parent, which must also be an metaproperty. */
	static Object create_metaprop (const Object& parent, const String& name);

	/*! Creates a copy of the referenced concrete object.
	 * The clone will have the same parent and metaproperties as the
	 * referenced object. It will carry most (all?) of the referenced
	 * object's properties, but none of its links. */
	Object clone () const;

	//! Removes the referenced object from the database immediately.
	void destroy ();

	/*! Remove the object from the database after \a lifespan has passed.
	 *  The timing is implemented by adding a delete tweq to the object, so
	 *  any existing delete tweq is overridden. Abstract objects are not
	 *  supported. */
	void schedule_destruction (Time lifespan);

	//@}

	//! \name Object numbers and names
	//@{

	/*! The number of the referenced object.
	 * This number may or may not refer to a currently valid object. It may
	 * be #NONE if no object is referenced. */
	Number number;

	/*! Indicates that no object is currently referenced.
	 * This pseudo-reference (not valid as an actual object reference) is
	 * used to indicate that an object wrapper does not currently reference
	 * an object, or that a method that returns an object did not have any
	 * object to return. */
	static const Object NONE;

	/*! Refers to the set of all relevant objects when passed to certain
	 * methods. This pseudo-reference (not valid as an actual object
	 * reference) can be used as an argument to certain methods, as
	 * indicated in their documentation. It refers to every object that is
	 * relevant in a certain context, such as links of a certain flavor from
	 * a specific object to any other object. Its meaning is undefined when
	 * it is used in a context for which it is not intended. */
	static const Object ANY;

	/*! Refers back to a context-relevant object when passed to certain
	 * methods. This pseudo-reference (not valid as an actual object
	 * reference) can be used as an argument to certain methods, as
	 * indicated in their documentation. It refers to an object that is
	 * relevant in context, such as the object whose method is being called
	 * or the host of the calling script. Its meaning is undefined when it
	 * is used in a context for which it is not intended. */
	static const Object SELF;

	//! Returns whether this wrapper and another reference the same object.
	bool operator == (const Object&) const;

	//! Returns whether this wrapper and another reference different objects.
	bool operator != (const Object&) const;

	//! Sorts this object wrapper and another by object #number.
	bool operator < (const Object&) const;

	//! Compares this object wrapper's #number with the given number.
	bool operator == (Number) const;

	//! Contrasts this object wrapper's #number with the given number.
	bool operator != (Number) const;

	/*! Returns the engine-internal name of the referenced object, if any.
	 * This is the name that may be used to look up the object uniquely. */
	String get_name () const;

	/*! Sets the engine-internal name of the referenced object.
	 * This is the name that may be used to look up the object uniquely. */
	void set_name (const String&);

	/*! Returns the formatted DromEd-style name of the referenced object.
	 * The returned string includes the object number, the name of a named
	 * object, and the name of an unnamed object's archetype. It matches the
	 * format used throughout DromEd and is suitable for contexts such as
	 * the monolog, but not for display to players. */
	String get_editor_name () const;

	/*! Returns the translated in-game name of the referenced object.
	 * This is the name referenced by the Inventory&rarr;%Object Name
	 * property. It  will be returned in the current game or FM language. */
	String get_display_name () const;

	/*! Returns the translated in-game description of the referenced object.
	 * This is the name referenced by the Inventory&rarr;Long Description
	 * property. It will be returned in the current game or FM language. */
	String get_description () const;

	//@}
	//! \name Inheritance and transience
	//@{

	//! The type of an object within the overall object system.
	enum class Type
	{
		NONE,		//!< A nonexistent object or empty reference.
		CONCRETE,	/*!< A specific object that exists in a mission
		                 * at a position in the game world. */
		ARCHETYPE,	/*!< An abstract object from which concrete and
		                 * other abstract objects may inherit. */
		METAPROPERTY	/*!< An abstract bundle of properties which may
		                 * be held by objects separate from their
		                 * archetypes. */
	};

	//! Returns the type of the referenced object.
	Type get_type () const;

	/*! Returns whether the referenced object inherits from the given one.
	 * The referenced object inherits from \a ancestor if \a ancestor is
	 * its direct parent archetype, one of its indirect parent archetypes,
	 * or one of its directly or indirectly held metaproperties. */
	bool inherits_from (const Object& ancestor) const;

	/*! Returns a list of all ancestors of the referenced object.
	 * Ancestors include the direct parent archetype, indirect parent
	 * archetypes (the parent of the parent and so on), and metaproperties
	 * held by the object itself or any direct or indirect parent. The list
	 * is in inheritance order: the highest-priority ancestor for property
	 * inheritance is first in the list, and the lowest-priority is last. */
	List get_ancestors () const;

	/*! Returns a list of all descendants of the referenced object.
	 * The referenced object must be an archetype or metaproperty. If \a
	 * include_indirect is \c true, the list will include descendants of
	 * descendants at unlimited depth; if it is \c false, the list will
	 * only include direct children of an archetype or direct holders of a
	 * metaproperty. */
	List get_descendants (bool include_indirect) const;

	//! Returns the immediate parent archetype of the referenced object.
	Object get_archetype () const;

	/*! Changes the immediate parent archetype of the referenced object.
	 * \warning This is not a typical action for the Dark %Engine, and is
	 * likely to lead to unexpected issues. Use with caution. */
	void set_archetype (const Object& archetype);

	/*! Returns whether the referenced object directly holds the given
	 * metaproperty. To include ancestors holding the metaproperty, use
	 * inherits_from() instead. */
	bool has_metaprop (const Object& metaprop) const;

	/*! Adds the given metaproperty to the referenced object.
	 * If \a single is \c true, the \a metaprop will not be added again
	 * if it is already directly held by the referenced object; if \c false,
	 * the \a metaprop will be added regardless.
	 * \return Whether the metaproperty needed to be added.
	 */
	bool add_metaprop (const Object& metaprop, bool single = true);

	/*! Removes the given metaproperty to the referenced object.
	 * \return Whether the metaproperty needed to be removed.
	 */
	bool remove_metaprop (const Object& metaprop);

	/*! Returns whether the referenced object is transient.
	 * Transient objects are not stored in any mission, gamesys, or save. */
	bool is_transient () const;

	/*! Sets whether the referenced object is transient.
	 * Transient objects are not stored in any mission, gamesys, or save. */
	void set_transient (bool);

	//@}
	//! \name World position
	//@{

	//! Returns the referenced object's current location in the world.
	Vector get_location () const;

	//! Teleports the referenced object to the given location in the world.
	void set_location (const Vector&);

	//! Returns the referenced object's current rotation (facing direction).
	Vector get_rotation () const;

	//! Rotates the referenced object to face the given direction.
	void set_rotation (const Vector&);

	/*! Teleports and rotates the referenced object.
	 * If \a relative is #NONE, the \a location and \a rotation are absolute.
	 * If \a relative is a concrete object, the \a location and \a rotation
	 * are interpreted relative to that object. \a relative may be #SELF to
	 * interpret the coordinates relative to the object's current position. */
	void set_position (const Vector& location, const Vector& rotation,
		const Object& relative = NONE);

	/*! Translates the given object-relative location to world-relative.
	 * The \a relative location is translated based on the object's current
	 * location and rotation, so this is not equivalent to <tt>\a relative +
	 * get_location().</tt> */
	Vector object_to_world (const Vector& relative) const;

	//@{
	//! \name Miscellaneous

	/*! Returns the object which currently contains the referenced object.
	 * If the referenced object is not contained, returns #NONE. Containment
	 * is the existence of a \c Contains link from the container to this
	 * object. */
	Object get_container () const;

	/*! Returns whether the referenced object has renderer refs.
	 * Refs are a system for tracking whether an object needs to be rendered
	 * or not, primarily based on whether it is contained by another object.
	 * See the Renderer&rarr;Has Refs property. */
	bool has_refs () const;

	/*! Destroys this object wrapper.
	 * The referenced object itself is not destroyed. */
	virtual ~Object ();

	//@}

private:
	static Number find (const String& name);
};

/*! Outputs the DromEd-style name of the referenced object to the given stream.
 * This is the output of the object's get_editor_name() method. \relates Object */
std::ostream& operator << (std::ostream&, const Object&);

} // namespace Thief

#include <Thief/Object.inl>

#endif // THIEF_OBJECT_HH

