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
Link::_get_data_field (const String& field, LGMultiBase& multi) const
{
	if (exists ())
		SService<ILinkToolsSrv> (LG)->LinkGetData
			(multi, number, field.data ());
	else
		multi.clear ();
}

void
Link::_set_data_field (const String& field, const LGMultiBase& multi)
{
	if (!exists ()) throw std::runtime_error ("link does not exist");
	SService<ILinkToolsSrv> (LG)->LinkSetData
		(number, field.data (), multi);
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

void
Link::subscribe (const Object& source, Flavor flavor, const Object& _host)
{
	IRelation* relation = SInterface<ILinkManager> (LG)->GetRelation
		(flavor.number);
	if (!relation) throw std::runtime_error ("can't subscribe to flavor");
	Object host = (_host == Object::SELF) ? source : _host;
	SService<IDarkHookScriptService> (LG)->InstallRelHook (host.number,
		kDHNotifyDefault, relation->Describe ()->szName, source.number);
}

void
Link::unsubscribe (const Object& source, Flavor flavor, const Object& _host)
{
	IRelation* relation = SInterface<ILinkManager> (LG)->GetRelation
		(flavor.number);
	if (!relation) throw std::runtime_error ("can't subscribe to flavor");
	Object host = (_host == Object::SELF) ? source : _host;
	SService<IDarkHookScriptService> (LG)->UninstallRelHook
		(host.number, relation->Describe ()->szName, source.number);
}




// LinkChangeMessage

MESSAGE_WRAPPER_IMPL_ (LinkChangeMessage,
	MESSAGE_TYPENAME_TEST ("sDHNotifyMsg") &&
	MESSAGE_AS (sDHNotifyMsg)->typeDH == kDH_Relation)

LinkChangeMessage::LinkChangeMessage (Event event, Flavor flavor,
		Link::Number number, const Object& source, const Object& dest)
	: Message (new sDHNotifyMsg ())
{
	IRelation* relation = SInterface<ILinkManager> (LG)->GetRelation
		(flavor.number);

	message->message = "DHNotify"; // local name is generated by Script
	MESSAGE_AS (sDHNotifyMsg)->typeDH = kDH_Relation;
	MESSAGE_AS (sDHNotifyMsg)->sRel.event = eLinkEvent (event);
	MESSAGE_AS (sDHNotifyMsg)->sRel.pRel = relation;
	MESSAGE_AS (sDHNotifyMsg)->sRel.pszRelName =
		relation ? relation->Describe ()->szName : "";
	MESSAGE_AS (sDHNotifyMsg)->sRel.lLinkId = number;
	MESSAGE_AS (sDHNotifyMsg)->sRel.iLinkSource = source.number;
	MESSAGE_AS (sDHNotifyMsg)->sRel.iLinkDest = dest.number;
}

MESSAGE_ACCESSOR (LinkChangeMessage::Event, LinkChangeMessage, get_event,
	sDHNotifyMsg, sRel.event)

Flavor
LinkChangeMessage::get_flavor () const
{
	auto relation = MESSAGE_AS (sDHNotifyMsg)->sRel.pRel;
	if (relation)
		return relation->GetID ();
	else
		return MESSAGE_AS (sDHNotifyMsg)->sRel.pszRelName;
}

MESSAGE_ACCESSOR (Link::Number, LinkChangeMessage, get_number,
	sDHNotifyMsg, sRel.lLinkId)

MESSAGE_ACCESSOR (Object, LinkChangeMessage, get_source,
	sDHNotifyMsg, sRel.iLinkSource)

MESSAGE_ACCESSOR (Object, LinkChangeMessage, get_dest,
	sDHNotifyMsg, sRel.iLinkDest)



// CorpseLink

FLAVORED_LINK_IMPL (Corpse)

CorpseLink
CorpseLink::create (const Object& source, const Object& dest,
	bool propagate_source_scale)
{
	CorpseLink link = Link::create (flavor (), source, dest);
	link.set_propagate_source_scale (propagate_source_scale);
	return link;
}

bool
CorpseLink::get_propagate_source_scale () const
{
	return get_data_field<bool> ("Propagate Source Scale?");
}

void
CorpseLink::set_propagate_source_scale (bool propagate_source_scale)
{
	set_data_field ("Propagate Source Scale?", propagate_source_scale);
}



// FlinderizeLink

FLAVORED_LINK_IMPL (Flinderize)

FlinderizeLink
FlinderizeLink::create (const Object& source, const Object& dest, int count,
	float impulse, bool scatter, const Vector& offset)
{
	FlinderizeLink link = Link::create (flavor (), source, dest);
	link.set_count (count);
	link.set_impulse (impulse);
	link.set_scatter (scatter);
	link.set_offset (offset);
	return link;
}

int
FlinderizeLink::get_count () const
{
	return get_data_field<int> ("Count");
}

void
FlinderizeLink::set_count (int count)
{
	set_data_field ("Count", count);
}

float
FlinderizeLink::get_impulse () const
{
	return get_data_field<float> ("Impulse");
}

void
FlinderizeLink::set_impulse (float impulse)
{
	set_data_field ("Impulse", impulse);
}

bool
FlinderizeLink::get_scatter () const
{
	return get_data_field<bool> ("Scatter?");
}

void
FlinderizeLink::set_scatter (bool scatter)
{
	set_data_field ("Scatter?", scatter);
}

Vector
FlinderizeLink::get_offset () const
{
	return get_data_field<Vector> ("Offset");
}

void
FlinderizeLink::set_offset (const Vector& offset)
{
	set_data_field ("Offset", offset);
}



// ScriptParamsLink

FLAVORED_LINK_IMPL (ScriptParams)

ScriptParamsLink
ScriptParamsLink::create (const Object& source, const Object& dest,
	const String& data)
{
	ScriptParamsLink link = Link::create (flavor (), source, dest);
	link.set_data (data);
	return link;
}

String
ScriptParamsLink::get_data () const
{
	auto data = static_cast<const char*> (get_data_raw ());
	return data ? data : String ();
}

void
ScriptParamsLink::set_data (const String& data)
{
	set_data_raw (data.data ());
}



//TODO wrap link: MetaProp - tMetaPropertyPriority

//TODO wrap link: CurWeapon - int (meaningful data??)
//TODO wrap link: GunFlash - sFlashData
//TODO wrap link: WeaponOffset - sWeaponOffset
//TODO wrap link: WHBlock - sHaloBlockData

//TODO wrap link: Projectile - sProjectileData



} // namespace Thief

