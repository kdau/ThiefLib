/******************************************************************************
 *  module.cc
 *
 *  Part of the template for ThiefLib script modules
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "SampleScript.hh" //CUSTOMIZE to include your scripts' headers

#include "version.rc"
#include <Thief/module.hh>

THIEF_MODULE_BEGIN (MODULE_NAME)
	THIEF_SCRIPT ("SampleScript", "Script", SampleScript) //CUSTOMIZE
THIEF_MODULE_END

