// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  

// This removes warnings in Vis C about the CodeWarrior pragma 'force_active'
//
#pragma warning( disable : 4068 )

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <limits.h>

#include "mfcstub.h"

#include "debugnew.h"

#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "globals.h"

#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "iltmath.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"

#include "ltobjref.h"

#include "ltobjref.h"

#include "factory.h"

#include "serverutilities.h"
#include "gameservershell.h"
#include "commonutilities.h"

// Infrequently changed, often included files:
#include "gamebase.h"
#include "butelistreader.h"
#include "clientlightfx.h"
#include "clientservershared.h"
#include "commandmgr.h"
#include "commandobject.h"
#include "aiclassfactory.h"
#include "cvartrack.h"
#include "controller.h"
#include "debrisfuncs.h"
#include "destructiblemodel.h"
#include "fastheap.h"
#include "faststack.h"
#include "prop.h"
#include "bankedlist.h"
#include "editable.h"
#include "serverutilities.h"

#endif // __STDAFX_H__