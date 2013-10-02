/*! \file module.cc
 * Part of the template for ThiefLib script modules
 * Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 */

// CUSTOMIZE to include your scripts' header files instead.
#include "SampleScript.hh"

#include "version.rc"
#include <Thief/Module.hh>

THIEF_MODULE_BEGIN (MODULE_NAME)
	// CUSTOMIZE to a list of THIEF_SCRIPT calls for each of your scripts.
	THIEF_SCRIPT ("SampleScript", "Script", SampleScript)
THIEF_MODULE_END

