// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICommandHandler.h
//
// PURPOSE : Base clase for objects that receive messages from controls
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_LTGUICOMMANDHANDLER_H_)
#define _LTGUICOMMANDHANDLER_H_


#include <cstdint>


class CLTGUICommandHandler
{
public:
	CLTGUICommandHandler();
	virtual ~CLTGUICommandHandler();

    uint32  SendCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2)
	{
		return OnCommand(nCommand, nParam1, nParam2);
	}

protected:
    virtual uint32 OnCommand(uint32 nCommand, std::uintptr_t nParam1, std::uintptr_t nParam2)
	{
		return 0;
	}
};

#endif // _LTGUICOMMANDHANDLER_H_