/******************************************************************************
 *  AI.cc
 *
 *  This file is part of ThiefLib, a library for Thief 1/2 script modules.
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts and the Object Script Library
 *  Copyright (C) 2005-2013 Tom N Harris <telliamed@whoopdedo.org>
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

#include "Private.hh"

namespace Thief {



// AI

PROXY_CONFIG (AI, mode, "AI_mode", nullptr, AI::Mode, Mode::NORMAL);
PROXY_CONFIG (AI, frozen_start, "AI_Frozen", "StartTime", Time, 0ul);
PROXY_CONFIG (AI, frozen_duration, "AI_Frozen", "Duration", Time, 0ul);
PROXY_CONFIG (AI, behavior_set, "AI", "Behavior set", String, "");
PROXY_CONFIG (AI, aggression, "AI_Aggression", nullptr,
	AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, aptitude, "AI_Aptitude", nullptr, AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, defensiveness, "AI_Defensive", nullptr,
	AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, dodginess, "AI_Dodginess", nullptr, AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, hearing, "AI_Hearing", nullptr, AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, sloth, "AI_Sloth", nullptr, AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, verbosity, "AI_Verbosity", nullptr, AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, vision, "AI_Vision", nullptr, AI::Rating, Rating::NONE);
PROXY_CONFIG (AI, time_warp, "TimeWarp", nullptr, float, 1.0f);
PROXY_CONFIG (AI, uses_doors, "AI_UsesDoors", nullptr, bool, true);
PROXY_CONFIG (AI, needs_big_doors, "AI_IsBig", nullptr, bool, false);
PROXY_NEG_CONFIG (AI, flash_vulnerable, "NoFlash", nullptr, bool, false);
PROXY_NEG_CONFIG (AI, head_tracks_player, "CretHTrack", nullptr, bool, false);
PROXY_NEG_CONFIG (AI, investigates, "AI_InvKnd", nullptr, bool, false);
PROXY_CONFIG (AI, patrols, "AI_Patrol", nullptr, bool, false);
PROXY_CONFIG (AI, patrols_randomly, "AI_PtrlRnd", nullptr, bool, false);
PROXY_CONFIG (AI, traverses_water, "AI_UseWater", nullptr, bool, false);
PROXY_CONFIG (AI, notices_bodies, "AI_NtcBody", nullptr, bool, true);
PROXY_NEG_CONFIG (AI, notices_cameras, "AI_IgCam", nullptr, bool, false);
PROXY_CONFIG (AI, notices_damage, "AI_NoticeDmg", nullptr, bool, true);
PROXY_NEG_CONFIG (AI, notices_other_ai, "AI_OnlyPlayer", nullptr, bool, false);
PROXY_CONFIG (AI, notices_projectiles, "AI_SeesPrj", nullptr, bool, true);
PROXY_CONFIG (AI, creature_type, "Creature", nullptr,
	AI::CreatureType, CreatureType::NONE);
PROXY_CONFIG (AI, creature_scale, "CretScale", nullptr, float, 1.0f);
PROXY_CONFIG (AI, is_small_creature, "AI_IsSmall", nullptr, bool, false);
PROXY_CONFIG (AI, motion_tags, "MotActorTagList", nullptr, String, "");
PROXY_CONFIG (AI, motion_tags_standing, "AI_StandTags", nullptr, String, "");
PROXY_CONFIG (AI, voice, "SpchVoice", nullptr, String, "");
PROXY_BIT_CONFIG (AI, is_speaking, "Speech", "flags", 1u, false);
PROXY_CONFIG (AI, last_speech_time, "Speech", "time", Time, 0ul);
PROXY_CONFIG (AI, last_speech_schema, "Speech", "schemaID",
	Object, Object::NONE);
PROXY_CONFIG (AI, last_speech_concept, "Speech", "concept", int, 0);
PROXY_BIT_CONFIG (AI, is_innocent, "DarkStat", nullptr, 1u, false);
PROXY_BIT_CONFIG (AI, is_robot, "DarkStat", nullptr, 16u, false);

OBJECT_TYPE_IMPL_ (AI, Rendered (), Interactive (), Physical (),
		SpherePhysical (), Damageable (), Being (), Container (),
	PROXY_INIT (mode),
	PROXY_INIT (frozen_start),
	PROXY_INIT (frozen_duration),
	PROXY_INIT (behavior_set),
	PROXY_INIT (aggression),
	PROXY_INIT (aptitude),
	PROXY_INIT (defensiveness),
	PROXY_INIT (dodginess),
	PROXY_INIT (hearing),
	PROXY_INIT (sloth),
	PROXY_INIT (verbosity),
	PROXY_INIT (vision),
	PROXY_INIT (time_warp),
	PROXY_INIT (uses_doors),
	PROXY_INIT (needs_big_doors),
	PROXY_INIT (flash_vulnerable),
	PROXY_INIT (head_tracks_player),
	PROXY_INIT (investigates),
	PROXY_INIT (patrols),
	PROXY_INIT (patrols_randomly),
	PROXY_INIT (traverses_water),
	PROXY_INIT (notices_bodies),
	PROXY_INIT (notices_cameras),
	PROXY_INIT (notices_damage),
	PROXY_INIT (notices_other_ai),
	PROXY_INIT (notices_projectiles),
	PROXY_INIT (creature_type),
	PROXY_INIT (creature_scale),
	PROXY_INIT (is_small_creature),
	PROXY_INIT (motion_tags),
	PROXY_INIT (motion_tags_standing),
	PROXY_INIT (voice),
	PROXY_INIT (is_speaking),
	PROXY_INIT (last_speech_time),
	PROXY_INIT (last_speech_schema),
	PROXY_INIT (last_speech_concept),
	PROXY_INIT (is_innocent),
	PROXY_INIT (is_robot)
)

bool
AI::is_ai () const
{
	return behavior_set.exists ();
}

size_t
AI::count_ais ()
{
	return SInterface<IAIManager> (LG)->CountAIs ();
}

AIs
AI::get_all_ais ()
{
	SInterface<IAIManager> AIM (LG);
	AIs everyone;
	tAIIter iter;
	for (IAI* ai = AIM->GetFirst (&iter); ai; ai = AIM->GetNext (&iter))
		everyone.push_back (ai->GetID ());
	AIM->GetDone (&iter);
	return everyone;
}



// AI: state

bool
AI::is_dead () const
{
	return SInterface<IAIManager> (LG)->GetAI (number)->IsDead ();
}

bool
AI::is_dying () const
{
	return SInterface<IAIManager> (LG)->GetAI (number)->IsDying ();
}

void
AI::set_mode_dead ()
{
	SInterface<IAIManager> (LG)->GetAI (number)->Kill ();
}

void
AI::set_mode_normal ()
{
	SInterface<IAIManager> (LG)->GetAI (number)->Resurrect ();
}

void
AI::freeze (Time duration)
{
	SInterface<IAIManager> (LG)->GetAI (number)->Freeze (duration);
}

void
AI::unfreeze ()
{
	SInterface<IAIManager> (LG)->GetAI (number)->Unfreeze ();
}



// AI: actions

THIEF_ENUM_CODING (AI::Speed, BOTH, CODE,
	THIEF_ENUM_VALUE (SLOW, "slow"),
	THIEF_ENUM_VALUE (NORMAL, "normal"),
	THIEF_ENUM_VALUE (FAST, "fast"),
)

THIEF_ENUM_CODING (AI::ActionPriority, BOTH, CODE,
	THIEF_ENUM_VALUE (LOW, "low"),
	THIEF_ENUM_VALUE (NORMAL, "normal"),
	THIEF_ENUM_VALUE (HIGH, "high"),
)

bool
AI::_go_to_location (const Object& nearby, Speed speed, ActionPriority priority,
	const LGMultiBase& result_data)
{
	LGBool result;
	SService<IAIScrSrv> (LG)->MakeGotoObjLoc (result, number, nearby.number,
		eAIScriptSpeed (speed), eAIActionPriority (priority),
		result_data);
	return result;
}

bool
AI::_frob_object (const Object& target, const Object& tool,
	ActionPriority priority, const LGMultiBase& result_data)
{
	LGBool result;
	if (tool != Object::NONE)
		SService<IAIScrSrv> (LG)->MakeFrobObjWith (result, number,
			target.number, tool.number, eAIActionPriority (priority),
			result_data);
	else
		SService<IAIScrSrv> (LG)->MakeFrobObj (result, number,
			target.number, eAIActionPriority (priority),
			result_data);
	return result;
}



// AI: alertness and awareness
//TODO wrap property: AI: AI Core\Alertness cap = AI_AlertCap
//TODO wrap property: AI: AI Core\Alertness sense multipliers = AI_AlSnMul
//TODO wrap property: AI: AI Core\Awareness capacitor = AI_AwareCap
//TODO wrap property: AI: AI Core\Awareness delay (react time) = AI_AwrDel2
//TODO wrap property: AI: AI Core\Suprise [0, 1, Rad] = AISuprise
//TODO wrap property (const): AI: State\Current alertness = AI_Alertness

THIEF_ENUM_CODING (AI::Alert, BOTH, CODE,
	THIEF_ENUM_VALUE (NONE, "none"),
	THIEF_ENUM_VALUE (LOW, "low"),
	THIEF_ENUM_VALUE (MODERATE, "moderate"),
	THIEF_ENUM_VALUE (HIGH, "high"),
)

AI::Alert
AI::get_current_alert () const
{
	return AI::Alert (SService<IAIScrSrv> (LG)->GetAlertLevel (number));
}

void
AI::set_minimum_alert (Alert alert)
{
	SService<IAIScrSrv> (LG)->SetMinimumAlert (number,
		eAIScriptAlertLevel (alert));
}

void
AI::clear_alertness ()
{
	SService<IAIScrSrv> (LG)->ClearAlertness (number);
}



// AI: model
//TODO wrap property: Renderer\Light-based transparency = LBAlpha
//TODO wrap property: Renderer\Mesh Attach = MeshAttach
//TODO wrap property: Renderer\Mesh Textures = MeshTex



// AI: motions

bool
AI::play_motion (const String& motion)
{
	LGBool result;
	SService<IPuppetSrv> (LG)->PlayMotion (result, number, motion.data ());
	return result;
}



// AI: responses
//TODO wrap: enum eAIResponsePriority
//TODO wrap property: AI: Responses\Alert response = AI_AlrtRsp
//TODO wrap property: AI: Responses\Body response = AI_BodyRsp
//TODO wrap property: AI: Responses\Sense combat response = AI_CbtRsp
//TODO wrap property: AI: Responses\Signal response = AI_SigRsp
//TODO wrap property: AI: Responses\Suspicious response = AI_SuspRsp
//TODO wrap property: AI: Responses\Threat response = AI_ThrtRsp



// AI: speech
//TODO wrap property: AI: AI Core\Broadcast customization = AI_BcstSet

void
AI::set_speech_enabled (bool enabled)
{
	SService<IAIScrSrv> (LG)->SetScriptFlags (number, enabled ? 0 : 1);
}

void
AI::halt_speech ()
{
	SService<ISoundScrSrv> (LG)->HaltSpeech (number);
}



// AI: miscellaneous

void
AI::send_signal (const String& signal)
{
	SService<IAIScrSrv> (LG)->Signal (number, signal.data ());
}



// Conversation
//TODO wrap property: AI: Conversations\Conversation = AI_Converation [sic]

PROXY_CONFIG (Conversation, save_conversation, "AI_SaveConverse", nullptr,
	bool, false);

OBJECT_TYPE_IMPL_ (Conversation,
	PROXY_INIT (save_conversation)
)

bool
Conversation::is_conversation () const
{
	return Property (*this, "AI_Converation").exists (); //TODO use a PropField
}

bool
Conversation::start_conversation ()
{
	LGBool result;
	SService<IAIScrSrv> (LG)->StartConversation (result, number);
	return result;
}

// The IAIConversationManager::GetNumActors method always returns zero.

Object
Conversation::get_actor (size_t _number) const
{
	Object::Number _actor = Object::NONE;
	return SInterface<IAIManager> (LG)->GetConversationManager ()->
		GetActorObj (number, _number - 1, &_actor) // zero-based
			? _actor : Object::NONE;
}

void
Conversation::set_actor (size_t _number, const Object& actor)
{
	SInterface<IAIManager> (LG)->GetConversationManager ()->
		SetActorObj (number, _number, actor.number); // one-based
}

void
Conversation::remove_actor (size_t _number)
{
	SInterface<IAIManager> (LG)->GetConversationManager ()->
		RemoveActorObj (number, _number); // one-based
}



// AIAwarenessLink

PROXY_BIT_CONFIG (AIAwarenessLink, seen, "Flags", nullptr, SEEN, false);
PROXY_BIT_CONFIG (AIAwarenessLink, heard, "Flags", nullptr, HEARD, false);
PROXY_BIT_CONFIG (AIAwarenessLink, can_raycast, "Flags", nullptr,
	CAN_RAYCAST, false);
PROXY_BIT_CONFIG (AIAwarenessLink, have_los, "Flags", nullptr, HAVE_LOS, false);
PROXY_BIT_CONFIG (AIAwarenessLink, blind, "Flags", nullptr, BLIND, false);
PROXY_BIT_CONFIG (AIAwarenessLink, deaf, "Flags", nullptr, DEAF, false);
PROXY_BIT_CONFIG (AIAwarenessLink, highest, "Flags", nullptr, HIGHEST, false);
PROXY_BIT_CONFIG (AIAwarenessLink, firsthand, "Flags", nullptr,
	FIRSTHAND, false);
PROXY_CONFIG (AIAwarenessLink, current_level, "Level", nullptr,
	AIAwarenessLink::Level, Level::NONE);
PROXY_CONFIG (AIAwarenessLink, peak_level, "Peak Level", nullptr,
	AIAwarenessLink::Level, Level::NONE);
PROXY_CONFIG (AIAwarenessLink, pulse_level, "Last pulse level", nullptr,
	AIAwarenessLink::Level, Level::NONE);
PROXY_CONFIG (AIAwarenessLink, level_time, "Level enter time", nullptr,
	Time, 0ul);
PROXY_CONFIG (AIAwarenessLink, contact_location, "Pos last contact", nullptr,
	Vector, Vector ());
PROXY_CONFIG (AIAwarenessLink, contact_time, "Time last contact", nullptr,
	Time, 0ul);
PROXY_CONFIG (AIAwarenessLink, firsthand_time, "Last true contact", nullptr,
	Time, 0ul);
PROXY_CONFIG (AIAwarenessLink, vision_cone, "Vision cone", nullptr, int, 0);
PROXY_CONFIG (AIAwarenessLink, update_time, "Time last update", nullptr,
	Time, 0ul);
PROXY_CONFIG (AIAwarenessLink, los_update_time, "Time last update LOS", nullptr,
	Time, 0ul);

FLAVORED_LINK_IMPL_ (AIAwareness,
	PROXY_INIT (seen),
	PROXY_INIT (heard),
	PROXY_INIT (can_raycast),
	PROXY_INIT (have_los),
	PROXY_INIT (blind),
	PROXY_INIT (deaf),
	PROXY_INIT (highest),
	PROXY_INIT (firsthand),
	PROXY_INIT (current_level),
	PROXY_INIT (peak_level),
	PROXY_INIT (pulse_level),
	PROXY_INIT (level_time),
	PROXY_INIT (contact_location),
	PROXY_INIT (contact_time),
	PROXY_INIT (firsthand_time),
	PROXY_INIT (vision_cone),
	PROXY_INIT (update_time),
	PROXY_INIT (los_update_time)
)

AIAwarenessLink
AIAwarenessLink::create (const Object& source, const Object& dest,
	unsigned flags, Level level, Time time, const Vector& location,
	int vision_cone)
{
	sAIAwareness data = {
		dest.number, flags, eAIAwareLevel (level), eAIAwareLevel (level),
		time, time, LGVector (location), eAIAwareLevel (level),
		vision_cone, time, time, 0, time, 0
#ifdef IS_THIEF2
		, 0
#endif
	};
	AIAwarenessLink link = Link::create (flavor (), source, dest, &data);
	return link;
}

inline const sAIAwareness&
get_ai_awareness_data (const AIAwarenessLink& link)
{
	const void* data = link.get_data_raw ();
	if (!data) throw std::runtime_error ("invalid link");
	return *static_cast<const sAIAwareness*> (data);
}

void
AIAwarenessLink::update_level (Level level, Time time)
{
	sAIAwareness data = get_ai_awareness_data (*this);
	data.Level = eAIAwareLevel (level);
	if (data.PeakLevel < eAIAwareLevel (level))
		data.PeakLevel = eAIAwareLevel (level);
	data.LevelEnterTime = time;
	set_data_raw (&data);
}

void
AIAwarenessLink::update_contact (const Vector& location, Time time,
	bool _firsthand)
{
	firsthand = _firsthand;
	sAIAwareness data = get_ai_awareness_data (*this);
	data.PosLastContact = LGVector (location);
	data.TimeLastContact = time;
	if (_firsthand) data.TimeLastFirstHand = time;
	set_data_raw (&data);
}

void
AIAwarenessLink::update (Time time, bool _have_los)
{
	have_los = _have_los;
	sAIAwareness data = get_ai_awareness_data (*this);
	data.TimeLastUpdate = time;
	if (_have_los) data.TimeLastUpdateLOS = time;
	set_data_raw (&data);
}



// CreatureAttachmentLink

PROXY_CONFIG (CreatureAttachmentLink, joint, "Joint", nullptr,
	CreatureAttachmentLink::Joint, Joint::NONE);

FLAVORED_LINK_IMPL_ (CreatureAttachment,
	PROXY_INIT (joint)
)

CreatureAttachmentLink
CreatureAttachmentLink::create (const Object& source, const Object& dest,
	Joint joint)
{
	CreatureAttachmentLink link = Link::create (flavor (), source, dest);
	link.joint = joint;
	return link;
}



//TODO wrap link: AIConversationActor - int ("Actor ID")
//TODO wrap link: AIProjectile - sAIProjectileRel
//TODO wrap link: AIAttack - eAIPriority
//TODO wrap link: AIFleeDest - sAIFleeDest
//TODO wrap link: AINoFlee - sAINoFleeLink
//TODO wrap link: AIWatchObj - sAIWatchPoint (propdefs.h: sAIWatchPoint - link data same as property?)
//TODO wrap link: AIDefendObj - sAIDefendPoint
//TODO wrap link: AICamera - sAICameraLinkData
//TODO wrap link: AIFollowObj - sAIFollowLink,
//TODO wrap link: AISuspiciousLink - sAISuspiciousLink
//TODO wrap link: AIDoor - int (data needed??)



// AIActionResultMessage

MESSAGE_WRAPPER_IMPL (AIActionResultMessage, sAIObjActResultMsg)

AIActionResultMessage::AIActionResultMessage (Action action, Result result,
		const Object& target, const LGMultiBase& data)
	: Message (new sAIObjActResultMsg ())
{
	message->message = "ObjActResult";
	MESSAGE_AS (sAIObjActResultMsg)->action = eAIAction (action);
	MESSAGE_AS (sAIObjActResultMsg)->result = eAIActionResult (result);
	MESSAGE_AS (sAIObjActResultMsg)->result_data = data;
	MESSAGE_AS (sAIObjActResultMsg)->target = target.number;
}

MESSAGE_ACCESSOR (AIActionResultMessage::Action, AIActionResultMessage,
	get_action, sAIObjActResultMsg, action)
MESSAGE_ACCESSOR (AIActionResultMessage::Result, AIActionResultMessage,
	get_result, sAIObjActResultMsg, result)
MESSAGE_ACCESSOR (Object, AIActionResultMessage, get_target,
	sAIObjActResultMsg, target)

void
AIActionResultMessage::_get_result_data (LGMultiBase& value) const
{
	value = MESSAGE_AS (sAIObjActResultMsg)->result_data;
}



// AIAlertnessMessage

MESSAGE_WRAPPER_IMPL_ (AIAlertnessMessage,
	MESSAGE_TYPENAME_TEST ("sAIAlertnessMsg") ||
	MESSAGE_TYPENAME_TEST ("sAIHighAlertMsg"))

AIAlertnessMessage::AIAlertnessMessage (AI::Alert new_level,
	AI::Alert old_level, bool high_alert)
	: Message (high_alert
		? static_cast<sScrMsg*> (new sAIHighAlertMsg ())
		: static_cast<sScrMsg*> (new sAIAlertnessMsg ()))
{
	message->message = high_alert ? "HighAlert" : "Alertness";

	// Since the structures are identical, branching casts aren't needed.
	MESSAGE_AS (sAIAlertnessMsg)->level = eAIScriptAlertLevel (new_level);
	MESSAGE_AS (sAIAlertnessMsg)->oldLevel = eAIScriptAlertLevel (old_level);
}

bool
AIAlertnessMessage::is_high_alert () const
{
	return CIString (get_name ()) == "HighAlert";
}

MESSAGE_ACCESSOR (AI::Alert, AIAlertnessMessage, get_new_level,
	sAIAlertnessMsg, level)
MESSAGE_ACCESSOR (AI::Alert, AIAlertnessMessage, get_old_level,
	sAIAlertnessMsg, oldLevel)



// AIAttackMessage

MESSAGE_WRAPPER_IMPL (AIAttackMessage, sAttackMsg)

AIAttackMessage::AIAttackMessage (Event event, const Object& weapon)
	: Message (new sAttackMsg ())
{
	switch (event)
	{
	case WINDUP: message->message = "StartWindup"; break;
	case START:  message->message = "StartAttack"; break;
	case END:
	default:     message->message = "EndAttack"; break;
	}
	MESSAGE_AS (sAttackMsg)->weapon = weapon.number;
}

AIAttackMessage::Event
AIAttackMessage::get_event () const
{
	CIString name = get_name ();
	if (name == "StartWindup") return WINDUP;
	if (name == "StartAttack") return START;
	if (name == "EndAttack") return END;
	throw MessageWrapError (message, typeid (*this), "invalid event");
}

MESSAGE_ACCESSOR (Object, AIAttackMessage, get_weapon, sAttackMsg, weapon)



//TODO wrap message sMediumTransMsg: MediumTransition (T1ND??)
//TODO wrap property: AI: Utility\Track Medium = AI_TrackM



// AIModeChangeMessage

MESSAGE_WRAPPER_IMPL (AIModeChangeMessage, sAIModeChangeMsg)

AIModeChangeMessage::AIModeChangeMessage (AI::Mode new_mode, AI::Mode old_mode)
	: Message (new sAIModeChangeMsg ())
{
	message->message = "AIModeChange";
	MESSAGE_AS (sAIModeChangeMsg)->mode = eAIMode (new_mode);
	MESSAGE_AS (sAIModeChangeMsg)->previous_mode = eAIMode (old_mode);
}

MESSAGE_ACCESSOR (AI::Mode, AIModeChangeMessage, get_new_mode,
	sAIModeChangeMsg, mode)
MESSAGE_ACCESSOR (AI::Mode, AIModeChangeMessage, get_old_mode,
	sAIModeChangeMsg, previous_mode)



// AIMotionMessage

MESSAGE_WRAPPER_IMPL (AIMotionMessage, sBodyMsg)

AIMotionMessage::AIMotionMessage (Event event, const char* motion_name,
		int motion_flag)
	: Message (new sBodyMsg ())
{
	switch (event)
	{
	case START:        message->message = "MotionStart"; break;
	case FLAG_REACHED: message->message = "MotionFlagReached"; break;
	case END:
	default:           message->message = "MotionEnd"; break;
	}
	MESSAGE_AS (sBodyMsg)->ActionType = sBodyMsg::eBodyAction (event);
	MESSAGE_AS (sBodyMsg)->MotionName = motion_name;
	MESSAGE_AS (sBodyMsg)->FlagValue = motion_flag;
}

MESSAGE_ACCESSOR (AIMotionMessage::Event, AIMotionMessage, get_event,
	sBodyMsg, ActionType)
MESSAGE_ACCESSOR (const char*, AIMotionMessage, get_motion_name,
	sBodyMsg, MotionName)
MESSAGE_ACCESSOR (int, AIMotionMessage, get_motion_flag, sBodyMsg, FlagValue)



// AIPatrolPointMessage

MESSAGE_WRAPPER_IMPL (AIPatrolPointMessage, sAIPatrolPointMsg)

AIPatrolPointMessage::AIPatrolPointMessage (const Object& patrol_point)
	: Message (new sAIPatrolPointMsg ())
{
	message->message = "PatrolPoint";
	MESSAGE_AS (sAIPatrolPointMsg)->patrolObj = patrol_point.number;
}

MESSAGE_ACCESSOR (Object, AIPatrolPointMessage, get_patrol_point,
	sAIPatrolPointMsg, patrolObj)



// AISignalMessage

MESSAGE_WRAPPER_IMPL (AISignalMessage, sAISignalMsg)

AISignalMessage::AISignalMessage (const char* signal)
	: Message (new sAISignalMsg ())
{
	message->message = "SignalAI";
	MESSAGE_AS (sAISignalMsg)->signal = signal;
}

MESSAGE_ACCESSOR (String, AISignalMessage, get_signal, sAISignalMsg, signal)



/*TODO Create MeleeCombatant : AI and wrap these properties:
 * AI: Ability Settings\HtoHCombat: Audio Response = HTHAudioResp
 * AI: Ability Settings\HtoHCombat: Distances = HTHCombatDist
 * AI: Ability Settings\HtoHCombat: Grunt Always = HTHGruntAlways
 * AI: Ability Settings\HtoHCombat: Motion Response = HTHMotionResp
 */



/*TODO Create RangedCombatant : AI and wrap these properties:
 * AI: Ability Settings\RangedCombat: Audio Response = RangedAudioResp
 * AI: Ability Settings\RangedCombat: Grunt Always = RngdGruntAlways
 * AI: Ranged Combat\Ranged Combat = AIRCProp
 * AI: Ranged Combat\Ranged Combat Applicabilities = AI_RngApps
 * AI: Ranged Combat\Ranged Combat Flee = AI_RngFlee
 * AI: Ranged Combat\Ranged Combat Ranges = AIRCRanges
 * AI: Ranged Combat\Ranged Combat Shoot = AI_RngShoot
 * AI: Ranged Combat\Ranged Combat Wound Motion = AIRCWound
 * AI: Ranged Combat\Ranged Combat Wound Sound = AIRCWndSnd
 */



//TODO Create SuicideCombatant : AI and wrap property: AI: Ability Settings\Frog-beast: Explode range = DAI_FrogExpl



//TODO Create Device : AI and wrap property: AI: Ability Settings\Device: parameters = AI_Device

//TODO Create Camera : Device and wrap property: AI: Ability Settings\Camera: parameters = AI_Camera

/*TODO Create Turret : Device and wrap these properties:
 * AI: Ability Settings\Turret: parameters = AI_Turret
 * Gun\AI Gun Description = AIGunDesc
 * Gun\Base Gun Description = BaseGunDesc
 * Gun\Gun State = GunState
 */



/*TODO wrap the following AI properties in appropriate locations:
 *
 * AI: AI Core\Efficiency settings = AI_Efficiency
 *
 * AI: AI Core\Movement: max speed = AI_MoveSpeed
 * AI: AI Core\Movement: turn rate = AI_TurnRate
 * AI: AI Core\Movement: z offset = AI_MoveZOffset
 *
 * AI: AI Core\Vision Joint = AI_VisJoint
 * AI: AI Core\Vision Type = AI_VisType
 * AI: AI Core\Vision description = AI_VisDesc
 *
 * AI: Ability Settings: Inform\Expiration = AI_InfExp
 * AI: Ability Settings: Inform\Inform delay = AI_InfDly
 * AI: Ability Settings: Inform\Inform others = AI_InfOtr
 * AI: Ability Settings: Inform\Max. Passes = AI_InfMxP
 * AI: Ability Settings: Inform\Min. dist to informed = AI_InfDst
 * AI: Ability Settings\Inform: Response options = AI_InfRsp
 * AI: Utility\inform from last seen = AI_InfFrm
 * AI: Utility\Immediate inform = AI_InfNow
 *
 * AI: Ability Settings\Combat: Non-hostile = AI_NonHst
 * AI: Ability Settings\Combat: Timing = AI_CbtTiming
 * AI: Ability Settings\Non-combat: Dmg Response Params = AI_NCDmgRsp
 * AI: Ability Settings\Non-combat: Respond to damage = AI_NCDmRsp
 *
 * AI: Ability Settings\Flee: Condition for flee = AI_FleeConds
 * AI: Ability Settings\Flee: If Aware of AI/Player = AI_FleeAwr
 *
 * AI: Ability Settings\Idle: Returns to origin = AI_IdlRetOrg
 * AI: Ability Settings\Idling: Directions = AI_IdleDirs
 * AI: Ability Settings\Idling: Should fidget = AI_Fidget
 * AI: State\Idling: Origin = AI_IdleOrgn

 * Creature\Current Pose = CretPose
 * Creature\Is Non-Physical = NonPhysCreature
 * Dark GameSys\AltLinkLocation = AltLink
 * Dark GameSys\BeltLinkLocation = BeltLink
 */



} // namespace Thief

