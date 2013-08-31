/******************************************************************************
 *  Property.cc
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



// ObjectProperty

ObjectProperty::ObjectProperty (const String& _property,
		const Object& _object, bool add_if_missing)
	: property (SInterface<IPropertyManager> (LG)->GetPropertyNamed
		(_property.data ())),
	  object (_object)
{
	if (property) property->AddRef ();
	if (property && add_if_missing && !exists ())
		add ();
}

ObjectProperty::ObjectProperty (const char* _property,
		const Object& _object, bool add_if_missing)
	: property (SInterface<IPropertyManager> (LG)->GetPropertyNamed
		(_property)),
	  object (_object)
{
	if (property) property->AddRef ();
	if (property && add_if_missing && !exists ())
		add ();
}

ObjectProperty::ObjectProperty (const ObjectProperty& copy)
	: property (copy.property), object (copy.object)
{
	if (property) property->AddRef ();
}

ObjectProperty::~ObjectProperty ()
{
	if (property) property->Release ();
}

String
ObjectProperty::get_property () const
{
	return (property && property->Describe ())
		? property->Describe ()->szName : String ();
}

Object
ObjectProperty::get_object () const
{
	return object;
}

bool
ObjectProperty::exists (bool inherited) const
{
	if (!object.exists () || !property) return false;
	return inherited
		? property->IsRelevant (object.number)
		: property->IsSimplyRelevant (object.number);
}

bool
ObjectProperty::add ()
{
	if (!object.exists () || !property) return false;
	return property->Create (object.number) == S_OK;
}

bool
ObjectProperty::copy_from (const Object& source)
{
	if (!object.exists () || !source.exists () || !property)
		return false;
	return property->Copy (object.number, source.number) == S_OK;
}

bool
ObjectProperty::remove ()
{
	if (!object.exists () || !property) return false;
	return property->Delete (object.number) == S_OK;
}

bool
ObjectProperty::subscribe (const String& property, const Object& object,
	const Object& _host)
{
	if (property.empty ()) return false;
	Object host = (_host == Object::SELF) ? object : _host;
	return SService<IDarkHookScriptService> (LG)->InstallPropHook
		(host.number, kDHNotifyDefault, property.data (),
			object.number);
}

bool
ObjectProperty::unsubscribe (const String& property, const Object& object,
	const Object& _host)
{
	if (property.empty ()) return false;
	Object host = (_host == Object::SELF) ? object : _host;
	SService<IDarkHookScriptService> (LG)->UninstallPropHook
		(host.number, property.data (), object.number);
	return true;
}

void
ObjectProperty::_get (LGMultiBase& value) const
{
	if (!object.exists () || !property)
		value.clear ();
	else
		SService<IPropertySrv> (LG)->Get (value, object.number,
			property->Describe ()->szName, nullptr);
}

bool
ObjectProperty::_set (const LGMultiBase& value)
{
	if (!object.exists () || !property) return false;
	if (!exists (false) && !add ()) return false;
	return SService<IPropertySrv> (LG)->Set (object.number,
		property->Describe ()->szName, nullptr, value) == S_OK;
}

void
ObjectProperty::_get_field (const char* field, LGMultiBase& value) const
{
	if (!object.exists () || !property)
		value.clear ();
	else
		SService<IPropertySrv> (LG)->Get (value, object.number,
			property->Describe ()->szName, field);
}

bool
ObjectProperty::_set_field (const char* field, const LGMultiBase& value,
	bool add_if_missing)
{
	if (!object.exists () || !property) return false;
	if (!exists (false) && (!add_if_missing || !add ())) return false;
	return SService<IPropertySrv> (LG)->Set (object.number,
		property->Describe ()->szName, field, value) == S_OK;
}



// PropFieldBase

void
PropFieldBase::get (const Object& object, const char* property,
	const char* field, LGMultiBase& value) const
{
	ObjectProperty objprop (property, object);
	if (!objprop.exists ())
		{}
	else if (field)
		objprop._get_field (field, value);
	else
		objprop._get (value);
}

void
PropFieldBase::set (Object& object, const char* property, const char* field,
	const LGMultiBase& value)
{
	ObjectProperty objprop (property, object);
	if (field ? !objprop._set_field (field, value, true)
	          : !objprop._set (value))
		throw std::runtime_error ("could not set property field");
}

bool
PropFieldBase::get_bit (const FieldProxyConfig<bool>& config,
	const Object& object) const
{
	LGMulti<unsigned> field (config.default_value ? config.bitmask : 0u);
	get (object, config.major, config.minor, field);
	return field & config.bitmask;	
}

void
PropFieldBase::set_bit (const FieldProxyConfig<bool>& config, Object& object,
	bool value)
{
	LGMulti<unsigned> field; field = 0u;
	get (object, config.major, config.minor, field);

	if (value)
		field = field | config.bitmask;
	else
		field = field & ~config.bitmask;

	ObjectProperty objprop (config.major, object);
	if (config.minor ? !objprop._set_field (config.minor, field, true)
	                 : !objprop._set (field))
		throw std::runtime_error ("could not set property field");
}



// PropertyChangeMessage

MESSAGE_WRAPPER_IMPL_ (PropertyChangeMessage,
	MESSAGE_TYPENAME_TEST ("sDHNotifyMsg") &&
	MESSAGE_AS (sDHNotifyMsg)->typeDH == kDH_Property)

PropertyChangeMessage::PropertyChangeMessage (Event event, bool inherited,
		const char* property, const Object& object)
	: Message (new sDHNotifyMsg ())
{
	message->message = "DHNotify"; // local name is generated by Script
	MESSAGE_AS (sDHNotifyMsg)->typeDH = kDH_Property;
	MESSAGE_AS (sDHNotifyMsg)->sProp.event =
		ePropEvent ((event + 1) | (inherited ? kProp_Inherited : 0));
	MESSAGE_AS (sDHNotifyMsg)->sProp.pProp =
		SInterface<IPropertyManager> (LG)->GetPropertyNamed (property);
	MESSAGE_AS (sDHNotifyMsg)->sProp.pszPropName = property;
	MESSAGE_AS (sDHNotifyMsg)->sProp.idObj = object.number;
}

PropertyChangeMessage::Event
PropertyChangeMessage::get_event () const
{
	return Event ((MESSAGE_AS (sDHNotifyMsg)->sProp.event
		& ~kProp_Inherited) - 1);
}

bool
PropertyChangeMessage::is_inherited () const
{
	return MESSAGE_AS (sDHNotifyMsg)->sProp.event & kProp_Inherited;
}

String
PropertyChangeMessage::get_property () const
{
	return MESSAGE_AS (sDHNotifyMsg)->sProp.pszPropName
		? MESSAGE_AS (sDHNotifyMsg)->sProp.pszPropName : String ();
}

MESSAGE_ACCESSOR (Object, PropertyChangeMessage, get_object,
	sDHNotifyMsg, sProp.idObj)



/*TODO wrap the following properties in appropriate locations:
 * AI: Utility\Blocks AI Vision = AI_BlkVis
 * AI: Utility\Watch: Watch link defaults = AI_WtchPnt (propdefs.h: sAIWatchPoint)
 * Dark GameSys\Stats = DarkStat: "FoundBody"
 * Engine Features\Combine Type = CombineType
 * Engine Features\Suspicious = SuspObj
 * Inventory\Block Frob? = BlockFrob
 * Prox\Fungus = Fungus
 * Trap\Quest Var = TrapQVar
 */



/* The following properties and property fields: are not instantiated in the
 * stock T2 dark.gam, have no documented use, work only in SS2, were never
 * implemented at all, are better accessed through methods in script services,
 * and/or are just clearly unsuited for direct use by scripts. No PropField
 * proxies have been created for them, but they may be accessed with the generic
 * Property wrapper.
 *
 * AI: AI Core\Free sense knowledge = AI_FreeKnow
 * AI: AI Core\Handed-off proxy = AI_IsProxy
 * AI: AI Core\Motion tags = AI_MotTags
 * AI: AI Core\No Multiplayer Ghost = AI_NoGhost
 * AI: AI Core\No Multiplayer Handoff = AI_NoHandoff
 * AI: AI Core\Projectile: Visible launch = AI_LaunchVis
 * AI: AI Core\Sound tags = AI_SndTags
 * AI: AI Core\Visibility Modifier = AI_VisModifier
 * AI: Debug\HtoHModeOverride = HTHModeOverride
 * AI: Utility\Angle Limits = AngleLimit
 * AI: Utility\Is Frustrated = AI_Frustrated
 * Difficulty\Close (Open) Door = DiffClose
 * Difficulty\Destroy = DiffDestroy
 * Difficulty\Lock (Unlock) = DiffLock
 * Difficulty\Permit = DiffPermit
 * Difficulty\Remove Properties = DiffRemoveProp
 * Difficulty\Script = DiffScript
 * Difficulty\Turn On (Off) = DiffTurnOn
 * CSArrow
 * CSProjectile
 * CSProperty
 * (Dark GameSys\Stats = DarkStat): "Enemy"
 * Editor\Auto-Multibrush = AutoVBR
 * Editor\Brush Name = Brush
 * Editor\Editor Comments = EdComment
 * Editor\Has Brush = HasBrush
 * Engine Features\From Briefcase? = FromBriefcase
 * (Engine Features\KeyDst = KeyDst): "MasterBit"
 * Engine Features\Preload = Preload
 * Game: Damage Model\Weapon Damage = WeaponDamage
 * Game: Damage Model\Weapon Type = WeaponType
 * Inventory\Being Taken = InvBeingTaken
 * LockCnt
 * ModelNumber
 * Motions\Gait Desc = MotGaitDesc
 * Motions\Motor Controller = MotorController
 * Motions\Phys Limits = MotPhysLimits
 * Motions\Player Limb Offsets = MotPlyrLimbOff
 * Motions\Sword Action Type = SwordActionType
 * (all Networking properties)
 * Object System\Donor Type = DonorType
 * Object System\Fixture = Fixture
 * Object System\Symbolic Name = SymName
 * (Physics: Model\Controls = PhysControl): "Axis Velocity"
 * (Physics: Model\Dimensions = PhysDims): "Point vs Not Special"
 * (Physics: Model\Dimensions = PhysDims): "Point vs Terrain"
 * (Physics: Model\Type = PhysType): "Special"
 * Physics: Projectile\Launcher Mass = LauncherMass
 * Physics: Terrain\Elasticity = Elasticity
 * Puppet
 * Renderer\Bump Map = Bump Map
 * Renderer\Face State = Face State
 * Renderer\Face Textures = Face
 * Schema\Action = SchActionSnd
 * Schema\Message = SchMsg
 * Script\TerrReplaceDestroy = TerrRepDestroy
 * Scripts
 * SFX\Particle = Particle
 * SFX\Particle Type = ParticleType
 * SFX\Spark = Spark
 * Sound\Face Motions = Face Pos
 * Speech\Last Played = SpchNextPlay
 * Speech\Pause Max = MaxSpchPause
 * Speech\Pause Min = MinSpchPause
 * Speech\Voice Index = VoiceIdx
 * Texture\Index = TextureID
 * Weapon\BaseDamage = BaseWpnDmg
 * Weapon\CurDamage = CurWpnDmg
 * Weapon\Mode Change Metaproperty = ModeChangeMeta
 * Weapon\Mode Change-back Metaproperty = ModeUnchngeMeta
 */



} // namespace Thief

