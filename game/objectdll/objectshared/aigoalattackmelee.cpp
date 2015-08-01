// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAttackMelee.cpp
//
// PURPOSE : AIGoalAttackMelee implementation
//
// CREATED : 10/09/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "aigoalattackmelee.h"
#include "aigoalmgr.h"
#include "animatorplayer.h"
#include "ai.h"

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalAttackMelee, kGoal_AttackMelee);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIGoalAttackMelee::CAIGoalAttackMelee()
{
	m_eWeaponType = kAIWeap_Melee;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::Save / Load
//
//	PURPOSE:	Save / Load
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackMelee::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);
}

void CAIGoalAttackMelee::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIGoalAttackMelee::ActivateGoal
//
//	PURPOSE:	Activate goal.
//
// ----------------------------------------------------------------------- //

void CAIGoalAttackMelee::ActivateGoal()
{
	super::ActivateGoal();
}

