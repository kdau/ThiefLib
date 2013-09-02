/******************************************************************************
 *  SampleScript.cc
 *
 *  Part of the template for ThiefLib script modules
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 *
 *****************************************************************************/

#include "SampleScript.hh"

SampleScript::SampleScript (const String& _name, const Object& _host)
	: Script (_name, _host)
{}

Message::Result
SampleScript::on_frob_world_end (FrobMessage&)
{
	Mission::show_text ("Hello world!");
	return Message::CONTINUE;
}

