/*! \file SampleScript.cc
 * Part of the template for ThiefLib script modules
 * Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 */

#include "SampleScript.hh"

SampleScript::SampleScript (const String& _name, const Object& _host)
	: Script (_name, _host)
{
	listen_message ("Sim", &SampleScript::on_sim);
	listen_message ("FrobWorldEnd", &SampleScript::on_frob_world_end);
}

Message::Result
SampleScript::on_sim (SimMessage& message)
{
	if (message.event == SimMessage::START)
		Mission::show_text ("Hello world!");
	else // event == FINISH
		log (Log::INFO, "Goodbye world!");
	return Message::HALT;
}

Message::Result
SampleScript::on_frob_world_end (FrobMessage&)
{
	Mission::show_text ("Stop, that tickles!");
	return Message::HALT;
}

