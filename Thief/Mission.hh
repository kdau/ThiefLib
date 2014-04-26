//! \file Mission.hh Global aspects of a mission and its interface

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

#ifndef THIEF_MISSION_HH
#define THIEF_MISSION_HH

#include <Thief/Base.hh>

namespace Thief {



/*! A difficulty level for a mission.
 * For the %Thief games, the difficulty may alter a variety of mission aspects
 * including the player's vulnerability to injury, which objectives are required
 * of the player, what restrictions are applied to the player's actions, how
 * many NPCs are encountered, or whether doors are locked, unlocked, or open.
 * Scripts may, of course, make difficulty-based adjustments at any time by
 * calling Mission::get_difficulty() or Mission::check_difficulty(). */
enum class Difficulty
{
	NOT_EXPERT = -4, /*!< Matches \a NORMAL or \a HARD but not \a EXPERT.
	                  * This value is meant for Mission::check_difficulty()
	                  * and is not a valid difficulty setting. */
	NOT_NORMAL = -3, /*!< Matches \a HARD or \a EXPERT but not \a NORMAL.
	                  * This value is meant for Mission::check_difficulty()
	                  * and is not a valid difficulty setting. */
	ANY = -2,        /*!< Matches any of the three difficulties.
	                  * This value is meant for Mission::check_difficulty()
	                  * and is not a valid difficulty setting. */
	NONE = -1,       /*!< Matches none of the three difficulties.
	                  * This value is meant for Mission::check_difficulty()
	                  * and is not a valid difficulty setting. */
	NORMAL = 0,      //!< The lowest of the three %Thief difficulties.
	HARD = 1,        //!< The middle of the three %Thief difficulties.
	EXPERT = 2       //!< The highest of the three %Thief difficulties.
};



/*! A message indicating the chosen difficulty level for the mission.
 * The message is sent immediately after mission load, before even the
 * \c BeginScript message. It is only sent to scripts hosted on objects whose
 * \c %Difficulty\\%Script property has the chosen difficulty enabled.
 * \note %Message name: \c Difficulty */
class DifficultyMessage : public Message
{
public:
	/*! Constructs a new \c %Difficulty message.
	 * \warning Since the \c %Difficulty message is expected prior to the
	 * start of the mission, synthesizing it later could have unexpected
	 * consequences. */
	DifficultyMessage (Difficulty);

	THIEF_MESSAGE_WRAP (DifficultyMessage);

	//! The chosen difficulty level.
	const Difficulty difficulty;
};



/*! A group of settings for the fog rendering effect.
 * In NewDark, each room may be in one of eight fog zones, use the mission's
 * global fog, or have no fog at all, as set by Room::fog_zone. The global fog
 * and each of the fog zones may be configured individually. Use the
 * Mission::get_fog() and Mission::set_fog() methods to populate and apply these
 * settings. */
struct Fog
{
	//! A zone with independent fog settings.
	enum Zone
	{
		DISABLED = -1, /*!< Disables fog for a room.
		                * This value is valid for Room::fog_zone but not
		                * for Mission::set_fog(). */
		GLOBAL = 0,    //!< Uses or accesses the global fog settings.
		ZONE_1,        //!< Uses or accesses the settings for fog zone 1.
		ZONE_2,        //!< Uses or accesses the settings for fog zone 2.
		ZONE_3,        //!< Uses or accesses the settings for fog zone 3.
		ZONE_4,        //!< Uses or accesses the settings for fog zone 4.
		ZONE_5,        //!< Uses or accesses the settings for fog zone 5.
		ZONE_6,        //!< Uses or accesses the settings for fog zone 6.
		ZONE_7,        //!< Uses or accesses the settings for fog zone 7.
		ZONE_8,        //!< Uses or accesses the settings for fog zone 8.
//! \cond HIDDEN_SYMBOLS
		_MAX_ZONE = 8
//! \endcond
	};

	//! Constructs the given fog settings.
	Fog (const Color& color = Color (), float distance = 0.0f);

	//! The color of the fog when at full opacity.
	Color color;

	/*! The distance from the player at which the fog should be at full
	 * opacity, in feet (DU). A distance of zero disables fog in the zone so
	 * configured. */
	float distance;

	/*! Interpolates between two fog distance values.
	 * This is a replacement for the global interpolate() function. It
	 * properly interpolates between disabled (\c 0.0) and enabled (<tt>&gt;
	 * 0.0</tt>) fog, by substituting a very large distance value for zero
	 * at intermediate points.
	 *
	 * The \a global argument indicates whether to use a much larger
	 * multiplier (\c 1000.0 instead of \c 10.0) in calculating the large
	 * distance. This is often needed for the global fog since it applies to
	 * the NewSky skybox, whose camera distance is usually very large. */
	static float interpolate_distance (bool global, float from, float to,
		float weight = 0.5f, Curve = Curve::LINEAR);
};



/*! A group of settings for the precipitation (weather) particle effect.
 * %Precipitation is a relatively efficient particle effect that may configured
 * at the mission level. Individual rooms may enable or disable it
 * (Room::precipitation). %Precipitation will only fall from the "sky hack"
 * texture (\c 249). Use the Mission::get_precipitation() and
 * Mission::set_precipitation() methods to populate and apply these settings. */
struct Precipitation
{
	//! Constructs default precipitation settings.
	Precipitation ();

	//! A type of precipitation appearance and behavior.
	enum class Type
	{
		SNOW, /*!< The precipitation drops should fall like snow.
		       * The #snow_jitter variable is relevant for \a SNOW, but
		       * the #rain_length and splash variables are not.
		       * Falling leaves also use this type. */
		RAIN  /*!< The precipitation drops should fall like rain.
		       * The #rain_length and splash variables are relevant for
		       * \a RAIN, but the #snow_jitter variable is not. */
	};

	/*! The type of precipitation that will fall.
	 * While the appearance of the drops is largely controlled by the
	 * #texture, the \a type controls details of the drops' movement,
	 * including which of the type-specific variables are relevant. */
	Type type;

	/*! How many new drops should fall per second.
	 * %Precipitation is disabled by setting this variable to zero. */
	float frequency;

	//! How fast each drop should fall, in feet (DU) per second.
	float speed;

	/*! How far away from the player the drops may be rendered.
	 * Drops will only be simulated between the camera location and a circle
	 * of this radius. If an area open to the sky is visible but outside
	 * this radius, for instance, no precipitation will be rendered for it.
	 *
	 * This variable also controls how far above the camera drops are
	 * created. If a point above the camera is within the horizontal radius
	 * but outside of the world (inside a solid), no drops will originate
	 * there. */
	float visible_distance;

	//! The horizontal radius of each drop in feet (DU).
	float radius;

	/*! The opacity of the drops, between \c 0.0 (transparent) and \c 1.0
	 * (opaque). */
	float opacity;

	/*! The brightness of the drops, between \c 0.0 (black) and \c 1.0
	 * (full value from the texture). */
	float brightness;

	/*! How far the drops should randomly drift as they fall, in feet (DU).
	 * \pre This variable only applies to Type::SNOW precipitation. */
	float snow_jitter;

	/*! The vertical length of each drops in feet (DU).
	 * \pre This variable only applies to Type::RAIN precipitation. */
	float rain_length;

	/*! How often a drop-on-ground collision should produce a splash.
	 * The frequency ranges from \c 0.0 (never) to \c 1.0 (always).
	 * \pre This variable only applies to Type::RAIN precipitation. */
	float splash_frequency;

	/*! The horizontal radius of any splashes produced, in feet (DU).
	 * \pre This variable only applies to Type::RAIN precipitation. */
	float splash_radius;

	/*! The vertical length of any splashes produced, in feet (DU).
	 * \pre This variable only applies to Type::RAIN precipitation. */
	float splash_height;

	/*! How long any splashes produced should remain visible.
	 * \pre This variable only applies to Type::RAIN precipitation. */
	float splash_duration;

	/*! The name of the bitmap image file to use as a drop texture.
	 * All precipitation textures must be located in the game's or FM's
	 * \c bitmap directory. */
	String texture;

	//! How much wind shear should be applied to the drops per second.
	Vector wind;
};



/*! Control of an overall mission, its interface, and global aspects of the game
 * world. The Mission class is not instantiable; all members are static.
 *
 * Fan-mission-related methods only return meaningful values if the player is
 * using a fan mission loader such as FMSel that implements NewDark's FM
 * Selector API. Other loaders do not provide the needed information to the
 * engine.
 *
 * \nosubgrouping */
class Mission
{
public:
	//! \name Identity
	//@{

	/*! Returns whether a fan mission is currently loaded.
	 * \pre The return value is only meaningful if the player is using an
	 * FM Selector API-compatible fan mission loader. */
	static bool is_fm ();

	/*! Returns the short, internal name of the current fan mission.
	 * If no fan mission is loaded, returns an empty string.
	 * \pre The return value is only meaningful if the player is using an
	 * FM Selector API-compatible fan mission loader. */
	static String get_fm_name ();

	/*! Returns the path to the current fan mission's main directory.
	 * The path is relative to the main game directory.
	 * If no fan mission is loaded, returns an empty string.
	 * \pre The return value is only meaningful if the player is using an
	 * FM Selector API-compatible fan mission loader. */
	static String get_fm_path ();

	/*! Prepends the current fan mission's path to the given one.
	 * The resulting path is relative to the main game directory.
	 * If no fan mission is loaded, returns \a relative_path unmodified.
	 * Since pre-NewDark fan mission loaders work by copying files into
	 * the main game directory, this method should work with any fan
	 * mission loader. */
	static String get_path_in_fm (const String& relative_path);

#ifdef IS_THIEF2
	/*! Returns the mission number of the current mission.
	 * This method always returns \c 0 in DromEd. \t2only */
	static int get_number ();
#endif

	/*! Returns the name of the currently loaded mission file.
	 * No directory component is included. */
	static String get_mis_file ();

	/*! Returns the name of the currently loaded gamesys file.
	 * No directory component is included. */
	static String get_gam_file ();

	//@}
	//! \name Disposition
	//@{

#ifdef IS_THIEF2
	/*! Sets which mission will begin after this one ends in success.
	 * The change will still be subject to the settings in \c missflag.str:
	 * if the current mission has the \c end flag set, the campaign will
	 * end anyway; if the requested next mission has the \c skip flag set,
	 * the engine will continue to the mission after it. This method has no
	 * effect in DromEd game mode. \warning The engine does not check the
	 * validity of mission numbers, and will crash if it tries to load a
	 * nonexistent mission. \t2only */
	static void set_next (int number);
#endif

	/*! Fades the player's view to black.
	 * The view cannot be faded back in again, so this method is only
	 * suitable for use at mission end. See Flash for an alternative. 
	 * This method has no effect in DromEd game mode if \c no_endgame is set.
	 * \param duration How long the fade should last. */
	static void fade_to_black (Time duration);

	/*! Ends the mission immediately.
	 * This method has no effect in DromEd game mode if \c no_endgame is set.
	 * \param win Whether the mission is a success (\c true) or
	 * failure (\c false). */
	static void end (bool win = false);

	/*! Saves the current game to the quick save slot.
	 * The player will see the usual on-screen notification. */
	static void quick_save ();

	/*! Loads the game saved in the quick save slot.
	 * The current game will be lost. */
	static void quick_load ();

	//@}
	//! \name Difficulties
	//@{

	/*! Returns the chosen difficulty level for the current mission.
	 * If no difficulty has been set (such as in DromEd game mode), returns
	 * Difficulty::NORMAL to match the behavior of stock scripts. */
	static Difficulty get_difficulty ();

	/*! Returns whether the chosen difficulty matches the given difficulty.
	 * If \a allowed is a single difficulty, it must match exactly; if it
	 * is a range such as Difficulty::NOT_NORMAL, it can match any
	 * difficulty in that range. If no difficulty has been set (such as in
	 * DromEd game mode), matches against Difficulty::NORMAL. */
	static bool check_difficulty (Difficulty allowed);

	//@}
	//! \name Rendering effects
	//@{

	/*! Sets the texture associated with the given environment map zone.
	 * NewDark's environment mapping feature supports two types of texture
	 * specification: a map texture associated with each base texture, or a
	 * a map texture associated with each room that any environment-mapped
	 * base texture in that room may use. To implement the second type, each
	 * room is assigned to one of 64 zones, the first of which is a "global"
	 * zone. \param zone An environment map zone number, between 0 and 63.
	 * \param texture A path a bitmap image file, relative to the game or FM
	 * main  directory. The file should not be used as a terrain or object
	 * texture in the same mission. An empty string clears the texture for
	 * the given zone. */
	static void set_envmap_texture (unsigned zone, const String& texture);

	//! Returns the current Fog settings for the given zone.
	static Fog get_fog (Fog::Zone);

	//! Sets the Fog for the given zone to the given settings.
	static void set_fog (Fog::Zone, const Fog&);

	//! Returns the mission's current Precipitation (weather) settings.
	static Precipitation get_precipitation ();

	//! Sets the mission's Precipitation (weather) to the given settings.
	static void set_precipitation (const Precipitation&);

	//@}
	//! \name Text messages and books
	//@{

	/*! Returns a translated string from a resource file.
	 * \param directory The resource directory to search in, such as \c
	 * "strings". For the \c "books" directory, use \c "strings" here and
	 * prefix the \a file with \c "..\\books\\" (or use get_book_text ()).
	 * \param file The name of the strings file to use, without the \c ".str"
	 * extension. \param name The name of the string to retrieve. */
	static String get_text (const String& directory, const String& file,
		const String& name);

	/*! Calculates an appropriate on-screen display duration for a string.
	 * The algorithm multiplies the \a word_duration by the number of words
	 * of three letters or more in the string, or by 10 if there are fewer
	 * than 10 words of that length. */
	static Time calc_text_duration (const String& text,
		Time word_duration = 500ul);

	/*! Displays the given text message on screen.
	 * \param text The text string to display. Line breaks in the string
	 * will be preserved. Like all on-screen text in %Thief, the character
	 * set is limited and may vary by font. \param duration How long the
	 * text may be displayed. The actual time may be shorter if another
	 * message is displayed before it has elapsed. \param color The color
	 * of the displayed text. By default (or if \c #000000 is passed
	 * explicitly), the text will be displayed in the color defined by the
	 * \c game_color_text config variable. */
	static void show_text (const String& text, Time duration = 0ul,
		const Color& color = Color ());

	/*! Returns the translated text of one page of a book file.
	 * \param book The name of the book file, without the \c ".str"
	 * extension. \param page The page to retrieve. */
	static String get_book_text (const String& book, int page = 0);

	/*! Suspends game mode and displays the given book.
	 * \param book The name of the book file, without the \c ".str"
	 * extension. \param art The name of the book interface art directory
	 * to use. \param reload Whether to check for a new version of the book
	 * file on disk. For efficiency, this should not be \c true unless
	 * needed for a specific situation. \a reload works in the main game
	 * executable as well as DromEd. */
	static void show_book (const String& book, const String& art,
		bool reload = false);

	//@}
	//! \name Other metagame screens
	//@{

	//! Suspends game mode and displays the map interface.
	static void show_map ();

#ifdef IS_THIEF2

	/*! Returns whether the player has visited the given automap location.
	 * Automaps display the player's current location and previously visited
	 * locations based on the Room::automap_page and Room::automap_location
	 * values of rooms the player has entered. A \a page is a screen in the
	 * map interface, while a \a location is a highlightable section of one
	 * map image. */
	static bool has_visited_automap_location (int page, int location);

	/*! Marks the given automap location as having been visited.
	 * The marking of locations is usually handled automatically, but a
	 * script can manually add locations with this method. The feature has
	 * been used to incrementally reveal sections of a player character's
	 * notes, sketches, etc. on "map" pages. */
	static void visit_automap_location (int page, int location);

#endif // IS_THIEF2

	//! Suspends game mode and displays the objectives screen.
	static void show_objectives ();

	/*! Suspends game mode and displays the image at the given path.
	 * The image will be displayed centered against a black background. Any
	 * click or non-modifier keypress will return to game mode. If the
	 * requested image is not found or cannot be loaded, a blank screen is
	 * shown. \param image A path to a bitmap image file, relative to the
	 * game or FM main directory. The extension may be included or omitted. */
	static void show_image (const String& image);

	/*! Suspends game mode and plays the video file at the given path.
	 * \param video A path to a video file, relative to the game or FM main
	 * directory and including the extension. A bare file name may be given
	 * if the file is in the \c movies directory. If the requested video is
	 * not found or cannot be loaded, nothing happens.
	 * \note Any environmental ambient is stopped before the video plays.
	 * Since the in-mission video feature was intended for the success
	 * and failure videos, it does not reactivate any environmental ambient
	 * after returning to game mode. */
	static void show_video (const String& video);

	//! Suspends game mode and displays the in-mission menu.
	static void show_menu ();

	//! Suspends game mode and displays the game loading interface.
	static void show_load_screen ();

	//! Suspends game mode and displays the game saving interface.
	static void show_save_screen ();

	//@}
};



} // namespace Thief

#endif // THIEF_MISSION_HH

