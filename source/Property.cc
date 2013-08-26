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



// Property

Property::Property (const Object& _object, const String& name,
		bool add_if_missing)
	: object (_object.number),
	  property (SInterface<IPropertyManager> (LG)->GetPropertyNamed
	  	(name.data ()))
{
	if (property) property->AddRef ();
	if (property && add_if_missing && !exists ())
		add ();
}

Property::Property (const Object& _object, const char* name,
		bool add_if_missing)
	: object (_object.number),
	  property (SInterface<IPropertyManager> (LG)->GetPropertyNamed (name))
{
	if (property) property->AddRef ();
	if (property && add_if_missing && !exists ())
		add ();
}

Property::Property (const Property& copy)
	: object (copy.object), property (copy.property)
{
	if (property) property->AddRef ();
}

Property::~Property ()
{
	if (property) property->Release ();
}

Object
Property::get_object () const
{
	return object;
}

String
Property::get_name () const
{
	return (property && property->Describe ())
		? property->Describe ()->szName : String ();
}

bool
Property::exists (bool inherited) const
{
	if (!Object (object).exists () || !property) return false;
	return inherited
		? property->IsRelevant (object)
		: property->IsSimplyRelevant (object);
}

bool
Property::add ()
{
	if (!Object (object).exists () || !property) return false;
	return property->Create (object) == S_OK;
}

bool
Property::copy_from (const Object& source)
{
	if (!Object (object).exists () || !source.exists () || !property) return false;
	return property->Copy (object, source.number) == S_OK;
}

bool
Property::remove ()
{
	if (!Object (object).exists () || !property) return false;
	return property->Delete (object) == S_OK;
}

void
Property::subscribe (const Object& _host)
{
	if (!property)
		throw std::runtime_error ("can't subscribe to Property::NONE");
	Object host = (_host == Object::SELF) ? object : _host;
	SService<IDarkHookScriptService> (LG)->InstallPropHook (host.number,
		kDHNotifyDefault, property->Describe ()->szName, object);
}

void
Property::unsubscribe (const Object& _host)
{
	if (!property)
		throw std::runtime_error ("can't subscribe to Property::NONE");
	Object host = (_host == Object::SELF) ? object : _host;
	SService<IDarkHookScriptService> (LG)->UninstallPropHook
		(host.number, property->Describe ()->szName, object);
}

void
Property::_get (LGMultiBase& value) const
{
	if (!Object (object).exists () || !property)
		value.clear ();
	else
		SService<IPropertySrv> (LG)->Get (value, object,
			property->Describe ()->szName, nullptr);
}

bool
Property::_set (const LGMultiBase& value)
{
	if (!Object (object).exists () || !property) return false;
	if (!exists (false) && !add ()) return false;
	return SService<IPropertySrv> (LG)->Set (object,
		property->Describe ()->szName, nullptr, value) == S_OK;
}

void
Property::_get_field (const String& field, LGMultiBase& value) const
{
	if (!Object (object).exists () || !property)
		value.clear ();
	else
		SService<IPropertySrv> (LG)->Get (value, object,
			property->Describe ()->szName,
			field.empty () ? nullptr : field.data ());
}

bool
Property::_set_field (const String& field, const LGMultiBase& value,
	bool add_if_missing)
{
	if (!Object (object).exists () || !property) return false;
	if (!exists (false) && (!add_if_missing || !add ())) return false;
	return SService<IPropertySrv> (LG)->Set (object,
		property->Describe ()->szName,
		field.empty () ? nullptr : field.data (), value) == S_OK;
}



// PropFieldBase

void
PropFieldBase::get (const Object& object, const char* _property,
	const char* field, LGMultiBase& value) const
{
	Property property (object, _property);
	if (field)
		property._get_field (field, value);
	else
		property._get (value);
}

void
PropFieldBase::set (const Object& object, const char* _property,
	const char* field, const LGMultiBase& value)
{
	Property property (object, _property);
	if (field ? !property._set_field (field, value, true)
			: !property._set (value))
		throw std::runtime_error ("could not set property field");
}

bool
PropFieldBase::get_bit (const PropFieldConfig<bool>& config,
	const Object& object) const
{
	LGMulti<unsigned> field;
	get (object, config.property, config.field, field);
	return field & config.bitmask;	
}

void
PropFieldBase::set_bit (const PropFieldConfig<bool>& config,
	const Object& object, bool value)
{
	Property property (object, config.property);
	unsigned field = config.field
		? property.get_field (config.field, 0u) : property.get (0u);

	if (value)
		field |= config.bitmask;
	else
		field &= ~config.bitmask;

	LGMulti<unsigned> multi (field);
	if (config.field ? !property._set_field (config.field, multi, true)
			: !property._set (multi))
		throw std::runtime_error ("could not set property field");
}



// PropertyChangeMessage

MESSAGE_WRAPPER_IMPL_ (PropertyChangeMessage,
	MESSAGE_TYPENAME_TEST ("sDHNotifyMsg") &&
	MESSAGE_AS (sDHNotifyMsg)->typeDH == kDH_Property)

PropertyChangeMessage::PropertyChangeMessage (Event event, bool inherited,
		const Property& property)
	: Message (new sDHNotifyMsg ())
{
	message->message = "DHNotify"; // local name is generated by Script
	MESSAGE_AS (sDHNotifyMsg)->typeDH = kDH_Property;
	MESSAGE_AS (sDHNotifyMsg)->sProp.event =
		ePropEvent (event | (inherited ? kProp_Inherited : 0));
	MESSAGE_AS (sDHNotifyMsg)->sProp.pProp = property.property;
	MESSAGE_AS (sDHNotifyMsg)->sProp.pszPropName =
		property.property ? property.property->Describe ()->szName : "";
	MESSAGE_AS (sDHNotifyMsg)->sProp.idObj = property.object;
}

PropertyChangeMessage::Event
PropertyChangeMessage::get_event () const
{
	return Event (MESSAGE_AS (sDHNotifyMsg)->sProp.event & ~kProp_Inherited);
}

bool
PropertyChangeMessage::is_inherited () const
{
	return MESSAGE_AS (sDHNotifyMsg)->sProp.event & kProp_Inherited;
}

String
PropertyChangeMessage::get_prop_name () const
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
 * Prox\Blood = Blood
 * Prox\Fungus = Fungus
 * Script\Timing = ScriptTiming
 * Trap\Quest Var = TrapQVar
 */



/* The following properties and property fields: are not instantiated in the
 * stock T2 dark.gam, have no documented use, work only in SS2, were never
 * implemented at all, are better accessed through methods in script services,
 * and/or are just clearly unsuited for direct use by scripts. No specialized
 * wrapper methods have been created for them, but they may be accessed with
 * the generic Property wrapper.
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
 * Dark GameSys\Stats = DarkStat: "Enemy"
 * Editor\Auto-Multibrush = AutoVBR
 * Editor\Brush Name = Brush
 * Editor\Editor Comments = EdComment
 * Editor\Has Brush = HasBrush
 * Engine Features\From Briefcase? = FromBriefcase
 * Engine Features\KeyDst = KeyDst: "MasterBit"
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

