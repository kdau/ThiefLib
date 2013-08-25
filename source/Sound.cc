/******************************************************************************
 *  Sound.cc
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

static bool _not (const bool& value) { return !value; }



// SoundSchema

#undef OBJECT_TYPE
#define OBJECT_TYPE SoundSchema

#ifdef IS_THIEF2
#define _SOUND_NET , kSoundNetNormal
#else
#define _SOUND_NET
#endif

PF_CONFIG (last_sample, "SchLastSample", nullptr, int, -1);
PFB_CONFIG (retrigger, "SchPlayParams", "Flags", 1u, false);
PFB_CONFIG (pan_position, "SchPlayParams", "Flags", 2u, false);
PFB_CONFIG (pan_range, "SchPlayParams", "Flags", 4u, false);
PF_CONFIG_ (repeat, "SchPlayParams", "Flags", bool, false, 8u, _not, _not);
PF_CONFIG_ (cache, "SchPlayParams", "Flags", bool, false, 16u, _not, _not);
PFB_CONFIG (stream, "SchPlayParams", "Flags", 32u, false);
PFB_CONFIG (play_once, "SchPlayParams", "Flags", 64u, false);
PF_CONFIG_ (combat, "SchPlayParams", "Flags", bool, false, 128u, _not, _not);
PFB_CONFIG (net_ambient, "SchPlayParams", "Flags", 256u, false);
PFB_CONFIG (local_spatial, "SchPlayParams", "Flags", 512u, false);
PFB_CONFIG (is_noise, "SchPlayParams", "Flags", 65536u, false);
PFB_CONFIG (is_speech, "SchPlayParams", "Flags", 131072u, false);
PFB_CONFIG (is_ambient, "SchPlayParams", "Flags", 262144u, false);
PFB_CONFIG (is_music, "SchPlayParams", "Flags", 524288u, false);
PFB_CONFIG (is_meta_ui, "SchPlayParams", "Flags", 1048576u, false);
PF_CONFIG (volume, "SchPlayParams", "Volume", int, 0);
PF_CONFIG (pan, "SchPlayParams", "Pan", int, 0);
PF_CONFIG (fade, "SchPlayParams", "Fade", int, 0);
PF_CONFIG (initial_delay, "SchPlayParams", "Initial Delay", Time, 0ul);
PF_CONFIG (attenuation_factor, "SchAttFac", nullptr, float, 1.0f);
PF_CONFIG (priority, "SchPriority", nullptr, int, 0);
PFB_CONFIG (loop_is_poly, "SchLoopParams", "Flags", 1u, false);
PF_CONFIG (loop_max_samples, "SchLoopParams", "Max Samples", unsigned, 0u);
PF_CONFIG (loop_min_interval, "SchLoopParams", "Interval Min", Time, 0ul);
PF_CONFIG (loop_max_interval, "SchLoopParams", "Interval Max", Time, 0ul);
PF_CONFIG (ai_value, "AI_SndType", "Type", SoundSchema::AIValue,
	SoundSchema::AIValue::NONE);
PF_CONFIG (ai_signal, "AI_SndType", "Signal", String, "");

OBJECT_TYPE_IMPL_ (SoundSchema,
	PF_INIT (last_sample),
	PF_INIT (retrigger),
	PF_INIT (pan_position),
	PF_INIT (pan_range),
	PF_INIT (repeat),
	PF_INIT (cache),
	PF_INIT (stream),
	PF_INIT (play_once),
	PF_INIT (combat),
	PF_INIT (net_ambient),
	PF_INIT (local_spatial),
	PF_INIT (is_noise),
	PF_INIT (is_speech),
	PF_INIT (is_ambient),
	PF_INIT (is_music),
	PF_INIT (is_meta_ui),
	PF_INIT (volume),
	PF_INIT (pan),
	PF_INIT (fade),
	PF_INIT (initial_delay),
	PF_INIT (attenuation_factor),
	PF_INIT (priority),
	PF_INIT (loop_is_poly),
	PF_INIT (loop_max_samples),
	PF_INIT (loop_min_interval),
	PF_INIT (loop_max_interval),
	PF_INIT (ai_value),
	PF_INIT (ai_signal)
)

bool
SoundSchema::is_sound_schema () const
{
	return Property (*this, "SchPlayParams").exists ();
}

bool
SoundSchema::play (const Object& source, const Object& _host)
{
	Object host = (_host == Object::SELF) ? source : _host;
	LGBool success;
	SService<ISoundScrSrv> (LG)->PlaySchemaAtObject
		(success, host.number, number, source.number _SOUND_NET);
	return success;
}

bool
SoundSchema::play (const Vector& source, const Object& host)
{
	LGBool success;
	SService<ISoundScrSrv> (LG)->PlaySchemaAtLocation
		(success, host.number, number, LGVector (source) _SOUND_NET);
	return success;
}

bool
SoundSchema::play_ambient (const Object& host)
{
	LGBool success;
	SService<ISoundScrSrv> (LG)->PlaySchemaAmbient
		(success, host.number, number _SOUND_NET);
	return success;
}

bool
SoundSchema::played_as_voiceover () const
{
	return SInterface<IVoiceOverSys> (LG)->AlreadyPlayed (number);
}

bool
SoundSchema::play_voiceover (const Object& host)
{
	LGBool success;
	SService<ISoundScrSrv> (LG)->PlayVoiceOver
		(success, host.number, number);
	return success;
}

bool
SoundSchema::halt (const Object& source, const Object& _host)
{
	Object host = (_host == Object::SELF) ? source : _host;
	LGBool success;
	SService<ISoundScrSrv> (LG)->HaltSchema
		(success, source.number, get_name ().data (), host.number);
	return success;
}

size_t
SoundSchema::halt_all (const Object& source, const Object& _host)
{
	Object host = (_host == Object::SELF) ? source : _host;
	LGBool success;
	SService<ISoundScrSrv> (LG)->HaltSchema
		(success, source.number, "", host.number);
	return success;
}

// ISoundScrSrv::PreLoad seems to always fail.

bool
SoundSchema::play_by_tags (const String& tags, Tagged location,
	const Object& source1, const Object& source2, const Object& _host)
{
	Object host = (_host == Object::SELF) ? source1 : _host;
	LGBool success;
	SService<ISoundScrSrv> (LG)->PlayEnvSchema
		(success, host.number, tags.data (), source1.number,
			source2.number, eEnvSoundLoc (location) _SOUND_NET);
	return success;
}



// AmbientHacked

#undef OBJECT_TYPE
#define OBJECT_TYPE AmbientHacked

PF_CONFIG_ (active, "AmbientHacked", "Flags", bool, false, 4u, _not, _not);
PF_CONFIG (ambient_schema_1, "AmbientHacked", "Schema Name", String, "");
PF_CONFIG (ambient_schema_2, "AmbientHacked", "Aux Schema 1", String, "");
PF_CONFIG (ambient_schema_3, "AmbientHacked", "Aux Schema 2", String, "");
PF_CONFIG (ambient_radius, "AmbientHacked", "Radius", int, 0);
PF_CONFIG (ambient_volume, "AmbientHacked", "Override Volume", int, 0);
PFB_CONFIG (environmental, "AmbientHacked", "Flags", 1u, false);
PFB_CONFIG (fade_gradually, "AmbientHacked", "Flags", 2u, false);
PFB_CONFIG (turn_off_after, "AmbientHacked", "Flags", 256u, false);
PFB_CONFIG (remove_prop_after, "AmbientHacked", "Flags", 8u, false);
PFB_CONFIG (destroy_after, "AmbientHacked", "Flags", 128u, false);

OBJECT_TYPE_IMPL_ (AmbientHacked,
	PF_INIT (active),
	PF_INIT (ambient_schema_1),
	PF_INIT (ambient_schema_2),
	PF_INIT (ambient_schema_3),
	PF_INIT (ambient_radius),
	PF_INIT (ambient_volume),
	PF_INIT (environmental),
	PF_INIT (fade_gradually),
	PF_INIT (turn_off_after),
	PF_INIT (remove_prop_after),
	PF_INIT (destroy_after)
)

bool
AmbientHacked::is_ambient_hacked () const
{
	return active.exists ();
}



/*TODO wrap these properties, perhaps creating SoundSource : Object:
 * Schema\Class Tags = Class Tags
 * Schema\Material Tags = Material Tags
 */



// SchemaDoneMessage

MESSAGE_WRAPPER_IMPL (SchemaDoneMessage, sSchemaDoneMsg)

SchemaDoneMessage::SchemaDoneMessage (const Vector& location,
		const Object& target, const char* schema_name)
	: Message (new sSchemaDoneMsg ())
{
	message->message = "SchemaDone";
	MESSAGE_AS (sSchemaDoneMsg)->coordinates = LGVector (location);
	MESSAGE_AS (sSchemaDoneMsg)->targetObject = target.number;
	MESSAGE_AS (sSchemaDoneMsg)->name = schema_name;
}

Vector
SchemaDoneMessage::get_location () const
{
	return LGVector (&MESSAGE_AS (sSchemaDoneMsg)->coordinates);
}

MESSAGE_ACCESSOR (Object, SchemaDoneMessage, get_target,
	sSchemaDoneMsg, targetObject)

MESSAGE_ACCESSOR (String, SchemaDoneMessage, get_schema_name,
	sSchemaDoneMsg, name)

Object
SchemaDoneMessage::get_schema () const
{
	const char* schema_name = MESSAGE_AS (sSchemaDoneMsg)->name;
	return schema_name ? Object (schema_name) : Object::NONE;
}



// VoiceOverLink

FLAVORED_LINK_IMPL (VoiceOver)

VoiceOverLink
VoiceOverLink::create (const Object& source, const Object& dest,
	bool play_when_focused, bool play_when_contained)
{
	VoiceOverLink link = Link::create (flavor (), source, dest);
	link.set_play_when_focused (play_when_focused);
	link.set_play_when_contained (play_when_contained);
	return link;
}

bool
VoiceOverLink::get_play_when_focused () const
{
	return get_data_field<unsigned> ("Events") & 1u;
}

void
VoiceOverLink::set_play_when_focused (bool play_when_focused)
{
	unsigned events = get_data_field<unsigned> ("Events");
	if (play_when_focused)
		events |= 1u;
	else
		events &= ~1u;
	set_data_field ("Events", events);
}

bool
VoiceOverLink::get_play_when_contained () const
{
	return get_data_field<unsigned> ("Events") & 2u;
}

void
VoiceOverLink::set_play_when_contained (bool play_when_contained)
{
	unsigned events = get_data_field<unsigned> ("Events");
	if (play_when_contained)
		events |= 2u;
	else
		events &= ~2u;
	set_data_field ("Events", events);
}



} // namespace Thief

