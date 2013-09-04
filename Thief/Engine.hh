/******************************************************************************
 *  Engine.hh
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

#ifndef THIEF_ENGINE_HH
#define THIEF_ENGINE_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>

namespace Thief {



// Monolog: logging for DromEd

class Monolog : public std::ostream
{
public:
	Monolog ();
	virtual ~Monolog ();

	void attach (MPrintfProc proc);

private:
	class Streambuf;
	std::unique_ptr<Streambuf> buf;
};

extern Monolog mono;



// CanvasSize: screen extents

struct CanvasSize
{
	int w, h;

	CanvasSize ();
	CanvasSize (int w, int h);

	bool valid () const;
	bool operator == (const CanvasSize&) const;
	bool operator != (const CanvasSize&) const;
};



// Version: Dark Engine version checking

struct Version
{
	Version (int major, int minor);
	int major;
	int minor;

	bool operator == (const Version&) const;
	bool operator < (const Version&) const;
	bool operator >= (const Version&) const;
};



// Engine

class Engine
{
public:
	static String get_app_name ();
	static String get_short_app_name ();
	static Version get_version ();

	// Game mode and sim status

	enum class Mode { GAME, EDIT };
	static Mode get_mode ();
	static bool is_editor ();
	static bool is_sim ();

	// Graphics

	static CanvasSize get_canvas_size ();
	static float get_aspect_ratio ();
	static int get_directx_version ();

	static bool rendered_this_frame (const Object&);

	enum class RaycastMode { TERRAIN = -1, NEAREST, ANY, FAST };

	struct RaycastHit
	{
		enum Type { NONE, TERRAIN, OBJECT, MESH } type;
		Vector location;
		Object object;
	};

	static RaycastHit raycast (RaycastMode, const Vector& from,
		const Vector& to, bool include_ai = true); //TESTME

	static RaycastHit raycast (RaycastMode, const Object& from,
		const Object& to, bool include_ai = true); //TESTME

	// Configuration

	static bool has_config (const String& variable);

	template <typename T> static T get_config (const String& variable);
	// ...only valid for int, float, and String specializations (below)

	static float get_binding_config (const String& variable);

	static bool is_command_bound (const String& command);
	static String get_command_binding (const String& command);

	// Miscellaneous

	static String find_file_in_path (const String& type, const String& file);

	static int random_int (int minimum, int maximum);
	static float random_float ();

	static void run_command (const String& command,
		const String& arguments = String ());

	static void write_to_game_log (const String&);
};

template<> int Engine::get_config (const String& variable);
template<> float Engine::get_config (const String& variable);
template<> String Engine::get_config (const String& variable);



// GameModeChangeMessage

class GameModeChangeMessage : public Message
{
public:
	GameModeChangeMessage (bool resuming, bool suspending);
	THIEF_MESSAGE_WRAP (GameModeChangeMessage);

	bool is_resuming () const;
	bool is_suspending () const;
};



// SimMessage

class SimMessage : public Message
{
public:
	SimMessage (bool starting);
	THIEF_MESSAGE_WRAP (SimMessage);

	bool is_starting () const;
};



} // namespace Thief

#endif // THIEF_ENGINE_HH

