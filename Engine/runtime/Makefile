###############################################################
#
# Makefile for the Engine-Dynamic library Server. Auto-generates
# subordinate makefiles as necessary based on Visual C++ 2002 
# project files. Depends on python and xsltproc programs.
#
# (c) 2004 Monolith Productions, Inc. All Rights Reserved.
# (c) 2004 TouchdownEntertainment, Inc. All Rights Reserved
#
# dlj - 11/04 modified for 2002 and Jupiter 
#
###############################################################

all: Makefile.top build

Makefile.top: Engine.sln
	$(SHELL) -ec 'python ../../Linux/sln2makefile.py -s Engine.sln -p DLL_Server > Makefile.top'

build:
	make -f Makefile.top VCPROJSCRIPTPATH=../../Linux	

clean:
	make -f Makefile.top clean
	rm -f Makefile.top

rebuild: clean build

