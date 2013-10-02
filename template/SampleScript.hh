/*! \file SampleScript.hh A very basic example script.
 * Part of the template for ThiefLib script modules
 * Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 */

#ifndef SAMPLESCRIPT_HH
#define SAMPLESCRIPT_HH

#include <Thief/Thief.hh>
using namespace Thief;

class SampleScript : public Script
{
public:
	SampleScript (const String& name, const Object& host);

private:
	Message::Result on_sim (SimMessage&);

	Message::Result on_frob_world_end (FrobMessage&);
	Parameter<bool> ticklish;
};

#endif // SAMPLESCRIPT_HH

