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
#include "OSL.hh"

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
PROXY_NEG_CONFIG (AI, flash_vulnerable, "NoFlash", nullptr, bool, true);
PROXY_NEG_CONFIG (AI, head_tracks_player, "CretHTrack", nullptr, bool, true);
PROXY_NEG_CONFIG (AI, investigates, "AI_InvKnd", nullptr, bool, true);
PROXY_CONFIG (AI, patrols, "AI_Patrol", nullptr, bool, false);
PROXY_CONFIG (AI, patrols_randomly, "AI_PtrlRnd", nullptr, bool, false);
PROXY_CONFIG (AI, traverses_water, "AI_UseWater", nullptr, bool, false);
PROXY_CONFIG (AI, notices_bodies, "AI_NtcBody", nullptr, bool, true);
PROXY_NEG_CONFIG (AI, notices_cameras, "AI_IgCam", nullptr, bool, true);
PROXY_CONFIG (AI, notices_damage, "AI_NoticeDmg", nullptr, bool, true);
PROXY_NEG_CONFIG (AI, notices_other_ai, "AI_OnlyPlayer", nullptr, bool, true);
PROXY_CONFIG (AI, notices_projectiles, "AI_SeesPrj", nullptr, bool, true);
PROXY_CONFIG (AI, current_alert, "AI_Alertness", "Level", AI::Alert, Alert::NONE);
PROXY_CONFIG (AI, peak_alert, "AI_Alertness", "Peak", AI::Alert, Alert::NONE);
PROXY_CONFIG (AI, min_alert, "AI_AlertCap", "Min level", AI::Alert, Alert::NONE);
PROXY_CONFIG (AI, relax_alert, "AI_AlertCap", "Min relax after peak",
	AI::Alert, Alert::LOW);
PROXY_CONFIG (AI, max_alert, "AI_AlertCap", "Max level", AI::Alert, Alert::HIGH);
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
	SoundSchema, Object::NONE);
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
	PROXY_INIT (current_alert),
	PROXY_INIT (peak_alert),
	PROXY_INIT (min_alert),
	PROXY_INIT (relax_alert),
	PROXY_INIT (max_alert),
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
		everyone.emplace_back (ai->GetID ());
	AIM->GetDone (&iter);
	return everyone;
}



// AI: state
//TODO wrap property: AI: AI Core\Efficiency settings = AI_Efficiency

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
AI::face_object (const Object& target)
{
	if (!target.exists ()) return false;

	Vector location = get_location (),
		target_loc = target.get_location (),
		new_rotation;

	double delta_x = location.x - target_loc.x,
		delta_y = location.y - target_loc.y;

	if (std::abs (delta_x) >= Vector::EPSILON)
		new_rotation.z = std::atan (delta_y / delta_x) * 90.0 / M_PI_2
			+ ((delta_x > 0.0) ? 180.0 : 0.0);
	else if (std::abs (delta_y) >= Vector::EPSILON)
		new_rotation.z = (delta_y > 0.0) ? 270.0 : 90.0;
	else
		return false;

	set_rotation (new_rotation);
	return true;
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
//TODO wrap property: AI: AI Core\Alertness sense multipliers = AI_AlSnMul
//TODO wrap property: AI: AI Core\Awareness capacitor = AI_AwareCap
//TODO wrap property: AI: AI Core\Awareness delay (react time) = AI_AwrDel2
//TODO wrap property: AI: AI Core\Suprise [0, 1, Rad] = AISuprise

THIEF_ENUM_CODING (AI::Alert, BOTH, CODE,
	THIEF_ENUM_VALUE (NONE, "none"),
	THIEF_ENUM_VALUE (LOW, "low"),
	THIEF_ENUM_VALUE (MODERATE, "moderate"),
	THIEF_ENUM_VALUE (HIGH, "high"),
)

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
//TODO wrap rest of property: AI: Conversations\Conversation = AI_Converation

PROXY_CONFIG (Conversation, abort_level, "AI_Converation", "Abort level",
	AI::Alert, AI::Alert::NONE);
PROXY_CONFIG (Conversation, abort_priority, "AI_Converation", "Abort priority",
	AI::Priority, AI::Priority::DEFAULT);
PROXY_CONFIG (Conversation, save_conversation, "AI_SaveConverse", nullptr,
	bool, false);

OBJECT_TYPE_IMPL_ (Conversation,
	PROXY_INIT (abort_level),
	PROXY_INIT (abort_priority),
	PROXY_INIT (save_conversation)
)

bool
Conversation::is_conversation () const
{
	return abort_level.exists ();
}

bool
Conversation::start_conversation ()
{
	LGBool result;
	SService<IAIScrSrv> (LG)->StartConversation (result, number);
	return result;
}

// The IAIConversationManager::GetNumActors method always returns zero.

AI
Conversation::get_actor (size_t _number) const
{
	Object::Number _actor = Object::NONE.number;
	return SInterface<IAIManager> (LG)->GetConversationManager ()->
		GetActorObj (number, _number - 1, &_actor) // zero-based
			? Object (_actor) : Object::NONE;
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

bool
Conversation::subscribe (const Object& host)
{
	return SService<IOSLService> (LG)->subscribe_conversation (*this, host);
}

bool
Conversation::unsubscribe (const Object& host)
{
	return SService<IOSLService> (LG)->unsubscribe_conversation (*this, host);
}



//TODO wrap link: AIConversationActor - int ("Actor ID")



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
AIAwarenessLink::update (Time time, bool updated_los)
{
	sAIAwareness data = get_ai_awareness_data (*this);
	data.TimeLastUpdate = time;
	if (updated_los)
		data.TimeLastUpdateLOS = time;
	set_data_raw (&data);
}



// CreatureAttachmentLink

PROXY_CONFIG (CreatureAttachmentLink, joint, "Joint", nullptr,
	AI::Joint, AI::Joint::NONE);

FLAVORED_LINK_IMPL_ (CreatureAttachment,
	PROXY_INIT (joint)
)

CreatureAttachmentLink
CreatureAttachmentLink::create (const Object& source, const Object& dest,
	AI::Joint joint)
{
	CreatureAttachmentLink link = Link::create (flavor (), source, dest);
	link.joint = joint;
	return link;
}



// DetailAttachementLink

PROXY_CONFIG (DetailAttachementLink, type, "Type", nullptr,
	DetailAttachementLink::Type, Type::OBJECT);
PROXY_CONFIG (DetailAttachementLink, vhot_submodel, "vhot/sub #", nullptr,
	int, 0);
PROXY_CONFIG (DetailAttachementLink, joint, "joint", nullptr,
	AI::Joint, AI::Joint::NONE);
PROXY_CONFIG (DetailAttachementLink, rel_location, "rel pos", nullptr,
	Vector, Vector ());
PROXY_CONFIG (DetailAttachementLink, rel_rotation, "rel rot", nullptr,
	Vector, Vector ());
PROXY_CONFIG (DetailAttachementLink, decal_normal, "decal plane norm", nullptr,
	Vector, Vector ());
PROXY_CONFIG (DetailAttachementLink, decal_dist, "decal plane dist", nullptr,
	float, 0.0f);

FLAVORED_LINK_IMPL_ (DetailAttachement,
	PROXY_INIT (type),
	PROXY_INIT (vhot_submodel),
	PROXY_INIT (joint),
	PROXY_INIT (rel_location),
	PROXY_INIT (rel_rotation),
	PROXY_INIT (decal_normal),
	PROXY_INIT (decal_dist)
)

DetailAttachementLink
DetailAttachementLink::create (const Object& source, const Object& dest,
	Type type, int vhot_submodel, AI::Joint joint, Vector rel_location,
	Vector rel_rotation, Vector decal_normal, float decal_dist)
{
	DetailAttachementLink link = Link::create (flavor (), source, dest);
	link.type = type;
	link.vhot_submodel = vhot_submodel;
	link.joint = joint;
	link.rel_location = rel_location;
	link.rel_rotation = rel_rotation;
	link.decal_normal = decal_normal;
	link.decal_dist = decal_dist;
	return link;
}



//TODO wrap link: AIWatchObj - sAIWatchPoint (propdefs.h: sAIWatchPoint - link data same as property?)
//TODO wrap link: AIDefendObj - sAIDefendPoint
//TODO wrap link: AIFollowObj - sAIFollowLink,

//TODO wrap link: AICamera - sAICameraLinkData
//TODO wrap link: AISuspiciousLink - sAISuspiciousLink
//TODO wrap link: AIDoor - int (data needed??)



// AIActionResultMessage

MESSAGE_WRAPPER_IMPL (AIActionResultMessage, sAIObjActResultMsg),
	action (Action (MESSAGE_AS (sAIObjActResultMsg)->action)),
	result (Result (MESSAGE_AS (sAIObjActResultMsg)->result)),
	target (MESSAGE_AS (sAIObjActResultMsg)->target)
{}

AIActionResultMessage::AIActionResultMessage (Action _action, Result _result,
		const Object& _target, const LGMultiBase& data)
	: Message (new sAIObjActResultMsg ()), action (_action),
	  result (_result), target (_target)
{
	message->message = "ObjActResult";
	MESSAGE_AS (sAIObjActResultMsg)->action = eAIAction (action);
	MESSAGE_AS (sAIObjActResultMsg)->result = eAIActionResult (result);
	MESSAGE_AS (sAIObjActResultMsg)->result_data = data;
	MESSAGE_AS (sAIObjActResultMsg)->target = target.number;
}

void
AIActionResultMessage::_get_result_data (LGMultiBase& value) const
{
	value = MESSAGE_AS (sAIObjActResultMsg)->result_data;
}



// AIAlertnessMessage

MESSAGE_WRAPPER_IMPL_ (AIAlertnessMessage,
	MESSAGE_TYPENAME_TEST ("sAIAlertnessMsg") ||
	MESSAGE_TYPENAME_TEST ("sAIHighAlertMsg")),
	high_alert (MESSAGE_NAME_TEST ("HighAlert")),
	new_level (AI::Alert (MESSAGE_AS (sAIAlertnessMsg)->level)),
	old_level (AI::Alert (MESSAGE_AS (sAIAlertnessMsg)->oldLevel))
{}

AIAlertnessMessage::AIAlertnessMessage (AI::Alert _new_level,
	AI::Alert _old_level, bool _high_alert)
	: Message (high_alert
		? static_cast<sScrMsg*> (new sAIHighAlertMsg ())
		: static_cast<sScrMsg*> (new sAIAlertnessMsg ())),
	  high_alert (_high_alert), new_level (_new_level),
	  old_level (_old_level)
{
	message->message = high_alert ? "HighAlert" : "Alertness";

	// Since the structures are identical, the same casts can be used.
	MESSAGE_AS (sAIAlertnessMsg)->level = eAIScriptAlertLevel (new_level);
	MESSAGE_AS (sAIAlertnessMsg)->oldLevel = eAIScriptAlertLevel (old_level);
}



//TODO wrap message sMediumTransMsg: MediumTransition (T1ND??)
//TODO wrap property: AI: Utility\Track Medium = AI_TrackM



// AIModeMessage

MESSAGE_WRAPPER_IMPL (AIModeMessage, sAIModeChangeMsg),
	new_mode (AI::Mode (MESSAGE_AS (sAIModeChangeMsg)->mode)),
	old_mode (AI::Mode (MESSAGE_AS (sAIModeChangeMsg)->previous_mode))
{}

AIModeMessage::AIModeMessage (AI::Mode _new_mode, AI::Mode _old_mode)
	: Message (new sAIModeChangeMsg ()), new_mode (_new_mode),
	  old_mode (_old_mode)
{
	message->message = "AIModeChange";
	MESSAGE_AS (sAIModeChangeMsg)->mode = eAIMode (new_mode);
	MESSAGE_AS (sAIModeChangeMsg)->previous_mode = eAIMode (old_mode);
}



// AIMotionMessage

MESSAGE_WRAPPER_IMPL (AIMotionMessage, sBodyMsg),
	event (Event (MESSAGE_AS (sBodyMsg)->ActionType)),
	motion (MESSAGE_AS (sBodyMsg)->MotionName),
	motion_flag (MESSAGE_AS (sBodyMsg)->FlagValue)
{}

AIMotionMessage::AIMotionMessage (Event _event, const String& _motion,
		int _motion_flag)
	: Message (new sBodyMsg ()), event (_event), motion (_motion),
	  motion_flag (_motion_flag)
{
	switch (event)
	{
	case START:        message->message = "MotionStart"; break;
	case FLAG_REACHED: message->message = "MotionFlagReached"; break;
	case END:
	default:           message->message = "MotionEnd"; break;
	}
	MESSAGE_AS (sBodyMsg)->ActionType = sBodyMsg::eBodyAction (event);
	MESSAGE_AS (sBodyMsg)->MotionName = motion.data ();
	MESSAGE_AS (sBodyMsg)->FlagValue = motion_flag;
}



// AIPatrolPointMessage

MESSAGE_WRAPPER_IMPL (AIPatrolPointMessage, sAIPatrolPointMsg),
	patrol_point (MESSAGE_AS (sAIPatrolPointMsg)->patrolObj)
{}

AIPatrolPointMessage::AIPatrolPointMessage (const Object& _patrol_point)
	: Message (new sAIPatrolPointMsg ()), patrol_point (_patrol_point)
{
	message->message = "PatrolPoint";
	MESSAGE_AS (sAIPatrolPointMsg)->patrolObj = patrol_point.number;
}



// AISignalMessage

MESSAGE_WRAPPER_IMPL (AISignalMessage, sAISignalMsg),
	signal (MESSAGE_AS (sAISignalMsg)->signal)
{}

AISignalMessage::AISignalMessage (const String& _signal)
	: Message (new sAISignalMsg ()), signal (_signal)
{
	message->message = "SignalAI";
	MESSAGE_AS (sAISignalMsg)->signal = signal.data ();
}



// ConversationMessage

MESSAGE_WRAPPER_IMPL (ConversationMessage, ConversationMessageImpl),
	conversation (MESSAGE_AS (ConversationMessageImpl)->conversation)
{}

ConversationMessage::ConversationMessage (const Object& _conversation)
	: Message (new ConversationMessageImpl ()), conversation (_conversation)
{
	message->message = "ConversationEnd";
	MESSAGE_AS (ConversationMessageImpl)->conversation = conversation;
}



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

