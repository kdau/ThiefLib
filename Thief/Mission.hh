/******************************************************************************
 *  Mission.hh
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

#ifndef THIEF_MISSION_HH
#define THIEF_MISSION_HH

#include <Thief/Base.hh>

namespace Thief {



enum class Difficulty
{
	NOT_EXPERT = -4,
	NOT_NORMAL = -3,
	ANY = -2,
	NONE = -1,
	NORMAL = 0,
	HARD = 1,
	EXPERT = 2
};

class DifficultyMessage : public Message //TESTME
{
public:
	DifficultyMessage (Difficulty);
	THIEF_MESSAGE_WRAP (DifficultyMessage);

	Difficulty get_difficulty () const;
};



enum class AmbientLightZone
{
	GLOBAL = 0,
	ZONE_1, ZONE_2, ZONE_3, ZONE_4, ZONE_5, ZONE_6, ZONE_7, ZONE_8,
	_MAX_ZONE = 8
};



enum class EnvironmentMapZone
{
	GLOBAL = 0,
	ZONE_1,  ZONE_2,  ZONE_3,  ZONE_4,  ZONE_5,  ZONE_6,  ZONE_7,  ZONE_8,
	ZONE_9,  ZONE_10, ZONE_11, ZONE_12, ZONE_13, ZONE_14, ZONE_15, ZONE_16,
	ZONE_17, ZONE_18, ZONE_19, ZONE_20, ZONE_21, ZONE_22, ZONE_23, ZONE_24,
	ZONE_25, ZONE_26, ZONE_27, ZONE_28, ZONE_29, ZONE_30, ZONE_31, ZONE_32,
	ZONE_33, ZONE_34, ZONE_35, ZONE_36, ZONE_37, ZONE_38, ZONE_39, ZONE_40,
	ZONE_41, ZONE_42, ZONE_43, ZONE_44, ZONE_45, ZONE_46, ZONE_47, ZONE_48,
	ZONE_49, ZONE_50, ZONE_51, ZONE_52, ZONE_53, ZONE_54, ZONE_55, ZONE_56,
	ZONE_57, ZONE_58, ZONE_59, ZONE_60, ZONE_61, ZONE_62, ZONE_63,
	_MAX_ZONE = 63
};



struct Fog
{
	enum Zone
	{
		DISABLED = -1, GLOBAL = 0,
		ZONE_1, ZONE_2, ZONE_3, ZONE_4, ZONE_5, ZONE_6, ZONE_7, ZONE_8,
		_MAX_ZONE = 8
	};

	Fog (const Color& color = Color (), float distance = 0.0f);
	Color color;
	float distance;

	static float interpolate_distance (bool global, float from, float to,
		float weight = 0.5f, Curve = Curve::LINEAR);
};



struct Precipitation
{
	Precipitation ();

	enum class Type { SNOW, RAIN } type;

	float frequency;
	float speed;

	float visible_distance;

	float radius;
	float opacity;
	float brightness;

	float snow_jitter;

	float rain_length;
	float splash_frequency;
	float splash_radius;
	float splash_height;
	float splash_duration;

	String texture;

	Vector wind;
};



class Mission
{
public:
	static bool is_fm ();
	static String get_fm_name ();
	static String get_fm_path ();
	static String get_path_in_fm (const String& relative_path);

#ifdef IS_THIEF2
	static int get_number ();
#endif
	static String get_mis_file ();
	static String get_gam_file ();

	// Disposition

#ifdef IS_THIEF2
	static void set_next (int number);
#endif
	static void fade_to_black (Time duration);
	static void end (bool win = false);

	static void quick_save ();
	static void quick_load ();

	// Difficulty

	static Difficulty get_difficulty ();
	static bool check_difficulty (Difficulty allowed);

	// Rendering effects

	static void set_envmap_texture (EnvironmentMapZone,
		const String& texture);

	static Fog get_fog (Fog::Zone);
	static void set_fog (Fog::Zone, const Fog&);

	static Precipitation get_precipitation ();
	static void set_precipitation (const Precipitation&);

	// Text messages and books

	static String get_text (const String& directory, const String& file,
		const String& name);

	static Time calc_text_duration (const String& text,
		Time word_duration = 500ul);

	static void show_text (const String& text, Time duration = 0ul,
		const Color& color = Color ());

	static String get_book_text (const String& book, int page = 0);

	static void show_book (const String& book, const String& art);

	// Other metagame screens

	static void show_map ();
#ifdef IS_THIEF2
	static bool has_visited_automap_location (int page, int location); //TESTME
	static void visit_automap_location (int page, int location); //TESTME
#endif // IS_THIEF2

	static void show_objectives ();

	static void show_image (const String& image);
	static void show_movie (const String& movie);

	static void show_menu ();
	static void show_load_screen ();
	static void show_save_screen ();
};



} // namespace Thief

#endif // THIEF_MISSION_HH

