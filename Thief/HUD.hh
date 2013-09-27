//! \file HUD.hh Drawing of HUD (head-up display) elements on screen.

/*  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 */

#ifndef THIEF_HUD_HH
#define THIEF_HUD_HH

#include <Thief/Base.hh>
#include <Thief/Engine.hh>

namespace Thief {



//! Represents a point (pixel) on the canvas (screen).
struct CanvasPoint
{
	//! The X-axis (horizontal) coordinate of the point, in pixels.
	int x;

	//! The Y-axis (vertical) coordinate of the point, in pixels.
	int y;

	//! Constructs a new point at the canvas origin.
	CanvasPoint ();

	//! Constructs a new point at the given coordinates.
	CanvasPoint (int x, int y);

	/*! Returns whether both coordinates are non-negative.
	 * This method does not check whether the coordinates lie within the
	 * current canvas area. */
	bool valid () const;

	//! Returns whether the coordinates are within the current canvas area.
	bool onscreen () const;

	//! Returns whether two canvas points are equal.
	bool operator == (const CanvasPoint&) const;

	//! Returns whether two canvas points are unequal.
	bool operator != (const CanvasPoint&) const;

	/*! Returns the negation of the coordinates.
	 * The negation is unlikely to be a valid point itself, of course, but
	 * may be useful in calculations. */
	CanvasPoint operator - () const;

	//! Returns the sum of these coordinates and the given ones.
	CanvasPoint operator + (const CanvasPoint&) const;

	//! Returns the difference of these coordinates and the given ones.
	CanvasPoint operator - (const CanvasPoint&) const;

	//! Returns the coordinates multiplied by the given factor.
	CanvasPoint operator * (int) const;

	//! Returns the coordinates divided by the given divisor.
	CanvasPoint operator / (int) const;

	//! Adds the given coordinates to these coordinates.
	CanvasPoint& operator += (const CanvasPoint&);

	//! Subtracts the given coordinates from these coordinates.
	CanvasPoint& operator -= (const CanvasPoint&);

	//! The origin of the canvas plane at (0, 0).
	static const CanvasPoint ORIGIN;

	//! Represents that the requested point is not within the canvas area.
	static const CanvasPoint OFFSCREEN;
};



//! Represents a rectangular portion of the canvas (screen).
struct CanvasRect : public CanvasPoint, public CanvasSize
{
	//! Constructs a new rectangle of zero size at the origin.
	CanvasRect ();

	//! Constructs a new rectangle with the given coordinates.
	CanvasRect (int x, int y, int w, int h);

	/*! Constructs a new rectangle of the given size with its top left
	 * corner at the given point. */
	CanvasRect (CanvasPoint position, CanvasSize size);

	//! Constructs a new rectangle of the given size at the canvas origin.
	explicit CanvasRect (CanvasSize size);

	//! Returns whether the rectangle has a non-negative size.
	bool valid () const;

	/*! Returns whether the rectangle is within the current canvas area.
	 * If \a allow_partical is \c true, at least one pixel of the rectangle
	 * must be on the canvas; if \c false, the entire rectangle must be on
	 * the canvas. */
	bool onscreen (bool allow_partial = true) const;

	//! Returns whether two canvas rectangles are identical.
	bool operator == (const CanvasRect&) const;

	//! Returns whether two canvas rectangles are not identical.
	bool operator != (const CanvasRect&) const;

	/*! Returns a rectangle translated from this one by the given distance.
	 * The area of the resulting rectangle is unchanged. */
	CanvasRect operator + (const CanvasPoint&) const;

	/*! Returns a rectangle translated from this one by the opposite of the
	 * given distance. The area of the resulting rectangle is unchanged. */
	CanvasRect operator - (const CanvasPoint&) const;

	//! Instructs certain methods to use the full area available to them.
	static const CanvasRect NOCLIP;

	/*! Represents that the requested rectangle would be entirely outside
	 * the canvas area. */
	static const CanvasRect OFFSCREEN;
};



/*! Represents a bitmap image loaded into memory to be drawn on the HUD.
 * \note A maximum of 128 HUD bitmap frames can be loaded at any time (counting
 * frames of an animation individually). */
class HUDBitmap
{
public:
	//! A handle to a loaded HUD bitmap.
	typedef std::shared_ptr<HUDBitmap> Ptr;

	//! The index of a frame in an animated bitmap.
	typedef size_t Frame;

	/*! Indicates that a bitmap is static (non-animated).
	 * Used where an animation frame would otherwise be expected. */
	static const Frame STATIC;

	/*! Loads a bitmap image from the given filesystem path.
	 * If \a animation is \c true, the method will attempt to load as many
	 * frames as are available, following the engine's standard scheme for
	 * naming animation frames. If \a animation is \c false, the method will
	 * only attempt to load the exact \a path given. \return A handle to the
	 * loaded bitmap, or an empty handle if an error occurred. */
	static Ptr load (const String& path, bool animation);

	//! Destroys a HUD bitmap handle and unloads the bitmap from memory.
	virtual ~HUDBitmap ();

	//! Returns the filesystem path from which the bitmap was loaded.
	String get_path () const { return path; }

	//! Returns the pixel size of the first frame of the bitmap.
	CanvasSize get_size () const;

	//! Returns the number of frames in an animated bitmap, or #STATIC.
	Frame count_frames () const;

	/*! Draws the given frame on the current HUD element.
	 * \pre Must be called from the drawing cycle of a HUD element.
	 * \warning This method does not handle many of the details covered
	 * by HUDElement::draw_bitmap. In most cases, call that method instead. */
	void draw (Frame frame, CanvasPoint position,
		CanvasRect clip = CanvasRect::NOCLIP) const;

private:
	friend class OSL;
	HUDBitmap (const String& path, bool animation);

	const String path;

	typedef int Handle;
	std::vector<Handle> frames;
};



/*! Provides a minimal interface for creating HUD elements.
 * This class handles registration of a HUD element with the central HUD handler.
 * HUDElement provides the full set of methods needed to draw an element. */
class HUDElementBase
{
public:
	//! A stacking priority for a HUD element.
	typedef int ZIndex;

	//! An event to be handled by HUD elements.
	enum class Event
	{
		ENTER_GAME_MODE, /*!< The user has returned to game mode.
		                  * Elements may respond to this event by
		                  * updating their dimensions, in case the
		                  * canvas size has changed. */
		DRAW_STAGE_1,    /*!< Draw a non-overlay element in this frame.
		                  * Overlay elements may do preparatory work
		                  * at this stage. */
		DRAW_STAGE_2     //!< Draw an overlay element in this frame.
	};

protected:
	/*! Registers the element with the central HUD handler.
	 * No check is performed for a redundant call. \param priority The
	 * Z-index of the element relative to others. \return Whether the
	 * registration was successful. */
	bool initialize (ZIndex priority);

	/*! Deregisters the element with the central HUD handler.
	 * No check is performed for a redundant call. \return Whether the
	 * deregistration was successful. */
	bool deinitialize ();

	/*! Handles a HUD event.
	 * A derived class must implement this method to handle HUD events. */
	virtual void on_event (Event event) = 0;

private:
	friend class OSL;
};



/*! Draws a HUD (head-up display) element to the screen.
 * A head-up display (HUD) element is an interface element drawn on screen above
 * the rendered 3D world. In the standard Thief interface, the HUD elements are
 * the selected weapon, the selected item, the health bar, the visibility gem,
 * the breath bar, and any on-screen text message. By inheriting from this class,
 * a script may supplement or replace these elements.
 *
 * At a minimum, a derived class must implement the redraw() method to draw
 * itself to the screen when needed, as well as calling the initialize() method
 * at an appropriate time. A derived class may also override the prepare()
 * method to determine whether it should be drawn in a given frame and perform
 * any needed calculations or resource allocations.
 *
 * Currently, all HUD elements are of non-overlay type. These are redrawn to the
 * canvas (screen) in their entirety for every frame. The other type of element,
 * the overlay element, is not yet supported by ThiefLib due to unresolved bugs.
 * Overlay elements are redrawn only when they have changed; the engine copies
 * their buffers to the screen in each frame. As such, they also support smooth
 * scaling and variable opacity. There is a limit of 64 overlay elements at any
 * time.
 *
 * The Dark %Engine expects a central handler to do all HUD drawing. ThiefLib
 * provides such a handler, and it supports a theoretically unlimited number of
 * HUDElement instances. This handler is shared between all ThiefLib-based
 * script modules. However, ThiefLib-based HUD element scripts are incompatible
 * with any HUD drawing scripts not using ThiefLib's handler. */
class HUDElement : public HUDElementBase
{
public:
	//! Destroys a HUD element.
	virtual ~HUDElement ();

protected:
	/*! Constructs a HUD element.
	 * The element is not prepared to be drawn on screen until initialize()
	 * has been called. */
	HUDElement ();

	/*! Prepares the element to be drawn on screen.
	 * After this method is called, the element will be prepare()d and
	 * redraw()n as needed. Additional calls to this method have no effect.
	 * \param priority Determines how the element will be stacked on screen.
	 * An element with a higher priority than another will be drawn above it.
	 * Elements of the same priority will be stacked in a random but stable
	 * order. \return \c true the first time the element is successfully
	 * initialized, \c false otherwise. */
	bool initialize (ZIndex priority);

	/*! Frees resources associated with the element.
	 * After this method is called, the element can no longer be drawn. The
	 * method is called by the destructor, but may be called early by a
	 * derived class if the element is no longer needed. */
	void deinitialize ();

	/*! Schedules the element to be redrawn in the next frame.
	 * This method should be called whenever the drawn content of the
	 * element needs to change. Non-overlay elements are already drawn
	 * in every frame, so this method is informational only for them. */
	void schedule_redraw ();

	//@}
	//! \name Configuration
	//@{

//! \cond HIDDEN_SYMBOLS
	bool is_overlay () const;
#ifdef THIEF_USE_BROKEN_HUD_OVERLAY
	bool create_overlay ();
#endif
	void destroy_overlay ();
//! \endcond

#ifdef THIEF_USE_BROKEN_HUD_OVERLAY
	float get_opacity () const { return opacity; }
	void set_opacity (float opacity);
#endif

	//! Returns the top left corner of the element's drawing area.
	CanvasPoint get_position () const { return _position; }

	//! Sets the top left corner of the element's drawing area.
	void set_position (CanvasPoint);

	//! Returns the size of the element's drawing area.
	CanvasSize get_size () const { return _size; }

	/*! Sets the size of the element's drawing area.
	 * The size is only enforced for overlay elements. */
	void set_size (CanvasSize);

#ifdef THIEF_USE_BROKEN_HUD_OVERLAY
	float get_scale () const { return scale; }
	void set_scale (float);
#endif

	//! Returns the current drawing color.
	Color get_drawing_color () const { return drawing_color; }

	/*! Sets the current drawing color.
	 * This color will be used to draw lines, boxes, and text.
	 * \pre This method can only be called from prepare() or redraw(). */
	void set_drawing_color (const Color&);

	//! Returns the current drawing offset.
	CanvasPoint get_drawing_offset () const { return drawing_offset; }

	/*! Sets the current drawing offset.
	 * All drawing methods will offset their positions by this value. It is
	 * not reset automatically between draw cycles, so callers should reset
	 * it to CanvasPoint::ORIGIN when the offset is no longer needed.
	 * \pre This method can only be called from prepare() or redraw(). */
	void set_drawing_offset (CanvasPoint offset = CanvasPoint::ORIGIN);

	/*! Adjusts the current drawing offset by the given amount.
	 * \pre This method can only be called from prepare() or redraw(). */
	void adjust_drawing_offset (CanvasPoint by);

	//@}
	//! \name Drawing
	//@{

#ifdef THIEF_USE_BROKEN_HUD_OVERLAY
	/*! Fills the overlay element area with the given color.
	 * \param color_index The index of a color in the standard %Thief
	 * palette. \param opacity The opacity of the color to draw, between
	 * 0.0 and 1.0. \pre This method can only be called from redraw() and
	 * only on an overlay element. */
	void fill_overlay (int color_index, float opacity);
#endif // THIEF_USE_BROKEN_HUD_OVERLAY

	/*! Fills the given area with the drawing color.
	 * If \a area is CanvasRect::NOCLIP, the entire element will be filled.
	 * \pre This method can only be called from redraw(). */
	void fill_area (CanvasRect area = CanvasRect::NOCLIP);

	/*! Draws a rectangular border around the given area.
	 * The border will be one pixel thick and drawn in the current drawing
	 * color. If \a area is CanvasRect::NOCLIP, the border will be drawn
	 * around the entire element.
	 * \pre This method can only be called from redraw(). */
	void draw_box (CanvasRect area = CanvasRect::NOCLIP);

	/*! Draws a line on the element between the given points.
	 * The line will be drawn in the current drawing color.
	 * \pre This method can only be called from redraw(). */
	void draw_line (CanvasPoint from, CanvasPoint to);

	/*! Draws text on the element.
	 * The text will be drawn in the font named in the \c gui_font config
	 * variable and in the current drawing color. Line breaks in the string
	 * will be preserved. Like all on-screen text in %Thief, the character
	 * set is limited and may vary by font. \param text The text to
	 * be drawn. \param position The top left corner of the text, relative
	 * to the element. \pre This method can only be called from redraw(). */
	void draw_text (const String& text,
		CanvasPoint position = CanvasPoint::ORIGIN);

	/*! Draws a bitmap image on the element.
	 * \param bitmap The bitmap to draw. \param frame The frame of an
	 * animated bitmap to draw, or HUDBitmap::STATIC for a static bitmap.
	 * \param position The top left corner of the drawn portion of the
	 * bitmap, relative to the element. \param clip The portion of the
	 * bitmap to draw, or CanvasRect::NOCLIP to draw its entirety.
	 * \pre This method can only be called from redraw(). */
	void draw_bitmap (const HUDBitmap::Ptr& bitmap,
		HUDBitmap::Frame frame = HUDBitmap::STATIC,
		CanvasPoint position = CanvasPoint::ORIGIN,
		CanvasRect clip = CanvasRect::NOCLIP);

	//@}
	//! \name Calculations
	//@{

	/*! Calculates the canvas area required to draw the given text.
	 * \pre This method can only be called from prepare() or redraw(). */
	CanvasSize get_text_size (const String& text) const;

	/*! Returns the point on the canvas, if any, in the direction of the
	 * given world location. If the direction of the location is offscreen,
	 * returns CanvasPoint::OFFSCREEN. The location itself need not be
	 * visible. \pre This method can only be called from prepare() or
	 * redraw(). */
	CanvasPoint location_to_canvas (const Vector& location) const;

	/*! Returns the portion of the canvas, if any, where the given object
	 * would be visible. If the object would be entirely offscreen, returns
	 * CanvasRect::OFFSCREEN. \pre This method can only be called from
	 * prepare() or redraw().*/
	CanvasRect object_to_canvas (const Object& object) const;

	/*! Returns the point on the canvas, if any, in the direction of the
	 * centroid of the given object. If the direction of the centroid is
	 * offscreen, returns CanvasPoint::OFFSCREEN. The object itself need not
	 * be visible. \pre This method can only be called from prepare() or
	 * redraw(). */
	CanvasPoint centroid_to_canvas (const Object& object) const;

	//@}
	//! \name Implemented by derived classes
	//@{

	/*! Prepares the element for inclusion in a frame.
	 * A derived class may override this method to perform any other needed
	 * per-frame preparation, such as checking whether the element should be
	 * enabled, updating its size and position, or scheduling a redraw.
	 * The base implementation does nothing but return \c true.
	 * \return Whether the element should be included in the next frame. */
	virtual bool prepare ();

	/*! Draws the content of the element.
	 * A derived class must implement this method to perform the actual
	 * drawing of the element. For a non-overlay element, the method will be
	 * called in every frame where prepare() returns \c true; for an overlay
	 * element, it will be called only in frames where a redraw is needed. */
	virtual void redraw () = 0;

	//@}

private:
	void do_offset (CanvasPoint& point) const;
	void do_offset (CanvasRect& area) const;

	virtual void on_event (Event event);

	bool initialized, should_draw, needs_redraw, drawing;

	typedef int Handle;
	Handle overlay;
	float opacity;

	CanvasPoint _position;
	CanvasSize _size;
	float scale;

	Color drawing_color;
	CanvasPoint drawing_offset;
};



} // namespace Thief

#include <Thief/HUD.inl>

#endif // THIEF_HUD_HH

