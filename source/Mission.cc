/******************************************************************************
 *  Mission.cc
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



// Difficulty

THIEF_ENUM_CODING (Difficulty, BOTH, VALUE,
	THIEF_ENUM_VALUE (NOT_EXPERT, "0,1", "0, 1", "0-1"),
	THIEF_ENUM_VALUE (NOT_NORMAL, "1,2", "1, 2", "1-2"),
	THIEF_ENUM_VALUE (ANY, "any", "0,1,2", "0, 1, 2", "0-2"),
	THIEF_ENUM_VALUE (NONE, "none", "-"),
	THIEF_ENUM_VALUE (NORMAL, "normal", "n"),
	THIEF_ENUM_VALUE (HARD, "hard", "h"),
	THIEF_ENUM_VALUE (EXPERT, "expert", "e"),
)



// DifficultyMessage

MESSAGE_WRAPPER_IMPL (DifficultyMessage, sDiffScrMsg)

DifficultyMessage::DifficultyMessage (Difficulty difficulty)
	: Message (new sDiffScrMsg ())
{
	message->message = "Difficulty";
	MESSAGE_AS (sDiffScrMsg)->difficulty = int (difficulty);
}

MESSAGE_ACCESSOR (Difficulty, DifficultyMessage, get_difficulty,
	sDiffScrMsg, difficulty)



// AmbientLightZone

THIEF_ENUM_CODING (AmbientLightZone, BOTH, VALUE,
	THIEF_ENUM_VALUE (GLOBAL, "global"),
	THIEF_ENUM_VALUE (ZONE_1, "zone1"),
	THIEF_ENUM_VALUE (ZONE_2, "zone2"),
	THIEF_ENUM_VALUE (ZONE_3, "zone3"),
	THIEF_ENUM_VALUE (ZONE_4, "zone4"),
	THIEF_ENUM_VALUE (ZONE_5, "zone5"),
	THIEF_ENUM_VALUE (ZONE_6, "zone6"),
	THIEF_ENUM_VALUE (ZONE_7, "zone7"),
	THIEF_ENUM_VALUE (ZONE_8, "zone8"),
)



// EnvironmentMapZone

// There isn't much value in making a 64-item list here.
THIEF_ENUM_CODING (EnvironmentMapZone, VALUE, VALUE)



// Fog

THIEF_ENUM_CODING (Fog::Zone, BOTH, VALUE,
	THIEF_ENUM_VALUE (DISABLED, "disabled"),
	THIEF_ENUM_VALUE (GLOBAL, "global"),
	THIEF_ENUM_VALUE (ZONE_1, "zone1"),
	THIEF_ENUM_VALUE (ZONE_2, "zone2"),
	THIEF_ENUM_VALUE (ZONE_3, "zone3"),
	THIEF_ENUM_VALUE (ZONE_4, "zone4"),
	THIEF_ENUM_VALUE (ZONE_5, "zone5"),
	THIEF_ENUM_VALUE (ZONE_6, "zone6"),
	THIEF_ENUM_VALUE (ZONE_7, "zone7"),
	THIEF_ENUM_VALUE (ZONE_8, "zone8"),
)

Fog::Fog (const Color& _color, float _distance)
	: color (_color), distance (_distance)
{}

float
Fog::interpolate_distance (bool global, float from, float to, float weight,
	Curve curve)
{
	float multiplier = global ? 1000.0f : 10.0f;
	if (weight >= 1.0f)
		return to;
	else if (from == 0.0f)
		from = to * multiplier;
	else if (to == 0.0f)
		to = from * multiplier;
	return Thief::interpolate (from, to, weight, curve);
}



// Precipitation

Precipitation::Precipitation ()
	: type (Type::SNOW), frequency (0.0f), speed (0.0f),
	  visible_distance (0.0f), radius (0.0f), opacity (0.0f),
	  brightness (0.0f), snow_jitter (0.0f), rain_length (0.0f),
	  splash_frequency (0.0f), splash_radius (0.0f), splash_height (0.0f),
	  splash_duration (0.0f), texture (), wind (0.0f, 0.0f, 0.0f)
{}

THIEF_ENUM_CODING (Precipitation::Type, CODE, CODE,
	THIEF_ENUM_VALUE (SNOW, "snow"),
	THIEF_ENUM_VALUE (RAIN, "rain"),
)



// Mission

bool
Mission::is_fm ()
{
	LGString junk;
	return SService<IVersionSrv> (LG)->GetCurrentFM (junk) != S_FALSE;
}

String
Mission::get_fm_name ()
{
	LGString name;
	SService<IVersionSrv> (LG)->GetCurrentFM (name);
	return name;
}

String
Mission::get_fm_path ()
{
	LGString name;
	SService<IVersionSrv> (LG)->GetCurrentFMPath (name);
	return name;
}

String
Mission::get_path_in_fm (const String& relative_path)
{
	LGString path;
	SService<IVersionSrv> (LG)->FMizePath (relative_path.data (), path);
	if ((const char*) path == relative_path.data ()) path.owned = false;
	return path;
}

#ifdef IS_THIEF2
int
Mission::get_number ()
{
	return SService<IDarkGameSrv> (LG)->GetCurrentMission ();
}
#endif // IS_THIEF2

String
Mission::get_mis_file ()
{
	LGString file;
	SService<IVersionSrv> (LG)->GetMap (file);
	return file;
}

String
Mission::get_gam_file ()
{
	LGString file;
	SService<IVersionSrv> (LG)->GetGamsys (file);
	return file;
}



// Mission: disposition

#ifdef IS_THIEF2
void
Mission::set_next (int number)
{
	SService<IDarkGameSrv> (LG)->SetNextMission (number);
}
#endif // IS_THIEF2

void
Mission::fade_to_black (Time duration)
{
	SService<IDarkGameSrv> (LG)->FadeToBlack (duration / 1000.0f);
}

void
Mission::end (bool win)
{
	if (win)
	{
		if (!Engine::has_config ("no_endgame"))
			Engine::run_command ("win_mission");
	}
	else
		SService<IDarkGameSrv> (LG)->EndMission ();
}

void
Mission::quick_save ()
{
	Engine::run_command ("quick_save");
}

void
Mission::quick_load ()
{
	Engine::run_command ("quick_load");
}



// Mission: difficulty

Difficulty
Mission::get_difficulty ()
{
	return Difficulty (QuestVar ("difficulty").get ());
}

bool
Mission::check_difficulty (Difficulty allowed)
{
	Difficulty actual = get_difficulty ();
	switch (allowed)
	{
	case Difficulty::NOT_NORMAL: return actual != Difficulty::NORMAL;
	case Difficulty::NOT_EXPERT: return actual != Difficulty::EXPERT;
	case Difficulty::ANY:        return true;
	default:                     return actual == allowed;
	}
}



// Mission: rendering effects

void
Mission::set_envmap_texture (EnvironmentMapZone zone, const String& texture)
{
	if (Engine::get_version () < Version (1, 20))
		throw std::runtime_error
			("SetEnvMapTexture not implemented before version 1.20");

	SService<IEngineSrv> (LG)->SetEnvMapZone
		(int (zone), texture.empty () ? nullptr : texture.data ());
}

Fog
Mission::get_fog (Fog::Zone zone)
{
	Fog fog; int red = 0, green = 0, blue = 0;

	if (zone == Fog::GLOBAL)
		SService<IEngineSrv> (LG)->GetFog
			(red, green, blue, fog.distance);

	else if (zone > Fog::GLOBAL && zone <= Fog::_MAX_ZONE)
		SService<IEngineSrv> (LG)->GetFogZone
			(zone, red, green, blue, fog.distance);

	fog.color = Color (red, green, blue);
	return fog;
}

void
Mission::set_fog (Fog::Zone zone, const Fog& fog)
{
	if (zone == Fog::GLOBAL)
		SService<IEngineSrv> (LG)->SetFog (fog.color.red,
			fog.color.green, fog.color.blue, fog.distance);

	else if (zone > Fog::GLOBAL && zone <= Fog::_MAX_ZONE)
		SService<IEngineSrv> (LG)->SetFogZone (zone, fog.color.red,
			fog.color.green, fog.color.blue, fog.distance);
}

Precipitation
Mission::get_precipitation ()
{
	Precipitation precip;
	int type;
	LGString texture;
	LGVector wind;
	SService<IEngineSrv> (LG)->GetWeather (type, precip.frequency,
		precip.speed, precip.visible_distance, precip.radius,
		precip.opacity, precip.brightness, precip.snow_jitter,
		precip.rain_length, precip.splash_frequency,
		precip.splash_radius, precip.splash_height,
		precip.splash_duration, texture, wind);
	precip.type = Precipitation::Type (type);
	precip.texture = (const char*) texture;
	precip.wind = wind;
	return precip;
}

void
Mission::set_precipitation (const Precipitation& precip)
{
	SService<IEngineSrv> (LG)->SetWeather (int (precip.type),
		precip.frequency, precip.speed, precip.visible_distance,
		precip.radius, precip.opacity, precip.brightness,
		precip.snow_jitter, precip.rain_length, precip.splash_frequency,
		precip.splash_radius, precip.splash_height,
		precip.splash_duration, precip.texture.data (),
		LGVector (precip.wind));
}



// Mission: text messages and books

String
Mission::get_text (const String& directory, const String& file,
	const String& name)
{
	LGString result;
	SService<IDataSrv> (LG)->GetString (result, file.data (),
		name.data (), "", directory.data ());
	return result;
}

Time
Mission::calc_text_duration (const String& text, Time word_duration)
{
	if (text.empty ()) return 0ul;

	size_t words = 0u, word_length = 0u;

	for (char ch : text)
		if (!std::isspace (ch))
			++word_length;
		else if (word_length > 0u)
		{
			if (word_length >= 3u) ++words;
			word_length = 0u;
		}

	if (word_length >= 3) ++words;

	return word_duration * std::max (10u, words);
}

void
Mission::show_text (const String& text, Time duration, const Color& color)
{
	if (duration == 0ul) duration = calc_text_duration (text);
	SService<IDarkUISrv> (LG)->TextMessage (text.data (), color, duration);
}

String
Mission::get_book_text (const String& book, int page)
{
	std::ostringstream file, name;
	file << "..\\books\\" << book;
	name << "page_" << page;
	return get_text ("strings", file.str (), name.str ());
}

void
Mission::show_book (const String& book, const String& art)
{
	SService<IDarkUISrv> (LG)->ReadBook (book.data (), art.data ());
}



// Mission: other information screens

void
Mission::show_map ()
{
	Engine::run_command ("automap");
}

#ifdef IS_THIEF2

bool
Mission::has_visited_automap_location (int page, int location)
{
	return SService<IDarkGameSrv> (LG)->GetAutomapLocationVisited
		(page, location);
}

void
Mission::visit_automap_location (int page, int location)
{
	SService<IDarkGameSrv> (LG)->SetAutomapLocationVisited (page, location);
}

#endif // IS_THIEF2

void
Mission::show_objectives ()
{
	Engine::run_command ("objectives");
}

void
Mission::show_image (const String& image)
{
	Engine::run_command ("show_image", image);
}

void
Mission::show_movie (const String& movie)
{
	Engine::run_command ("movie", movie);
}

void
Mission::show_menu ()
{
	Engine::run_command ("sim_menu");
}

void
Mission::show_load_screen ()
{
	Engine::run_command ("load_game");
}

void
Mission::show_save_screen ()
{
	Engine::run_command ("save_game");
}



} // namespace Thief

