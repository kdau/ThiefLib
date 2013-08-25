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



// Rendered
//TODO wrap property: Renderer\Alpha Blend Mode = BlendMode
//TODO wrap property: Renderer\Alpha Test Threshold = RendAlphaRef
//TODO wrap property: Renderer\Distance Alpha = DistAlpha
//TODO wrap property: Renderer\Render Order = RenderOrder

#undef OBJECT_TYPE
#define OBJECT_TYPE Rendered

PF_CONFIG (render_type, "RenderType", nullptr, Rendered::RenderType,
	Rendered::RenderType::NORMAL);
PF_CONFIG (z_bias, "Z-Bias", nullptr, int, 0);
PF_CONFIG (opacity, "RenderAlpha", nullptr, float, 1.0f);
PF_CONFIG (model, "ModelName", "", String, "");
PF_CONFIG (model_scale, "Scale", "", Vector, Vector (1.0f, 1.0f, 1.0f));
PF_CONFIG (replacement_texture_0, "OTxtRepr0", nullptr, String, "");
PF_CONFIG (replacement_texture_1, "OTxtRepr1", nullptr, String, "");
PF_CONFIG (replacement_texture_2, "OTxtRepr2", nullptr, String, "");
PF_CONFIG (replacement_texture_3, "OTxtRepr3", nullptr, String, "");
PF_CONFIG (joint_position_1, "JointPos", "Joint 1", float, 0.0f);
PF_CONFIG (joint_position_2, "JointPos", "Joint 2", float, 0.0f);
PF_CONFIG (joint_position_3, "JointPos", "Joint 3", float, 0.0f);
PF_CONFIG (joint_position_4, "JointPos", "Joint 4", float, 0.0f);
PF_CONFIG (joint_position_5, "JointPos", "Joint 5", float, 0.0f);
PF_CONFIG (joint_position_6, "JointPos", "Joint 6", float, 0.0f);
PF_CONFIG (extra_light, "ExtraLight", "Amount (-1..1)", float, 0.0f);
PF_CONFIG (extra_light_additive, "ExtraLight", "Additive?", bool, false);
PF_CONFIG (self_illumination, "SelfIllum", nullptr, float, 0.0f);
PF_CONFIG (static_shadow, "Immobile", nullptr, bool, false);
PF_CONFIG (force_static_shadow, "StatShad", nullptr, bool, false);
PF_CONFIG (dynamic_shadow, "Shadow", nullptr, int, 0);
PF_CONFIG (visibility_shadow, "ObjShad", nullptr, bool, false);

OBJECT_TYPE_IMPL_ (Rendered,
	PF_INIT (render_type),
	PF_INIT (z_bias),
	PF_INIT (opacity),
	PF_INIT (model),
	PF_INIT (model_scale),
	PF_INIT (replacement_texture_0),
	PF_INIT (replacement_texture_1),
	PF_INIT (replacement_texture_2),
	PF_INIT (replacement_texture_3),
	PF_INIT (joint_position_1),
	PF_INIT (joint_position_2),
	PF_INIT (joint_position_3),
	PF_INIT (joint_position_4),
	PF_INIT (joint_position_5),
	PF_INIT (joint_position_6),
	PF_INIT (extra_light),
	PF_INIT (extra_light_additive),
	PF_INIT (self_illumination),
	PF_INIT (static_shadow),
	PF_INIT (force_static_shadow),
	PF_INIT (dynamic_shadow),
	PF_INIT (visibility_shadow)
)



// Light
//TODO wrap property: Renderer\Spotlight = Spotlight
//TODO wrap property: Renderer\SpotlightAndAmbient = SpotAmb

#undef OBJECT_TYPE
#define OBJECT_TYPE Light

PF_CONFIG (hue, "LightColor", "hue", float, 0.0f);
PF_CONFIG (saturation, "LightColor", "saturation", float, 0.0f);

OBJECT_TYPE_IMPL_ (Light,
	PF_INIT (hue),
	PF_INIT (saturation)
)



// AnimLight
//TODO wrap property: Renderer\Anim Light = AnimLight (propdefs.h: sAnimLight) (Can some of the service methods be replaced with PropFields?)

OBJECT_TYPE_IMPL (AnimLight)

bool
AnimLight::is_anim_light () const
{
	return Property (*this, "AnimLight").exists ();
}

bool
AnimLight::is_light_active () const
{
	return !Property (*this, "AnimLight").get_field ("inactive", true);
}

void
AnimLight::set_light_active (bool active)
{
	if (active)
		SService<ILightScrSrv> (LG)->Activate (number);
	else
		SService<ILightScrSrv> (LG)->Deactivate (number);
}

AnimLight::Mode
AnimLight::get_light_mode () const
{
	return Mode (SService<ILightScrSrv> (LG)->GetMode (number));
}

void
AnimLight::set_light_mode (Mode mode)
{
	SService<ILightScrSrv> (LG)->SetMode (number, int (mode));
}

void
AnimLight::set_light_range (float minimum, float maximum)
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

#undef OBJECT_TYPE
#define OBJECT_TYPE DynamicLight

PF_CONFIG (brightness, "SelfLit", nullptr, int, 0);

OBJECT_TYPE_IMPL_ (DynamicLight,
	PF_INIT (brightness)
)

bool
DynamicLight::is_dynamic_light () const
{
	return brightness > 0;
}



//TODO Create StaticLight : Light and wrap property: Renderer\Light = Light



// Flash
//TODO wrap property: SFX\FlashBombInfo = RenderFlash (propdefs.h: sRenderFlash)

OBJECT_TYPE_IMPL (Flash)

bool
Flash::is_flash () const
{
	return Property (*this, "ParticleGroup").exists (); //TODO use a PropField
}

void
Flash::flash_world ()
{
	SService<IDarkPowerupsSrv> (LG)->TriggerWorldFlash (number);
}



// ParticleGroup
//TODO wrap property: SFX\Particles = ParticleGroup
//TODO wrap property: SFX\Particle Launch Info = PGLaunchInfo
//TODO wrap property: SFX\FrameAnimationConfig = FrameAniConfig
//TODO wrap property: SFX\FrameAnimationState = FrameAniState
//TODO wrap link: ParticleAttachement - sParticleAttachLinkData

OBJECT_TYPE_IMPL (ParticleGroup)

bool
ParticleGroup::is_particle_group () const
{
	return Property (*this, "ParticleGroup").exists (); //TODO use a PropField
}

void
ParticleGroup::set_particles_active (bool active)
{
	SService<IPGroupSrv> (LG)->SetActive (number, active);
}



//TODO wrap link: DetailAttachement - sDetailAttachLinkData



// TextureSwapper

#undef OBJECT_TYPE
#define OBJECT_TYPE TextureSwapper

PF_CONFIG (old_texture, "TerrRepOff", nullptr, String, "");
PF_CONFIG (new_texture, "TerrRepOn", nullptr, String, "");
PF_CONFIG (swap_radius, "TextureRadius", nullptr, float, 0.0f);

OBJECT_TYPE_IMPL_ (TextureSwapper,
	PF_INIT (old_texture),
	PF_INIT (new_texture),
	PF_INIT (swap_radius)
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



/*TODO wrap the following properties, perhaps creating {Bitmapped,Camera,Corona,Distortion}
 * Renderer\Bitmap Animation = BitmapAnimation
 * Renderer\Bitmap Color = BitmapColor
 * Renderer\Bitmap Worldspace = BitmapWorld
 * Renderer\Camera Overlay = CameraOverlay
 * Renderer\Corona = Corona
 * SFX\Heat Disks = HeatDisks
 */



} // namespace Thief

