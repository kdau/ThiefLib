//! \file Link.hh Relationships between game objects.

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

#ifndef THIEF_LINK_HH
#define THIEF_LINK_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>

namespace Thief {



/*! A type of relationship (Link) between game objects.
 * A Link's flavor identifies what sort of relationship it represents between
 * its source and destination objects. The flavor also determines what data
 * fields are associated with the link. See Link for more information.
 *
 * Every flavor has a reverse, which is the flavor of the reverse of links of
 * that flavor. The number of a reverse flavor is the negative of its
 * corresponding forward flavor, and its name is the forward flavor's name with
 * a tilde (\c ~) prepended. For example, a forward link of the "ScriptParams"
 * (forward) flavor has a corresponding reverse link of the "~ScriptParams"
 * (reverse) flavor. */
struct Flavor
{
	/*! An engine-internal flavor number.
	 * These numbers are not stable between engine versions, and should not
	 * be used directly. Use flavor names instead. */
	typedef long Number;

	/*! Refers to links of any flavor.
	 * This is not a valid value for the flavor of a specific link, but can
	 * be used in any context where links are being searched or filtered. */
	static const Flavor ANY;

	//! The engine-internal number of the flavor.
	Number number;

	/*! Constructs a reference to the flavor with the given number.
	 * These numbers are not stable between engine versions, and should not
	 * be used directly. The number is not checked for validity. Use flavor
	 * names instead. */
	explicit Flavor (Number);

	//! Returns whether this flavor is the same as the given one.
	bool operator == (const Flavor&) const;

	//! Returns whether this flavor is different from as the given one.
	bool operator != (const Flavor&) const;

	/*! Returns whether this flavor should sort before the given one.
	 * Sorting is based on the flavors' engine-internal numbers. */
	bool operator < (const Flavor&) const;

	//! Returns whether this is a reverse flavor.
	bool is_reverse () const;

	/*! Returns the reverse of this flavor.
	 * For normal flavors, the reverse is the reverse flavor.
	 * For reverse flavors, the reverse is the normal flavor.
	 * #ANY is its own reverse. */
	Flavor get_reverse () const;

	/*! Constructs a reference to the named flavor.
	 * \throw std::runtime_error if no flavor exists with the given \a name. */
	Flavor (const String& name);

	/*! Constructs a reference to the named flavor.
	 * If \a name is a null pointer, refers to #ANY.
	 * \throw std::runtime_error if no flavor exists with the given \a name. */
	Flavor (const char* name);

	//! Returns the name of the flavor.
	String get_name () const;
};

//! Outputs the name of the given flavor to the given stream. \relates Flavor
std::ostream& operator << (std::ostream&, const Flavor&);



/*! A relationship between two game objects.
 * Dark %Engine links express a variety of relationships between game objects.
 * Each link has a source Object and a destination Object. The relationship is
 * indicated by the link's Flavor (type).
 *
 * Many flavors have additional data associated with them. These data can be
 * accessed through methods on this class or through fields on flavor-specific
 * derived classes.
 *
 * Every link and link flavor has a reverse. A reverse link has the forward
 * link's destination object as its source object and the forward link's source
 * object as its destination object. The two links are paired such that changing
 * one's data or destroying it affects the other identically. */
class Link
{
public:
	//! A list of references to links.
	typedef std::vector<Link> List;

	//! %Link numbers uniquely identify links within the mission and gamesys.
	typedef long Number;

	/*! The number of the referenced link.
	 * This number may or may not refer to a currently valid link. It may
	 * be #NONE if no link is referenced. */
	Number number;

	/*! Indicates that no link is currently referenced.
	 * This pseudo-reference (not valid as an actual link reference) is
	 * used to indicate that a link wrapper does not currently reference a
	 * link, or that a method that returns a link did not have any link to
	 * return. */
	static const Link NONE;

	//! Constructs a link wrapper not referencing any link.
	Link ();

	/*! Constructs a link wrapper referencing a link with the given number.
	 * The number will not be checked for validity. */
	explicit Link (Number);

	/*! Creates a link of the given flavor between the given objects.
	 * If a \a data pointer is given, it will be copied to the link's data
	 * structure. It must point to memory at least as long as the data
	 * structure for the given flavor. \return The link created. */
	static Link create (Flavor, const Object& source,
		const Object& dest, const void* data = nullptr);

	//! Returns whether the wrapper currently references a valid link.
	bool exists () const;

	//! Removes the referenced link from the database immediately.
	bool destroy ();

	//! Returns whether this wrapper and another reference the same link.
	bool operator == (const Link&) const;

	//! Returns whether this wrapper and another reference different links.
	bool operator != (const Link&) const;

	/*! Returns the reverse of this link.
	 * For a forward link, returns the corresponding reverse link.
	 * For a reverse link, returns the corresponding forward link.
	 * #NONE is its own reverse. */
	Link get_reverse () const;

	//! Returns the flavor of the link.
	Flavor get_flavor () const;

	//! Returns the object that is the source of the link.
	Object get_source () const;

	//! Returns the object that is the destination of the link.
	Object get_dest () const;

	/*! Returns the current value of the named data field on the link.
	 * Flavor-specific Link subclasses provide a superior interface to link
	 * data, and should be used wherever possible.
	 * \param field The name of the data field as it appears in the DromEd
	 * link data window, or an empty string if only one unnamed field
	 * appears. */
	template <typename T>
	T get_data_field (const String& field) const;

	/*! Sets the named data field on the link to the given value.
	 * Flavor-specific Link subclasses provide a superior interface to link
	 * data, and should be used wherever possible.
	 * \param field The name of the data field as it appears in the DromEd
	 * link data window, or an empty string if only one unnamed field
	 * appears.
	 * \param value The value to set. While complete type checking is not
	 * performed, the value must be of an overall compatible type
	 * (FIXME LGMulti). */
	template <typename T>
	void set_data_field (const String& field, const T& value);

	/*! Returns a pointer to the raw data structure for the link.
	 * Not all link flavors have data, so this pointer may be null even
	 * for a valid link. The raw data should not be modified; copy it and
	 * call set_data_raw() with the altered copy instead. Flavor-specific
	 * Link subclasses provide a superior interface to link data, and should
	 * be used wherever possible. get_data_field() is also a safer option. */
	const void* get_data_raw () const;

	/*! Copies the given data to the raw data structure for the link.
	 * Not all link flavors have data, so this method may do nothing even
	 * for a valid link. The current raw data can be retrieved with
	 * get_data_raw(). Flavor-specific Link subclasses provide a superior
	 * interface to link data, and should be used wherever possible.
	 * set_data_field() is also a safer option. */
	void set_data_raw (const void*);

	/*! Returns whether any links match the given criteria.
	 * \param flavor The flavor of the links to consider, or Flavor::ANY to
	 * consider links of any flavor. \param source The source object of the
	 * links to consider, or Object::ANY to consider links from any object.
	 * \param dest The destination object of the links to consider, or
	 * Object::ANY to consider links to any object. */
	static bool any_exist (Flavor flavor, const Object& source = Object::ANY,
		const Object& dest = Object::ANY);

	/*! Returns the only link matching the given criteria.
	 * This method is used with criteria that are singleton: only one link
	 * matching the criteria should exist at any time. No matching link may
	 * exist, in which case Link::NONE is returned.
	 * \throw std::runtime_error if more than one link matches the criteria.
	 * \param flavor The flavor of the link. Flavor::ANY is usually not
	 * valid here, since the singleton guarantee is flavor-specific. \param
	 * source The source object of the link, or Object::ANY to consider
	 * links from any object. \param dest The destination object of the link,
	 * or Object::ANY to consider links to any object. */
	static Link get_one (Flavor flavor, const Object& source = Object::ANY,
		const Object& dest = Object::ANY);

	//! Whether to include links to/from ancestors in a search.
	enum class Inheritance
	{
		NONE,       //!< Do not include links to/from any ancestors.
		SOURCE,     /*!< Include links from ancestors of the source
		             * object. */
		DESTINATION /*!< Include links to ancestors of the destination
		             * object. */
	};

	/*! Returns a randomly chosen link matching the given criteria.
	 * \param flavor The flavor of the link, or Flavor::ANY to consider
	 * links of any flavor. \param source The source object of the link, or
	 * Object::ANY to consider links from any object. \param dest The
	 * destination object of the link, or Object::ANY to consider links to
	 * any object. \param inheritance Whether to consider links from
	 * ancestors of the given source and/or destination. */
	static Link get_any (Flavor flavor = Flavor::ANY,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY,
		Inheritance inheritance = Inheritance::NONE);

	/*! Returns a list of links matching the given criteria.
	 * \param flavor The flavor of the links, or Flavor::ANY to include
	 * links of any flavor. \param source The source object of the links, or
	 * Object::ANY to include links from any object. \param dest The
	 * destination object of the links, or Object::ANY to include links to
	 * any object. \param inheritance Whether to include links from
	 * ancestors of the given source and/or destination. */
	static List get_all (Flavor flavor = Flavor::ANY,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY,
		Inheritance inheritance = Inheritance::NONE);

	/*! Outputs a table of links matching the given criteria to the monolog.
	 * The table includes the links' flavors, sources, and destinations, but
	 * not their data.\param flavor The flavor of the links, or Flavor::ANY
	 * to include links of any flavor. \param source The source object of
	 * the links, or Object::ANY to include links from any object. \param
	 * dest The destination object of the links, or Object::ANY to include
	 * links to any object. \param inheritance Whether to include links from
	 * ancestors of the given source and/or destination. */
	static void dump_links (Flavor flavor = Flavor::ANY,
		const Object& source = Object::ANY,
		const Object& dest = Object::ANY,
		Inheritance inheritance = Inheritance::NONE);

	/*! Subscribes to changes on any links of the given flavor from the
	 * given object. "LinkCreate", "LinkChange", and "LinkDestroy" messages
	 * will be sent to all scripts on the given \a host object when such
	 * changes occur. \param flavor The flavor of links to monitor.
	 * Flavor::ANY is \b not valid here. \param source The source object to
	 * monitor links for, or Object::ANY to monitor all links of the given
	 * flavor. \param host The object to notify of changes. If Object::SELF,
	 * the source object of the links will be notified. (Object::SELF is not
	 * valid if \a source is Object::ANY). */
	static bool subscribe (Flavor flavor, const Object& source,
		const Object& host = Object::SELF);

	/*! Unsubscribes from changes on links of the given flavor from the
	 * given object. The \a flavor and \a source object must match the
	 * original subscribe() call. Only messages to scripts on the given
	 * \a host object will stop. */
	static bool unsubscribe (Flavor flavor, const Object& source,
		const Object& host = Object::SELF);

private:
	THIEF_FIELD_PROXY_TEMPLATE friend class LinkField;
	void _get_data_field (const char* field, LGMultiBase& multi) const;
	void _set_data_field (const char* field, const LGMultiBase& multi);
};



/*! A field of a Link's data structure.
 * This class serves as a proxy to a member of the link data structure that
 * underlies a field on a link flavor class. Like all field proxies, it is not
 * intended for construction by ThiefLib users. */
THIEF_FIELD_PROXY_TEMPLATE
class LinkField
{
public:
	//! Constructs a new link field proxy for the given link and index.
	LinkField (Link&, size_t index);

	//! Returns the current value of the field.
	operator Type () const;

	//! Sets the field to the given value.
	LinkField& operator = (const Type&);

	//! Returns whether the field currently has the given value.
	bool operator == (const Type&) const;

	//! Returns whether the field does not currently have the given value.
	bool operator != (const Type&) const;

private:
	Link& link;
	size_t index;
};

/*! Outputs the current value of the given link field to the given stream.
 * \relates LinkField */
THIEF_FIELD_PROXY_TEMPLATE
std::ostream& operator << (std::ostream&,
	const THIEF_FIELD_PROXY_CLASS (LinkField)&);



} // namespace Thief

#include <Thief/Link.inl>

namespace Thief {



/*! A link that replaces one object with another when it is slain.
 * When an object with a Corpse link or links is slain, it is destroyed and an
 * instance of each Corpse-linked archetype is created at its former position. */
class CorpseLink : public Link
{
public:
	THIEF_LINK_FLAVOR (Corpse)

	/*! Creates a Corpse link between the given objects.
	 * The data field will be set as given. \return The link created. */
	static CorpseLink create (const Object& source,
		const Object& dest, bool propagate_source_scale);

	/*! Whether to copy the Reactor::source_scale of the slain object to its
	 * corpse. */
	THIEF_LINK_FIELD (bool, propagate_source_scale);
};



/*! A link that creates debris when an object is slain.
 * Flinderization is separate from and additional to the source object's
 * corpsing process, controlled by CorpseLink(s). For each Flinderize link
 * from an object, #count instances of the destination archetype are created
 * at the source object's position when it is slain. The source object is not
 * necessarily destroyed. */
class FlinderizeLink : public Link
{
public:
	THIEF_LINK_FLAVOR (Flinderize)

	/*! Creates a Flinderize link between the given objects.
	 * The data fields will be set as given. \return The link created. */
	static FlinderizeLink create (const Object& source, const Object& dest,
		int count, float impulse = 0.0f, bool scatter = false,
		const Vector& offset = Vector ());

	//! How many flinders (instances of the destination object) to create.
	THIEF_LINK_FIELD (int, count);

	//! The initial velocity of the created flinders.
	THIEF_LINK_FIELD (float, impulse);

	//! Whether to randomly offset flinders from the source object.
	THIEF_LINK_FIELD (bool, scatter);

	/*! The offset from the source object where flinders will be created.
	 * This field is ignored if #scatter is \c true. */
	THIEF_LINK_FIELD (Vector, offset);
};



/*! A link that provides information to scripts.
 * \c ScriptParams links are used by custom scripts for a wide variety of
 * purposes. Their data are strings that are typically a particular name
 * recognized by a script; for example, the stock \c StdDoor script recognizes
 * a \c ScriptParams link with the data \c Double to provide correct behavior
 * for double doors. */
class ScriptParamsLink : public Link
{
public:
	THIEF_LINK_FLAVOR (ScriptParams)

	/*! Returns a list of ScriptParams links matching the given criteria,
	 * including the data string. \param source The source object of the
	 * links, or Object::ANY to include links from any object. \param data
	 * The data string of the links, compared case-insensitive. \param
	 * inheritance Whether to include links from ancestors of the given
	 * source object. \param reverse Whether to search for ~ScriptParams
	 * links instead of ScriptParams links. */
	static List get_all_by_data (const Object& source, const CIString& data,
		Inheritance inheritance = Inheritance::NONE,
		bool reverse = false);

	/*! Returns the only link matching the given criteria, including the.
	 * data string. This method is used with ScriptParams links that are
	 * singleton: only one ScriptParams link with the given data should
	 * exist on an object at any time. No matching link may exist, in which
	 * case Link::NONE is returned.
	 * \throw std::runtime_error if more than one link matches the criteria.
	 * \param source The source object of the link, or Object::ANY to
	 * consider links from any object. \param data The data string of the
	 * link, compared case-insensitive. \param reverse Whether to search for
	 * a ~ScriptParams links instead of a ScriptParams link. */
	static ScriptParamsLink get_one_by_data (const Object& source,
		const CIString& data, bool reverse = false);

	/*! Creates a ScriptParams link between the given objects.
	 * The data string will be set to \a data. \return The link created. */
	static ScriptParamsLink create (const Object& source,
		const Object& dest, const CIString& data = CIString ());

	//! The freeform string data associated with the link.
	THIEF_LINK_FIELD (String, data);
};



} // namespace Thief

#endif // THIEF_LINK_HH

