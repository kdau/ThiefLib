/******************************************************************************
 *  SampleScript.hh
 *
 *  Part of the template for ThiefLib script modules
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 *
 *****************************************************************************/

#ifndef SAMPLESCRIPT_HH
#define SAMPLESCRIPT_HH

#include <Thief/Thief.hh>
using namespace Thief;

class SampleScript : public Script
{
public:
	SampleScript (const String& name, const Object& host);

private:
	Message::Result on_frob_world_end (FrobMessage&);
};

#endif // SAMPLESCRIPT_HH

