/******************************************************************************
 *  HUD.cc
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

#include <windows.h>
#undef DrawText // ugh, Windows...
#undef GetClassName // ugh, Windows...
#undef LoadBitmap // ugh, Windows...

namespace Thief {

#define INVALID_HANDLE -1



// HUDBitmap

#define MAX_BITMAP_HANDLE 128

const HUDBitmap::Frame
HUDBitmap::STATIC = 0;

HUDBitmap::HUDBitmap (const String& _path, bool animation)
	: path (_path)
{
	SService<IDarkOverlaySrv> DOS (LG);

	char dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_splitpath (path.data (), nullptr, dir, fname, ext);

	Frame max_frames = animation ? MAX_BITMAP_HANDLE : 1;
	for (Frame frame = STATIC; frame < max_frames; ++frame)
	{
		std::ostringstream _file;
		_file << fname;
		if (frame != STATIC) _file << '_' << frame;
		_file << ext;

		String file = _file.str ();
		Handle handle = DOS->GetBitmap (file.data (), dir);
		if (handle != INVALID_HANDLE)
			frames.push_back (handle);
		else if (frame != STATIC) // Try _1 even if plain is missing.
			break;
	}

	if (frames.empty ())
		throw std::runtime_error ("bitmap file invalid or not found");
}

HUDBitmap::~HUDBitmap ()
{
	SService<IDarkOverlaySrv> DOS (LG);
	for (auto frame : frames)
		DOS->FlushBitmap (frame);
}

CanvasSize
HUDBitmap::get_size () const
{
	CanvasSize size;
	SService<IDarkOverlaySrv> (LG)->GetBitmapSize
		(frames.front (), size.w, size.h);
	return size;
}

HUDBitmap::Frame
HUDBitmap::count_frames () const
{
	return frames.size ();
}

void
HUDBitmap::draw (Frame frame, CanvasPoint position, CanvasRect clip) const
{
	SService<IDarkOverlaySrv> DOS (LG);
	if (clip == CanvasRect::NOCLIP)
		DOS->DrawBitmap (frames.at (frame), position.x, position.y);
	else
	{
		CanvasSize size;
		DOS->GetBitmapSize (frames.front (), size.w, size.h);
		if (clip.w == CanvasRect::NOCLIP.w) clip.w = size.w - clip.x;
		if (clip.h == CanvasRect::NOCLIP.h) clip.h = size.h - clip.y;
		DOS->DrawSubBitmap (frames.at (frame), position.x, position.y,
			clip.x, clip.y, clip.w, clip.h);
	}
}



// HUDImpl

class HUDImpl : public IDarkOverlayHandler, public HUD
{
public:
	HUDImpl ();
	virtual ~HUDImpl ();

	// IDarkOverlayHandler
	STDMETHOD_ (void, DrawHUD) ();
	STDMETHOD_ (void, DrawTOverlay) ();
	STDMETHOD_ (void, OnUIEnterMode) ();
};

HUDImpl::HUDImpl ()
{
	SService<IDarkOverlaySrv> (LG)->SetHandler (this);
}

HUDImpl::~HUDImpl ()
{
	SService<IDarkOverlaySrv> (LG)->SetHandler (nullptr);
}

STDMETHODIMP_ (void)
HUDImpl::DrawHUD ()
{
	for (auto& element : elements)
		element.second->on_draw_stage_1 ();
}

STDMETHODIMP_ (void)
HUDImpl::DrawTOverlay ()
{
	for (auto& element : elements)
		element.second->on_draw_stage_2 ();
}

STDMETHODIMP_ (void)
HUDImpl::OnUIEnterMode ()
{
	for (auto& element : elements)
		element.second->on_enter_game_mode ();
}



// HUD

HUD::HUD ()
{}

HUD::~HUD ()
{}

HUD::Ptr
HUD::get ()
{
	static WeakPtr impl;
	Ptr ptr = impl.lock ();
	if (!ptr)
	{
		ptr = Ptr (new HUDImpl ());
		impl = ptr;
	}
	return ptr;
}

void
HUD::register_element (HUDElement& element, ZIndex priority)
{
	elements.insert (std::make_pair (priority, &element));
}

void
HUD::unregister_element (HUDElement& element)
{
	for (auto entry = elements.begin (); entry != elements.end (); ++entry)
		if (entry->second == &element)
		{
			elements.erase (entry);
			break;
		}
}

HUDBitmap::Ptr
HUD::load_bitmap (const String& path, bool animation)
{
	HUDBitmap::Ptr bitmap;

	// Look for an existing bitmap first.
	Bitmaps::iterator existing = bitmaps.find (path);
	if (existing != bitmaps.end ())
	{
		bitmap = existing->second.lock ();
		if (bitmap)
			return bitmap;
		else
			bitmaps.erase (existing);
	}

	try
	{
		// The bitmap hasn't been loaded yet, so load it now.
		bitmap = HUDBitmap::Ptr (new HUDBitmap (path, animation));
		bitmaps.insert (std::make_pair (path, bitmap));
	}
	catch (std::exception& e)
	{
		mono << "Warning: Could not load bitmap at \"" << path
			<< "\": " << e.what () << "." << std::endl;
	}

	return bitmap;
}



// HUDElement

#define CHECK_OVERLAY(retval) \
	if (!is_overlay ()) \
	{ \
		mono << "HUDElement::" << __func__ << "called for " \
			"non-overlay element; ignoring." << std::endl; \
		return retval; \
	}

#define CHECK_DRAWING(retval) \
	if (!drawing) \
	{ \
		mono << "HUDElement::" << __func__ << "called outside of " \
			"draw cycle; ignoring." << std::endl; \
		return retval; \
	}

HUDElement::HUDElement ()
	: hud (), should_draw (false), needs_redraw (true), drawing (false),
	  overlay (INVALID_HANDLE), opacity (1.0f),
	  _position (0, 0), _size (1, 1), scale (1.0f),
	  drawing_color (0xFFFFFFu), drawing_offset ()
{}

HUDElement::~HUDElement ()
{
	deinitialize ();
}

bool
HUDElement::initialize (HUD::ZIndex priority)
{
	if (hud) // The element has already been registered.
		return false;
	else if (hud = HUD::get ()) // Register the element with the handler.
	{
		hud->register_element (*this, priority);
		return true;
	}
	else
		return false;
}

void
HUDElement::deinitialize ()
{
	// Destroy any overlay.
	destroy_overlay ();

	// Unregister with the handler and dereference it.
	if (hud)
	{
		hud->unregister_element (*this);
		hud.reset ();
	}
}

void
HUDElement::schedule_redraw ()
{
	needs_redraw = true;
}

bool
HUDElement::is_overlay () const
{
	return overlay > INVALID_HANDLE;
}

bool
HUDElement::create_overlay ()
{
	if (is_overlay ()) return true;

	return false; //TODO Fix the oddities with overlays and re-enable them.

	overlay = SService<IDarkOverlaySrv> (LG)->CreateTOverlayItem
		(_position.x, _position.y, _size.w, _size.h, 255, true);

	if (is_overlay ())
	{
		set_opacity (opacity);
		schedule_redraw ();
		return true;
	}
	else
	{
		mono << "Error: Could not create a HUD overlay." << std::endl;
		return false;
	}
}

void
HUDElement::destroy_overlay ()
{
	if (is_overlay ())
	{
		SService<IDarkOverlaySrv> (LG)->DestroyTOverlayItem (overlay);
		overlay = INVALID_HANDLE;
		schedule_redraw ();
	}
}

void
HUDElement::set_opacity (float _opacity)
{
	opacity = _opacity;
	CHECK_OVERLAY ();
	SService<IDarkOverlaySrv> (LG)->UpdateTOverlayAlpha
		(overlay, std::min (255, std::max (0, int (opacity * 255.0f))));
}

void
HUDElement::set_position (CanvasPoint position)
{
	if (_position == position) return;
	_position = position;
	if (is_overlay ())
	{
		SService<IDarkOverlaySrv> (LG)->UpdateTOverlayPosition
			(overlay, _position.x, _position.y);
	}
	else
		schedule_redraw ();
}

void
HUDElement::set_size (CanvasSize size)
{
	if (_size == size) return;
	_size = size;

	// This is not desirable. Set the size before creating an overlay.
	if (is_overlay ())
	{
		destroy_overlay (); // Destroy the overlay at the old size.
		create_overlay (); // Create a new one at the new size.
		if (scale != 1.0f) set_scale (scale); // Restore if needed.
		schedule_redraw ();
	}

	// For non-overlay elements, size is ignored.
}

void
HUDElement::set_scale (float _scale)
{
	scale = _scale;
	CHECK_OVERLAY ();
	SService<IDarkOverlaySrv> (LG)->UpdateTOverlaySize
		(overlay, _size.w * scale, _size.h * scale);
}

void
HUDElement::set_drawing_color (const Color& color)
{
	drawing_color = color;
	CHECK_DRAWING ();
	SService<IDarkOverlaySrv> (LG)->SetTextColor
		(drawing_color.red, drawing_color.green, drawing_color.blue);
}

void
HUDElement::set_drawing_offset (CanvasPoint offset)
{
	CHECK_DRAWING ();
	drawing_offset = offset;
}

void
HUDElement::adjust_drawing_offset (CanvasPoint by)
{
	CHECK_DRAWING ();
	drawing_offset += by;
}

void
HUDElement::fill_background (int color_index, float _opacity)
{
	CHECK_OVERLAY ();
	CHECK_DRAWING ();
	SService<IDarkOverlaySrv> (LG)->FillTOverlay (color_index,
		std::min (255, std::max (0, int (_opacity * 255.0f))));
}

void
HUDElement::fill_area (CanvasRect area)
{
	CHECK_DRAWING ();
	do_offset (area);
	SService<IDarkOverlaySrv> DOS (LG);
	for (int y = area.y; y < area.y + area.h; ++y)
		DOS->DrawLine (area.x, y, area.x + area.w, y);
}

void
HUDElement::draw_box (CanvasRect area)
{
	CHECK_DRAWING ();
	do_offset (area);
	SService<IDarkOverlaySrv> DOS (LG);
	DOS->DrawLine (area.x, area.y, area.x + area.w, area.y);
	DOS->DrawLine (area.x, area.y, area.x, area.y + area.h);
	DOS->DrawLine (area.x + area.w, area.y,
		area.x + area.w, area.y + area.h);
	DOS->DrawLine (area.x, area.y + area.h,
		area.x + area.w, area.y + area.h);
}

void
HUDElement::draw_line (CanvasPoint from, CanvasPoint to)
{
	CHECK_DRAWING ();
	do_offset (from);
	do_offset (to);
	SService<IDarkOverlaySrv> (LG)->DrawLine (from.x, from.y, to.x, to.y);
}

CanvasSize
HUDElement::get_text_size (const String& text) const
{
	CanvasSize size;
	CHECK_DRAWING (size);
	SService<IDarkOverlaySrv> (LG)->GetStringSize
		(text.data (), size.w, size.h);
	return size;
}

void
HUDElement::draw_text (const String& text, CanvasPoint position)
{
	CHECK_DRAWING ();
	do_offset (position);
	SService<IDarkOverlaySrv> (LG)->DrawString
		(text.data (), position.x, position.y);
}

void
HUDElement::draw_bitmap (const HUDBitmap::Ptr& bitmap, HUDBitmap::Frame frame,
	CanvasPoint position, CanvasRect clip)
{
	CHECK_DRAWING ();
	do_offset (position);
	if (bitmap)
		bitmap->draw (frame, position, clip);
	else
		mono << "Error: Cannot draw a nonexistent bitmap." << std::endl;
}

CanvasPoint
HUDElement::location_to_canvas (const Vector& location) const
{
	CHECK_DRAWING (CanvasPoint::OFFSCREEN);
	CanvasPoint position;
	bool onscreen = SService<IDarkOverlaySrv> (LG)->WorldToScreen
		(LGVector (location), position.x, position.y);
	return onscreen ? position : CanvasPoint::OFFSCREEN;
}

CanvasRect
HUDElement::object_to_canvas (const Object& object) const
{
	CHECK_DRAWING (CanvasRect::OFFSCREEN);
	int x1, y1, x2, y2;
	bool onscreen = SService<IDarkOverlaySrv> (LG)->GetObjectScreenBounds
		(object.number, x1, y1, x2, y2);
	return onscreen ? CanvasRect (x1, y1, x2 - x1, y2 - y1)
		: CanvasRect::OFFSCREEN;
}

CanvasPoint
HUDElement::centroid_to_canvas (const Object& object) const
{
	Vector centroid = object.get_location ();
	CanvasPoint position = location_to_canvas (centroid);
	if (!position.valid ()) // Centroid is offscreen, so check the bounds.
	{
		CanvasRect bounds = object_to_canvas (object);
		if (bounds.valid ()) // Use the center of the bounds instead.
		{
			position.x = bounds.x + bounds.w / 2;
			position.y = bounds.y + bounds.h / 2;
		}
	}
	return position;
}

bool
HUDElement::prepare ()
{
	return true;
}

void
HUDElement::do_offset (CanvasPoint& point) const
{
	point += drawing_offset;
	if (!is_overlay ()) point += _position;
}

void
HUDElement::do_offset (CanvasRect& area) const
{
	do_offset (static_cast<CanvasPoint&> (area));
	if (area.w == CanvasRect::NOCLIP.w) area.w = _size.w;
	if (area.h == CanvasRect::NOCLIP.h) area.h = _size.h;
}

void
HUDElement::on_draw_stage_1 ()
{
	drawing = true;
	should_draw = prepare ();
	if (should_draw && !is_overlay ())
	{
		needs_redraw = false;
		redraw ();
	}
	drawing = false;
}

void
HUDElement::on_draw_stage_2 ()
{
	if (!should_draw || !is_overlay ()) return;
	SService<IDarkOverlaySrv> DOS (LG);
	if (needs_redraw && DOS->BeginTOverlayUpdate (overlay))
	{
		needs_redraw = false;
		drawing = true;
		redraw ();
		drawing = false;
		DOS->EndTOverlayUpdate ();
	}
	DOS->DrawTOverlayItem (overlay);
}

void
HUDElement::on_enter_game_mode ()
{}



} // namespace Thief
