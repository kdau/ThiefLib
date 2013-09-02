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

extern "C" const GUID IID_IHUD = THIEF_IHUD_GUID;
extern "C" const GUID IID_IParameterCache = THIEF_IParameterCache_GUID;
extern "C" const GUID IID_IOSLService = THIEF_IOSLService_GUID;



// HUDImpl::ElementInfo

HUDImpl::ElementInfo::ElementInfo (HUDElement& _element,
		HUD::Callback _callback, HUD::ZIndex _priority)
	: element (_element),
	  callback (_callback),
	  priority (_priority)
{}

bool
HUDImpl::ElementInfo::operator == (const ElementInfo& rhs) const
{
	return &element == &rhs.element;
}

bool
HUDImpl::ElementInfo::operator < (const ElementInfo& rhs) const
{
	return priority < rhs.priority ||
		(priority == rhs.priority && &element < &rhs.element);
}



// HUDImpl

HUDImpl::HUDImpl (bool _sim)
{
	if (_sim) sim ();
}

void
HUDImpl::sim ()
{
	SService<IDarkOverlaySrv> (LG)->SetHandler (this);
}

HUDImpl::~HUDImpl ()
{
	SService<IDarkOverlaySrv> (LG)->SetHandler (nullptr);
}

STDMETHODIMP_ (void)
HUDImpl::DrawHUD ()
{
	for (auto& element : elements)
		element.callback (element.element, HUD::Event::DRAW_STAGE_1);
}

STDMETHODIMP_ (void)
HUDImpl::DrawTOverlay ()
{
	for (auto& element : elements)
		element.callback (element.element, HUD::Event::DRAW_STAGE_2);
}

STDMETHODIMP_ (void)
HUDImpl::OnUIEnterMode ()
{
	for (auto& element : elements)
		element.callback (element.element, HUD::Event::ENTER_GAME_MODE);
}

STDMETHODIMP_ (bool)
HUDImpl::register_element (HUDElement& element, HUD::Callback callback,
	HUD::ZIndex priority)
{
	elements.insert (ElementInfo (element, callback, priority));
	return true;
}

STDMETHODIMP_ (bool)
HUDImpl::unregister_element (HUDElement& element)
{
	for (auto entry = elements.begin (); entry != elements.end (); ++entry)
		if (&entry->element == &element)
		{
			elements.erase (entry);
			return true;
		}
	return false;
}

STDMETHODIMP_ (HUDBitmap::Ptr)
HUDImpl::load_bitmap (const String& path, bool animation)
{
	HUDBitmap::Ptr bitmap;

	// Look for an existing bitmap first.
	Bitmaps::iterator existing = bitmaps.find (path);
	if (existing != bitmaps.end ())
	{
		bitmap = existing->second.lock ();
		if (bitmap)
			return bitmap;
		else
			bitmaps.erase (existing);
	}

	try
	{
		// The bitmap hasn't been loaded yet, so load it now.
		bitmap = HUDBitmap::Ptr (new HUDBitmap (path, animation));
		bitmaps.insert (std::make_pair (path, bitmap));
	}
	catch (std::exception& e)
	{
		mono << "Warning: Could not load bitmap at \"" << path
			<< "\": " << e.what () << "." << std::endl;
	}

	return bitmap;
}



// OSL

bool
OSL::initialized = false;

OSL::ListenedFlavors
OSL::listened_flavors;

OSL::ListenedProperties
OSL::listened_properties;

inline bool
OSL::LinkContext::operator < (const LinkContext& rhs) const
{
	return flavor < rhs.flavor ||
		(flavor == rhs.flavor && object < rhs.object);
}

OSL::OSL ()
	: sim (false), hud (nullptr), param_cache (nullptr)
{
	if (initialized)
		throw std::runtime_error ("Thief::OSL already initialized.");

	static sDispatchListenerDesc sim_listener
		{ &IID_IOSLService, 0xF, on_sim, nullptr };
	sim_listener.pData = this;
	SInterface<ISimManager> (LG)->Listen (&sim_listener);

	initialized = true;
}

OSL::~OSL ()
{
	initialized = false;
	SInterface<ISimManager> (LG)->Unlisten (&IID_IOSLService);
	if (hud) { delete hud; hud = nullptr; }
	if (param_cache) { delete param_cache; param_cache = nullptr; }
}

STDMETHODIMP_ (void)
OSL::Init ()
{}

STDMETHODIMP_ (void)
OSL::End ()
{}

STDMETHODIMP_ (SInterface<IHUD>)
OSL::get_hud ()
{
	if (!hud)
		try { hud = new HUDImpl (sim); }
		catch (...) {}
	return hud;
}

STDMETHODIMP_ (SInterface<IParameterCache>)
OSL::get_param_cache ()
{
	if (!param_cache)
		try { param_cache = new ParameterCache (); }
		catch (...) {}
	return param_cache;
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
		relation->Listen (kRelationFull, on_link_event, this);
		listened_flavors.insert (flavor);
	}

	LinkContext context = { flavor, source };
	if (link_subscriptions.find (context) == link_subscriptions.end ())
		link_subscriptions.insert (std::make_pair (context, host));

	return true;
}

STDMETHODIMP_ (bool)
OSL::unsubscribe_links (const Flavor& flavor, const Object& source,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? source : _host;

	auto iter = link_subscriptions.find ({ flavor, source });
	if (iter == link_subscriptions.end ()) return false;

	link_subscriptions.erase (iter);
	return true;
}

STDMETHODIMP_ (bool)
OSL::subscribe_property (const Property& property, const Object& object,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? object : _host;

	if (!property.iface || host == Object::NONE)
		return false; //TODO Allow subscription to all objects.

	if (listened_properties.find (property) == listened_properties.end ())
	{
		auto handle = property.iface->Listen (kPropertyFull,
			on_property_event,
			reinterpret_cast<PropListenerData> (this));
		listened_properties.insert (std::make_pair (property, handle));
	}

	ObjectProperty context (property, object);
	if (property_subscriptions.find (context) ==
	    property_subscriptions.end ())
		property_subscriptions.insert (std::make_pair (context, host));

	return true;
}

STDMETHODIMP_ (bool)
OSL::unsubscribe_property (const Property& property, const Object& object,
	const Object& _host)
{
	Object host = (_host == Object::SELF) ? object : _host;

	auto iter = property_subscriptions.find ({ property, object });
	if (iter == property_subscriptions.end ()) return false;
	property_subscriptions.erase (iter);

	// Unlisten from the property if no longer needed.
	bool need_property = false;
	for (auto& subscription : property_subscriptions)
		if (subscription.first.get_property () == property)
		{
			need_property = true;
			break;
		}
	if (!need_property)
	{
		auto listen_iter = listened_properties.find (property);
		if (listen_iter != listened_properties.end ())
		{
			listen_iter->first.iface->Unlisten
				(listen_iter->second);
			listened_properties.erase (listen_iter);
		}
	}

	return true;
}

int __cdecl
OSL::on_sim (const sDispatchMsg* message, const sDispatchListenerDesc* desc)
{
	if (!initialized || !message || !desc)
		return 0;

	auto self = reinterpret_cast<OSL*> (desc->pData);
	switch (message->dwEventId)
	{

	case kSimStart:
		self->sim = true;
		try
		{
			if (self->hud)
				self->hud->sim ();
		}
		catch (...) {}
		break;

	case kSimStop:
		self->sim = false;
		try
		{
			if (self->hud)
			{
				delete self->hud;
				self->hud = nullptr;
			}

			if (self->param_cache)
				self->param_cache->reset ();

			self->link_subscriptions.clear ();
			self->property_subscriptions.clear ();

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

void __stdcall
OSL::on_link_event (sRelationListenMsg* _message, void* _self)
{
	if (!initialized || !_message || !_self || !(_message->event & 0xF))
		return;

	auto self = reinterpret_cast<OSL*> (_self);

	// Translate the event type.
	LinkChangeMessage::Event event;
	switch (_message->event & 0xF)
	{
	case 0:
	case kRelationChange:
		event = LinkChangeMessage::CHANGE; break;
	case kRelationAdd:
	case kRelationAdd | kRelationChange:
		event = LinkChangeMessage::ADD; break;
	default:
		event = LinkChangeMessage::REMOVE; break;
	}

	LinkChangeMessage message (event, _message->flavor, _message->lLink,
		_message->source, _message->dest);

	// Send message to object-specific subscribers.
	auto range = self->link_subscriptions.equal_range
		({ _message->flavor, _message->source });
	for (auto iter = range.first; iter != range.second; ++iter)
		message.send (Object::NONE, iter->second);

	// Send message to generic subscribers.
	range = self->link_subscriptions.equal_range
		({ _message->flavor, Object::ANY });
	for (auto iter = range.first; iter != range.second; ++iter)
		message.send (Object::NONE, iter->second);
}

void __stdcall
OSL::on_property_event (sPropertyListenMsg* _message, PropListenerData _self)
{
	if (!initialized || !_message || !_self || (_message->event & 8) != 0 ||
	    (_message->event & 48) == kPropertyInherited)
		return;

	auto self = reinterpret_cast<OSL*> (_self);

	// Translate the event type.
	PropertyChangeMessage::Event event;
	switch (_message->event & 0x7)
	{
	case 0:
	case kPropertyChange:
		event = PropertyChangeMessage::CHANGE; break;
	case kPropertyAdd:
	case kPropertyAdd | kPropertyChange:
		event = PropertyChangeMessage::ADD; break;
	default:
		event = PropertyChangeMessage::REMOVE; break;
	}

	PropertyChangeMessage message (event,
		_message->event & kPropertyInherited,
		_message->iPropId, _message->iObjId);

	// Send message to object-specific subscribers.
	auto range = self->property_subscriptions.equal_range
		({ _message->iPropId, _message->iObjId });
	for (auto iter = range.first; iter != range.second; ++iter)
		message.send (Object::NONE, iter->second);

	// Send message to generic subscribers.
	range = self->property_subscriptions.equal_range
		({ _message->iPropId, Object::ANY });
	for (auto iter = range.first; iter != range.second; ++iter)
		message.send (Object::NONE, iter->second);
}



} // namespace Thief



// Exported functions

extern "C"
bool __declspec(dllexport)
ThiefLibOSLInit (IScriptMan* manager, MPrintfProc mprintf, IMalloc* allocator)
{
	if (Thief::OSL::initialized)
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
		if (mprintf) mprintf
			("Could not set up the ThiefLib support library: %s.\n",
				e.what ());
		return false;
	}
	catch (...)
	{
		if (mprintf) mprintf
			("Could not set up the ThiefLib support library.\n");
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
