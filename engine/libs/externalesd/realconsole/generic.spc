
COPY %SPEC%\*.* %ENCSDK%\ 

CMPPREF DisplayName "RealArcade Developer SDK"
CMPPREF Product1 "RealArcadeGames:1.0"

COPY %SPEC%\Setup.ini %ENCSDK%\Setup.ini 
COPY %SPEC%\GameFind.Xml %ENCSDK%\GameFind.Xml 

REG HKCR "SOFTWARE\RealNetworks\Games\RealArcade Developer SDK.rngcsdk"
REG HKCR "SOFTWARE\RealNetworks\Games\RealArcade Developer SDK.rngcsdk\1.0"
REG HKCR "SOFTWARE\RealNetworks\Games\RealArcade Developer SDK.rngcsdk\1.0" "Executable" "bin\realarcadegp.exe"
REG HKCR "SOFTWARE\RealNetworks\Games\RealArcade Developer SDK.rngcsdk\1.0" "InstallPath" "%ENCSDK%"
REG HKCR "SOFTWARE\RealNetworks\Games\RealArcade Developer SDK.rngcsdk\1.0" "InstallPath" "%ENCSDK%"
STARTMENU Real\Games "RealArcade Developer SDK" "%INSTALL%\RNArcade.exe" "/show f0702908-8d63-452c-af24-d4fa4d7d48e5"
REG HKCR "SOFTWARE\RealNetworks\Games\RealArcade Developer SDK.rngcsdk\1.0" "LicenseKey" "f4d4b51-6ddde4eb"
