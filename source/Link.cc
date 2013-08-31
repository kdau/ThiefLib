/******************************************************************************
 *  Link.cc
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

#include "Private.hh"
#include "OSL.hh"

namespace Thief {



// Flavor

const Flavor::Number
Flavor::ANY = 0;

Flavor::Flavor (const String& name)
	: number (SService<ILinkToolsSrv> (LG)->LinkKindNamed (name.data ()))
{}

Flavor::Flavor (const char* name)
	: number (ANY)
{
	if (name)
		number = SService<ILinkToolsSrv> (LG)->LinkKindNamed (name);
}

String
Flavor::get_name () const
{
	LGString name;
	SService<ILinkToolsSrv> (LG)->LinkKindName (name, number);
	return name;
}



// Link

const Link::Number
Link::NONE = 0;

Link
Link::create (Flavor flavor, const Object& source, const Object& dest,
	const void* data)
{
	if (data)
		return SInterface<ILinkManager> (LG)->AddFull
			(source.number, dest.number, flavor.number,
				const_cast<void*> (data));
	else
		return SInterface<ILinkManager> (LG)->Add
			(source.number, dest.number, flavor.number);
}

bool
Link::exists () const
{
	sLink info;
	return SInterface<ILinkManager> (LG)->Get (number, &info);
}

bool
Link::destroy ()
{
	return SInterface<ILinkManager> (LG)->Remove (number) == S_OK;
}

Object
Link::get_source () const
{
	sLink info;
	return SInterface<ILinkManager> (LG)->Get (number, &info)
		? info.source.id : Object::NONE;
}

Object
Link::get_dest () const
{
	sLink info;
	return SInterface<ILinkManager> (LG)->Get (number, &info)
		? info.dest.id : Object::NONE;
}

const void*
Link::get_data_raw () const
{
	return exists () ? SInterface<ILinkManager> (LG)->GetData (number)
		: nullptr;
}

void
Link::set_data_raw (const void* data)
{
	if (!exists ()) throw std::runtime_error ("link does not exist");
	SInterface<ILinkManager> (LG)->SetData
		(number, const_cast<void*> (data));
}

void
Link::_get_data_field (const char* field, LGMultiBase& multi) const
{
	if (exists ())
		SService<ILinkToolsSrv> (LG)->LinkGetData
			(multi, number, field);
	else
		multi.clear ();
}

void
Link::_set_data_field (const char* field, const LGMultiBase& multi)
{
	if (!exists ()) throw std::runtime_error ("link does not exist");
	SService<ILinkToolsSrv> (LG)->LinkSetData
		(number, field, multi);
}



// Link: static methods for multiple links

bool
Link::any_exist (Flavor flavor, const Object& source, const Object& dest)
{
	return SInterface<ILinkManager> (LG)->AnyLinks
		(flavor.number, source.number, dest.number);
}

Link
Link::get_one (Flavor flavor, const Object& source, const Object& dest)
{
	return SInterface<ILinkManager> (LG)->GetSingleLink
		(flavor.number, source.number, dest.number);
}

Link
Link::get_any (Flavor flavor, const Object& source, const Object& dest,
	Inheritance inheritance)
{
	Links links = get_all (flavor, source, dest, inheritance);
	return links.at (Engine::random_int (0, links.size () - 1));
}

Links
Link::get_all (Flavor flavor, const Object& source, const Object& dest,
	Inheritance inheritance)
{
	SInterface<ILinkManager> LM (LG);
	std::vector<ILinkQuery*> queries;
	Links links;

	queries.push_back (LM->Query
		(source.number, dest.number, flavor.number));

	if (source != Object::ANY && inheritance == Inheritance::SOURCE)
		for (auto& ancestor : source.get_ancestors ())
			queries.push_back (LM->Query
				(ancestor.number, dest.number, flavor.number));

	if (dest != Object::ANY && inheritance == Inheritance::DEST)
		for (auto& ancestor : dest.get_ancestors ())
			queries.push_back (LM->Query
				(source.number, ancestor.number, flavor.number));

	for (auto& query : queries)
		if (query)
		{
			for (; !query->Done (); query->Next ())
				if (query->ID () != Link::NONE)
					links.push_back (query->ID ());
			query->Release ();
		}

	return links;
}

void
Link::dump_links (Flavor flavor, const Object& source, const Object& dest,
	Inheritance inheritance)
{
	if (!Engine::is_editor ()) return; // Monolog is a no-op in the game.

	mono << "Dumping";
	if (flavor != Flavor::ANY)
		mono << ' ' << flavor.get_name ();
	mono << " links";
	if (source != Object::ANY)
	{
		mono << " from " << source.get_editor_name ();
		if (inheritance == Inheritance::SOURCE)
			mono << " and its ancestors";
	}
	if (dest != Object::ANY)
	{
		mono << " to " << source.get_editor_name ();
		if (inheritance == Inheritance::DEST)
			mono << " and its ancestors";
	}
	mono << "...\n";

	mono << "Number     Flavor                 Source                 Destination\n";
	mono << "========   ====================   ====================   ====================\n";

	mono << std::left;
	for (auto& link : get_all (flavor, source, dest, inheritance))
		mono << std::setw (8) << link.number << "   "
			<< std::setw (20) << link.get_flavor ().get_name () << "   "
			<< std::setw (20) << link.get_source ().get_editor_name () << "   "
			<< std::setw (20) << link.get_dest ().get_editor_name ()
			<< '\n';

	mono << std::flush << std::internal;
}

bool
Link::subscribe (Flavor flavor, const Object& source, const Object& host)
{
	return SService<IOSLService> (LG)->subscribe_links
		(flavor, source, host);
}

bool
Link::unsubscribe (Flavor flavor, const Object& source, const Object& host)
{
	return SService<IOSLService> (LG)->unsubscribe_links
		(flavor, source, host);
}



// LinkFieldBase

bool
LinkFieldBase::get_bit (const FieldProxyConfig<bool>& config,
	const Link& link) const
{
	LGMulti<unsigned> raw (config.default_value ? config.bitmask : 0u);
	link._get_data_field (config.major, raw);
	return raw & config.bitmask;
}

void
LinkFieldBase::set_bit (const FieldProxyConfig<bool>& config, Link& link,
	bool value)
{
	LGMulti<unsigned> field (0u);
	link._get_data_field (config.major, field);

	if (value)
		field = field | config.bitmask;
	else
		field = field & ~config.bitmask;

	link._set_data_field (config.major, field);
}



// LinkChangeMessage

MESSAGE_WRAPPER_IMPL_ (LinkChangeMessage,
	MESSAGE_TYPENAME_TEST ("LinkChangeMessageImpl"))

LinkChangeMessage::LinkChangeMessage (Event event, Flavor flavor,
		Link::Number link, const Object& source, const Object& dest)
	: Message (new LinkChangeMessageImpl ())
{
	message->message = "LinkChangeMessage";
	MESSAGE_AS (LinkChangeMessageImpl)->event = event;
	MESSAGE_AS (LinkChangeMessageImpl)->flavor = flavor;
	MESSAGE_AS (LinkChangeMessageImpl)->link = link;
	MESSAGE_AS (LinkChangeMessageImpl)->source = source;
	MESSAGE_AS (LinkChangeMessageImpl)->dest = dest;
}

MESSAGE_ACCESSOR (LinkChangeMessage::Event, LinkChangeMessage, get_event,
	LinkChangeMessageImpl, event)

MESSAGE_ACCESSOR (Flavor, LinkChangeMessage, get_flavor,
	LinkChangeMessageImpl, flavor)

MESSAGE_ACCESSOR (Link::Number, LinkChangeMessage, get_link,
	LinkChangeMessageImpl, link)

MESSAGE_ACCESSOR (Object, LinkChangeMessage, get_source,
	LinkChangeMessageImpl, source)

MESSAGE_ACCESSOR (Object, LinkChangeMessage, get_dest,
	LinkChangeMessageImpl, dest)



// CorpseLink

PROXY_CONFIG (CorpseLink, propagate_source_scale, "Propagate Source Scale?",
	nullptr, bool, false);

FLAVORED_LINK_IMPL_ (Corpse,
	PROXY_INIT (propagate_source_scale)
)

CorpseLink
CorpseLink::create (const Object& source, const Object& dest,
	bool propagate_source_scale)
{
	CorpseLink link = Link::create (flavor (), source, dest);
	link.propagate_source_scale = propagate_source_scale;
	return link;
}



// FlinderizeLink

PROXY_CONFIG (FlinderizeLink, count, "Count", nullptr, int, 0);
PROXY_CONFIG (FlinderizeLink, impulse, "Impulse", nullptr, float, 0.0f);
PROXY_CONFIG (FlinderizeLink, scatter, "Scatter?", nullptr, bool, false);
PROXY_CONFIG (FlinderizeLink, offset, "Offset", nullptr, Vector, Vector ());

FLAVORED_LINK_IMPL_ (Flinderize,
	PROXY_INIT (count),
	PROXY_INIT (impulse),
	PROXY_INIT (scatter),
	PROXY_INIT (offset)
)

FlinderizeLink
FlinderizeLink::create (const Object& source, const Object& dest, int count,
	float impulse, bool scatter, const Vector& offset)
{
	FlinderizeLink link = Link::create (flavor (), source, dest);
	link.count = count;
	link.impulse = impulse;
	link.scatter = scatter;
	link.offset = offset;
	return link;
}



// ScriptParamsLink

PROXY_CONFIG (ScriptParamsLink, data, nullptr, nullptr, String, "");

FLAVORED_LINK_IMPL_ (ScriptParams,
	PROXY_INIT (data)
)

ScriptParamsLink
ScriptParamsLink::create (const Object& source, const Object& dest,
	const String& data)
{
	ScriptParamsLink link = Link::create (flavor (), source, dest);
	link.data = data;
	return link;
}



//TODO wrap link: MetaProp - tMetaPropertyPriority

//TODO wrap link: CurWeapon - int (meaningful data??)
//TODO wrap link: GunFlash - sFlashData
//TODO wrap link: WeaponOffset - sWeaponOffset
//TODO wrap link: WHBlock - sHaloBlockData

//TODO wrap link: Projectile - sProjectileData



} // namespace Thief

