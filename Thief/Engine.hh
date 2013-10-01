/*! \file Engine.hh Low-level functions of the Dark %Engine and information on
 * its status.
 */
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

#ifndef THIEF_ENGINE_HH
#define THIEF_ENGINE_HH

#include <Thief/Base.hh>
#include <Thief/Message.hh>

namespace Thief {



/*! Handles all logging output intended for mission authors.
 * This class is an output stream for logging messages about ThiefLib and script
 * activity that are intended for those creating, editing, and testing missions
 * in DromEd. It should not be used for messages intended for players. */
class Monolog : public std::ostream
{
public:
	/*! Constructs a logging stream.
	 * The stream initially has no buffer and discards all data. */
	Monolog ();

	//! Destroys a logging stream.
	virtual ~Monolog ();

	/*! Attaches the logging stream to the given MPrintf function.
	 * After attachment, buffered data will be sent to this function at
	 * line end or after a certain number of characters are pending.
	 * \warning ThiefLib attaches the #mono object to the DromEd monolog
	 * automatically. This method should not usually be called by others. */
	void attach (MPrintfProc proc);

	/*! Sends the given string directly to the log, bypassing the buffer.
	 * A newline will be added at the end of the string, whether or not one
	 * is already present. While the given line will not be interrupted,
	 * it is possible that it will interrupt a line already in progress. */
	void log (const String&);

	/*! Sends the given formatted string directly to the log, bypassing the
	 * buffer. A newline will be added at the end of the string, whether or
	 * not one is already present. While the given line will not be
	 * interrupted, it is possible that it will interrupt a line already in
	 * progress. */
	void log (const boost::format&);

private:
	class Streambuf;
	std::unique_ptr<Streambuf> buf;
};

/*! A logging stream that outputs to the monolog in DromEd.
 * The monolog can be viewed in the \c monolog.txt file or in a configurable
 * separate window. In the main game executable, the monolog does not exist and
 * any data sent to this stream is discarded. */
extern Monolog mono;

/*! A logging stream that discards any data sent to it.
 * This stream is always valid and may be used to conditionally discard some
 * log messages. */
extern Monolog null_mono;



//! Represents the size of an area of the canvas (screen) or the canvas itself.
struct CanvasSize
{
	//! The width of the area, in pixels.
	int w;

	//! The height of the area, in pixels.
	int h;

	//! Constructs a new size of zero width and height.
	CanvasSize ();

	//! Constructs a new size of the given width and height.
	CanvasSize (int w, int h);

	//! Returns whether the width and height are both non-negative.
	bool valid () const;

	//! Returns whether two canvas sizes are equal in both dimensions.
	bool operator == (const CanvasSize&) const;

	//! Returns whether two canvas sizes are unequal in either dimension.
	bool operator != (const CanvasSize&) const;
};



//! Represents the version number of the Dark %Engine or a supporting module.
struct Version
{
	//! Constructs a new version number as specified.
	Version (int major, int minor);

	//! The major version of a file typically increases with major changes.
	int major;

	//! The minor version of a file typically increases with any minor change.
	int minor;

	//! Returns whether two version numbers are equal.
	bool operator == (const Version&) const;

	/*! Returns whether this version number is less than another.
	 * This method is suitable for sorting purposes. */
	bool operator < (const Version&) const;

	/*! Returns whether this version number is greater than or equal to
	 * another. This method can be used to check that the version number
	 * meets a given minimum requirement. */
	bool operator >= (const Version&) const;
};

//! Outputs the given version number to the given stream. \relates Version
std::ostream& operator << (std::ostream&, const Version&);



/*! Provides access to low-level Dark %Engine functions and status.
 * The Engine class is not instantiable; all members are static.
 *
 * Dark %Engine configuration variables are distributed across the following
 * configuration files:
 *   - \c cam.cfg (variables controllable by players in-game)
 *   - \c cam_ext.cfg (NewDark-specific variables)
 *   - \c dark.cfg (often distributed with FMs)
 *   - \c darkinst.cfg (variables relating to install paths)
 *   - \c dromed.cfg (variables used only by DromEd; not read the game executable)
 *   - \c menus.cfg (DromEd's menu hierarchy; not read by the game executable)
 *   - \c user.cfg (variables for hand editing by players)
 *
 * \nosubgrouping */
class Engine
{
public:
	//! \name Application identity
	//@{

	//! Returns the concise human-readable name of the running application.
	static String get_app_name ();

	/*! Returns a longer human-readable name of the running application.
	 * This name may contain various extraneous bits, including the version
	 * number. */
	static String get_long_app_name ();

	/*! Returns the version number of the running application.
	 * For NewDark, this is the NewDark patch version. */
	static Version get_version ();

	//@}
	//! \name Game mode and sim status
	//@{

	//! A mode of the Dark %Engine.
	enum class Mode
	{
		GAME, /*!< The engine is currently in the game interface.
		       * Unlike with e.g. GameModeMessage, this definition of
		       * "game mode" includes game menus, readables, and other
		       * secondary interface elements. */
		EDIT  /*!< The engine is currently in the mission editor window.
		       * This mode only applies to DromEd. */
	};

	//! Returns the current mode of the engine.
	static Mode get_mode ();

	//! Returns whether the application is an editor (DromEd).
	static bool is_editor ();

	//! Returns whether the simulation (a mission) is currently running.
	static bool is_sim ();

	//@}
	//! \name Graphics
	//@{

	/*! Returns the current size of the canvas (screen).
	 * This is primarily of use to HUD elements, but might also be useful
	 * for choosing resolution-appropriate effect detail levels, camera
	 * angles, etc. */
	static CanvasSize get_canvas_size ();

	//! Returns the current canvas (screen) aspect ratio.
	static float get_aspect_ratio ();

	//! Returns the major version of DirectX being used (6 or 9).
	static int get_directx_version ();

	/*! Returns whether the given object was rendered for the last frame.
	 * A rendered object may still not be visible for various other
	 * reasons (such as the bounding box but not any of the polys being
	 * on screen, or the opacity being very low or zero). */
	static bool rendered_this_frame (const Object&);

	//! A level of detail at which to raycast between points.
	enum class RaycastMode
	{
		TERRAIN = -1, //!< Check only for terrain between points.
		NEAREST,      /*!< Find the blocking terrain or object nearest
		               * to the origin of the ray. */
		ANY,          /*!< Find any blocking object or the blocking
		               * terrain nearest to the origin of the ray. */
		FAST          //!< Find any blocking terrain or object.
	};

	/*! Represents a hit, or lack thereof, while raycasting between points.
	 * Returned by the Engine::raycast methods. */
	struct RaycastHit
	{
		enum Type
		{
			NONE,    //!< Nothing was hit.
			TERRAIN, //!< A terrain poly was hit.
			OBJECT,  //!< A non-mesh object was hit.
			MESH     //!< A mesh object (AI or Rope) was hit.
		};

		//! The type of hit that occurred, if any.
		Type type;

		//! Where the hit occurred, if the #type is not #NONE.
		Vector location;

		//! The object that was hit, if the #type is #OBJECT or #MESH.
		Object object;

		//! Returns whether a hit occurred.
		operator bool () const { return type != NONE; }
	};

	/*! Tests whether anything lies between two points in the game world.
	 * This method performs a raycast between the given \a from and \a to
	 * points. Ray casting tests for intersections between a ray (the line
	 * segment between the points) and any terrain polygons or object models
	 * which may lie within it. The result can be used to determine whether
	 * two points have a line of sight to each other or to identify where
	 * a straight-line projectile or light source might hit.
	 *
	 * The \a mode determines how detailed of a raycast is performed. If
	 * \a include_mesh is \c false, mesh objects (which include AIs and
	 * ropes) will not be considered in the raycast.
	 *
	 * Ray casting is an expensive operation, less so for the #TERRAIN mode
	 * but especially for the #NEAREST mode. Use it in moderation. */
	static RaycastHit raycast (RaycastMode mode, const Vector& from,
		const Vector& to, bool include_mesh = true);

	/*! Tests whether anything lies between two objects in the game world.
	 * This method is similar to the Vector overload of raycast(), except
	 * that it uses the centroids of objects \a from and \a to as the ends
	 * of the ray. These two objects will also be ignored by the raycast.
	 * See the Vector overload for more information. */
	static RaycastHit raycast (RaycastMode mode, const Object& from,
		const Object& to, bool include_mesh = true);

	//@}
	//! \name Configuration
	//@{

	//! Returns whether the given variable is set in any configuration file.
	static bool has_config (const String& variable);

	/*! Returns the current value of the given configuration variable.
	 * This template is only defined for \c int, \c float, and String
	 * return types. */
	template <typename T> static T get_config (const String& variable);

	/*! Returns the current value of the given binding config variable.
	 * The binding config variables are stored in <tt>.bnd</tt> files
	 * instead of <tt>.cfg</tt> files, and are always <tt>float</tt>s. */
	static float get_binding_config (const String& variable);

	//! Returns whether the given command is bound to any key combination.
	static bool is_command_bound (const String& command);

	/*! Returns the key combination to which the given command is bound.
	 * Returns an empty string if the command is not bound. */
	static String get_command_binding (const String& command);

	//@}
	//! \name Miscellaneous
	//@{

	/*! Locates a file within one of the game install paths.
	 * \param type The name of the config variable whose value is a plus-
	 * sign-separated list of paths to search in.
	 * \param file The name of, or relative path to, the file being sought.
	 * \return An absolute path to the file if found, or an empty string if
	 * not. */
	static String find_file_in_path (const String& type, const String& file);

	/*! Returns a pseudo-random integer between \a minimum and \a maximum,
	 * inclusive. */
	static int random_int (int minimum, int maximum);

	/*! Returns a pseudo-random floating-point value between 0.0 and 1.0,
	 * inclusive. */
	static float random_float ();

	/*! Returns a pseudo-random floating-point value between \a minimum and
	 *\a maximum, inclusive. */
	static float random_float (float minimum, float maximum);

	/*! Runs the given game \a command, with any given \a arguments.
	 * Multiple arguments may be specified in the \a arguments string
	 * separated by spaces. */
	static void run_command (const String& command,
		const String& arguments = String ());

	/*! Writes the given message to the game log.
	 * The game log is not the monolog; it is a separate file (\c thief.log,
	 * \c thief2.log, or \c dromed.log) that is written by the main game
	 * executable as well. \note The game log is rarely used by scripts.
	 * Few editors or players would think to check for output there. */
	static void write_to_game_log (const String&);

	//@}
};

//! \cond HIDDEN_SYMBOLS
template<> int Engine::get_config (const String& variable);
template<> float Engine::get_config (const String& variable);
template<> String Engine::get_config (const String& variable);
//! \endcond



/*! %Message about a change in the game mode.
 * A game mode message is sent whenever the user enters or leaves game mode due
 * to a menu or other interface screen being viewed, including at mission start
 * and end. \note %Message name: \c DarkGameModeChange */
class GameModeMessage : public Message
{
public:
	//! A type of change to the game mode.
	enum Event
	{
		SUSPEND, /*!< The player is leaving game mode to view a menu or
		          * at mission end. */
		RESUME   /*!< The player is entering game mode after viewing a
		          * menu or at mission start. */
	};

	//! Constructs a new \c DarkGameModeChange message for the given event.
	GameModeMessage (Event);

	THIEF_MESSAGE_WRAP (GameModeMessage);

	//! The type of change to the game mode.
	const Event event;
};



/*! %Message about a change in the simulation (mission) status.
 * A sim message is sent when the mission starts and when it ends.
 * \note %Message name: \c Sim
 * \note To supplement \c Sim, ThiefLib creates a generic message \c PostSim in
 * the cycle immediately following the starting SimMessage (#event == #START).
 * That corresponds to the time when other scripts' preparatory work, including
 * the spawning of the \c Player object, is complete. This \c PostSim message is
 * posted only to objects that host at least one ThiefLib script, though any
 * scripts on those objects will receive it. */
class SimMessage : public Message
{
public:
	//! A type of change to the simulation status.
	enum Event
	{
		START, //!< The simulation (mission) has just started.
		FINISH //!< The simulation (mission) is about to end.
	};

	//! Constructs a new \c Sim message for the given event.
	SimMessage (Event);

	THIEF_MESSAGE_WRAP (SimMessage);

	//! The type of change to the simulation status.
	const Event event;
};



} // namespace Thief

#endif // THIEF_ENGINE_HH

