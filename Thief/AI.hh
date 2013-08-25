/******************************************************************************
 *  AI.hh
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

#ifndef THIEF_AI_HH
#define THIEF_AI_HH

#include <Thief/Base.hh>
#include <Thief/Link.hh>
#include <Thief/Message.hh>
#include <Thief/Physics.hh>
#include <Thief/Types.hh>

namespace Thief {

class AI;
typedef std::vector<AI> AIs;



class AI : public virtual Interactive, public Being, public Container
{
public:
	THIEF_OBJECT_TYPE (AI)
	bool is_ai () const;

	static size_t count_ais ();
	static AIs get_all_ais ();

	// State

	enum class Mode
	{
		ASLEEP,
		SUPER_EFFICIENT,
		EFFICIENT,
		NORMAL,
		COMBAT,
		DEAD
	};

	THIEF_PROP_FIELD_CONST (Mode, mode); //TESTME
	bool is_dead () const; //TESTME
	bool is_dying () const; //TESTME

	void set_mode_dead ();
	void set_mode_normal ();

	THIEF_PROP_FIELD_CONST (Time, frozen_start);
	THIEF_PROP_FIELD_CONST (Time, frozen_duration);
	void freeze (Time duration);
	void unfreeze ();

	// Actions

	enum class Speed { SLOW, NORMAL, FAST };
	enum class ActionPriority { LOW, NORMAL, HIGH };

	bool go_to_location (const Object& nearby, Speed = Speed::NORMAL,
		ActionPriority = ActionPriority::NORMAL); //TESTME

	template <typename T>
	bool go_to_location (const Object& nearby, Speed, ActionPriority,
		const T& result_data); //TESTME

	bool frob_object (const Object& target,
		const Object& tool = Object::NONE,
		ActionPriority = ActionPriority::NORMAL); //TESTME

	template <typename T>
	bool frob_object (const Object& target, const Object& tool,
		ActionPriority, const T& result_data); //TESTME

	// Abilities and attributes

	THIEF_PROP_FIELD (String, behavior_set); //TESTME

	enum class Rating
		{ NONE, WELL_BELOW, BELOW, AVERAGE, ABOVE, WELL_ABOVE };
	THIEF_PROP_FIELD (Rating, aggression); //TESTME
	THIEF_PROP_FIELD (Rating, aptitude); //TESTME
	THIEF_PROP_FIELD (Rating, defensiveness); //TESTME
	THIEF_PROP_FIELD (Rating, dodginess); //TESTME
	THIEF_PROP_FIELD (Rating, hearing); //TESTME
	THIEF_PROP_FIELD (Rating, sloth); //TESTME
	THIEF_PROP_FIELD (Rating, verbosity); //TESTME
	THIEF_PROP_FIELD (Rating, vision); //TESTME

	THIEF_PROP_FIELD (float, time_warp); //TESTME

	THIEF_PROP_FIELD (bool, uses_doors); //TESTME
	THIEF_PROP_FIELD (bool, needs_big_doors); //TESTME

	THIEF_PROP_FIELD (bool, flash_vulnerable); //TESTME

	THIEF_PROP_FIELD (bool, head_tracks_player); //TESTME

	THIEF_PROP_FIELD (bool, investigates); //TESTME

	THIEF_PROP_FIELD (bool, patrols);
	THIEF_PROP_FIELD (bool, patrols_randomly); //TESTME

	THIEF_PROP_FIELD (bool, traverses_water); //TESTME

	// Alertness and awareness

	THIEF_PROP_FIELD (bool, notices_bodies); //TESTME
	THIEF_PROP_FIELD (bool, notices_cameras); //TESTME
	THIEF_PROP_FIELD (bool, notices_damage); //TESTME
	THIEF_PROP_FIELD (bool, notices_other_ai); //TESTME
	THIEF_PROP_FIELD (bool, notices_projectiles); //TESTME

	enum class Alert { NONE, LOW, MODERATE, HIGH };

	Alert get_current_alert () const; //TESTME
	void set_minimum_alert (Alert); //TESTME
	void clear_alertness (); //TESTME

	// Model

	enum class CreatureType
	{
		NONE = -1, HUMANOID, PLAYER_ARM, PLAYER_BOW_ARM, BURRICK,
		SPIDER, BUGBEAST, CRAYMAN, CONSTANTINE, APPARITION, SWEEL, ROPE,
		ZOMBIE, SMALL_SPIDER, FROG, CUTTY,
#ifdef IS_THIEF2
		AVATAR, ROBOT, SMALL_ROBOT, SPIDER_BOT
#endif
	};
	THIEF_PROP_FIELD (CreatureType, creature_type); //TESTME
	THIEF_PROP_FIELD (float, creature_scale); //TESTME
	THIEF_PROP_FIELD (bool, is_small_creature); //TESTME

	// Motions

	THIEF_PROP_FIELD (String, motion_tags); //TESTME
	THIEF_PROP_FIELD (String, motion_tags_standing); //TESTME

	bool play_motion (const String& motion); //TESTME

	// Speech

	THIEF_PROP_FIELD_CONST (String, voice);

	THIEF_PROP_FIELD_CONST (bool, is_speaking);
	THIEF_PROP_FIELD_CONST (Time, last_speech_time);
	THIEF_PROP_FIELD_CONST (Object, last_speech_schema);
	THIEF_PROP_FIELD_CONST (int, last_speech_concept);

	void set_speech_enabled (bool enabled); //TESTME
	void halt_speech (); //TESTME

	// Miscellaneous

	THIEF_PROP_FIELD (bool, is_innocent);
	THIEF_PROP_FIELD (bool, is_robot); //TESTME

	void send_signal (const String& signal); //TESTME

private:
	bool _go_to_location (const Object& nearby, Speed, ActionPriority,
		const LGMultiBase& result_data);

	bool _frob_object (const Object& target, const Object& tool,
		ActionPriority, const LGMultiBase& result_data);
};



class Conversation : public virtual Object
{
public:
	THIEF_OBJECT_TYPE (Conversation)
	bool is_conversation () const;

	bool start_conversation ();

	Object get_actor (size_t number) const;
	void set_actor (size_t number, const Object& actor);
	void remove_actor (size_t number);

	THIEF_PROP_FIELD (bool, save_conversation);
};



THIEF_FLAVORED_LINK (AIAwareness)
{
	THIEF_FLAVORED_LINK_COMMON (AIAwareness)

	enum Flags
	{
		SEEN = 1, HEARD = 2,
		CAN_RAYCAST = 4, HAVE_LOS = 8,
		BLIND = 16, DEAF = 32,
		HIGHEST = 64, FIRSTHAND = 128
	};

	enum class Level { NONE, LOW, MODERATE, HIGH };

	static AIAwarenessLink create (const Object& source, const Object& dest,
		unsigned flags, Level, Time, const Vector&, int vision_cone);

	unsigned get_flags () const;
	void set_flags (unsigned);
	void clear_flags (unsigned);

	Level get_current_level () const;
	Level get_peak_level () const;
	Level get_pulse_level () const;
	Time get_level_time () const;
	void set_level (Level, Time);

	Vector get_contact_location () const;
	Time get_contact_time () const;
	Time get_firsthand_time () const;
	void update_contact (const Vector&, Time, bool firsthand);

	int get_vision_cone () const;
	void set_vision_cone (int);

	Time get_update_time () const;
	Time get_los_update_time () const;
	void update (Time, bool have_los);
};



THIEF_FLAVORED_LINK (CreatureAttachment)
{
	THIEF_FLAVORED_LINK_COMMON (CreatureAttachment)

	enum class Joint
	{
		NONE, HEAD, NECK, ABDOMEN, BUTT,
		LEFT_SHOULDER, RIGHT_SHOULDER, LEFT_ELBOW, RIGHT_ELBOW,
		LEFT_WRIST, RIGHT_WRIST, LEFT_FINGERS, RIGHT_FINGERS,
		LEFT_HIP, RIGHT_HIP, LEFT_KNEE, RIGHT_KNEE,
		LEFT_ANKLE, RIGHT_ANKLE, LEFT_TOE, RIGHT_TOE, TAIL
	};

	static CreatureAttachmentLink create (const Object& source,
		const Object& dest, Joint = Joint::NONE);

	Joint get_joint () const;
	void set_joint (Joint);
};



class AIActionResultMessage : public Message //TESTME
{
public:
	enum Action { NONE, GOTO, FROB, MANEUVER };
	enum Result { DONE, FAILED, NOT_ATTEMPTED };

	AIActionResultMessage (Action action, Result result,
		const Object& target, const LGMultiBase& data);
	THIEF_MESSAGE_WRAP (AIActionResultMessage);

	Action get_action () const;
	Result get_result () const;
	Object get_target () const;
	MULTI_GET (get_result_data);

private:
	void _get_result_data (LGMultiBase& value) const;
};



class AIAlertnessMessage : public Message //TESTME HighAlert only
{
public:
	AIAlertnessMessage (AI::Alert new_level, AI::Alert old_level,
		bool high_alert = false);
	THIEF_MESSAGE_WRAP (AIAlertnessMessage);

	bool is_high_alert () const;
	AI::Alert get_new_level () const;
	AI::Alert get_old_level () const;
};



class AIAttackMessage : public Message //TESTME
{
public:
	enum Event { WINDUP, START, END };

	AIAttackMessage (Event, const Object& weapon);
	THIEF_MESSAGE_WRAP (AIAttackMessage);

	Event get_event () const;
	Object get_weapon () const;
};



class AIModeChangeMessage : public Message
{
public:
	AIModeChangeMessage (AI::Mode new_mode, AI::Mode old_mode);
	THIEF_MESSAGE_WRAP (AIModeChangeMessage);

	AI::Mode get_new_mode () const;
	AI::Mode get_old_mode () const;
};



class AIMotionMessage : public Message //TESTME
{
public:
	enum Event { START, END, FLAG_REACHED };

	AIMotionMessage (Event event, const char* motion_name,
		int motion_flag = 0);
	THIEF_MESSAGE_WRAP (AIMotionMessage);

	Event get_event () const;
	const char* get_motion_name () const;
	int get_motion_flag () const;
};



class AIPatrolPointMessage : public Message //TESTME
{
public:
	AIPatrolPointMessage (const Object& patrol_point);
	THIEF_MESSAGE_WRAP (AIPatrolPointMessage);

	Object get_patrol_point () const;
};



class AISignalMessage : public Message //TESTME
{
public:
	AISignalMessage (const char* signal);
	THIEF_MESSAGE_WRAP (AISignalMessage);

	String get_signal () const;
};



} // namespace Thief

#include <Thief/AI.inl>

#endif // THIEF_AI_HH

