// (C)2007 S2 Games
// c_precipemitter.h
//
//=============================================================================
#ifndef __C_PRECIPEMITTER_H__
#define __C_PRECIPEMITTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
#include "c_simpleparticle.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CPrecipEmitterDef
//=============================================================================
class CPrecipEmitterDef : public IEmitterDef
{
private:
	// Emitter Properties
	tstring						m_sOwner;
	CRangei						m_riLife;
	CRangei						m_riExpireLife;
	CRangei						m_riCount;
	CRangei						m_riTimeNudge;
	CRangei						m_riDelay;
	bool						m_bLoop;
	CTemporalPropertyRangef		m_rfSpawnRate;
	CTemporalPropertyRangei		m_riMinParticleLife;
	CTemporalPropertyRangei		m_riMaxParticleLife;
	int							m_iParticleLifeAlloc;
	CTemporalPropertyRangei		m_riParticleTimeNudge;
	CTemporalPropertyRangef		m_rfGravity;
	CTemporalPropertyRangef		m_rfMinSpeed;
	CTemporalPropertyRangef		m_rfMaxSpeed;
	CTemporalPropertyRangef		m_rfMinAcceleration;
	CTemporalPropertyRangef		m_rfMaxAcceleration;
	CTemporalPropertyRangef		m_rfMinAngle;
	CTemporalPropertyRangef		m_rfMaxAngle;
	CTemporalPropertyRangef		m_rfMinInheritVelocity;
	CTemporalPropertyRangef		m_rfMaxInheritVelocity;
	CTemporalPropertyRangef		m_rfLimitInheritVelocity;
	ResHandle					m_hMaterial;
	CVec3f						m_v3Dir;
	EDirectionalSpace			m_eDirectionalSpace;
	float						m_fDrag;
	float						m_fFriction;
	CRangef						m_rfDrawDistance;
	bool						m_bCollide;
	CTemporalPropertyv3			m_tv3ParticleColor;
	CTemporalPropertyRangef		m_rfParticleAlpha;
	CTemporalPropertyRangef		m_rfParticleScale;
	float						m_fDepthBias;

public:
	virtual ~CPrecipEmitterDef();
	CPrecipEmitterDef
	(
		const tstring &sOwner,
		const CRangei &riLife,
		const CRangei &riExpireLife,
		const CRangei &riCount,
		const CRangei &riTimeNudge,
		const CRangei &riDelay,
		bool bLoop,
		const CTemporalPropertyRangef &rfSpawnRate,
		const CTemporalPropertyRangei &riMinParticleLife,
		const CTemporalPropertyRangei &riMaxParticleLife,
		int iParticleLifeAlloc,
		const CTemporalPropertyRangei &riParticleTimeNudge,
		const CTemporalPropertyRangef &rfGravity,
		const CTemporalPropertyRangef &rfMinSpeed,
		const CTemporalPropertyRangef &rfMaxSpeed,
		const CTemporalPropertyRangef &rfMinAcceleration,
		const CTemporalPropertyRangef &rfMaxAcceleration,
		const CTemporalPropertyRangef &rfMinAngle,
		const CTemporalPropertyRangef &rfMaxAngle,
		const CTemporalPropertyRangef &rfMinInheritVelocity,
		const CTemporalPropertyRangef &rfMaxInheritVelocity,
		const CTemporalPropertyRangef &rfLimitInheritVelocity,
		ResHandle hMaterial,
		const CVec3f &v3Dir,
		EDirectionalSpace eDirectionalSpace,
		float fDrag,
		float fFriction,
		const CRangef &rfRange,
		bool bCollide,
		const CTemporalPropertyv3 &tv3ParticleColor,
		const CTemporalPropertyRangef &m_rfParticleAlpha,
		const CTemporalPropertyRangef &m_rfParticleScale,
		float fDepthBias
	);

	IEmitter*	Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner);

	const tstring&	GetOwner() const	{ return m_sOwner; }
	int			GetLife() const			{ return m_riLife; }
	int			GetExpireLife() const	{ return m_riExpireLife; }
	int			GetCount() const		{ return m_riCount; }
	int			GetTimeNudge() const	{ return m_riTimeNudge; }
	int			GetDelay() const		{ return m_riDelay; }
	bool		GetLoop() const			{ return m_bLoop; }

	CTemporalPropertyf		GetSpawnRate() const			{ return m_rfSpawnRate; }
	CTemporalPropertyi		GetMinParticleLife() const		{ return m_riMinParticleLife; }
	CTemporalPropertyi		GetMaxParticleLife() const		{ return m_riMaxParticleLife; }
	int						GetParticleLifeAlloc() const	{ return m_iParticleLifeAlloc; }
	CTemporalPropertyi		GetParticleTimeNudge() const	{ return m_riParticleTimeNudge; }
	CTemporalPropertyf		GetGravity() const				{ return m_rfGravity; }
	CTemporalPropertyf		GetMinSpeed() const				{ return m_rfMinSpeed; }
	CTemporalPropertyf		GetMaxSpeed() const				{ return m_rfMaxSpeed; }
	CTemporalPropertyf		GetMinAcceleration() const		{ return m_rfMinAcceleration; }
	CTemporalPropertyf		GetMaxAcceleration() const		{ return m_rfMaxAcceleration; }
	CTemporalPropertyf		GetMinAngle() const				{ return m_rfMinAngle; }
	CTemporalPropertyf		GetMaxAngle() const				{ return m_rfMaxAngle; }
	CTemporalPropertyf		GetMinInheritVelocity() const	{ return m_rfMinInheritVelocity; }
	CTemporalPropertyf		GetMaxInheritVelocity() const	{ return m_rfMaxInheritVelocity; }
	CTemporalPropertyf		GetLimitInheritVelocity() const	{ return m_rfLimitInheritVelocity; }

	ResHandle				GetMaterial() const				{ return m_hMaterial; }
	const CVec3f&			GetDir() const					{ return m_v3Dir; }
	EDirectionalSpace		GetDirectionalSpace() const		{ return m_eDirectionalSpace; }
	float					GetDrag() const					{ return m_fDrag; }
	float					GetFriction() const				{ return m_fFriction; }

	float					GetDrawDistance() const			{ return m_rfDrawDistance; }

	bool					GetCollide() const			{ return m_bCollide; }

	const CTemporalPropertyv3&	GetParticleColor() const	{ return m_tv3ParticleColor; }
	CTemporalPropertyf		GetParticleAlpha() const		{ return m_rfParticleAlpha; }
	CTemporalPropertyf		GetParticleScale() const		{ return m_rfParticleScale; }

	float					GetDepthBias() const			{ return m_fDepthBias; }
};

//=============================================================================
// CPrecipEmitter
//=============================================================================
class CPrecipEmitter : public IEmitter
{
private:
	float		m_fSelectionWeightRange;

	float		m_fAccumulator;

	uint		m_uiFrontSlot;
	uint		m_uiBackSlot;

	ParticleList	m_vParticles;

	int			m_iSpawnCount;

	CVec3f		m_v3LastBaseVelocity;

	float		m_fLastLerp;
	float		m_fLastTime;

	bool		m_bLastActive;

	// Emitter Properties
	int			m_iCount;

	CTemporalPropertyf		m_rfSpawnRate;
	CTemporalPropertyi		m_riMinParticleLife;
	CTemporalPropertyi		m_riMaxParticleLife;
	CTemporalPropertyi		m_riParticleTimeNudge;
	CTemporalPropertyf		m_rfGravity;
	CTemporalPropertyf		m_rfMinSpeed;
	CTemporalPropertyf		m_rfMaxSpeed;
	CTemporalPropertyf		m_rfMinAcceleration;
	CTemporalPropertyf		m_rfMaxAcceleration;
	CTemporalPropertyf		m_rfMinAngle;
	CTemporalPropertyf		m_rfMaxAngle;
	CTemporalPropertyf		m_rfMinInheritVelocity;
	CTemporalPropertyf		m_rfMaxInheritVelocity;
	CTemporalPropertyf		m_rfLimitInheritVelocity;

	ResHandle			m_hMaterial;
	CVec3f				m_v3Dir;
	float				m_fDrag;
	float				m_fFriction;

	float				m_fDrawDistance;

	bool		m_bCollide;

	CTemporalPropertyv3			m_tv3ParticleColor;
	CTemporalPropertyf			m_tfParticleAlpha;
	CTemporalPropertyf			m_tfParticleScale;

	float			m_fDepthBias;

public:
	virtual ~CPrecipEmitter();
	CPrecipEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CPrecipEmitterDef &eSettings);

	virtual void	ResumeFromPause(uint uiMilliseconds);
	bool	Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

	uint	GetNumBillboards();
	bool	GetBillboard(uint uiIndex, SBillboard &outBillboard);
};
//=============================================================================
#endif	//__C_PRECIPEMITTER_H__
