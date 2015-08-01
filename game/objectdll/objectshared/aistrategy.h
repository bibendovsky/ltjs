// (c) 2001 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_STRATEGY_H__
#define __AI_STRATEGY_H__

#include "aiclassfactory.h"

enum EnumAIStrategyType
{
	kStrat_InvalidType= -1,
	#define STRATEGY_TYPE_AS_ENUM 1
	#include "aistrategytypeenums.h"
	#undef STRATEGY_TYPE_AS_ENUM

	kStrat_Count,
};


#endif
