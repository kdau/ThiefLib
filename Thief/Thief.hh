//! \file Thief.hh Main header file which includes all others.

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

#ifndef THIEF_THIEF_HH
#define THIEF_THIEF_HH

#include <Thief/Base.hh>
#include <Thief/AI.hh>
#include <Thief/ActReact.hh>
#include <Thief/Combat.hh>
#include <Thief/Engine.hh>
#include <Thief/HUD.hh>
#include <Thief/Link.hh>
#include <Thief/Message.hh>
#include <Thief/Mission.hh>
#include <Thief/Module.hh>
#include <Thief/Object.hh>
#include <Thief/Parameter.hh>
#include <Thief/Player.hh>
#include <Thief/Property.hh>
#include <Thief/QuestVar.hh>
#include <Thief/Rendering.hh>
#include <Thief/Script.hh>
#include <Thief/Security.hh>
#include <Thief/Sound.hh>
#include <Thief/Tweq.hh>
#include <Thief/Types.hh>

/*! \mainpage
 * ThiefLib is a modern wrapper library for creating custom script modules for
 * the Dark %Engine games %Thief II: The Metal Age and %Thief: The Dark Project
 * / %Thief Gold. It builds on the previous work of GayleSaver and Telliamed in
 * documenting the engine's often-perplexing script API while offering a
 * simplified, modern-C++ interface in which the first-class objects and methods
 * are coherently organized and clearly mapped to in-game results.
 *
 * ThiefLib is still in development and should be used with caution. It is the
 * base library for <a href="http://kdau.github.io/KDScript/">KDScript</a> as of
 * version 0.7.0 of that module, but many elements not used there have not seen
 * extensive testing. See the TODO list for other work remaining to be done.
 * Particularly, about half of the library is still undocumented.
 *
 * \section requirements Requirements
 *
 * The library itself requires the following to be compiled:
 *   - a recent (GCC 4.8 or later) MinGW build environment
 *   - the <a href="http://www.boost.org/">Boost</a> libraries (currently Format
 *     and Preprocessor, but possibly more later)
 *   - <a href="https://github.com/kdau/lg">my modified version</a> of
 *     Telliamed's lg library
 *
 * Script modules based on this library require only MinGW (GCC 4.8+) and Boost
 * to be compiled. To run, they require:
 *   - either Windows (XP or later) or Linux (with a recent version of Wine)
 *   - TMA and/or TG/TDP, depending on build options
 *   - the NewDark patch (preferably the latest version)
 *
 * \section credits Credits and legal information
 *
 * Copyright (C) 2012-2014 Kevin Daughtridge &lt;kevin AT kdau DOT com&gt;.
 *
 * ThiefLib contains or is adapted in part from the following works:
 *   - the <a href="https://github.com/whoopdedo/lg">lg library</a>, by
 *     Tom N Harris (Telliamed) &lt;telliamed AT whoopdedo DOT org&gt;
 *   - the <a href="https://github.com/whoopdedo/dh2">DarkHook 2</a> library,
 *     copyright (C) 2005-2011 Tom N Harris
 *   - the <a href="https://github.com/whoopdedo/scriptlib">Object Script
 *     Library</a> (ScriptLib), copyright (C) 2011 Tom N Harris
 *   - <a href="https://github.com/whoopdedo/publicscripts">Public Scripts</a>,
 *     copyright (C) 2005-2011 Tom N Harris
 *   - syntax conventions used in NVScript, by Nameless Voice
 *   - <a href="https://github.com/TheWatcher/twscript">TWScript</a>,
 *     copyright (C) 2013 Chris Page (The Watcher) &lt;chris AT starforge DOT co
 *     DOT uk&gt;
 *
 * This library is free software: you can redistribute it and/or modify it under
 * the terms of the <a href="http://www.gnu.org/licenses/gpl.html">GNU General
 * Public License</a> as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Further permission is granted to link any compiled binary module including
 * this library with any licensed copy of a Dark Engine game or level editor,
 * whether patched or unpatched, so long as the compiled work complies with the
 * terms of the appropriate End-User License Agreement issued by Eidos
 * Interactive.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 */

#endif // THIEF_THIEF_HH

