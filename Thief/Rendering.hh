/******************************************************************************
 *  Rendering.hh
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

#ifndef THIEF_RENDERING_HH
#define THIEF_RENDERING_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/Property.hh>

namespace Thief {



// Rendered

class Rendered : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Rendered)

	enum class RenderType { NORMAL, NONE, UNLIT, EDITOR_ONLY, CORONA_ONLY };
	THIEF_PROP_FIELD (RenderType, render_type);
	THIEF_PROP_FIELD (int, z_bias);

	THIEF_PROP_FIELD (float, opacity);

	THIEF_PROP_FIELD (String, model);
	THIEF_PROP_FIELD (Vector, model_scale);

	THIEF_PROP_FIELD (String, replacement_texture_0); //TESTME et seq.
	THIEF_PROP_FIELD (String, replacement_texture_1);
	THIEF_PROP_FIELD (String, replacement_texture_2);
	THIEF_PROP_FIELD (String, replacement_texture_3);

	THIEF_PROP_FIELD (float, joint_position_1); //TESTME et seq.
	THIEF_PROP_FIELD (float, joint_position_2);
	THIEF_PROP_FIELD (float, joint_position_3);
	THIEF_PROP_FIELD (float, joint_position_4);
	THIEF_PROP_FIELD (float, joint_position_5);
	THIEF_PROP_FIELD (float, joint_position_6);

	THIEF_PROP_FIELD (float, extra_light);
	THIEF_PROP_FIELD (bool, extra_light_additive);

	THIEF_PROP_FIELD (float, self_illumination); //TESTME

	THIEF_PROP_FIELD_CONST (bool, static_shadow); //TESTME
	THIEF_PROP_FIELD_CONST (bool, force_static_shadow); //TESTME
	THIEF_PROP_FIELD (int, dynamic_shadow);
	THIEF_PROP_FIELD (bool, visibility_shadow); //TESTME
};



// Light

class Light : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Light)

	// These fields should be considered constant for non-dynamic lights.
	THIEF_PROP_FIELD (float, hue);
	THIEF_PROP_FIELD (float, saturation);
};



// AnimLight

class AnimLight : public Light
{
public:
	THIEF_OBJECT_TYPE (AnimLight)
	bool is_anim_light () const; //TESTME

	bool is_light_active () const; //TESTME
	void set_light_active (bool); //TESTME

	enum class Mode
	{
		FLIP, SLIDE, RANDOM,
		MINIMUM, MAXIMUM, ZERO,
		SMOOTH_BRIGHTEN, SMOOTH_DIM,
		RANDOM_COHERENT, FLICKER
	};
	Mode get_light_mode () const;
	void set_light_mode (Mode);

	void set_light_range (float minimum, float maximum); //TESTME

	void subscribe_light (); //TESTME
	void unsubscribe_light (); //TESTME
};



// DynamicLight

class DynamicLight : public Light
{
public:
	THIEF_OBJECT_TYPE (DynamicLight)
	bool is_dynamic_light () const;

	THIEF_PROP_FIELD (int, brightness);
};



// Flash

class Flash : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Flash)
	bool is_flash () const; //TESTME

	void flash_world (); //TESTME
};



// ParticleGroup

class ParticleGroup : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (ParticleGroup)
	bool is_particle_group () const; //TESTME

	void set_particles_active (bool); //TESTME
};



// TextureSwapper

class TextureSwapper : public Object
{
public:
	THIEF_OBJECT_TYPE (TextureSwapper)
	bool is_texture_swapper () const;

	THIEF_PROP_FIELD (String, old_texture);
	THIEF_PROP_FIELD (String, new_texture);
	THIEF_PROP_FIELD (float, swap_radius);

	bool swap_textures ();

	static bool swap_textures (const Vector& center, float radius,
		const String& old_texture, const String& new_texture);
};



} // namespace Thief

#endif // THIEF_RENDERING_HH
