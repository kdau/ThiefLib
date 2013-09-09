/******************************************************************************
 *  Rendering.cc
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
PROXY_ARRAY_CONFIG (Rendered, replacement_texture, 4u, String, "",
	PROXY_ARRAY_ITEM ("OTxtRepr0", nullptr),
	PROXY_ARRAY_ITEM ("OTxtRepr1", nullptr),
	PROXY_ARRAY_ITEM ("OTxtRepr2", nullptr),
	PROXY_ARRAY_ITEM ("OTxtRepr3", nullptr));
PROXY_ARRAY_CONFIG (Rendered, joint_position, 6u, float, 0.0f,
	PROXY_ARRAY_ITEM ("JointPos", "Joint 1"),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 2"),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 3"),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 4"),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 5"),
	PROXY_ARRAY_ITEM ("JointPos", "Joint 6"));
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
//TODO wrap rest of property: Renderer\Anim Light = AnimLight

PROXY_NEG_CONFIG (AnimLight, initially_on, "AnimLight", "inactive", bool, true);

OBJECT_TYPE_IMPL_ (AnimLight, Light (),
	PROXY_INIT (initially_on)
)

bool
AnimLight::is_anim_light () const
{
	return initially_on.exists ();
}

AnimLight::Mode
AnimLight::get_light_mode () const //TODO Replace with PropField?
{
	return Mode (SService<ILightScrSrv> (LG)->GetMode (number));
}

void
AnimLight::set_light_mode (Mode mode) //TODO Replace with PropField?
{
	SService<ILightScrSrv> (LG)->SetMode (number, int (mode));
}

void
AnimLight::set_light_range (float minimum, float maximum) //TODO Replace with PropField?
{
	SService<ILightScrSrv> (LG)->Set
		(number, int (get_light_mode ()), minimum, maximum);
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

OBJECT_TYPE_IMPL_ (DynamicLight, Light (),
	PROXY_INIT (brightness)
)

bool
DynamicLight::is_dynamic_light () const
{
	return brightness > 0;
}



//TODO Create StaticLight : Light and wrap property: Renderer\Light = Light



// Bitmapped

PROXY_CONFIG (Bitmapped, bitmap_size_x, "BitmapWorld", "x size (feet)",
	float, 0.0f);
PROXY_CONFIG (Bitmapped, bitmap_size_y, "BitmapWorld", "y size (feet)",
	float, 0.0f);
PROXY_NEG_BIT_CONFIG (Bitmapped, uv_tiling, "BitmapWorld", "Flags", 16u, false);
PROXY_CONFIG (Bitmapped, tile_size_x, "BitmapWorld", "x feet per tile",
	float, 0.0f);
PROXY_CONFIG (Bitmapped, tile_size_y, "BitmapWorld", "y feet per tile",
	float, 0.0f);
PROXY_BIT_CONFIG (Bitmapped, apply_lighting, "BitmapWorld", "Flags", 4u, false);
PROXY_CONFIG (Bitmapped, bitmap_color, "BitmapColor", nullptr, Color,
	Color (0xffffff)); //FIXME This is not setting correctly (only the red channel is taking effect).
PROXY_BIT_CONFIG (Bitmapped, double_sided, "BitmapWorld", "Flags", 1u, false);
PROXY_BIT_CONFIG (Bitmapped, flip_backside_uv, "BitmapWorld", "Flags", 2u, false);
PROXY_BIT_CONFIG (Bitmapped, face_camera, "BitmapWorld", "Flags", 8u, false);
PROXY_CONFIG (Bitmapped, axial_fade, "BitmapWorld", "Axial Fade", float, 0.0f);
PROXY_NEG_BIT_CONFIG (Bitmapped, animation_loop, "BitmapAnimation", "Flags",
	1u, false);

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
	PROXY_INIT (animation_loop)
)

bool
Bitmapped::is_bitmapped () const
{
	return bitmap_size_x.exists ();
}



// Flash
//TODO wrap rest of property: SFX\FlashBombInfo = RenderFlash

PROXY_CONFIG (Flash, world_duration, "RenderFlash", "world flash duration",
	Time, 0ul);
PROXY_CONFIG (Flash, screen_duration, "RenderFlash", "max screen duration (ms)",
	Time, 0ul);
PROXY_CONFIG (Flash, effect_duration, "RenderFlash", "after-effec duration (ms)",
	Time, 0ul);
PROXY_CONFIG (Flash, range, "RenderFlash", "range", float, 0.0f);

OBJECT_TYPE_IMPL_ (Flash,
	PROXY_INIT (world_duration),
	PROXY_INIT (screen_duration),
	PROXY_INIT (effect_duration),
	PROXY_INIT (range)
)

bool
Flash::is_flash () const
{
	return range.exists ();
}

void
Flash::flash_world ()
{
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
PROXY_CONFIG (ParticleGroup, particle_size, "ParticleGroup",
	"size of particle", float, 0.0f);

OBJECT_TYPE_IMPL_ (ParticleGroup,
	PROXY_INIT (particle_count),
	PROXY_INIT (particle_size)
)

bool
ParticleGroup::is_particle_group () const
{
	return particle_count.exists ();
}

void
ParticleGroup::set_particles_active (bool active) //TODO Replace with PropField?
{
	SService<IPGroupSrv> (LG)->SetActive (number, active);
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



/*TODO wrap the following properties, perhaps creating {Camera,Corona,Distortion}:
 * Renderer\Camera Overlay = CameraOverlay
 * Renderer\Corona = Corona
 * SFX\Heat Disks = HeatDisks
 */



} // namespace Thief

