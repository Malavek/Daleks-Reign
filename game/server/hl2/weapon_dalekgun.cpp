//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "basehlcombatweapon.h"
#include "decals.h"
#include "beam_shared.h"
#include "AmmoDef.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "soundenvelope.h"
#include "soundent.h"
#include "shake.h"
#include "explode.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "ai_memory.h"
#include "basecombatcharacter.h"

#include "weapon_dalekgun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CTEDalekGunExplosion::CTEDalekGunExplosion(const char *name) : BaseClass(name)
{
	m_nType = 0;
	m_vecDirection.Init();
}

CTEDalekGunExplosion::~CTEDalekGunExplosion(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : msg_dest - 
//			delay - 
//			*origin - 
//			*recipient - 
//-----------------------------------------------------------------------------
void CTEDalekGunExplosion::Create(IRecipientFilter& filter, float delay)
{
	engine->PlaybackTempEntity( filter, delay, (void *)this, GetServerClass()->m_pTable, GetServerClass()->m_ClassID );
}

IMPLEMENT_SERVERCLASS_ST(CTEDalekGunExplosion, DT_TEDalekGunExplosion)
	SendPropInt( SENDINFO(m_nType), 2, SPROP_UNSIGNED ),
	SendPropVector( SENDINFO(m_vecDirection), -1, SPROP_COORD ),
END_SEND_TABLE()

static CTEDalekGunExplosion g_TEDalekGunExplosion("DalekGunExplosion");

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &pos - 
//			&angles - 
//-----------------------------------------------------------------------------
void TE_DalekGunExplosion(IRecipientFilter& filter, float delay,
	const Vector &pos, const Vector &dir, int type )
{
	g_TEDalekGunExplosion.m_vecOrigin = pos;
	g_TEDalekGunExplosion.m_vecDirection = dir;
	g_TEDalekGunExplosion.m_nType = type;

	//Send it
	g_TEDalekGunExplosion.Create(filter, delay);
}

//-----------------------------------------------------------------------------
// Gauss gun
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponDalekGun, DT_WeaponDalekGun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_dalekgun, CWeaponDalekGun);
PRECACHE_WEAPON_REGISTER(weapon_dalekgun);

acttable_t	CWeaponDalekGun::m_acttable[] =
{
	{ ACT_IDLE, ACT_IDLE_PISTOL, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_PISTOL, true },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD, ACT_RELOAD_PISTOL, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_PISTOL, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_PISTOL, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_PISTOL, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_PISTOL_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_PISTOL_LOW, false },
	{ ACT_COVER_LOW, ACT_COVER_PISTOL_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_PISTOL_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_PISTOL, false },
	{ ACT_WALK, ACT_WALK_PISTOL, false },
	{ ACT_RUN, ACT_RUN_PISTOL, false },
};


IMPLEMENT_ACTTABLE(CWeaponDalekGun);

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponDalekGun )

	DEFINE_FIELD( m_hViewModel, FIELD_EHANDLE),
	DEFINE_FIELD( m_flNextChargeTime, FIELD_TIME),
	DEFINE_SOUNDPATCH( m_sndCharge),
	DEFINE_FIELD( m_flChargeStartTime, FIELD_TIME),
	DEFINE_FIELD( m_bCharging, FIELD_BOOLEAN),
	DEFINE_FIELD( m_bChargeIndicated, FIELD_BOOLEAN),
	DEFINE_FIELD( m_flCoilMaxVelocity, FIELD_FLOAT),
	DEFINE_FIELD( m_flCoilVelocity, FIELD_FLOAT),
	DEFINE_FIELD( m_flCoilAngle, FIELD_FLOAT),

END_DATADESC()


ConVar sk_plr_dmg_dalekgun("sk_plr_dmg_dalekgun", "0");
ConVar sk_plr_max_dmg_dalekgun("sk_plr_max_dmg_dalekgun", "0");
ConVar sk_npc_dmg_dalekgun("sk_npc_dmg_dalekgun", "0");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponDalekGun::CWeaponDalekGun(void)
{
	m_hViewModel = NULL;
	m_flNextChargeTime	= 0;
	m_flChargeStartTime = 0;
	m_sndCharge			= NULL;
	m_bCharging			= false;
	m_bChargeIndicated	= false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::Precache(void)
{
	enginesound->PrecacheSound( "weapons/gauss/chargeloop.wav" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::DryFire(void)
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::Spawn(void)
{
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::Fire(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner == NULL)
			return;

		m_bCharging = false;

		if (m_hViewModel == NULL)
		{
			CBaseViewModel *vm = pOwner->GetViewModel();

			if (vm)
			{
				m_hViewModel.Set(vm);
			}
		}

		Vector	startPos = pOwner->Weapon_ShootPosition();
		Vector	aimDir = pOwner->GetAutoaimVector(AUTOAIM_5DEGREES);

		Vector vecUp, vecRight;
		VectorVectors(aimDir, vecRight, vecUp);

		QAngle	viewPunch;

		viewPunch.x = random->RandomFloat(-4.0f, -8.0f);
		viewPunch.y = random->RandomFloat(-0.25f, 0.25f);
		viewPunch.z = 0;

		pOwner->ViewPunch(viewPunch);

		float x, y, z;

		Vector	recoilForce = pOwner->BodyDirection2D() * -(250.0f);
		recoilForce[2] += 128.0f;

		pOwner->ApplyAbsVelocityImpulse(recoilForce);

		m_flNextPrimaryAttack = gpGlobals->curtime + 1.5f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 1.5f;

		//Gassian spread
		do {
			x = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
			y = random->RandomFloat(-0.5, 0.5) + random->RandomFloat(-0.5, 0.5);
			z = x*x + y*y;
		} while (z > 1);

		aimDir = aimDir + x * GetBulletSpread().x * vecRight + y * GetBulletSpread().y * vecUp;

		Vector	endPos = startPos + (aimDir * MAX_TRACE_LENGTH);

		//Shoot a shot straight out
		trace_t	tr;
		UTIL_TraceLine(startPos, endPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

		ClearMultiDamage();

		CBaseEntity *pHit = tr.m_pEnt;

		CTakeDamageInfo dmgInfo(this, pOwner, sk_plr_dmg_dalekgun.GetFloat(), DMG_SHOCK);

		if (pHit != NULL)
		{
			CalculateBulletDamageForce(&dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos);
			pHit->DispatchTraceAttack(dmgInfo, aimDir, &tr);
		}

		if (tr.DidHitWorld())
		{
			DrawBeam(tr.startpos, tr.endpos, 1.6, true);
			UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "AR2Impact");
			//UTIL_DecalTrace(&tr, "RedGlowFade");
		}
		else
		{
			DrawBeam(tr.startpos, tr.endpos, 1.6, true);
		}

		ApplyMultiDamage();

		UTIL_ImpactTrace(&tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "AR2Impact");

		CPVSFilter filter(tr.endpos);
		te->DalekGunExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);

		//m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

		AddViewKick();

		// Register a muzzleflash for the AI
		pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponDalekGun::ChargedFire(void)
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	bool penetrated = false;

	//Play shock sounds
	WeaponSound( SINGLE );
	WeaponSound( SPECIAL2 );

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	StopChargeSound();

	m_bCharging = false;
	m_bChargeIndicated = false;

	m_flNextPrimaryAttack	= gpGlobals->curtime + 3.0f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 3.0f;

	//Shoot a shot straight out
	Vector	startPos= pOwner->Weapon_ShootPosition();
	Vector	aimDir	= pOwner->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector	endPos	= startPos + ( aimDir * MAX_TRACE_LENGTH );
	
	trace_t	tr;
	UTIL_TraceLine( startPos, endPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );
	
	ClearMultiDamage();

	//Find how much damage to do
	float flChargeAmount = ( gpGlobals->curtime - m_flChargeStartTime ) / MAX_DALEKGUN_CHARGE_TIME;

	//Clamp this
	if ( flChargeAmount > 1.0f )
	{
		flChargeAmount = 1.0f;
	}

	//Determine the damage amount
	float flDamage = sk_plr_dmg_dalekgun.GetFloat() + ((sk_plr_max_dmg_dalekgun.GetFloat() - sk_plr_dmg_dalekgun.GetFloat()) * flChargeAmount);

	CBaseEntity *pHit = tr.m_pEnt;
	if ( tr.DidHitWorld() )
	{
		//Try wall penetration
		UTIL_ImpactTrace( &tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactDalekGun" );
		UTIL_DecalTrace( &tr, "RedGlowFade" );

		CPVSFilter filter( tr.endpos );
		te->DalekGunExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);
		
		Vector	testPos = tr.endpos + ( aimDir * 48.0f );

		UTIL_TraceLine( testPos, tr.endpos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );
			
		if ( tr.allsolid == false )
		{
			UTIL_DecalTrace( &tr, "RedGlowFade" );

			penetrated = true;
		}
	}
	else if ( pHit != NULL )
	{
		CTakeDamageInfo dmgInfo( this, pOwner, flDamage, DMG_SHOCK );
		CalculateBulletDamageForce( &dmgInfo, m_iPrimaryAmmoType, aimDir, tr.endpos );

		//Do direct damage to anything in our path
		pHit->DispatchTraceAttack( dmgInfo, aimDir, &tr );
	}

	ApplyMultiDamage();

	UTIL_ImpactTrace( &tr, GetAmmoDef()->DamageType(m_iPrimaryAmmoType), "ImpactDalekGun" );

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat( -4.0f, -8.0f );
	viewPunch.y = random->RandomFloat( -0.25f,  0.25f );
	viewPunch.z = 0;

	pOwner->ViewPunch( viewPunch );

	DrawBeam( startPos, tr.endpos, 9.6, true );

	Vector	recoilForce = pOwner->BodyDirection2D() * -( flDamage * 10.0f );
	recoilForce[2] += 128.0f;

	pOwner->ApplyAbsVelocityImpulse( recoilForce );

	CPVSFilter filter( tr.endpos );
	te->DalekGunExplosion(filter, 0.0f, tr.endpos, tr.plane.normal, 0);

	/*if ( penetrated == true )
	{
		RadiusDamage( CTakeDamageInfo( this, this, flDamage, DMG_SHOCK ), tr.endpos, 200.0f, CLASS_NONE );
	}

	// Register a muzzleflash for the AI
	pOwner->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::DrawBeam(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle)
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	//Check to store off our view model index
	if ( m_hViewModel == NULL )
	{
		CBaseViewModel *vm = pOwner->GetViewModel();

		if ( vm )
		{
			m_hViewModel.Set( vm );
		}
	}

	//Draw the main beam shaft
	CBeam *pBeam = CBeam::BeamCreate( DALEKGUN_BEAM_SPRITE, width );
	
	if ( useMuzzle )
	{
		pBeam->PointEntInit( endPos, m_hViewModel );
		pBeam->SetEndAttachment( 1 );
		pBeam->SetWidth( width / 20.0f );
		pBeam->SetEndWidth( width );
	}
	else
	{
		pBeam->SetStartPos( startPos );
		pBeam->SetEndPos( endPos );
		pBeam->SetWidth( width );
		pBeam->SetEndWidth( width / 26.0f );
	}

	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 100, 150, 255 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );

	//Draw electric bolts along shaft
	/*for ( int i = 0; i < 3; i++ )
	{
		pBeam = CBeam::BeamCreate( DALEKGUN_BEAM_SPRITE, (width/2.0f) + i );
		
		if ( useMuzzle )
		{
			pBeam->PointEntInit( endPos, m_hViewModel );
			pBeam->SetEndAttachment( 1 );
		}
		else
		{
			pBeam->SetStartPos( startPos );
			pBeam->SetEndPos( endPos );
		}
		
		pBeam->SetBrightness( random->RandomInt( 64, 255 ) );
		pBeam->SetColor( 255, 255, 150+random->RandomInt( 0, 64 ) );
		pBeam->RelinkBeam();
		pBeam->LiveForTime( 0.1f );
		pBeam->SetNoise( 1.6f * i );
		pBeam->SetEndWidth( 0.1f );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::PrimaryAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) < 25)
	{
		DryFire();
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}
	else if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) >= 25)
	{

		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner == NULL)
			return;

		WeaponSound(SINGLE);
		WeaponSound(SPECIAL2);

		SendWeaponAnim(ACT_VM_PRIMARYATTACK);

		//pOwner->m_fEffects |= EF_MUZZLEFLASH;

		m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

		pOwner->RemoveAmmo(25, m_iPrimaryAmmoType);

		Fire();

		m_flCoilMaxVelocity = 0.0f;
		m_flCoilVelocity = 1000.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponDalekGun::IncreaseCharge(void)
{
	if ( m_flNextChargeTime > gpGlobals->curtime )
		return;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	//Check our charge time
	if ( ( gpGlobals->curtime - m_flChargeStartTime ) > MAX_DALEKGUN_CHARGE_TIME )
	{
		//Notify the player they're at maximum charge
		if ( m_bChargeIndicated == false )
		{
			WeaponSound( SPECIAL2 );
			m_bChargeIndicated = true;
		}

		if ( ( gpGlobals->curtime - m_flChargeStartTime ) > DANGER_DALEKGUN_CHARGE_TIME )
		{
			//Damage the player
			WeaponSound( SPECIAL2 );
			
			// Add DMG_CRUSH because we don't want any physics force
			pOwner->TakeDamage( CTakeDamageInfo( this, this, 25, DMG_SHOCK | DMG_CRUSH ) );
			
			color32 dalekgunDamage = { 255, 128, 0, 128 };
			UTIL_ScreenFade(pOwner, dalekgunDamage, 0.2f, 0.2f, FFADE_IN);

			m_flNextChargeTime = gpGlobals->curtime + random->RandomFloat( 0.5f, 2.5f );
		}

		return;
	}

	//Decrement power
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

	//Make sure we can draw power
	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		ChargedFire();
		return;
	}

	m_flNextChargeTime = gpGlobals->curtime + DALEKGUN_CHARGE_TIME;
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponDalekGun::SecondaryAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return;

	if ( m_bCharging == false )
	{
		//Start looping animation
		SendWeaponAnim( ACT_VM_PULLBACK );
		
		//Start looping sound
		if ( m_sndCharge == NULL )
		{
			CPASAttenuationFilter filter( this );
			m_sndCharge	= (CSoundEnvelopeController::GetController()).SoundCreate( filter, entindex(), CHAN_STATIC, "weapons/gauss/chargeloop.wav", ATTN_NORM );
		}

		assert(m_sndCharge!=NULL);
		if ( m_sndCharge != NULL )
		{
			(CSoundEnvelopeController::GetController()).Play( m_sndCharge, 1.0f, 50 );
			(CSoundEnvelopeController::GetController()).SoundChangePitch( m_sndCharge, 250, 3.0f );
		}

		m_flChargeStartTime = gpGlobals->curtime;
		m_bCharging = true;
		m_bChargeIndicated = false;

		//Decrement power
		pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	}

	IncreaseCharge();
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::AddViewKick(void)
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat( -0.5f, -0.2f );
	viewPunch.y = random->RandomFloat( -0.5f,  0.5f );
	viewPunch.z = 0;

	pPlayer->ViewPunch( viewPunch );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::ItemPostFrame(void)
{
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	/*if ( pPlayer->m_afButtonReleased & IN_ATTACK2 )
	{
		if ( m_bCharging )
		{
			ChargedFire();
		}
	}*/

	m_flCoilVelocity = UTIL_Approach( m_flCoilMaxVelocity, m_flCoilVelocity, 10.0f );
	m_flCoilAngle = UTIL_AngleMod( m_flCoilAngle + ( m_flCoilVelocity * gpGlobals->frametime ) );

	static float fanAngle = 0.0f;

	fanAngle = UTIL_AngleMod( fanAngle + 2 );

	//Update spinning bits
	SetBoneController( 0, fanAngle );
	SetBoneController( 1, m_flCoilAngle );
	
	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::StopChargeSound(void)
{
	if ( m_sndCharge != NULL )
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut( m_sndCharge, 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponDalekGun::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	StopChargeSound();
	m_bCharging = false;
	m_bChargeIndicated = false;

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSound(SINGLE);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponDalekGun::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

		WeaponSound(SINGLE);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		Fire();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}


