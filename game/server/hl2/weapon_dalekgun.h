//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#include "basehlcombatweapon.h"

#ifndef WEAPON_DALEKGUN_H
#define WEAPON_DALEKGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "te_particlesystem.h"

#define DALEKGUN_BEAM_SPRITE "sprites/daleklaser.vmt"
//#define DALEKGUN_BEAM_SPRITE "sprites/laserbeam.vtf"

#define	DALEKGUN_CHARGE_TIME			0.2f
#define	MAX_DALEKGUN_CHARGE			16
#define	MAX_DALEKGUN_CHARGE_TIME		3
#define	DANGER_DALEKGUN_CHARGE_TIME	10

//=============================================================================
// Gauss explosion
//=============================================================================

class CTEDalekGunExplosion : public CTEParticleSystem
{
public:
	DECLARE_CLASS(CTEDalekGunExplosion, CTEParticleSystem);
	DECLARE_SERVERCLASS();

					CTEDalekGunExplosion(const char *name);
	virtual			~CTEDalekGunExplosion(void);

	virtual void	Test(const Vector& current_origin, const QAngle& current_angles) { };
	virtual	void	Create(IRecipientFilter& filter, float delay = 0.0f);

	CNetworkVar(int, m_nType);
	CNetworkVector(m_vecDirection);
};

//=============================================================================
// Gauss cannon
//=============================================================================

class CWeaponDalekGun : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponDalekGun, CBaseHLCombatWeapon);

	CWeaponDalekGun(void);

	DECLARE_SERVERCLASS();

	void	Spawn(void);
	void	Precache(void);
	void	PrimaryAttack(void);
	//void	SecondaryAttack(void);
	void	AddViewKick(void);
	void	DryFire(void);

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	bool	Holster(CBaseCombatWeapon *pSwitchingTo = NULL);

	void	ItemPostFrame(void);

	float	GetFireRate(void) { return 0.2f; }

	virtual const Vector &GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_1DEGREES;
		return cone;
	}

protected:

	void	Fire(void);
	//void	ChargedFire(void);

	void	StopChargeSound(void);

	void	DrawBeam(const Vector &startPos, const Vector &endPos, float width, bool useMuzzle = false);
	void	IncreaseCharge(void);

	EHANDLE			m_hViewModel;
	float			m_flNextChargeTime;
	CSoundPatch		*m_sndCharge;

	float			m_flChargeStartTime;
	bool			m_bCharging;
	bool			m_bChargeIndicated;

	float			m_flCoilMaxVelocity;
	float			m_flCoilVelocity;
	float			m_flCoilAngle;

	DECLARE_ACTTABLE();
};

#endif // WEAPON_DALEKGUN_H
