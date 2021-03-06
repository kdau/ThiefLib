/******************************************************************************
 *  Rendering.cc
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



// Rendered
//TODO wrap property: Renderer\Alpha Blend Mode = BlendMode
//TODO wrap property: Renderer\Alpha Test Threshold = RendAlphaRef
//TODO wrap property: Renderer\Distance Alpha = DistAlpha
//TODO wrap property: Renderer\Render Order = RenderOrder

PROXY_CONFIG (Rendered, render_type, "RenderType", nullptr,
	Rendered::RenderType, RenderType::NORMAL);
PROXY_CONFIG (Rendered, z_bias, "Z-Bias", nullptr, int, 0);
PROXY_CONFIG (Rendered, opacity, "RenderAlpha", nullptr, float, 1.0f);
PROXY_CONFIG (Rendered, model, "ModelName", "", String, "");
PROXY_CONFIG (Rendered, model_scale, "Scale", "",
	Vector, Vector (1.0f, 1.0f, 1.0f));
PROXY_ARRAY_CONFIG (Rendered, replacement_texture, 4u, String,
	PROXY_ARRAY_ITEM ("OTxtRepr0", nullptr, ""),
	PROXY_ARRAY_ITEM ("OTxtRepr1", nullptr, ""),
	PROXY_ARRAY_ITEM ("OTxtRepr2", nullptr, ""),
	PROXY_ARRAY_ITEM ("OTxtRepr3", nullptr, ""));
PROXY_ARRAY_CONFIG (Rendered, joint_position, 6u, float,
	PROXY_ARRAY_ITEM ("JointPos", "Joint 1", 0.0f),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 2", 0.0f),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 3", 0.0f),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 4", 0.0f),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 5", 0.0f),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 6", 0.0f));
PROXY_CONFIG (Rendered, extra_light, "ExtraLight", "Amount (-1..1)",
	float, 0.0f);
PROXY_CONFIG (Rendered, extra_light_additive, "ExtraLight", "Additive?",
	bool, false);
PROXY_CONFIG (Rendered, self_illumination, "SelfIllum", nullptr, float, 0.0f);
PROXY_CONFIG (Rendered, static_shadow, "Immobile", nullptr, bool, false);
PROXY_CONFIG (Rendered, force_static_shadow, "StatShad", nullptr, bool, false);
PROXY_CONFIG (Rendered, dynamic_shadow, "Shadow", nullptr, int, 0);
PROXY_CONFIG (Rendered, visibility_shadow, "ObjShad", nullptr, bool, false);

OBJECT_TYPE_IMPL_ (Rendered,
	PROXY_INIT (render_type),
	PROXY_INIT (z_bias),
	PROXY_INIT (opacity),
	PROXY_INIT (model),
	PROXY_INIT (model_scale),
	PROXY_ARRAY_INIT (replacement_texture, 4),
	PROXY_ARRAY_INIT (joint_position, 6),
	PROXY_INIT (extra_light),
	PROXY_INIT (extra_light_additive),
	PROXY_INIT (self_illumination),
	PROXY_INIT (static_shadow),
	PROXY_INIT (force_static_shadow),
	PROXY_INIT (dynamic_shadow),
	PROXY_INIT (visibility_shadow)
)



// Light
//TODO wrap property: Renderer\Spotlight = Spotlight
//TODO wrap property: Renderer\SpotlightAndAmbient = SpotAmb

PROXY_CONFIG (Light, hue, "LightColor", "hue", float, 0.0f);
PROXY_CONFIG (Light, saturation, "LightColor", "saturation", float, 0.0f);

OBJECT_TYPE_IMPL_ (Light,
	PROXY_INIT (hue),
	PROXY_INIT (saturation)
)



// AnimLight
//TODO Wrap "Dynamic Light" field of "AnimLight" property, possibly making some other fields non-const in support (or subclassing).

PROXY_CONFIG (AnimLight, light_mode, "AnimLight", "Mode",
	AnimLight::Mode, Mode::FLIP);
PROXY_CONFIG (AnimLight, rise_interval, "AnimLight", "millisecs to brighten",
	Time, 0ul);
PROXY_CONFIG (AnimLight, fall_interval, "AnimLight", "millisecs to dim",
	Time, 0ul);
PROXY_CONV_CONFIG (AnimLight, min_brightness, "AnimLight", "min brightness",
	int, 0, float);
PROXY_CONV_CONFIG (AnimLight, max_brightness, "AnimLight", "max brightness",
	int, 0, float);
PROXY_CONFIG (AnimLight, outer_radius, "AnimLight", "radius (0 for infinite)",
	float, 0.0f);
PROXY_CONFIG (AnimLight, inner_radius, "AnimLight", "inner radius (0 for none)",
	float, 0.0f);
PROXY_CONFIG (AnimLight, light_offset, "AnimLight", "offset from object",
	Vector, Vector ());
PROXY_CONFIG (AnimLight, soft_shadows, "AnimLight", "quad lit", bool, false);
PROXY_NEG_CONFIG (AnimLight, active, "AnimLight", "inactive", bool, true);
PROXY_CONV_CONFIG (AnimLight, state, "AnimLight", "currently rising?",
	AnimLight::State, State::FALLING, bool);
PROXY_CONFIG (AnimLight, elapsed_time, "AnimLight", "current countdown",
	Time, 0ul);

OBJECT_TYPE_IMPL_ (AnimLight, Light (),
	PROXY_INIT (light_mode),
	PROXY_INIT (rise_interval),
	PROXY_INIT (fall_interval),
	PROXY_INIT (min_brightness),
	PROXY_INIT (max_brightness),
	PROXY_INIT (outer_radius),
	PROXY_INIT (inner_radius),
	PROXY_INIT (light_offset),
	PROXY_INIT (soft_shadows),
	PROXY_INIT (active),
	PROXY_INIT (state),
	PROXY_INIT (elapsed_time)
)

bool
AnimLight::is_anim_light () const
{
	return light_mode.exists ();
}

void
AnimLight::subscribe_light ()
{
	SService<ILightScrSrv> (LG)->Subscribe (number);
}

void
AnimLight::unsubscribe_light ()
{
	SService<ILightScrSrv> (LG)->Unsubscribe (number);
}



// DynamicLight

PROXY_CONFIG (DynamicLight, brightness, "SelfLit", nullptr, int, 0);
PROXY_CONFIG (DynamicLight, radius, "SelfLitRad", nullptr, float, 10.0);

OBJECT_TYPE_IMPL_ (DynamicLight, Light (),
	PROXY_INIT (brightness),
	PROXY_INIT (radius)
)

bool
DynamicLight::is_dynamic_light () const
{
	return brightness > 0;
}



// StaticLight

PROXY_CONV_CONFIG (StaticLight, brightness, "Light", "brightness",
	int, 0, float);
PROXY_CONFIG (StaticLight, outer_radius, "Light", "radius (0 for infinite)",
	float, 0.0f);
PROXY_CONFIG (StaticLight, inner_radius, "Light", "inner radius (0 for none)",
	float, 0.0f);
PROXY_CONFIG (StaticLight, light_offset, "Light", "offset from object",
	Vector, Vector ());
PROXY_CONFIG (StaticLight, soft_shadows, "Light", "quad lit", bool, false);

OBJECT_TYPE_IMPL_ (StaticLight, Light (),
	PROXY_INIT (brightness),
	PROXY_INIT (outer_radius),
	PROXY_INIT (inner_radius),
	PROXY_INIT (light_offset),
	PROXY_INIT (soft_shadows)
)

bool
StaticLight::is_static_light () const
{
	return brightness.exists ();
}



// Bitmapped

PROXY_CONFIG (Bitmapped, bitmap_size_x, "BitmapWorld", "x size (feet)",
	float, 0.0f);
PROXY_CONFIG (Bitmapped, bitmap_size_y, "BitmapWorld", "y size (feet)",
	float, 0.0f);
PROXY_NEG_BIT_CONFIG (Bitmapped, uv_tiling, "BitmapWorld", "Flags", 16u, true);
PROXY_CONFIG (Bitmapped, tile_size_x, "BitmapWorld", "x feet per tile",
	float, 0.0f);
PROXY_CONFIG (Bitmapped, tile_size_y, "BitmapWorld", "y feet per tile",
	float, 0.0f);
PROXY_BIT_CONFIG (Bitmapped, apply_lighting, "BitmapWorld", "Flags", 4u, false);
PROXY_CONFIG (Bitmapped, bitmap_color, "BitmapColor", nullptr, Color,
	Color (0xffffff)); // see assignment operator specialization below
PROXY_BIT_CONFIG (Bitmapped, double_sided, "BitmapWorld", "Flags", 1u, false);
PROXY_BIT_CONFIG (Bitmapped, flip_backside_uv, "BitmapWorld", "Flags", 2u, false);
PROXY_BIT_CONFIG (Bitmapped, face_camera, "BitmapWorld", "Flags", 8u, false);
PROXY_CONFIG (Bitmapped, axial_fade, "BitmapWorld", "Axial Fade", float, 0.0f);
PROXY_CONFIG_ (Bitmapped, scroll_speed, "ConveyorVel", nullptr, float, 0.0f,
	Vector::Component::X, FieldProxyConfig<float>::component_getter,
	FieldProxyConfig<float>::component_setter);
PROXY_NEG_BIT_CONFIG (Bitmapped, animation_loop, "BitmapAnimation", "Flags",
	1u, true);

OBJECT_TYPE_IMPL_ (Bitmapped, Rendered (),
	PROXY_INIT (bitmap_size_x),
	PROXY_INIT (bitmap_size_y),
	PROXY_INIT (uv_tiling),
	PROXY_INIT (tile_size_x),
	PROXY_INIT (tile_size_y),
	PROXY_INIT (apply_lighting),
	PROXY_INIT (bitmap_color),
	PROXY_INIT (double_sided),
	PROXY_INIT (flip_backside_uv),
	PROXY_INIT (face_camera),
	PROXY_INIT (axial_fade),
	PROXY_INIT (scroll_speed),
	PROXY_INIT (animation_loop)
)

bool
Bitmapped::is_bitmapped () const
{
	return bitmap_size_x.exists ();
}

// This specialization works around a bizarre issue with the BitmapColor
// property that causes it to ignore the green and blue channels when set
// through an LGMulti of type INT.
template<>
PropField<Color, &Bitmapped::F_bitmap_color>&
PropField<Color, &Bitmapped::F_bitmap_color>::operator = (const Color& value)
{
	set_raw (object, "BitmapColor", reinterpret_cast<void*> (long (value)));
	return *this;
}



//TODO Create Corona and wrap property: Renderer\Corona = Corona



//TODO Create Distortion and wrap property: SFX\Heat Disks = HeatDisks (including NewDark-added "Z-compare" field)



// FlashConfig

//TODO wrap "flash color r", "flash color b", "flash color g" (how?)
PROXY_CONFIG (FlashConfig, world_duration, "RenderFlash",
	"world flash duration", Time, 0ul);
PROXY_CONFIG (FlashConfig, screen_duration, "RenderFlash",
	"max screen duration (ms)", Time, 0ul);
PROXY_CONFIG (FlashConfig, effect_duration, "RenderFlash",
	"after-effect duration (ms)", Time, 0ul);
PROXY_CONFIG (FlashConfig, range, "RenderFlash", "range", float, 0.0f);

OBJECT_TYPE_IMPL_ (FlashConfig,
	PROXY_INIT (world_duration),
	PROXY_INIT (screen_duration),
	PROXY_INIT (effect_duration),
	PROXY_INIT (range)
)

bool
FlashConfig::is_flash_config () const
{
	return get_type () == Type::ARCHETYPE && range.exists ();
}



// FlashPoint

OBJECT_TYPE_IMPL (FlashPoint)

bool
FlashPoint::is_flash_point () const
{
	return get_type () == Type::CONCRETE && Link::get_any ("RenderFlash",
		get_archetype (), ANY, Link::Inheritance::SOURCE).exists ();
}

void
FlashPoint::flash ()
{
	if (!is_flash_point ()) throw std::runtime_error ("not a flash point");
	SService<IDarkPowerupsSrv> (LG)->TriggerWorldFlash (number);
}



/*TODO Create FlowGroup and wrap these properties:
 * Renderer\Flow Group = FlowGroup (hidden type indicator?)
 * Renderer\Water Flow Color Index = FlowColor
 * Renderer\Water Texture Color = WaterColor
 */



// ParticleGroup
//TODO wrap rest of property: SFX\Particles = ParticleGroup
//TODO wrap property: SFX\Particle Launch Info = PGLaunchInfo
//TODO wrap property: SFX\FrameAnimationConfig = FrameAniConfig
//TODO wrap property: SFX\FrameAnimationState = FrameAniState
//TODO wrap link: ParticleAttachement - sParticleAttachLinkData

PROXY_CONFIG (ParticleGroup, particle_count, "ParticleGroup",
	"number of particles", int, 0);
PROXY_CONFIG (ParticleGroup, particle_size, "ParticleGroup", "size of particle",
	float, 0.0f);
PROXY_CONFIG (ParticleGroup, active, "ParticleGroup", "Active", bool, false);

OBJECT_TYPE_IMPL_ (ParticleGroup,
	PROXY_INIT (particle_count),
	PROXY_INIT (particle_size),
	PROXY_INIT (active)
)

bool
ParticleGroup::is_particle_group () const
{
	return particle_count.exists ();
}



// TextureSwapper

PROXY_CONFIG (TextureSwapper, old_texture, "TerrRepOff", nullptr, String, "");
PROXY_CONFIG (TextureSwapper, new_texture, "TerrRepOn", nullptr, String, "");
PROXY_CONFIG (TextureSwapper, swap_radius, "TextureRadius", nullptr,
	float, 0.0f);

OBJECT_TYPE_IMPL_ (TextureSwapper,
	PROXY_INIT (old_texture),
	PROXY_INIT (new_texture),
	PROXY_INIT (swap_radius)
)

bool
TextureSwapper::is_texture_swapper () const
{
	return old_texture.exists () && new_texture.exists ();
}

bool
TextureSwapper::swap_textures ()
{
	return SService<IAnimTextureSrv> (LG)->ChangeTexture (number,
		nullptr, String (old_texture).data (),
		nullptr, String (new_texture).data ())
			== S_OK;
}

bool
TextureSwapper::swap_textures (const Vector& _center, float radius,
	const String& old_texture, const String& new_texture)
{
	TextureSwapper swapper = Object::create_temp_fnord ();
	swapper.set_location (_center);
	swapper.swap_radius = radius;
	return SService<IAnimTextureSrv> (LG)->ChangeTexture (swapper.number,
		nullptr, old_texture.data (), nullptr, new_texture.data ())
			== S_OK;
}



} // namespace Thief

