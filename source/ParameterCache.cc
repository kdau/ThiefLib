/******************************************************************************
 *  ParameterCache.cc
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

#include "Private.hh"
#include "ParameterCache.hh"

namespace Thief {



// DesignNoteReader

inline bool
DesignNoteReader::handle_character (const char& ch)
{
	const char* next = &ch + 1;

	if (ch == '\0') // End of the DN.
	{
		handle_parameter ();
		return false;
	}

	if (!escaped && !quoted && ch == ';') // End of a parameter.
	{
		handle_parameter ();
		state = State::NAME;
		started = escaped = false;
		quoted = 0;
		spaces = 0u;
		name_begin = next;
		name_end = index_begin = index_end = nullptr;
		raw_value.clear ();
		return true;
	}

	if (!started && std::isspace (ch)) // Ignore leading spaces.
		switch (state)
		{
		case State::NAME: ++name_begin; return true;
		case State::INDEX: ++index_begin; return true;
		case State::VALUE: return true;
		}

	bool just_started = !started;
	started = true;

	switch (state)
	{
	case State::NAME:
		switch (ch)
		{
		case '[':
			if (!index_begin) // Begin the index.
			{
				state = State::INDEX;
				started = false;
				if (!name_end) name_end = &ch;
				index_begin = next;
			}
			else // Oops, bracket after index finished.
				return false;
			break;
		case '=': // Begin the value.
			state = State::VALUE;
			started = false;
			if (!name_end) name_end = &ch;

			// Remove trailing spaces from name.
			while (name_end > name_begin &&
					std::isspace (*(name_end - 1)))
				--name_end;
			break;
		default:
			if (name_end && !std::isspace (ch))
				return false; // Oops, extra characters.
			break;
		}
		break;

	case State::INDEX:
		if (ch == ']') // End the index and return to the name for '='.
		{
			state = State::NAME;
			started = true;
			index_end = &ch;
		}
		break;

	case State::VALUE:
		if (escaped) // Read in the escaped character literally.
		{
			raw_value.push_back (ch);
			escaped = false;
		}

		else if (ch == '\\' &&
			    (*next == '\\' || *next == '"' || *next == '\''))
			escaped = true; // Allow specific escape sequences only.

		else if (ch == quoted)
			quoted = 0; // Close the quotation marks.

		else if (just_started && (ch == '\'' || ch == '"'))
			quoted = ch; // Open the quotation marks.

		else
		{
			if (!quoted && std::isspace (ch))
				++spaces;
			else
				spaces = 0;

			raw_value.push_back (ch);
		}

		break;
	}

	return true;
}

void
DesignNoteReader::handle_parameter ()
{
	if (state != State::VALUE)
		return; // One or more pieces were missing.

	// Check that the difficulty index matches.
	if (index_begin && index_end > index_begin)
	{
		String index (index_begin, index_end - index_begin);
		try
		{
			Difficulty allowed = Difficulty
				(EnumCoding::get<Difficulty> ().decode (index));
			if (!Mission::check_difficulty (allowed))
				return; // Ignore a non-matching parameter.
		}
		catch (...)
		{
			return; // Ignore a parameter with an invalid index.
		}
	}

	// Remove trailing spaces from value.
	if (spaces)
		raw_value.erase (raw_value.size () - spaces, spaces);

	CIString name (name_begin, name_end - name_begin);
	raw_values [name] = raw_value;
}

DesignNoteReader::DesignNoteReader (const char* dn, RawValues& _raw_values)
	: raw_values (_raw_values), state (State::NAME),
	  started (false), escaped (false), quoted (0), spaces (0u),
	  name_begin (dn), name_end (nullptr),
	  index_begin (nullptr), index_end (nullptr)
{
	if (dn) while (handle_character (*dn++));
}



// ParameterCacheImpl

ParameterCacheImpl::ParameterCacheImpl ()
	: dn_prop (static_cast<IStringProperty*> (SInterface<IPropertyManager>
		(LG)->GetPropertyNamed ("DesignNote"))),
	  listen_handle (nullptr)
{
	if (!dn_prop)
		throw MissingResource (MissingResource::PROPERTY, "DesignNote",
			Object::NONE);
	listen_handle = dn_prop->Listen (kPropertyFull, on_dn_change,
		reinterpret_cast<PropListenerData> (this));
	SInterface<ITraitManager> (LG)->Listen (on_trait_change, this);
}

ParameterCacheImpl::~ParameterCacheImpl ()
{
	dn_prop->Unlisten (listen_handle);
	// It is not possible to unlisten from ITraitManager, so this dtor
	// should not be reached before application exit.
}

bool
ParameterCacheImpl::exists (const Object& object, const CIString& parameter,
	bool inherit)
{
	DesignNote* dn = update_object (object);
	if (!dn) return false;

	if (dn->state & DesignNote::RELEVANT)
	{
		auto raw_value = dn->raw_values.find (parameter);
		if (raw_value != dn->raw_values.end ())
			return true;
	}

	if (inherit || !(dn->state & DesignNote::RELEVANT))
		for (auto ancestor : dn->ancestors)
		{
			DesignNote& anc_dn = data [ancestor];
			auto anc_raw_value = anc_dn.raw_values.find (parameter);
			if (anc_raw_value != anc_dn.raw_values.end ())
				return true;
			else if (!inherit &&
					(anc_dn.state & DesignNote::RELEVANT))
				break;
		}

	return false;
}

const String*
ParameterCacheImpl::get (const Object& object, const CIString& parameter,
	bool inherit)
{
	DesignNote* dn = update_object (object);
	if (!dn) return nullptr;

	if (dn->state & DesignNote::RELEVANT)
	{
		auto raw_value = dn->raw_values.find (parameter);
		if (raw_value != dn->raw_values.end ())
			return &raw_value->second;
	}

	if (inherit || !(dn->state & DesignNote::RELEVANT))
		for (auto ancestor : dn->ancestors)
		{
			DesignNote& anc_dn = data [ancestor];
			auto anc_raw_value = anc_dn.raw_values.find (parameter);
			if (anc_raw_value != anc_dn.raw_values.end ())
				return &anc_raw_value->second;
			else if (!inherit &&
					(anc_dn.state & DesignNote::RELEVANT))
				break;
		}

	return nullptr;
}

bool
ParameterCacheImpl::set (const Object& object, const CIString& parameter,
	const String& value)
{
	DesignNote* dn = update_object (object);
	if (!dn || !(dn->state & DesignNote::EXISTENT)) return false;
	dn->state |= DesignNote::RELEVANT;
	dn->raw_values [parameter] = value;
	return write_dn (object);
}

bool
ParameterCacheImpl::copy (const Object& _source, const Object& _dest,
	const CIString& parameter)
{
	Parameter<String> keep_dest_watched (_dest, parameter, { "" });
	DesignNote* source = update_object (_source);
	DesignNote* dest = update_object (_dest);

	if (!source || (source->state & DesignNote::RELEVANT) ||
	    !dest || !(dest->state & DesignNote::EXISTENT))
		return false;

	auto iter = source->raw_values.find (parameter);
	if (iter == source->raw_values.end ())
		return false;

	dest->state |= DesignNote::RELEVANT;
	dest->raw_values [parameter] = iter->second;
	return write_dn (_dest);
}

bool
ParameterCacheImpl::remove (const Object& object, const CIString& parameter)
{
	DesignNote* dn = update_object (object);
	if (!dn || !(dn->state & DesignNote::RELEVANT)) return false;
	if (dn->raw_values.erase (parameter) == 0) return false;
	return write_dn (object);
}

void
ParameterCacheImpl::watch_object (const Object& object,
	const ParameterBase& watcher)
{
	data [object].direct_watchers.insert (&watcher);
	update_object (object);
	watcher.reparse ();
}

void
ParameterCacheImpl::unwatch_object (const Object& object,
	const ParameterBase& watcher)
{
	auto dn_iter = data.find (object);
	if (dn_iter != data.end ())
	{
		DesignNote& dn = dn_iter->second;
		dn.direct_watchers.erase (&watcher);
		if (dn.direct_watchers.empty ())
		{
			for (auto ancestor : dn.ancestors)
				unwatch_ancestor (ancestor);
			if (dn.indirect_watchers == 0)
				data.erase (dn_iter);
		}
	}
}

void
ParameterCacheImpl::dump (Monolog& log)
{
	log << "Dumping parameter cache (C = cached; E = object exists; R = DesignNote on object)...\n";
	for (auto& datum : data)
	{
		String name = datum.first.exists ()
			? datum.first.get_name () : "NONEXISTENT";
		if (name.empty ())
		{
			name = "[" + datum.first.get_archetype ().get_name ()
				+ "]";
		}
		log << boost::format ("  %|6| %|-24| [state: %||%||%||; "
			"watchers: %|| direct, %|| indirect]\n")
			% datum.first.number % name
			% ((datum.second.state & DesignNote::CACHED) ? "C" : "-")
			% ((datum.second.state & DesignNote::EXISTENT) ? "E" : "-")
			% ((datum.second.state & DesignNote::RELEVANT) ? "R" : "-")
			% datum.second.direct_watchers.size ()
			% datum.second.indirect_watchers;
		for (auto& raw_value : datum.second.raw_values)
			log << boost::format ("           %|-22| %||\n")
				% raw_value.first % raw_value.second;
	}
	log << std::flush;
}

void
ParameterCacheImpl::reset ()
{
	data.clear ();
}

STDMETHODIMP_ (void)
ParameterCacheImpl::on_dn_change (sPropertyListenMsg* message,
	PropListenerData _self)
{
	// Filter out an unidentified event type known to be irrelevant.
	if (!message || !_self || message->event & 8) return;

	Object object = Object (message->iObjId);
	auto self = reinterpret_cast<ParameterCacheImpl*> (_self);

	auto dn_iter = self->data.find (object);
	if (dn_iter == self->data.end ()) return;
	DesignNote& dn = dn_iter->second;

	if (object != self->current)
	{
		dn.state &= ~DesignNote::CACHED;
		self->update_object (object);
	}

	// Notify any directly watching parameters.
	for (auto& watcher : dn.direct_watchers)
		watcher->reparse ();
}

STDMETHODIMP_ (void)
ParameterCacheImpl::on_trait_change (const sHierarchyMsg* message, void* _self)
{
	if (!message || !_self) return;
	auto self = reinterpret_cast<ParameterCacheImpl*> (_self);

	auto iter = self->data.find (Object (message->iSubjId));
	if (iter != self->data.end ())
		self->update_ancestors (iter->first, iter->second);
}

DesignNote*
ParameterCacheImpl::update_object (const Object& object)
{
	auto dn_iter = data.find (object);
	if (dn_iter == data.end ()) return nullptr;

	DesignNote& dn = dn_iter->second;
	if (dn.state & DesignNote::CACHED) return &dn;

	// Reset the data.
	dn.state = DesignNote::CACHED;
	dn.raw_values.clear ();

	// Check whether the object currently exists.
	if (object.exists ())
	{
		dn.state |= DesignNote::EXISTENT;

		// Read the parameters, if a DN is present.
		if (dn_prop && dn_prop->IsSimplyRelevant (object.number))
		{
			dn.state |= DesignNote::RELEVANT;
			read_dn (object);
		}
	}

	update_ancestors (object, dn);
	return &dn;
}

void
ParameterCacheImpl::update_ancestors (const Object& object, DesignNote& dn)
{
	// Keep old ancestors for unwatching afterward.
	DesignNote::Ancestors old_ancestors (std::move (dn.ancestors));
	dn.ancestors.clear ();

	// Identify the object's ancestors, if it exists and is directly watched.
	if (object.exists () && !dn.direct_watchers.empty ())
		for (auto ancestor : object.get_ancestors ())
		{
			dn.ancestors.push_back (ancestor);
			++data [ancestor].indirect_watchers;
			update_object (ancestor);
		}

	// Unwatch the old ancestors.
	for (auto& old_ancestor : old_ancestors)
		unwatch_ancestor (old_ancestor);
}

void
ParameterCacheImpl::unwatch_ancestor (const Object& object)
{
	if (--data [object].indirect_watchers == 0 &&
	    data [object].direct_watchers.empty ())
		data.erase (object);
}

void
ParameterCacheImpl::read_dn (const Object& object)
{
	const char* dn = nullptr;
	dn_prop->GetSimple (object.number, &dn);
	if (dn) DesignNoteReader (dn, data [object].raw_values);
}

bool
ParameterCacheImpl::write_dn (const Object& object)
{
	current = object;
	try
	{
		String dn;
		dn.reserve (20 * data [object].raw_values.size ());

		for (auto& raw_value : data [object].raw_values)
		{
			dn.append (raw_value.first.data ());
			// No index needs to be written; the difficulty will not
			// change mid-sim, so only one value remains valid.
			dn.append ("=\"");
			for (auto& ch : raw_value.second)
				switch (ch)
				{
				case '"': dn.append ("\\\""); break;
				case '\\': dn.append ("\\\\"); break;
				default: dn.push_back (ch); break;
				}
			dn.append ("\";");
		}

		dn_prop->Set (object.number, dn.data ());
	}
	catch (...)
	{
		current = Object::NONE;
		return false;
	}
	current = Object::NONE;
	return true;
}

} // namespace Thief

