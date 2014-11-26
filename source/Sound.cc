/******************************************************************************
 *  Sound.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013-2014 Kevin Daughtridge <kevin@kdau.com>
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
 *
 *****************************************************************************/

#include "Private.hh"

namespace Thief {



// SoundSchema

#if defined(IS_THIEF2) || defined(IS_OSL)
#define _SOUND_NET , kSoundNetNormal
#else
#define _SOUND_NET
#endif

PROXY_CONFIG (SoundSchema, last_sample, "SchLastSample", nullptr, int, -1);
PROXY_BIT_CONFIG (SoundSchema, retrigger, "SchPlayParams", "Flags", 1u, false);
PROXY_BIT_CONFIG (SoundSchema, pan_position, "SchPlayParams", "Flags",
	2u, false);
PROXY_BIT_CONFIG (SoundSchema, pan_range, "SchPlayParams", "Flags", 4u, false);
PROXY_NEG_BIT_CONFIG (SoundSchema, repeat, "SchPlayParams", "Flags", 8u, true);
PROXY_NEG_BIT_CONFIG (SoundSchema, cache, "SchPlayParams", "Flags", 16u, true);
PROXY_BIT_CONFIG (SoundSchema, stream, "SchPlayParams", "Flags", 32u, false);
PROXY_BIT_CONFIG (SoundSchema, play_once, "SchPlayParams", "Flags", 64u, true);
PROXY_NEG_BIT_CONFIG (SoundSchema, combat, "SchPlayParams", "Flags",
	128u, true);
PROXY_BIT_CONFIG (SoundSchema, net_ambient, "SchPlayParams", "Flags",
	256u, false);
PROXY_BIT_CONFIG (SoundSchema, local_spatial, "SchPlayParams", "Flags",
	512u, false);
PROXY_BIT_CONFIG (SoundSchema, is_noise, "SchPlayParams", "Flags",
	65536u, false);
PROXY_BIT_CONFIG (SoundSchema, is_speech, "SchPlayParams", "Flags",
	131072u, false);
PROXY_BIT_CONFIG (SoundSchema, is_ambient, "SchPlayParams", "Flags",
	262144u, false);
PROXY_BIT_CONFIG (SoundSchema, is_music, "SchPlayParams", "Flags",
	524288u, false);
PROXY_BIT_CONFIG (SoundSchema, is_meta_ui, "SchPlayParams", "Flags",
	1048576u, false);
PROXY_CONFIG (SoundSchema, volume, "SchPlayParams", "Volume", int, 0);
PROXY_CONFIG (SoundSchema, pan, "SchPlayParams", "Pan", int, 0);
PROXY_CONFIG (SoundSchema, fade, "SchPlayParams", "Fade", int, 0);
PROXY_CONFIG (SoundSchema, initial_delay, "SchPlayParams", "Initial Delay",
	Time, 0ul);
PROXY_CONFIG (SoundSchema, attenuation_factor, "SchAttFac", nullptr,
	float, 1.0f);
PROXY_CONFIG (SoundSchema, priority, "SchPriority", nullptr, int, 0);
PROXY_BIT_CONFIG (SoundSchema, loop_is_poly, "SchLoopParams", "Flags",
	1u, false);
PROXY_CONFIG (SoundSchema, loop_max_samples, "SchLoopParams", "Max Samples",
	unsigned, 0u);
PROXY_CONFIG (SoundSchema, loop_min_interval, "SchLoopParams", "Interval Min",
	Time, 0ul);
PROXY_CONFIG (SoundSchema, loop_max_interval, "SchLoopParams", "Interval Max",
	Time, 0ul);
PROXY_CONFIG (SoundSchema, ai_value, "AI_SndType", "Type",
	SoundSchema::AIValue, AIValue::NONE);
PROXY_CONFIG (SoundSchema, ai_signal, "AI_SndType", "Signal", String, "");

OBJECT_TYPE_IMPL_ (SoundSchema,
	PROXY_INIT (last_sample),
	PROXY_INIT (retrigger),
	PROXY_INIT (pan_position),
	PROXY_INIT (pan_range),
	PROXY_INIT (repeat),
	PROXY_INIT (cache),
	PROXY_INIT (stream),
	PROXY_INIT (play_once),
	PROXY_INIT (combat),
	PROXY_INIT (net_ambient),
	PROXY_INIT (local_spatial),
	PROXY_INIT (is_noise),
	PROXY_INIT (is_speech),
	PROXY_INIT (is_ambient),
	PROXY_INIT (is_music),
	PROXY_INIT (is_meta_ui),
	PROXY_INIT (volume),
	PROXY_INIT (pan),
	PROXY_INIT (fade),
	PROXY_INIT (initial_delay),
	PROXY_INIT (attenuation_factor),
	PROXY_INIT (priority),
	PROXY_INIT (loop_is_poly),
	PROXY_INIT (loop_max_samples),
	PROXY_INIT (loop_min_interval),
	PROXY_INIT (loop_max_interval),
	PROXY_INIT (ai_value),
	PROXY_INIT (ai_signal)
)

bool
SoundSchema::is_sound_schema () const
{
	return volume.exists ();
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

PROXY_NEG_BIT_CONFIG (AmbientHacked, active, "AmbientHacked", "Flags",
	4u, true);
PROXY_ARRAY_CONFIG (AmbientHacked, ambient_schema, 3u, String,
	PROXY_ARRAY_ITEM ("AmbientHacked", "Schema Name", ""),
	PROXY_ARRAY_ITEM ("AmbientHacked", "Aux Schema 1", ""),
	PROXY_ARRAY_ITEM ("AmbientHacked", "Aux Schema 2", ""));
PROXY_CONFIG (AmbientHacked, ambient_radius, "AmbientHacked", "Radius", int, 0);
PROXY_CONFIG (AmbientHacked, ambient_volume, "AmbientHacked", "Override Volume",
	int, 0);
PROXY_BIT_CONFIG (AmbientHacked, environmental, "AmbientHacked", "Flags",
	1u, false);
PROXY_BIT_CONFIG (AmbientHacked, fade_gradually, "AmbientHacked", "Flags",
	2u, false);
PROXY_BIT_CONFIG (AmbientHacked, turn_off_after, "AmbientHacked", "Flags",
	256u, false);
PROXY_BIT_CONFIG (AmbientHacked, remove_prop_after, "AmbientHacked", "Flags",
	8u, false);
PROXY_BIT_CONFIG (AmbientHacked, destroy_after, "AmbientHacked", "Flags",
	128u, false);

OBJECT_TYPE_IMPL_ (AmbientHacked,
	PROXY_INIT (active),
	PROXY_ARRAY_INIT (ambient_schema, 3),
	PROXY_INIT (ambient_radius),
	PROXY_INIT (ambient_volume),
	PROXY_INIT (environmental),
	PROXY_INIT (fade_gradually),
	PROXY_INIT (turn_off_after),
	PROXY_INIT (remove_prop_after),
	PROXY_INIT (destroy_after)
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

MESSAGE_WRAPPER_IMPL (SchemaDoneMessage, "SchemaDone"),
	location (LGVector (&MESSAGE_AS (sSchemaDoneMsg)->coordinates)),
	sound_source (MESSAGE_AS (sSchemaDoneMsg)->targetObject),
	schema_name (MESSAGE_AS (sSchemaDoneMsg)->name),
	schema (schema_name)
{}

SchemaDoneMessage::SchemaDoneMessage (const Vector& _location,
		const Object& _sound_source, const String& _schema_name)
	: Message (new sSchemaDoneMsg ()), location (_location),
	  sound_source (_sound_source), schema_name (_schema_name)
{
	message->message = "SchemaDone";
	MESSAGE_AS (sSchemaDoneMsg)->coordinates = LGVector (location);
	MESSAGE_AS (sSchemaDoneMsg)->targetObject = sound_source.number;
	MESSAGE_AS (sSchemaDoneMsg)->name = schema_name.data ();
}



// VoiceOverLink

PROXY_BIT_CONFIG (VoiceOverLink, play_when_focused, "Events", nullptr,
	1u, false);
PROXY_BIT_CONFIG (VoiceOverLink, play_when_contained, "Events", nullptr,
	2u, false);

LINK_FLAVOR_IMPL (VoiceOver,
	PROXY_INIT (play_when_focused),
	PROXY_INIT (play_when_contained)
)

VoiceOverLink
VoiceOverLink::create (const Object& source, const Object& dest,
	bool play_when_focused, bool play_when_contained)
{
	VoiceOverLink link = Link::create (flavor (), source, dest);
	if (link != Link::NONE)
	{
		link.play_when_focused = play_when_focused;
		link.play_when_contained = play_when_contained;
	}
	return link;
}



} // namespace Thief

