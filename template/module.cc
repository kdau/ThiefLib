/*! \file module.cc
 * Part of the template for ThiefLib script modules
 * Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 */

#include <Thief/Thief.hh>
#include "version.rc"

// CUSTOMIZE to include all your scripts' header files.
#include "SampleScript.hh"

THIEF_MODULE (MODULE_NAME,
	// CUSTOMIZE to a list of THIEF_SCRIPT calls for each of your scripts.
	THIEF_SCRIPT ("SampleScript", "Script", SampleScript)
)

