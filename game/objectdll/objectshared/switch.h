// ----------------------------------------------------------------------- //
//
// MODULE  : Switch.h
//
// PURPOSE : A Switch object
//
// CREATED : 12/3/97
//
// ----------------------------------------------------------------------- //

#ifndef __SWITCH_H__
#define __SWITCH_H__


//
// Includes...
//

	#include "activeworldmodel.h"

LINKTO_MODULE( Switch );

//
// Structs...
//

class Switch : public ActiveWorldModel
{

	public:

		Switch();
		virtual ~Switch();
};


#endif // __SWITCH_H__