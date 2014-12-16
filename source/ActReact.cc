/******************************************************************************
 *  ActReact.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
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
 *
 *****************************************************************************/

#include "Private.hh"

namespace Thief {

//TODO Wrap everything in <lg/actreact.h> not covered below.



// Reaction

const Reaction
Reaction::NONE (0);

Reaction::Reaction (const String& name)
	: number (SService<IActReactSrv> (LG)->GetReactionNamed (name.data ()))
{
	if (*this == NONE)
		throw MissingResource (MissingResource::REACTION, name,
			Object::NONE);
}

String
Reaction::get_name () const
{
	LGString name;
	SService<IActReactSrv> (LG)->GetReactionName (name, number);
	return name;
}



// Stimulus
//TODO wrap property: Game: Dark\BloodMaxDamage = BloodMaxDmg

PROXY_CONFIG (Stimulus, causes_blood, "BloodCause", nullptr, bool, false);
PROXY_CONFIG (Stimulus, causes_knockout, "StimKO", nullptr, bool, false);

OBJECT_TYPE_IMPL_ (Stimulus,
	PROXY_INIT (causes_blood),
	PROXY_INIT (causes_knockout)
)

bool
Stimulus::is_stimulus () const
{
	// There is no property that all stimuli and only stimuli share. The
	// only available test is inheritance from the standard base archetype
	// for stimuli; this would break on any gamesys where it had been
	// renamed.
	return inherits_from (Object ("Stimulus"));
}



// Reagent

OBJECT_TYPE_IMPL (Reagent)

//TODO wrap method: IActReactSrv->React

#ifndef IS_OSL

void
Reagent::stimulate (const Stimulus& stimulus, float intensity,
	const Object& source)
{
#ifndef IS_THIEF2
	(void) source;
#endif
	SService<IActReactSrv> (LG)->Stimulate
		(number, stimulus.number, intensity
#ifdef IS_THIEF2
		, source.number
#endif
		);
}

#endif // !IS_OSL

void
Reagent::subscribe_stimulus (const Stimulus& stimulus)
{
	SService<IActReactSrv> (LG)->SubscribeToStimulus
		(number, stimulus.number);
}

void
Reagent::unsubscribe_stimulus (const Stimulus& stimulus)
{
	SService<IActReactSrv> (LG)->UnsubscribeToStimulus
		(number, stimulus.number);
}

//TODO wrap method: IActReactSrv->BeginContact
//TODO wrap method: IActReactSrv->EndContact
//TODO wrap method: IActReactSrv->SetSingleSensorContact
//TODO wrap property: Act/React\Source Scale = arSrcScale



// StimulusMessage

//TODO Create, wrapping sStimMsg ("*Stimulus").



// ReceptronLink (sReceptron)
// StimSensorLink (tSensorCount)
// ReactParamLink (ReceptronID)
// arSrcLink (sStimSourceData)
// arSrcDescLink (sStimSourceDesc)
// FlowContactLink (sContact)
// ScriptContactLink (sContact)

//TODO Create each if structure data is relevant, not covered elsewhere, and can be safely exposed.



} // namespace Thief

