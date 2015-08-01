
//
//
//
// G(ame)P(ackager)Installer DLL exported prototype functions.
//

#ifndef __SETUP_DEFINES
#define __SETUP_DEFINES

    typedef struct 
    {
      CHAR  componentIndex;
      CHAR  szComponentName[_MAX_PATH];
      CHAR  szComponentScript[_MAX_PATH];
      CHAR  szComponentVersion[_MAX_PATH];
    }COMPONENTLIST;

    #define MAX_FILESIZE         10240
    #define MAX_SETUP_FILE_LINE   1024
    #define MAX_SCRIPT_FILES       250

    #define SETUP_HEADER_LOW  0x5445535b //[SET...      
    #define SETUP_HEADER_HIGH 0x0a5d5055 //....UP]\n 

    #define ACTION_DECOMPRESS 0x01  
    #define ACTION_INSTALLING 0x02  
    #define ACTION_UNINSTALLING 0x03  
 
    #define IDS_EULA_TEXT                 "EULA = \0"
    #define IDS_APPDISPLAY_TEXT           "APPTITLE = \0"
    #define IDS_NUMCOMPONENTS_TEXT        "NUMCOMPONENTS = \0"
    #define IDS_COMPONENTNAME_TEXT        "COMPONENT %d NAME = \0"
    #define IDS_COMPONENTVERSION_TEXT     "            VERSION = \0"
    #define IDS_COMPONENTSCRIPTFILE_TEXT  "            SCRIPTFILE = \0"
    #define IDS_COMPONENTOPTIONAL_TEXT    "            OPTIONAL = \0"

#endif

typedef INT (__stdcall *REALARCADEINSTALLERDLL)( const CHAR* pszProductName,
                                              UINT  uiProductMajorMinorVersion,
                                        const CHAR* pszRgiFullSpec,
                                              BOOL  bInstallProduct,
                                        const CHAR* pszInstallToFolder,
                                        const CHAR* pszDecompressionDllPath,
                                        const CHAR* pszSetupDllPath
        	                            #ifdef SFXINSTALLER
                		                 ,UINT uiSFXOverhead
                                        #endif
                                        );

typedef INT (__stdcall *REALARCADEUNINSTALLERDLL)( const CHAR* pszProductName,
                                          UINT  uiProductMajorMinorVersion,
                                          const CHAR *pszSetupDllPath,
                                          const CHAR *pszSetupIniFullPath);

 
