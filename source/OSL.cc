/******************************************************************************
 *  OSL.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Dark Hook 2
 *  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
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
#include "OSL.hh"

#include <windef.h>
#include <winbase.h>

namespace Thief {

IScriptMan*
LG = nullptr;

extern "C" const GUID IID_IOSLService = THIEF_IOSLService_GUID;



// OSL::HUDElementInfo

OSL::HUDElementInfo::HUDElementInfo (HUDElementBase& _element,
		HUDElementBase::ZIndex _priority)
	: element (_element),
	  priority (_priority)
{}

bool
OSL::HUDElementInfo::operator == (const HUDElementInfo& rhs) const
{
	return &element == &rhs.element;
}

bool
OSL::HUDElementInfo::operator < (const HUDElementInfo& rhs) const
{
	return priority < rhs.priority ||
		(priority == rhs.priority && &element < &rhs.element);
}



// OSL

OSL*
OSL::self = nullptr;

OSL::OSL ()
	: is_hud_handler (false)
{
	if (self)
		throw std::runtime_error ("Thief::OSL already initialized.");
	self = this;

	static sDispatchListenerDesc sim_listener
		{ &IID_IOSLService, 0xF, on_sim, nullptr };
	SInterface<ISimManager> (LG)->Listen (&sim_listener);
}

OSL::~OSL ()
{
	self = nullptr;
	if (is_hud_handler)
		SService<IDarkOverlaySrv> (LG)->SetHandler (nullptr);
	SInterface<ISimManager> (LG)->Unlisten (&IID_IOSLService);
}

STDMETHODIMP_ (void)
OSL::Init ()
{}

STDMETHODIMP_ (void)
OSL::End ()
{}

STDMETHODIMP_ (ParameterCache*)
OSL::get_param_cache ()
{
	if (!param_cache)
		try { param_cache.reset (new ParameterCacheImpl ()); }
		catch (std::exception& e)
		{
			mono.log (boost::format ("ERROR: Could not create "
				"parameter cache: %||.") % e.what ());
		}
		catch (...) {}
	return param_cache.get ();
}

int __cdecl
OSL::on_sim (const sDispatchMsg* message, const sDispatchListenerDesc*)
{
	if (!self || !message)
		return 0;

	switch (message->dwEventId)
	{

	case kSimStart:
		if (!self->hud_elements.empty ())
			try
			{
				SService<IDarkOverlaySrv> (LG)->SetHandler
					(self);
				self->is_hud_handler = true;
			}
			catch (...) {}
		break;

	case kSimStop:
		try
		{
			if (self->param_cache)
				self->param_cache->reset ();

			self->is_hud_handler = false; // Doesn't survive the sim.
			self->hud_elements.clear ();
			self->hud_bitmaps.clear ();

			self->link_subscriptions.clear ();
			self->property_subscriptions.clear ();
			self->conversation_subscriptions.clear ();

			for (auto& listen : self->listened_properties)
				listen.first.iface->Unlisten (listen.second);
			self->listened_properties.clear ();
		}
		catch (...) {}
		break;

	default:
		break;
	}

	return 0;
}



// OSL: HUD

STDMETHODIMP_ (void)
OSL::DrawHUD ()
{
	for (auto& element : hud_elements)
		element.element.on_event (HUDElementBase::Event::DRAW_STAGE_1);
}

STDMETHODIMP_ (void)
OSL::DrawTOverlay ()
{
	for (auto& element : hud_elements)
		element.element.on_event (HUDElementBase::Event::DRAW_STAGE_2);
}

STDMETHODIMP_ (void)
OSL::OnUIEnterMode ()
{
	for (auto& element : hud_elements)
		element.element.on_event (HUDElementBase::Event::ENTER_GAME_MODE);
}

STDMETHODIMP_ (bool)
OSL::register_hud_element (HUDElementBase& element,
	HUDElementBase::ZIndex priority)
{
	if (!is_hud_handler)
		try
		{
			SService<IDarkOverlaySrv> (LG)->SetHandler (self);
			is_hud_handler = true;
		}
		catch (...) { return false; }
	hud_elements.insert (HUDElementInfo (element, priority));
	return true;
}

STDMETHODIMP_ (bool)
OSL::unregister_hud_element (HUDElementBase& element)
{
	for (auto entry = hud_elements.begin ();
	     entry != hud_elements.end (); ++entry)
		if (&entry->element == &element)
		{
			hud_elements.erase (entry);
			return true;
		}
	return false;
}

STDMETHODIMP_ (HUDBitmap::Ptr)
OSL::load_hud_bitmap (const String& path, bool animation)
{
	HUDBitmap::Ptr bitmap;

	// Look for an existing bitmap first.
	HUDBitmaps::iterator existing = hud_bitmaps.find (path);
	if (existing != hud_bitmaps.end ())
	{
		bitmap = existing->second.lock ();
		if (bitmap)
			return bitmap;
		else
			hud_bitmaps.erase (existing);
	}

	try
	{
		// The bitmap hasn't been loaded yet, so load it now.
		bitmap = HUDBitmap::Ptr (new HUDBitmap (path, animation));
		hud_bitmaps.insert (std::make_pair (path, bitmap));
	}
	catch (std::exception& e)
	{
		mono.log (boost::format ("WARNING: Could not load bitmap at "
			"\"%||\": %||.") % path % e.what ());
	}

	return bitmap;
}




// OSL: LinkCreate, LinkChange, and LinkDestroy messages

OSL::ListenedFlavors
OSL::listened_flavors;

inline bool
OSL::LinkContext::operator < (const LinkContext& rhs) const
{
	return flavor < rhs.flavor ||
		(flavor == rhs.flavor && object < rhs.object);
}

STDMETHODIMP_ (bool)
OSL::subscribe_links (const Flavor& flavor, const Object& source,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? source : _host;

	IRelation* relation =
		SInterface<ILinkManager> (LG)->GetRelation (flavor.number);
	if (!relation || relation->GetID () == 0 || host == Object::NONE)
		return false; //TODO Allow subscription to all flavors.

	if (listened_flavors.find (flavor) == listened_flavors.end ())
	{
		relation->Listen (kRelationFull, on_link_event, nullptr);
		listened_flavors.insert (flavor);
	}

	LinkContext context = { flavor, source };
	link_subscriptions.insert (std::make_pair (context, host));
	return true;
}

STDMETHODIMP_ (bool)
OSL::unsubscribe_links (const Flavor& flavor, const Object& source,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? source : _host;

	auto range = link_subscriptions.equal_range ({ flavor, source });
	for (auto iter = range.first; iter != range.second; ++iter)
		if (iter->second == host)
		{
			link_subscriptions.erase (iter);
			return true;
		}

	return false;
}

void __stdcall
OSL::on_link_event (sRelationListenMsg* _message, void*)
{
	if (!self || !_message || !(_message->event & 0xF))
		return;

	// Translate the event type.
	LinkMessage::Event event;
	switch (_message->event & 0xF)
	{
	case 0:
	case kRelationChange:
		event = LinkMessage::CHANGE; break;
	case kRelationAdd:
	case kRelationAdd | kRelationChange:
		event = LinkMessage::CREATE; break;
	case kRelationDelete:
		event = LinkMessage::DESTROY; break;
	default:
		return;
	}

	LinkMessage message (event, Flavor (_message->flavor), _message->lLink,
		Object (_message->source), Object (_message->dest));
	Object::Set recipients;

	// Include object-specific subscribers.
	auto range = self->link_subscriptions.equal_range
		({ message.flavor, message.source });
	for (auto iter = range.first; iter != range.second; ++iter)
		recipients.insert (iter->second);

	// Include generic subscribers.
	range = self->link_subscriptions.equal_range
		({ message.flavor, Object::ANY });
	for (auto iter = range.first; iter != range.second; ++iter)
		recipients.insert (iter->second);

	// Distribute the message.
	for (auto recipient : recipients)
		message.send (Object::NONE, recipient);
}



// OSL: PropertyChange message

OSL::ListenedProperties
OSL::listened_properties;

STDMETHODIMP_ (bool)
OSL::subscribe_property (const Property& property, const Object& object,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? object : _host;

	if (!property.iface || host == Object::NONE)
		return false; //TODO Allow subscription to all objects.

	if (listened_properties.find (property) == listened_properties.end ())
	{
		auto handle = property.iface->Listen
			(63, on_property_event, nullptr);
		listened_properties.insert (std::make_pair (property, handle));
	}

	ObjectProperty context (property, object);
	property_subscriptions.insert (std::make_pair (context, host));
	return true;
}

STDMETHODIMP_ (bool)
OSL::unsubscribe_property (const Property& property, const Object& object,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? object : _host;

	bool found_sub = false;
	auto range = property_subscriptions.equal_range ({ property, object });
	for (auto iter = range.first; iter != range.second; ++iter)
		if (iter->second == host)
		{
			property_subscriptions.erase (iter);
			found_sub = true;
			break;
		}

	if (!found_sub) return false;

	// Check whether the property is still needed.
	for (auto& subscription : property_subscriptions)
		if (subscription.first.get_property () == property)
			return true;

	// Unlisten from the property since it is no longer needed.
	auto listen_iter = listened_properties.find (property);
	if (listen_iter != listened_properties.end ())
	{
		listen_iter->first.iface->Unlisten (listen_iter->second);
		listened_properties.erase (listen_iter);
	}

	return true;
}

void __stdcall
OSL::on_property_event (sPropertyListenMsg* _message, PropListenerData)
{
	if (!self || !_message) return;

	// Exclude unwanted events with an unknown flag.
	if (_message->event & 0x8) return;

	// Exclude inherited events that are not relevant.
	bool inherited = _message->event & kPropertyInherited;
	if (inherited && !(_message->event & kPropertyInheritedIsRelevant))
		return;

	// Translate the event type.
	PropertyMessage::Event event;
	switch (_message->event & 0x7)
	{
	case 0: // For inherited changes.
	case kPropertyChange:
		event = PropertyMessage::CHANGE; break;
	case kPropertyAdd:
	case kPropertyAdd | kPropertyChange:
		event = PropertyMessage::INSTANTIATE; break;
	case kPropertyDelete:
		event = PropertyMessage::REMOVE; break;
	default:
		return;
	}

	PropertyMessage message (event, inherited,
		Property (_message->iPropId), Object (_message->iObjId));
	Object::Set recipients;

	// Include object-specific subscribers.
	auto range = self->property_subscriptions.equal_range
		({ message.property, message.object });
	for (auto iter = range.first; iter != range.second; ++iter)
		recipients.insert (iter->second);

	// Include generic subscribers.
	range = self->property_subscriptions.equal_range
		({ message.property, Object::ANY });
	for (auto iter = range.first; iter != range.second; ++iter)
		recipients.insert (iter->second);

	// Distribute the message.
	for (auto recipient : recipients)
		message.send (Object::NONE, recipient);
}



// OSL: ConversationEnd message

bool
OSL::listened_conversations = false;

STDMETHODIMP_ (bool)
OSL::subscribe_conversation (const Object& conversation, const Object& _host)
{
	Object host = (_host == Object::SELF) ? conversation : _host;

	if (!listened_conversations)
	{
		SInterface<IAIManager> (LG)->GetConversationManager ()->
			ListenConversationEnd (on_conversation_end);
		listened_conversations = true;
	}

	conversation_subscriptions.insert (std::make_pair (conversation, host));
	return true;
}

STDMETHODIMP_ (bool)
OSL::unsubscribe_conversation (const Object& conversation, const Object& _host)
{
	Object host = (_host == Object::SELF) ? conversation : _host;

	auto range = conversation_subscriptions.equal_range (conversation);
	for (auto iter = range.first; iter != range.second; ++iter)
		if (iter->second == host)
		{
			conversation_subscriptions.erase (iter);
			return true;
		}

	return false;
}

void __cdecl
OSL::on_conversation_end (Object::Number conversation)
{
	if (!self) return;

	ConversationMessage message { Object (conversation) };
	Object::Set recipients;

	// Include object-specific subscribers.
	auto range = self->conversation_subscriptions.equal_range
		(Object (conversation));
	for (auto iter = range.first; iter != range.second; ++iter)
		recipients.insert (iter->second);

	// Include generic subscribers.
	range = self->conversation_subscriptions.equal_range (Object::ANY);
	for (auto iter = range.first; iter != range.second; ++iter)
		recipients.insert (iter->second);

	// Distribute the message.
	for (auto recipient : recipients)
		message.send (Object::NONE, recipient);
}



} // namespace Thief



// Exported functions

extern "C"
bool __declspec(dllexport)
ThiefLibOSLInit (IScriptMan* manager, MPrintfProc mprintf, IMalloc* allocator)
{
	if (Thief::OSL::self)
		return true; // A copy of the OSL service already exists.

	// Attach various ThiefLib components to the engine.
	if (!manager || !allocator) return false;
	Thief::LG = manager;
	Thief::alloc.attach (allocator, OSL_NAME);
	Thief::mono.attach (mprintf);

	// Initialize and expose the OSL service.
	try
	{
		Thief::OSL* osl = new Thief::OSL ();
		manager->ExposeService (osl, Thief::IID_IOSLService);
		osl->Init (); // ExposeService apparently doesn't do this.
	}
	catch (std::exception& e)
	{
		if (mprintf)
			mprintf ("ERROR: Could not set up the ThiefLib support "
				"library: %s.\n", e.what ());
		return false;
	}
	catch (...)
	{
		if (mprintf)
			mprintf ("ERROR: Could not set up the ThiefLib support "
				"library.\n");
		return false;
	}

	return true;
}

extern "C"
BOOL WINAPI
DllMain (HINSTANCE dll, DWORD reason, PVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
		::DisableThreadLibraryCalls (dll);
	(void) reserved;
	return true;
}

