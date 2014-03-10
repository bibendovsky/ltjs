========================================================================
	SimpleSample.cpp - MSVC Auto generated Hello World App with changes
			   to show RealArcade integration.

	Author:		Andrew Johnston, Paul Ingalls
			RealNetworks 2001
========================================================================

This is the standard MSVC Hello world with a few additions to show
how to use the minimal RealArcade API calls. Everything is located in
SimpleSample.cpp. 

The two key functions to look for are:
	WinMain 	-	contains initialization and cleanup code
	CheckRNMessages -	a user written function that shows one way
				to handle RealArcade messages.


WinMain is the standard windows main routine. Here the game should make
sure to call RngLoadInterstitialLibrary(1) to call the RealNetworks
interstitial manager. The interstitial manager will prompt the user with
an opportunity to buy the game in the free version and should not be
called in the full version of the game. If arcade is installed on the
users system it will launch Arcade and take the user to the game 
mezzanine.

Once you have checked to see if RngLoadInterstitialLibrary(1) has 
returned true (1) the code goes ahead and RNInitConsole to initialize 
the console and to let Arcade know that the game is launching. It 
returns a handle to the console object for use within the game.

On exit from WinMain there is a call to RNTermConsole to terminate
the game session with Arcade. In addition you should call 
RngLoadInterstitialLibrary(0) to indicate the game is complete. If Arcade
is not running the interstitial will display a dialog that gives the user
to purchase the full game if it is not the full version.

CheckRNMessages is an example of a user written function that checks
for Arcade messages. This example only responds to the PING message
(keep alive messaging) by sending the required PONG message. Notice
it is called from the AppIdle and WndProc (windows messages) functions
to ensure that it will be checked regularly.

This allows RealArcade to keep in touch with the game and tell if it 
has hung. RealArcade periodically sends a "PING" message and expects 
to get a "PONG" message back to tell it's alive. Otherwise, after a few
minutes of silence RealArcade will kill the "game" assuming that it has
hung.

This is not an high priority message in the sense that you don't need
to check for it every tick of the game (frame, etc). You must put this 
PING/PONG checking somewhere that will get checked at least every 30 seconds
since RealArcade will kill you if you quit responding.


========================================================================
       WIN32 APPLICATION : SimpleSample
========================================================================


AppWizard has created this SimpleSample application for you.  

This file contains a summary of what you will find in each of the files that
make up your SimpleSample application.

SimpleSample.cpp
    This is the main application source file.

SimpleSample.dsp
    This file (the project file) contains information at the project level and
    is used to build a single project or subproject. Other users can share the
    project (.dsp) file, but they should export the makefiles locally.
	

/////////////////////////////////////////////////////////////////////////////
AppWizard has created the following resources:

SimpleSample.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
	Visual C++.

res\SimpleSample.ico
    This is an icon file, which is used as the application's icon (32x32).
    This icon is included by the main resource file SimpleSample.rc.

small.ico
    %%This is an icon file, which contains a smaller version (16x16)
	of the application's icon. This icon is included by the main resource
	file SimpleSample.rc.

/////////////////////////////////////////////////////////////////////////////
Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named SimpleSample.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

/////////////////////////////////////////////////////////////////////////////
Other notes:

AppWizard uses "TODO:" to indicate parts of the source code you
should add to or customize.


/////////////////////////////////////////////////////////////////////////////
