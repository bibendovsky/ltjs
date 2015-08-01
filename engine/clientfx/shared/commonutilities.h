// ----------------------------------------------------------------------- //
//
// MODULE  : CommonUtilities.h
//
// PURPOSE : Utility functions
//
// CREATED : 5/4/98
//
// (c) 1998-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMON_UTILITIES_H__
#define __COMMON_UTILITIES_H__

#ifdef _CLIENTBUILD
#include "iclientshell.h"
#include "iltclient.h"
#else
#include "iservershell.h"
#include "iltserver.h"
#endif

#pragma warning( disable : 4786 )
#include <set>
#include <string>
#include <list>
#include <vector>

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))

int GetRandom();
int GetRandom(int range);
int GetRandom(int lo, int hi);
float GetRandom(float min, float max);


// String containers
class CaselessGreater
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) > 0 );
	}
};

class CaselessLesser
{
public:
	
	bool operator()(const std::string & x, const std::string & y) const
	{
		return (stricmp(x.c_str(), y.c_str()) < 0 );
	}
};


typedef std::set<std::string,CaselessLesser> StringSet;
typedef std::list<std::string> StringList;
typedef std::vector<std::string> StringArray;


//
// LINKFROM_MODULE
// LINKTO_MODULE
//
// These macros can be used to ensure a module does not get optimized out by the linker when
// linking a static lib into a dll or exe.  If a module in a static lib contains symbols
// that are never referenced by the dll or exe linking with it, then the module will not
// be used in the link.  To ensure a module gets used establish a link between the unreferenced
// module and a known referenced module.  The referenced module can be part of the static lib
// as well, but it must be somehow connected to modules in the dll or exe.  The need for
// this comes from the use of class factory like architecture, where dummy objects use
// their constructors to do work, but the objects themselves are never referenced.
//
// To use, put the LINKFROM_MODULE(linkfrom_module_name) into the unreferenced module cpp, where 
// linkfrom_module_name is the name unreferenced module.  Then put the LINKTO_MODULE(linkto_module_name) into the
// known referenced module where the linkto_module_name is the name of the unreferenced module.
// 
#define LINKFROM_MODULE(linkfrom_module_name)					\
	int g_n##linkfrom_module_name = 0;

#define LINKTO_MODULE(linkto_module_name)						\
	extern int g_n##linkto_module_name;							\
	static int* s_pn##linkto_module_name = &g_n##linkto_module_name;


#endif // __COMMON_UTILITIES_H__