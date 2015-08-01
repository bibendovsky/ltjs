RealArcade Integration SDK Read Me

I) Build a package for your game - for generic installation
  1.	Place all of your files required for installation in a directory 
	structure as they would exist once installed.
  2.	Install RealArcade from the bin directory.
  3.	Run the RealArcadeGP application in the bin/Game Packager directory.
  4.	Run through the steps of the packaging wizard, make sure to use
	the GUID that your application uses in the Init method.
	- If your installation process is more involved then copying files
	  to a location, read the Custom Installs.htm file in the docs
	  directory.
  5.	Double click the generated RGI file, your game should be installed 
	and found by RealArcade.

II) Add API support to your game
  1. 	See src/SimpleSample for an example of using the smallest subset
	of API calls required for RealArcade integration
  2.	Again, make sure that the GameID in the INIT call matches that in the
 	gamefind xml file so that RealArcade can identify the game.

III) Add more advanced API support to the game
  1.	For now,  read the docs.  We will have a more involved example soon.


Feel free to email me with questions - ingalls@real.com

