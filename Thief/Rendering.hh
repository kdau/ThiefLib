//! \file Rendering.hh Visual rendering of objects, lights, and special effects.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_RENDERING_HH
#define THIEF_RENDERING_HH

#include <Thief/Base.hh>
#include <Thief/Object.hh>
#include <Thief/Property.hh>

namespace Thief {



/*! Base class for objects that are visible in the game world.
 * This includes only some types of visual special effects. See specific classes
 * of effect for more information. */
class Rendered : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Rendered)

	//! A type of visual rendering of an object.
	enum class RenderType
	{
		NORMAL, //!< The object is rendered and subject to lighting.
		NONE,   //!< The object is not rendered at all.
		UNLIT,  /*!< The object is rendered, but it is lit at maximum
		         * brightness without regard to surrounding lighting. */
		EDITOR_ONLY, /*!< The object is rendered and subject to lighting
		              * in the editor, including the editor's game mode,
		              * but it is not rendered by the game application. */
		CORONA_ONLY  /*!< The object itself is not rendered, but any
		              * corona on the object will be rendered. */
	};

	//! How and when the object is rendered visually.
	THIEF_PROP_FIELD (RenderType, render_type);

	//! The object's rendering priority in case of Z-fighting.
	THIEF_PROP_FIELD (int, z_bias);

	/*! How opaque or transparent the object appears.
	 * At \c 0.0, the object is invisible; at \c 1.0, nothing behind it can
	 * be seen through it. */
	THIEF_PROP_FIELD (float, opacity);

	//! The name of the rendering model that represents the object.
	THIEF_PROP_FIELD (String, model);

	/*! How much to resize an object's rendering model along each axis.
	 * This value does not affect the object's shadow (if any) or its
	 * physics model (see OBBPhysical::physics_size or
	 * SpherePhysical::physics_radius instead). */
	THIEF_PROP_FIELD (Vector, model_scale);

	/*! Filesystem paths to textures to be applied to the object's rendering
	 * model. The model must have designated surfaces to receive these
	 * textures by using the \c replace0, \c replace1, \c replace2, and/or
	 * \c replace3 placeholder textures. */
	THIEF_PROP_FIELD_ARRAY (String, replacement_texture, 4u);

	/*! The current positions of joints in the object's rendering model.
	 * An object can have up to six moveable joints. Each joint can be
	 * rotating, for which the position in an angle in degress, or
	 * translating, for which the position is in feet (DU). */
	THIEF_PROP_FIELD_ARRAY (float, joint_position, 6u);

	/*! An amount of additional light to be cast on the object.
	 * This amount can range from \c -1.0 (black) to \c 1.0 (fully bright).
	 * If #extra_light_additive is \c true, this amount is in addition to
	 * (or subtraction from) the natural surrounding lighting; if that field
	 * is \c false, this amount is a fixed lighting level for the object
	 * irrespective of surrounding lighting. */
	THIEF_PROP_FIELD (float, extra_light);

	//! Whether the #extra_light should add to or replace the natural light.
	THIEF_PROP_FIELD (bool, extra_light_additive);

	/*! How much to light any model surfaces designated as self-illuminated.
	 * These surfaces, usually light sources such as bulbs, interior-lit
	 * windows, etc., are given a level of self-illumination in a model
	 * file. This field controls how much of their possible self-illumination
	 * those surfaces receive, ranging from \c 0.0 (none) to \c 1.0 (full). */
	THIEF_PROP_FIELD (float, self_illumination); //TESTME

	/*! Whether the object is immobile and thus suitable to cast a shadow.
	 * Most Dark %Engine shadows are precalculated when lighting is
	 * processed, so mobile objects generally do not cast shadows. */
	THIEF_PROP_FIELD_CONST (bool, static_shadow); //TESTME

	/*! Whether to cast a shadow even though the object is mobile.
	 * The shadow will still be precalculated and static, so the object
	 * should only move in a way that would not affect the shadow it casts. */
	THIEF_PROP_FIELD_CONST (bool, force_static_shadow); //TESTME

	/*! How large of a mobile shadow to cast from the object.
	 * These shadows have an invariable round shape, while their size is
	 * controlled by this value. Due to a Dark %Engine bug, dynamic shadows
	 * have unusual colored spots that shift as objects move. */
	THIEF_PROP_FIELD (int, dynamic_shadow);

	//! Whether the object's shadow should affect the visibility of beings.
	THIEF_PROP_FIELD (bool, visibility_shadow); //TESTME
};



/*! Base class for objects that illuminate surrounding objects and terrain.
 * For dynamic lights, these two fields are fully variable. For non-dynamic
 * animated lights, changes to these fields will affect the light cast on
 * objects, but not that cast on terrain. For static lights, these fields should
 * be considered constant. */
class Light : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Light)

	/*! The hue of any light emitted by the object.
	 * This value ranges from red (\c 0.0) through green (\c 0.333) and blue
	 * (\c 0.667) back to red (\c 1.0) again. See the class description for
	 * details on constness. */
	THIEF_PROP_FIELD (float, hue);

	/*! The intensity of color in any light emitted by the object.
	 * This value ranges from white (\c 0.0) to fully saturated with color
	 * (\c 1.0). See the class description for details on constness. */
	THIEF_PROP_FIELD (float, saturation);
};



/*! An object that casts a light of variable brightness but fixed position.
 * Animated lights can vary their brightness in any of ten patterns (#Mode)
 * between minimum and maximum levels. Most other aspects of their light are
 * fixed when a mission's lighting is processed in the editor, however.
 *
 * Each animation #Mode is a pattern of change between brightness levels.
 * Depending on the mode, these changes may be constrained by #max_brightness,
 * #min_brightness, and/or zero brightness. Changes involve two states:
 * State::FALLING towards the minimum or zero, and State::RISING towards the
 * maximum. Each state takes place over an interval: the #fall_interval and the
 * #rise_interval, respectively. The animation may be #active (in progress) or
 * inactive (frozen at a particular brightness level).
 *
 * An animated light can also be influenced by scripts, generally along the
 * lines of the stock \c %AnimLight script. This adds a second layer of state:
 * the light can be "on" or "off" depending on whether it has most recently
 * received a \c TurnOn or \c TurnOff message. This is separate from whether
 * the animation is currently in progress (#active). Each mode is associated
 * with an expected "off" mode, usually Mode::ZERO. These behaviors are not
 * inherent to the engine, however. */
class AnimLight : public Light
{
public:
	THIEF_OBJECT_TYPE (AnimLight)

	//! Returns whether the object is configured to emit animated light.
	bool is_anim_light () const;

	/*! A pattern of changes in a light's brightness.
	 * Except as specified, all modes are expected to have zero brightness
	 * when "turned off" by the \c %AnimLight script or a similar one. */
	enum class Mode
	{
		FLIP,    //!< Jumps between minimum and maximum at intervals.
		SLIDE,   /*!< Gradually changes from minimum to maximum and vice
		          * versa over alternating intervals. */
		RANDOM,  /*!< Jumps to random values between the minimum and
		          * and maximum at intervals. */
		MINIMUM, /*!< Minimum when turned off (as initially), maximum
		          * when turned on; no animation. */
		MAXIMUM, /*!< Maximum when turned on (as initially), zero when
		          * turned off (minimum is ignored); no animation. */
		ZERO,    /*!< Zero when turned off (as initially), maximum when
		          * turned on (minimum is ignored); no animation. */
		SMOOTH_BRIGHTEN, /*!< Gradually changes from minimum to maximum
		                  * when turned on (as initially), then stops.
		                  * Gradually changes to minimum when turned off. */
		SMOOTH_DIM,      /*!< Gradually changes from maximum to minimum
		                  * when turned off (as initially), then stops.
		                  * Gradually changes to maximum when turned on. */
		RANDOM_COHERENT, /*!< Gradually changes to a new random value
		                  * between the minimum and maximum at each
		                  * interval. */
		FLICKER  /*!< Jumps between minimum and maximum at semi-random
		          * intervals influenced by the given intervals. */
	};

	//! The pattern of animation that the light exhibits.
	THIEF_PROP_FIELD (Mode, light_mode);

	//! How long it takes the light to reach its maximum brightness.
	THIEF_PROP_FIELD (Time, rise_interval);

	//! How long it takes the light to reach its minimum brightness.
	THIEF_PROP_FIELD (Time, fall_interval);

	/*! The least possible intensity of light emitted from the object.
	 * In some modes, it is possible for the object to emit no light at all
	 * (zero brightness) at times. */
	THIEF_PROP_FIELD (int, min_brightness);

	//! The greatest possible intensity of light emitted from the object.
	THIEF_PROP_FIELD (int, max_brightness);

	//! The radius of the area receiving any illumination from the light.
	THIEF_PROP_FIELD_CONST (float, outer_radius); //TESTME

	//! The radius of the area receiving the full brightness of the light.
	THIEF_PROP_FIELD_CONST (float, inner_radius); //TESTME

	//! At what offset from the object the light will be emitted.
	THIEF_PROP_FIELD_CONST (Vector, light_offset); //TESTME

	//! Whether quad lighting should be applied to create softer shadows.
	THIEF_PROP_FIELD_CONST (bool, soft_shadows); //TESTME

	/*! Whether the animation is currently playing.
	 * Note that this does not reflect whether the light is turned "on" or
	 * "off" by scripts (see the class description). When \c false, the
	 * animation is frozen at its current brightness level. */
	THIEF_PROP_FIELD (bool, active);

	//! A direction of change in a light's brightness.
	enum class State
	{
		FALLING, //!< Changing toward minimum or zero brightness.
		RISING   //!< Changing toward maximum brightness.
	};

	/*! The direction in which the light's brightness is currently changing.
	 * The intepretation of this state depends on the #light_mode. */
	THIEF_PROP_FIELD_CONST (State, state); //TESTME; non-const?

	/*! The time elapsed in the current interval. If the light is falling,
	 * this value is counting up towards #fall_interval; if it is rising, it
	 * is counting up towards #rise_interval. */
	THIEF_PROP_FIELD_CONST (Time, elapsed_time); //TESTME; non-const?

	//! Subscribes to \c LightChange messages on the object.
	void subscribe_light (); //TESTME

	//! Unsubscribes from \c LightChange messages on the object.
	void unsubscribe_light (); //TESTME
};



/*! An object that casts a moving light which follows its position.
 * Dynamic lights lack some of the features of static and animated lights, but
 * they are not fixed at one position in the world. They can be created and
 * destroyed during the simulation, and they track the position of the objects
 * they are emitted from. */
class DynamicLight : public Light
{
public:
	THIEF_OBJECT_TYPE (DynamicLight)

	//! Returns whether the object emits light dynamically.
	bool is_dynamic_light () const;

	//! The intensity of the light emitted from the object.
	THIEF_PROP_FIELD (int, brightness);

	/*! The radius of the area receiving any illumination from the light.
	 * \pre This property is only implemented by NewDark version 1.22 or
	 * later. */
	THIEF_PROP_FIELD (float, radius);
};



/*! An object that casts a fixed, unchanging light on its surroundings.
 * Static lights are computed entirely during the lighting process in the editor,
 * so all their properties are constant. They do not change in brightness, color,
 * position, or any other aspect during the simulation. Indeed, the object that
 * hosted the light during lighting need not continue to exist or to have these
 * properties, so they are not guaranteed to represent the conditions at the
 * time the mission's lighting was processed. */
class StaticLight : public Light
{
public:
	THIEF_OBJECT_TYPE (StaticLight)

	//! Returns whether the object is configured as a static light.
	bool is_static_light () const; //TESTME

	//! The intensity of the light emitted from the object.
	THIEF_PROP_FIELD_CONST (int, brightness); //TESTME

	//! The radius of the area receiving any illumination from the light.
	THIEF_PROP_FIELD_CONST (float, outer_radius); //TESTME

	//! The radius of the area receiving the full brightness of the light.
	THIEF_PROP_FIELD_CONST (float, inner_radius); //TESTME

	//! At what offset from the object the light will be emitted.
	THIEF_PROP_FIELD_CONST (Vector, light_offset); //TESTME

	//! Whether quad lighting should be applied to create softer shadows.
	THIEF_PROP_FIELD_CONST (bool, soft_shadows); //TESTME
};



/*! An object that is rendered as a flat bitmap.
 * %Bitmapped objects are primarily useful for their ability to be tiled with
 * the tiles moving in a loop, such as on a conveyor belt. In addition to the
 * properties here, a bitmapped object is identified by having a Rendered::model
 * that references a 2D bitmap image instead of a 3D object model. */
class Bitmapped : public virtual Rendered
{
public:
	THIEF_OBJECT_TYPE (Bitmapped)

	/*! Returns whether the object is configured for bitmap rendering.
	 * This checks for the existence of the bitmap properties, not whether
	 * the object's Rendered::model is a bitmap image. */
	bool is_bitmapped () const;

	//! The size in feet (DU) along the X axis of the entire bitmap.
	THIEF_PROP_FIELD (float, bitmap_size_x); //TESTME

	//! The size in feet (DU) along the Y axis of the entire bitmap.
	THIEF_PROP_FIELD (float, bitmap_size_y); //TESTME

	//! Whether the bitmap should be tiled.
	THIEF_PROP_FIELD (bool, uv_tiling); //TESTME

	//! The size in feet (DU) along the X axis of one tile of the bitmap.
	THIEF_PROP_FIELD (float, tile_size_x); //TESTME

	//! The size in feet (DU) along the X axis of one tile of the bitmap.
	THIEF_PROP_FIELD (float, tile_size_y); //TESTME

	/*! Whether to light the bitmap according to surrounding lighting.
	 * If \c false, the bitmap will be fully illuminated. */
	THIEF_PROP_FIELD (bool, apply_lighting);

	/*! A color cast to apply to the bitmap.
	 * The bitmap image's colors are multiplied with this color, allowing
	 * e.g. a monochrome bitmap to be colored in multiple ways. */
	THIEF_PROP_FIELD (Color, bitmap_color); //TESTME

	//! Whether to render the "back" side of the bitmap as well as the front.
	THIEF_PROP_FIELD (bool, double_sided); //TESTME

	/*! Whether to flip the "back" side of the bitmap to match the front.
	 * If \c false, the back side is a mirror image of the front side. */
	THIEF_PROP_FIELD (bool, flip_backside_uv); //TESTME

	/*! Whether the bitmap should be rotated on its axis to always face
	 * towards the camera. */
	THIEF_PROP_FIELD (bool, face_camera); //TESTME

	/*! How much to fade out the bitmap as the camera approaches the axis
	 * around which it rotates. The effect is disabled at \c 0.0 and covers
	 * the greatest angle at \c 1.0. Only used if #face_camera is \c true. */
	THIEF_PROP_FIELD (float, axial_fade); //TESTME

	/*! How fast the tiles should move, in feet (DU) per second.
	 * The stock \c ConveyorBelt script also applies this speed to the
	 * physics of objects coming into contact with the bitmap. */
	THIEF_PROP_FIELD (float, scroll_speed); //TESTME

	//! Whether the animation should continue to loop infinitely.
	THIEF_PROP_FIELD (bool, animation_loop); //TESTME
};

THIEF_SPECIALIZE_FIELD_PROXY_SET (PropField, Color, Bitmapped, bitmap_color);



/*! An archetype that configures a blinding flash effect.
 * Flashes have an unusual object setup. See the FlashPoint class for details. */
class FlashConfig : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (FlashConfig)

	//! Returns whether the object is an archetype with flash configuration.
	bool is_flash_config () const; //TESTME

	//! How long the flash will affect objects (AIs) in the world.
	THIEF_PROP_FIELD (Time, world_duration); //TESTME

	//! How long, at most, the flash will fill the screen.
	THIEF_PROP_FIELD (Time, screen_duration); //TESTME

	//! How long the screen will take to return to normal after the flash.
	THIEF_PROP_FIELD (Time, effect_duration); //TESTME

	//! How far from the flash point its effects will be felt.
	THIEF_PROP_FIELD (float, range); //TESTME
};



/*! An object that can emit a blinding flash effect.
 * A flash temporarily fills the screen with color and blinds nearby AIs that
 * are flash-vulnerable. Flashes have an unusual object setup, requiring three
 * pieces working together:
 * - A flash configuration archetype carries the properties that configure the
 *   color, duration, and range of the flash. These are wrapped here by the
 *   FlashConfig class.
 * - A flash point archetype has a \c RenderFlash link to a flash configuration
 *   archetype. (A configuration archetype can be linked to by more than one
 *   point archetype, but each point archetype can have only one configuration.)
 * - A concrete object inheriting from the flash point archetype. The flash
 *   itself is triggered on this object, which is wrapped by this class.
 */
class FlashPoint : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (FlashPoint)

	/*! Returns whether the object can emit a flash.
	 * This is \c true if the object is concrete and inherits from an
	 * archetype with a \c RenderFlash link. */
	bool is_flash_point () const;

	/*! Emits the configured flash.
	 * \throw std::runtime_error if this is not a valid flash point. */
	void flash ();
};



/*! An object rendered as a particle effect.
 * A particle group is not rendered as standard model. Its properties instead
 * configure a group of particles that are rendered within its boundaries. */
class ParticleGroup : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (ParticleGroup)

	//! Returns whether the object is a particle group.
	bool is_particle_group () const;

	//! How many particles are rendered in the group.
	THIEF_PROP_FIELD (int, particle_count); //TESTME

	//! The radius of each particle in feet (DU).
	THIEF_PROP_FIELD (float, particle_size); //TESTME

	//! Whether the particles are currently rendered.
	THIEF_PROP_FIELD (bool, active);
};



/*! A utility object that changes textures on terrain surfaces.
 * This object type is used to replace one texture with another on all terrain
 * surfaces within a given radius of the object. A static method is supplied
 * here to automatically create a temporary object for this purpose. */
class TextureSwapper : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (TextureSwapper)

	//! Returns whether the object is set up for texture swapping.
	bool is_texture_swapper () const;

	/*! The filesystem path to the texture to be replaced.
	 * This is a game-relative path such as \c fam/core/bricks. */
	THIEF_PROP_FIELD (String, old_texture);

	/*! The filesystem path to the texture to be applied instead.
	 * This is a game-relative path such as \c fam/core/clnbrik1. */
	THIEF_PROP_FIELD (String, new_texture);

	/*! The maximum distance from the object at which terrain surfaces
	 * should be retextured. */
	THIEF_PROP_FIELD (float, swap_radius);

	/*! Substitutes #new_texture for #old_texture on all terrain surfaces
	 * within #swap_radius of the object's current position.
	 * \return Whether the retexturing was successful. */
	bool swap_textures ();

	/*! Substitutes the \a new_texture for the \a old_texture on all
	 * terrain surfaces within \a radius of \a center. This method creates
	 * a temporary fnord object for this purpose.
	 * \return Whether the retexturing was successful. */
	static bool swap_textures (const Vector& center, float radius,
		const String& old_texture, const String& new_texture);
};



} // namespace Thief

#endif // THIEF_RENDERING_HH

