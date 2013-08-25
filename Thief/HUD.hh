/******************************************************************************
 *  HUD.hh
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

#ifndef THIEF_HUD_HH
#define THIEF_HUD_HH

#include <Thief/Base.hh>

namespace Thief {

class HUDElement;




class HUDBitmap
{
public:
	typedef std::shared_ptr<HUDBitmap> Ptr;
	typedef std::weak_ptr<HUDBitmap> WeakPtr;

	typedef size_t Frame;
	static const Frame STATIC;

	virtual ~HUDBitmap ();

	String get_path () const { return path; }
	CanvasSize get_size () const;
	Frame count_frames () const;

	void draw (Frame frame, CanvasPoint position,
		CanvasRect clip = CanvasRect::NOCLIP) const;

private:
	friend class HUD;
	HUDBitmap (const String& path, bool animation);

	const String path;

	typedef int Handle;
	std::vector<Handle> frames;
};



class HUD
{
public:
	typedef std::shared_ptr<HUD> Ptr;
	typedef std::weak_ptr<HUD> WeakPtr;

	typedef int ZIndex;

	virtual ~HUD ();

	static Ptr get ();

	void register_element (HUDElement&, ZIndex priority);
	void unregister_element (HUDElement&);

	HUDBitmap::Ptr load_bitmap (const String& path, bool animation);

private:
	friend class HUDImpl;
	HUD ();

	typedef std::multimap<ZIndex, HUDElement*> Elements;
	Elements elements;

	typedef std::map<String, HUDBitmap::WeakPtr> Bitmaps;
	Bitmaps bitmaps;
};



class HUDElement
{
public:
	virtual ~HUDElement ();

protected:
	HUDElement ();

	bool initialize (HUD::ZIndex priority);
	void deinitialize ();

	void schedule_redraw ();

	bool is_overlay () const; //TESTME
	bool create_overlay (); //TESTME
	void destroy_overlay (); //TESTME

	float get_opacity () const { return opacity; }
	void set_opacity (float opacity); //TESTME

	CanvasPoint get_position () const { return _position; }
	void set_position (CanvasPoint);

	CanvasSize get_size () const { return _size; }
	void set_size (CanvasSize);

	float get_scale () const { return scale; }
	void set_scale (float); //TESTME

	Color get_drawing_color () const { return drawing_color; }
	void set_drawing_color (const Color&);

	CanvasPoint get_drawing_offset () const { return drawing_offset; }
	void set_drawing_offset (CanvasPoint offset = CanvasPoint::ORIGIN);
	void adjust_drawing_offset (CanvasPoint by);

	void fill_background (int color_index, float opacity);
	void fill_area (CanvasRect area = CanvasRect::NOCLIP);
	void draw_box (CanvasRect area = CanvasRect::NOCLIP);
	void draw_line (CanvasPoint from, CanvasPoint to);

	CanvasSize get_text_size (const String& text) const;
	void draw_text (const String& text,
		CanvasPoint position = CanvasPoint::ORIGIN);

	void draw_bitmap (const HUDBitmap::Ptr& bitmap, HUDBitmap::Frame frame,
		CanvasPoint position = CanvasPoint::ORIGIN,
		CanvasRect clip = CanvasRect::NOCLIP);

	CanvasPoint location_to_canvas (const Vector& location) const;
	CanvasRect object_to_canvas (const Object& object) const;
	CanvasPoint centroid_to_canvas (const Object& object) const;

private:
	virtual bool prepare ();
	virtual void redraw () = 0;

	void do_offset (CanvasPoint& point) const;
	void do_offset (CanvasRect& area) const;

	friend class HUDImpl;
	void on_draw_stage_1 ();
	void on_draw_stage_2 ();
	void on_enter_game_mode ();

	HUD::Ptr hud;

	bool should_draw, needs_redraw, drawing;

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

#endif // THIEF_HUD_HH

