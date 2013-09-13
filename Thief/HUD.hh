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

#ifndef THIEF_HUD_HH
#define THIEF_HUD_HH

#include <Thief/Base.hh>
#include <Thief/Engine.hh>

namespace Thief {



struct CanvasPoint
{
	int x, y;

	CanvasPoint ();
	CanvasPoint (int x, int y);

	bool valid () const;
	bool operator == (const CanvasPoint&) const;
	bool operator != (const CanvasPoint&) const;

	CanvasPoint operator - () const;
	CanvasPoint operator + (const CanvasPoint&) const;
	CanvasPoint operator - (const CanvasPoint&) const;
	CanvasPoint operator * (int) const;
	CanvasPoint operator / (int) const;

	CanvasPoint& operator += (const CanvasPoint&);
	CanvasPoint& operator -= (const CanvasPoint&);

	static const CanvasPoint ORIGIN;
	static const CanvasPoint OFFSCREEN;
};



struct CanvasRect : public CanvasPoint, public CanvasSize
{
	CanvasRect ();
	CanvasRect (int x, int y, int w, int h);
	CanvasRect (CanvasPoint position, CanvasSize size);
	explicit CanvasRect (CanvasSize size); // at origin

	bool valid () const;
	bool operator == (const CanvasRect&) const;
	bool operator != (const CanvasRect&) const;

	CanvasRect operator + (const CanvasPoint&) const;
	CanvasRect operator - (const CanvasPoint&) const;

	static const CanvasRect NOCLIP;
	static const CanvasRect OFFSCREEN;
};



class HUDBitmap
{
public:
	typedef std::shared_ptr<HUDBitmap> Ptr;
	typedef std::weak_ptr<HUDBitmap> WeakPtr;

	typedef size_t Frame;
	static const Frame STATIC;

	static Ptr load (const String& path, bool animation);
	virtual ~HUDBitmap ();

	String get_path () const { return path; }
	CanvasSize get_size () const;
	Frame count_frames () const;

	void draw (Frame frame, CanvasPoint position,
		CanvasRect clip = CanvasRect::NOCLIP) const;

private:
	friend class OSL;
	HUDBitmap (const String& path, bool animation);

	const String path;

	typedef int Handle;
	std::vector<Handle> frames;
};



class HUDElementBase
{
public:
	typedef int ZIndex;

	enum class Event { ENTER_GAME_MODE, DRAW_STAGE_1, DRAW_STAGE_2 };

protected:
	bool initialize (ZIndex priority);
	bool deinitialize ();

private:
	friend class OSL;
	virtual void on_event (Event event) = 0;
};



class HUDElement : public HUDElementBase
{
public:
	virtual ~HUDElement ();

protected:
	HUDElement ();

	bool initialize (ZIndex priority);
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

