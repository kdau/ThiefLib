/******************************************************************************
 *  Sound.hh
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef THIEF_SOUND_HH
#define THIEF_SOUND_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/Property.hh>

namespace Thief {



// SoundSchema

class SoundSchema : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (SoundSchema)
	bool is_sound_schema () const;

	// Playback control

	bool play (const Object& source, const Object& host = Object::SELF);
	bool play (const Vector& source, const Object& host = Object::NONE);
	bool play_ambient (const Object& host = Object::NONE);

	bool played_as_voiceover () const;
	bool play_voiceover (const Object& host = Object::NONE);

	bool halt (const Object& source, const Object& host = Object::SELF);
	static size_t halt_all (const Object& source,
		const Object& host = Object::SELF);

	// Tagged schemas (env_tag)

	enum class Tagged
	{
		ON_OBJECT,
		AT_OBJECT_LOCATION,
		AMBIENT
	};

	static bool play_by_tags (const String& tags, Tagged location,
		const Object& source1 = Object::NONE,
		const Object& source2 = Object::NONE,
		const Object& host = Object::SELF);

	// Playback configuration

	THIEF_PROP_FIELD_CONST (int, last_sample); //TESTME

	THIEF_PROP_FIELD (bool, retrigger); //TESTME
	THIEF_PROP_FIELD (bool, pan_position); //TESTME
	THIEF_PROP_FIELD (bool, pan_range); //TESTME
	THIEF_PROP_FIELD (bool, repeat); //TESTME
	THIEF_PROP_FIELD (bool, cache); //TESTME
	THIEF_PROP_FIELD (bool, stream); //TESTME
	THIEF_PROP_FIELD (bool, play_once); //TESTME
	THIEF_PROP_FIELD (bool, combat); //TESTME
	THIEF_PROP_FIELD (bool, net_ambient); //TESTME
	THIEF_PROP_FIELD (bool, local_spatial); //TESTME

	THIEF_PROP_FIELD (bool, is_noise); //TESTME
	THIEF_PROP_FIELD (bool, is_speech); //TESTME
	THIEF_PROP_FIELD (bool, is_ambient); //TESTME
	THIEF_PROP_FIELD (bool, is_music); //TESTME
	THIEF_PROP_FIELD (bool, is_meta_ui); //TESTME

	THIEF_PROP_FIELD (int, volume); //TESTME
	THIEF_PROP_FIELD (int, pan); //TESTME
	THIEF_PROP_FIELD (int, fade); //TESTME
	THIEF_PROP_FIELD (Time, initial_delay); //TESTME
	THIEF_PROP_FIELD (float, attenuation_factor);
	THIEF_PROP_FIELD (int, priority);

	THIEF_PROP_FIELD (bool, loop_is_poly); //TESTME
	THIEF_PROP_FIELD (unsigned, loop_max_samples); //TESTME
	THIEF_PROP_FIELD (Time, loop_min_interval); //TESTME
	THIEF_PROP_FIELD (Time, loop_max_interval); //TESTME

	// Semantic value

	enum class AIValue
	{
		NONE, INFORM, MINOR_ANOMALY, MAJOR_ANOMALY, HIGH, COMBAT_HIGH
	};
	THIEF_PROP_FIELD (AIValue, ai_value);

	THIEF_PROP_FIELD (String, ai_signal);
};



// SoundSource

class SoundSource : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (SoundSource)

	THIEF_PROP_FIELD (String, class_tags);
	THIEF_PROP_FIELD (String, material_tags); //TESTME
};



// AmbientHacked

class AmbientHacked : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (AmbientHacked)
	bool is_ambient_hacked () const;

	THIEF_PROP_FIELD (bool, active);

	THIEF_PROP_FIELD_ARRAY (String, ambient_schema, 3u); //TESTME

	THIEF_PROP_FIELD (int, ambient_radius); //TESTME
	THIEF_PROP_FIELD (int, ambient_volume); //TESTME
	THIEF_PROP_FIELD (bool, environmental); //TESTME
	THIEF_PROP_FIELD (bool, fade_gradually); //TESTME

	THIEF_PROP_FIELD (bool, turn_off_after); //TESTME
	THIEF_PROP_FIELD (bool, remove_prop_after); //TESTME
	THIEF_PROP_FIELD (bool, destroy_after); //TESTME
};



// SchemaDoneMessage: "SchemaDone"

class SchemaDoneMessage : public Message //TESTME
{
public:
	SchemaDoneMessage (const Vector& location, const Object& sound_source,
		const String& schema_name);
	THIEF_MESSAGE_WRAP (SchemaDoneMessage);

	const Vector location;
	const Object sound_source;
	const String schema_name;
	const SoundSchema schema;
};



// VoiceOverLink

class VoiceOverLink : public Link //TESTME
{
public:
	THIEF_LINK_FLAVOR (VoiceOver)

	static VoiceOverLink create (const Object& source, const Object& dest,
		bool play_when_focused, bool play_when_contained);

	THIEF_LINK_FIELD (bool, play_when_focused);
	THIEF_LINK_FIELD (bool, play_when_contained);
};



} // namespace Thief

#endif // THIEF_SOUND_HH

