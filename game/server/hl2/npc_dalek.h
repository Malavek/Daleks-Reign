//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This is the base version of the dalek
//
//=============================================================================//

#ifndef NPC_DALEK_H
#define NPC_DALEK_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "ai_behavior.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_assault.h"
#include "ai_behavior_standoff.h"
#include "ai_sentence.h"
#include "ai_baseactor.h"
#include "npc_dalek_base.h"
#include "smoke_trail.h"
#include "physics_prop_ragdoll.h"

#define		DALEK_MAX_BEAMS				8

#define DALEK_BEAM_ALL		-1
#define	DALEK_BEAM_ZAP		0
#define	DALEK_BEAM_HEAL	1
#define DALEK_BEAM_DISPEL	2
//#define DALEK_LAMP_ALL		3
//#define DALEK_LAMP		4

class CBeam;
class CSprite;
class CDalekChargeToken;
class CDalekEffectDispel;
//class CDalekEffectLamp;

extern ConVar sk_dalek_zap_range;

/*enum DalekHealState_t
{
	HEAL_STATE_NONE,		// Not trying to heal
	HEAL_STATE_WARMUP,		// In the "warm-up" phase of healing
	HEAL_STATE_HEALING,		// In the process of healing
	HEAL_STATE_COOLDOWN,	// in the "cooldown" phase of healing
};*/

//=========================================================
//	>> CNPC_Dalek
//=========================================================
class CNPC_Dalek : public CNPC_Dalek_Base
{
	DECLARE_CLASS(CNPC_Dalek, CNPC_Dalek_Base);

public:
	CNPC_Dalek(void);

	virtual void	Spawn(void);
	virtual void	Precache(void);
	virtual float	MaxYawSpeed(void);

	//virtual	Vector  FacingPosition(void);
	virtual Vector	BodyTarget(const Vector &posSrc, bool bNoisy = true);

	virtual void	PrescheduleThink(void);
	virtual void	BuildScheduleTestBits(void);
	virtual void	OnScheduleChange(void);

	virtual int		RangeAttack1Conditions(float flDot, float flDist);	// Primary zap
	//virtual int		RangeAttack2Conditions(float flDot, float flDist);	// Concussive zap (larger)
	virtual bool	InnateWeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions);
	virtual int		MeleeAttack1Conditions(float flDot, float flDist); // For kick/punch
	virtual float	InnateRange1MinRange(void) { return 0.0f; }
	virtual float	InnateRange1MaxRange(void) { return sk_dalek_zap_range.GetFloat() * 12; }
	virtual int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	//virtual bool	FInViewCone(CBaseEntity *pEntity);
//	virtual bool	ShouldMoveAndShoot(void);


	// vorts have a very long head/neck swing, so debounce heavily
	virtual	float	GetHeadDebounce(void) { return 0.8; } // how much of previous head turn to use

	//virtual void		Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	//virtual void		AlertSound(void);
	Class_T				Classify(void);
	virtual void		HandleAnimEvent(animevent_t *pEvent);
	virtual Activity	NPC_TranslateActivity(Activity eNewActivity);

	//virtual int				TakeDamageFromCombineBall(const CTakeDamageInfo &info);
	virtual void			RagdollDeathEffect(CRagdollProp *pRagdoll, float flDuration);
	virtual bool			BecomeRagdoll(const CTakeDamageInfo &info, const Vector &forceVector);
	virtual void			StartSmoking();
	virtual void			StopSmoking(float flDelay = 0.1);
	virtual bool			IsSmoking() { return m_hSmoke != NULL; }
	virtual void			Explode();
	virtual bool			IsUsingAggressiveBehavior() { return m_bUseAggressiveBehavior; }

	/*CBaseEntity *	GetCannonTarget();
	bool			HasCannonTarget() const;
	bool			IsCannonTarget(CBaseEntity *pTarget) const;*/

	virtual bool	CreateVPhysics(void);

	virtual void	UpdateOnRemove(void);
	virtual void	Event_Killed(const CTakeDamageInfo &info);
	virtual	void	GatherConditions(void);
	virtual void	RunTask(const Task_t *pTask);
	virtual void	StartTask(const Task_t *pTask);
	virtual void	ClearSchedule(const char *szReason);

	//virtual void	DeclineFollowing(void);
	//virtual bool	CanBeUsedAsAFriend(void);
	//virtual bool	IsPlayerAlly(void) { return true; }

	// Override these to set behavior
	virtual int		TranslateSchedule(int scheduleType);
	virtual int		SelectSchedule(void);
	virtual int		SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);
	virtual bool	IsValidEnemy(CBaseEntity *pEnemy);
	//bool			IsLeading(void) { return (GetRunningBehavior() == &m_LeadBehavior && m_LeadBehavior.HasGoal()); }

	void			DeathSound(const CTakeDamageInfo &info);
	void			PainSound(const CTakeDamageInfo &info);

	virtual void	TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator);
	virtual void	SpeakSentence(int sentType);

	virtual Vector		GetShootEnemyDir(const Vector &shootOrigin, bool bNoisy = true);

	//virtual int		IRelationPriority(CBaseEntity *pTarget);
	//virtual Disposition_t 	IRelationType(CBaseEntity *pTarget);
	virtual bool	IsReadinessCapable(void) { return true; }
	virtual float	GetReadinessDecay() { return 30.0f; }
	//virtual bool	ShouldRegenerateHealth(void) { return m_bRegenerateHealth; }
	virtual bool	CanRunAScriptedNPCInteraction(bool bForced = false);
	virtual void	AimGun(void);
	virtual void	OnUpdateShotRegulator(void);

	//void	InputEnableArmorRecharge(inputdata_t &data);
	//void	InputDisableArmorRecharge(inputdata_t &data);
	//void	InputExtractBugbait(inputdata_t &data);
	//void	InputChargeTarget(inputdata_t &data);
	//void	InputDispel(inputdata_t &data);
	//void	InputBeginCarryNPC(inputdata_t &indputdata);
	//void	InputEndCarryNPC(inputdata_t &indputdata);

	// Health regeneration
	//void	InputEnableHealthRegeneration(inputdata_t &data);
	//void	InputDisableHealthRegeneration(inputdata_t &data);

	// color
	void	InputTurnBlue(inputdata_t &data);
	void	InputTurnBlack(inputdata_t &data);

	//virtual void	SetScriptedScheduleIgnoreConditions(Interruptability_t interrupt);
	virtual void    OnRestore(void);
	virtual bool	OverrideMoveFacing(const AILocalMoveGoal_t &move, float flInterval);
	virtual void	OnStartScene(void);
	virtual bool	IsInterruptable(void);
	virtual bool	CanFlinch(void);

	virtual void    OnChangeActivity(Activity eNewActivity);

	// used so a grub can notify me that I stepped on it. Says a line.
	//void	OnSquishedGrub(const CBaseEntity *pGrub);

	void	UpdateGunFacing(Vector &vecMuzzle, Vector &vecToTarget, Vector &vecAimDir, float *flTargetRange);

	int				SelectScheduleAttack();
	bool			IsUsingTacticalVariant(int variant);
	int				m_iTacticalVariant;
	bool			IsRunningApproachEnemySchedule();
	int				m_iGunDamage;

private:

	//int		NumAntlionsInRadius(float flRadius);
	//void	DispelAntlions(const Vector &vecOrigin, float flRadius, bool bDispel = true);
	//bool	HealGestureHasLOS(void);
	//bool	PlayerBelowHealthPercentage(CBasePlayer *pPlayer, float flPerc);
	//void	StartHealing(void);
	//void	StopHealing(bool bInterrupt = false);
	//void	MaintainHealSchedule(void);
	//bool	ShouldHealTarget(CBaseEntity *pTarget);
	//int		SelectHealSchedule(void);
	// Select the combat schedule
	int SelectCombatSchedule();

	// Should we charge the player?
	bool ShouldChargePlayer();

	// Chase the enemy, updating the target position as the player moves
	void StartTaskChaseEnemyContinuously(const Task_t *pTask);
	void RunTaskChaseEnemyContinuously(const Task_t *pTask);

	/*class CCombineStandoffBehavior : public CAI_ComponentWithOuter<CNPC_Dalek, CAI_StandoffBehavior>
	{
		typedef CAI_ComponentWithOuter<CNPC_Dalek, CAI_StandoffBehavior> BaseClass;

		virtual int SelectScheduleAttack()
		{
			int result = GetOuter()->SelectScheduleAttack();
			if (result == SCHED_NONE)
				result = BaseClass::SelectScheduleAttack();
			return result;
		}
	};*/

	//void	CreateBeamBlast(const Vector &vecOrigin);

	void	HeadlightGlowThink(void);
	void	CreateHeadlightGlow(void);
	void	ClearHeadlightGlow(void);
	void	HeadlightSpeechGlow(void);
	void	ValidToSmoke(void);
	virtual void		NPCThink(void);

protected:
	void			SetKickDamage(int nDamage) { m_nKickDamage = nDamage; }
	CAI_Sentence< CNPC_Dalek > *GetSentences() { return &m_Sentences; }
	virtual Activity GetFlinchActivity(bool bHeavyDamage, bool bGesture);
	CHandle<CSprite>		m_pHeadlightSpriteL;
	CHandle<CSprite>		m_pHeadlightSpriteR;
	//float	m_flNextNPCThink;

private:
	//=========================================================
	// Dalek schedules
	//=========================================================
	enum
	{
		SCHED_DALEK_STAND = BaseClass::NEXT_SCHEDULE,
		SCHED_DALEK_RANGE_ATTACK,
		SCHED_DALEK_HEAL,
		SCHED_DALEK_EXTRACT_BUGBAIT,
		SCHED_DALEK_FACE_PLAYER,
		SCHED_DALEK_RUN_TO_PLAYER,
		SCHED_DALEK_DISPEL_ANTLIONS,
		//SCHED_DALEK_MELEE,
		SCHED_DALEK_FLEE_FROM_BEST_SOUND,
		SCHED_DALEK_ALERT_FACE_BESTSOUND,
		SCHED_DALEK_PATROL,
		SCHED_DALEK_ASSAULT,
		SCHED_DALEK_ESTABLISH_LINE_OF_FIRE,
		SCHED_DALEK_PRESS_ATTACK,
		//SCHED_DALEK_TURN180,

	};

	//=========================================================
	// Dalek Tasks 
	//=========================================================
	enum
	{
		TASK_DALEK_HEAL_WARMUP = BaseClass::NEXT_TASK,
		TASK_DALEK_HEAL,
		TASK_DALEK_EXTRACT_WARMUP,
		TASK_DALEK_EXTRACT,
		TASK_DALEK_EXTRACT_COOLDOWN,
		TASK_DALEK_FIRE_EXTRACT_OUTPUT,
		TASK_DALEK_WAIT_FOR_PLAYER,
		TASK_DALEK_GET_HEAL_TARGET,
		TASK_DALEK_MELEE,
		TASK_DALEKFACE_TARGET,
		TASK_DALEKFACE_PLAYER,
		TASK_DALEKFACE_ENEMY,
		TASK_DALEK_DISPEL_ANTLIONS,
		TASK_DALEK_MOVESOUND,
		//TASK_DALEK_TURN180,
	};
	
	//=========================================================
	// Dalek Conditions
	//=========================================================
	enum
	{
		COND_DALEK_CAN_HEAL = BaseClass::NEXT_CONDITION,
		COND_DALEK_HEAL_TARGET_TOO_FAR,	// Outside or heal range
		COND_DALEK_HEAL_TARGET_BLOCKED,	// Blocked by an obstruction
		COND_DALEK_HEAL_TARGET_BEHIND_US,	// Not within our "forward" range
		COND_DALEK_HEAL_VALID,				// All conditions satisfied	
		COND_DALEK_DISPEL_ANTLIONS,		// Repulse all antlions around us
		COND_DALEK_MELEE,		
		COND_DALEK_NOT_SPEAKING,
	};

	int				m_nKickDamage;
	CAI_Sentence< CNPC_Dalek > m_Sentences;

	// ------------
	// Beams
	// ------------
	inline bool		InAttackSequence(void);
	void			ClearBeams(void);
	//void			ArmBeam(int beamType, int nHand);
	void			ZapBeam(int nHand);
	int				m_nLightningSprite;
	int				m_nLampSprite;

	// ---------------
	//  Glow
	// ----------------
	//void			ClearHandGlow(void);
	//void			ClearLampGlow(void);
	float			m_fGlowAge;
	float			m_fGlowChangeTime;
	bool			m_bGlowTurningOn;
	int				m_nCurGlowIndex;

	///CHandle<CDalekEffectDispel>	m_hHandEffect[2];
	//CHandle<CDalekEffectLamp>	m_hLampEffect[3];
	CHandle<SmokeTrail> m_hSmoke;
	CAI_FreePass m_PlayerFreePass;
	//CBoneFollowerManager m_BoneFollowerManager;

	/*Vector			m_blastHit;
	Vector			m_blastNormal;
	CNetworkVector(m_vecHitPos);
	EHANDLE			m_hCannonTarget;*/


	bool			ShouldExplodeFromDamage(const CTakeDamageInfo &info);
	bool			m_bExploding;
	bool			m_bUseAggressiveBehavior;

	bool			m_bDamageEnoughToExplode;

	//void			StartHandGlow(int beamType, int nHand);
	//void			EndHandGlow(int beamType = DALEK_BEAM_ALL);
	//void			StartLampGlow(int beamType, int nHand);
	//void			EndLampGlow(int beamType = DALEK_LAMP_ALL);
	//void			MaintainGlows(void);

	// ----------------
	//  Healing
	// ----------------
	//bool				m_bRegenerateHealth;
	//float				m_flNextHealTime;		// Next time allowed to heal player
	//EHANDLE				m_hHealTarget;			// The person that I'm going to heal.
	//bool				m_bPlayerRequestedHeal;	// This adds some priority to our heal (allows it to happen in combat, etc)
	//float				m_flNextHealTokenTime;

	//DalekHealState_t	m_eHealState;

	//CBaseEntity		*FindHealTarget(void);
	//bool			HealBehaviorAvailable(void);
	//void			SetHealTarget(CBaseEntity *pTarget, bool bPlayerRequested);
	//void			GatherHealConditions(void);

	//int				m_nNumTokensToSpawn;
	//float			m_flHealHinderedTime;
	float			m_flPainTime;
	float			m_nextLineFireTime;

	//bool			m_bArmorRechargeEnabled;
	//bool			m_bForceArmorRecharge;
	//float			m_flDispelTestTime;

	//bool			m_bExtractingBugbait;

	//bool			IsCarryingNPC(void) const { return m_bCarryingNPC; }
	//bool			m_bCarryingNPC;

	//COutputEvent	m_OnFinishedExtractingBugbait;
	//COutputEvent	m_OnFinishedChargingTarget;
	//COutputEvent	m_OnPlayerUse;

	//Adrian: Let's do it the right way!
	int				m_iLeftHandAttachment;
	int				m_iRightHandAttachment;
	int				m_iGunAttachment;
	int				m_iGunRefAttachment;
	int				m_iLeftLampAttachment;
	int				m_iRightLampAttachment;
	bool			m_bStopLoopingSounds;
	float			m_flAimDelay;			// Amount of time to suppress aiming

	// used for fading from green vort to blue vort
	CNetworkVar(bool, m_bIsBlue);
	CNetworkVar(float, m_flBlueEndFadeTime);

	// used for fading to black
	CNetworkVar(bool, m_bIsBlack);

public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
};

//=============================================================================
// 
//  Charge Token 
//	
//=============================================================================

class CDalekChargeToken : public CBaseEntity
{
	DECLARE_CLASS(CDalekChargeToken, CBaseEntity);

public:

	static CDalekChargeToken *CreateChargeToken(const Vector &vecOrigin, CBaseEntity *pOwner, CBaseEntity *pTarget);

	CDalekChargeToken(void);

	virtual void	Spawn(void);
	virtual void	Precache(void);
	//virtual unsigned int PhysicsSolidMaskForEntity(void) const;

	void	FadeAndDie(void);
	//void	SeekThink(void);
	//void	SeekTouch(CBaseEntity	*pOther);
	void	SetTargetEntity(CBaseEntity *pTarget) { m_hTarget = pTarget; }

private:

	Vector	GetSteerVector(const Vector &vecForward);

	float				m_flLifetime;
	EHANDLE				m_hTarget;

	CNetworkVar(bool, m_bFadeOut);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};

//=============================================================================
// 
//  Dispel Effect
//	
//=============================================================================

class CDalekEffectDispel : public CBaseEntity
{
	DECLARE_CLASS(CDalekEffectDispel, CBaseEntity);

public:

	static CDalekEffectDispel *CreateEffectDispel(const Vector &vecOrigin, CBaseEntity *pOwner, CBaseEntity *pTarget);

	CDalekEffectDispel(void);

	virtual void	Spawn(void);

	void	FadeAndDie(void);

private:

	CNetworkVar(bool, m_bFadeOut);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};

//=============================================================================
// 
//  Dispel Lamp
//	
//=============================================================================

/*class CDalekEffectLamp : public CBaseEntity
{
	DECLARE_CLASS(CDalekEffectLamp, CBaseEntity);

public:

	static CDalekEffectLamp *CreateEffectLamp(const Vector &vecOrigin, CBaseEntity *pOwner, CBaseEntity *pTarget);

	CDalekEffectLamp(void);

	virtual void	Spawn(void);

	void	FadeAndDie(void);

private:

	CNetworkVar(bool, m_bFadeOut);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};*/

#endif // NPC_DALEK_H
