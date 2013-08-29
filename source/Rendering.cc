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

PROXY_CONFIG (Rendered, render_type, "RenderType", nullptr,
	Rendered::RenderType, RenderType::NORMAL);
PROXY_CONFIG (Rendered, z_bias, "Z-Bias", nullptr, int, 0);
PROXY_CONFIG (Rendered, opacity, "RenderAlpha", nullptr, float, 1.0f);
PROXY_CONFIG (Rendered, model, "ModelName", "", String, "");
PROXY_CONFIG (Rendered, model_scale, "Scale", "",
	Vector, Vector (1.0f, 1.0f, 1.0f));
PROXY_CONFIG (Rendered, replacement_texture_0, "OTxtRepr0", nullptr, String, "");
PROXY_CONFIG (Rendered, replacement_texture_1, "OTxtRepr1", nullptr, String, "");
PROXY_CONFIG (Rendered, replacement_texture_2, "OTxtRepr2", nullptr, String, "");
PROXY_CONFIG (Rendered, replacement_texture_3, "OTxtRepr3", nullptr, String, "");
PROXY_CONFIG (Rendered, joint_position_1, "JointPos", "Joint 1", float, 0.0f);
PROXY_CONFIG (Rendered, joint_position_2, "JointPos", "Joint 2", float, 0.0f);
PROXY_CONFIG (Rendered, joint_position_3, "JointPos", "Joint 3", float, 0.0f);
PROXY_CONFIG (Rendered, joint_position_4, "JointPos", "Joint 4", float, 0.0f);
PROXY_CONFIG (Rendered, joint_position_5, "JointPos", "Joint 5", float, 0.0f);
PROXY_CONFIG (Rendered, joint_position_6, "JointPos", "Joint 6", float, 0.0f);
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
	PROXY_INIT (replacement_texture_0),
	PROXY_INIT (replacement_texture_1),
	PROXY_INIT (replacement_texture_2),
	PROXY_INIT (replacement_texture_3),
	PROXY_INIT (joint_position_1),
	PROXY_INIT (joint_position_2),
	PROXY_INIT (joint_position_3),
	PROXY_INIT (joint_position_4),
	PROXY_INIT (joint_position_5),
	PROXY_INIT (joint_position_6),
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
//TODO wrap property: Renderer\Anim Light = AnimLight (propdefs.h: sAnimLight) (Can some of the service methods be replaced with PropFields?)

OBJECT_TYPE_IMPL_ (AnimLight, Light ())

bool
AnimLight::is_anim_light () const
{
	return Property (*this, "AnimLight").exists (); //TODO Use a PropField.
}

bool
AnimLight::is_light_active () const
{
	return !Property (*this, "AnimLight").get_field ("inactive", true); //TODO Use a PropField.
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



/*TODO Create FlowGroup and wrap these properties:
 * Renderer\Flow Group = FlowGroup (hidden type indicator?)
 * Renderer\Water Flow Color Index = FlowColor
 * Renderer\Water Texture Color = WaterColor
 */



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



/*TODO wrap the following properties, perhaps creating {Bitmapped,Camera,Corona,Distortion}
 * Renderer\Bitmap Animation = BitmapAnimation
 * Renderer\Bitmap Color = BitmapColor
 * Renderer\Bitmap Worldspace = BitmapWorld
 * Renderer\Camera Overlay = CameraOverlay
 * Renderer\Corona = Corona
 * SFX\Heat Disks = HeatDisks
 */



} // namespace Thief

