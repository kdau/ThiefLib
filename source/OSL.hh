/******************************************************************************
 *  OSL.hh
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

#ifndef OSL_HH
#define OSL_HH

#include "Private.hh"
#include "ParameterCache.hh"



#ifdef IS_OSL
extern "C" bool ThiefLibOSLInit (IScriptMan*, MPrintfProc, IMalloc*);
#else
#define OSL_INIT_PROC "_ThiefLibOSLInit"
#endif

namespace Thief {

typedef bool (__cdecl *OSLInitProc) (IScriptMan*, MPrintfProc, IMalloc*);



// IOSLService

interface IOSLService : IScriptServiceBase
{
	STDMETHOD_ (ParameterCache*, get_param_cache) () PURE;

	STDMETHOD_ (bool, register_hud_element) (HUDElementBase&,
		HUDElementBase::ZIndex priority) PURE;
	STDMETHOD_ (bool, unregister_hud_element) (HUDElementBase&) PURE;
	STDMETHOD_ (HUDBitmap::Ptr, load_hud_bitmap) (const String& path,
		bool animation) PURE;

	STDMETHOD_ (bool, subscribe_links) (const Flavor&,
		const Object& source, const Object& host) PURE;
	STDMETHOD_ (bool, unsubscribe_links) (const Flavor&,
		const Object& source, const Object& host) PURE;

	STDMETHOD_ (bool, subscribe_property) (const Property&,
		const Object& object, const Object& host) PURE;
	STDMETHOD_ (bool, unsubscribe_property) (const Property&,
		const Object& object, const Object& host) PURE;

	STDMETHOD_ (bool, subscribe_conversation) (const Object& conversation,
		const Object& host) PURE;
	STDMETHOD_ (bool, unsubscribe_conversation) (const Object& conversation,
		const Object& host) PURE;
};

extern "C" const GUID IID_IOSLService;
#define THIEF_IOSLService_GUID { 0x709a2033, 0x3d7e, 0x4424, \
		{ 0x97, 0x1d, 0xed, 0xf5, 0x55, 0x45, 0x2b, 0x01 } }



// LinkMessageImpl

struct LinkMessageImpl : public sScrMsg
{
	typedef LinkMessage::Event Event;

	Event event;
	Flavor flavor;
	Link::Number link;
	Object source, dest;

	LinkMessageImpl ()
		: sScrMsg (),
		  event (Event::CHANGE),
		  flavor (Flavor::ANY), link (Link::NONE.number),
		  source (), dest ()
	{}

	virtual ~LinkMessageImpl ()
	{}

	virtual const char* __thiscall GetName () const
		{ return "LinkMessageImpl"; }
};



// PropertyMessageImpl

struct PropertyMessageImpl : public sScrMsg
{
	typedef PropertyMessage::Event Event;

	Event event;
	bool inherited;
	Property property;
	Object object;

	PropertyMessageImpl ()
		: sScrMsg (),
		  event (Event::CHANGE), inherited (false),
		  property (Property::NONE), object ()
	{}

	virtual ~PropertyMessageImpl ()
	{}

	virtual const char* __thiscall GetName () const
		{ return "PropertyMessageImpl"; }
};



// ConversationMessageImpl

struct ConversationMessageImpl : public sScrMsg
{
	Object conversation;

	ConversationMessageImpl ()
		: sScrMsg (),
		  conversation ()
	{}

	virtual ~ConversationMessageImpl ()
	{}

	virtual const char* __thiscall GetName () const
		{ return "ConversationMessageImpl"; }
};



#ifdef IS_OSL

class OSL : public cInterfaceImp<IOSLService, IID_Def<IOSLService>,
	kInterfaceImpStatic>, public IDarkOverlayHandler
{
public:
	virtual ~OSL ();

	// IUnknown
	STDMETHOD_ (void, Init) ();
	STDMETHOD_ (void, End) ();

	// IDarkOverlayHandler
	STDMETHOD_ (void, DrawHUD) ();
	STDMETHOD_ (void, DrawTOverlay) ();
	STDMETHOD_ (void, OnUIEnterMode) ();

	// IOSLService

	STDMETHOD_ (ParameterCache*, get_param_cache) ();

	STDMETHOD_ (bool, register_hud_element) (HUDElementBase&,
		HUDElementBase::ZIndex priority);
	STDMETHOD_ (bool, unregister_hud_element) (HUDElementBase&);
	STDMETHOD_ (HUDBitmap::Ptr, load_hud_bitmap) (const String& path,
		bool animation);

	STDMETHOD_ (bool, subscribe_links) (const Flavor&,
		const Object& source, const Object& host);
	STDMETHOD_ (bool, unsubscribe_links) (const Flavor&,
		const Object& source, const Object& host);

	STDMETHOD_ (bool, subscribe_property) (const Property&,
		const Object& object, const Object& host);
	STDMETHOD_ (bool, unsubscribe_property) (const Property&,
		const Object& object, const Object& host);

	STDMETHOD_ (bool, subscribe_conversation) (const Object& conversation,
		const Object& host);
	STDMETHOD_ (bool, unsubscribe_conversation) (const Object& conversation,
		const Object& host);

private:
	static OSL* self;
	bool sim;

	friend bool ::ThiefLibOSLInit (IScriptMan*, MPrintfProc, IMalloc*);
	OSL ();

	static int __cdecl on_sim (const sDispatchMsg*,
		const sDispatchListenerDesc*);

	std::unique_ptr<ParameterCacheImpl> param_cache;

	// HUD

	struct HUDElementInfo
	{
		HUDElementInfo (HUDElementBase&, HUDElementBase::ZIndex);
		bool operator == (const HUDElementInfo&) const;
		bool operator < (const HUDElementInfo&) const;

		HUDElementBase& element;
		HUDElementBase::ZIndex priority;
	};

	typedef std::set<HUDElementInfo> HUDElements;
	HUDElements hud_elements;

	typedef std::map<String, HUDBitmap::WeakPtr> HUDBitmaps;
	HUDBitmaps hud_bitmaps;

	// LinkChange, LinkAdd, and LinkRemove messages

	static void __stdcall on_link_event (sRelationListenMsg*, void*);

	typedef std::set<Flavor> ListenedFlavors;
	static ListenedFlavors listened_flavors;

	struct LinkContext
	{
		Flavor flavor;
		Object object;
		bool operator < (const LinkContext&) const;
	};
	typedef std::multimap<LinkContext, Object> LinkSubscriptions;
	LinkSubscriptions link_subscriptions;

	// PropertyChange message

	static void __stdcall on_property_event (sPropertyListenMsg*,
		PropListenerData);

	typedef std::map<Property, PropListenerHandle> ListenedProperties;
	static ListenedProperties listened_properties;

	typedef std::multimap<ObjectProperty, Object> PropertySubscriptions;
	PropertySubscriptions property_subscriptions;

	// ConversationEnd message

	static void __cdecl on_conversation_end (Object::Number);

	static bool listened_conversations;

	typedef std::multimap<Object, Object> ConversationSubscriptions;
	ConversationSubscriptions conversation_subscriptions;
};

#endif // IS_OSL



} // namespace Thief

DEFINE_IIDSTRUCT (Thief::IOSLService, Thief::IID_IOSLService);

#endif // OSL_HH

