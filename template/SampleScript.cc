/******************************************************************************
 *  SampleScript.cc
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

