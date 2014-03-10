
/************************************************************************************
 *                                                                                  *
 *                           The XZip SOLID export file                             *
 *                                                                                  *
 *                               XZip 2.0 (DLL)                                     *
 *                               by Sabin, Belu                                     *
 *                                                                                  *
 *                        (C) Copyright 2000 Netzip Inc.                            *
 *                      a subsidiary of RealNetworks, Inc.                          *
 *                                                                                  *
 ************************************************************************************/

#ifndef __XZIP_FUNCTIONS__
#define __XZIP_FUNCTIONS__

#include <windows.h>

/****************************************************************************************
 *
 *  XZip Version 2.0 VERSION INFORMATION  
 *
 ****************************************************************************************/

#define XZIP_MAJOR_VERSION 0x14 //2.0
#define XZIP_MINOR_VERSION 0x04 //3.0 + comment = 4.0

/****************************************************************************************
 *
 *  XZip Version 2.0 
 *
 *  Archive Commands  
 *
 ****************************************************************************************/



#define XZIP_ADD                       1  
#define XZIP_TEST                      2
#define XZIP_MOVE                      3
#define XZIP_LIST                      4
#define XZIP_LOCK                      5
#define XZIP_CREATE                    6 
#define XZIP_DELETE                    7
#define XZIP_REPAIR                    8
#define XZIP_EXTRACT                   9
#define XZIP_PROTECT                  10
#define XZIP_UNPROTECT                11 
#define XZIP_RENAME                   12 



/****************************************************************************************
 *
 *  XZip Version 2.0 
 *
 *  Error Codes
 *
 ****************************************************************************************/


#define ERROR_OPENING_FILE               1000  // " The following file could not be opened ! File does not exists, could not be created, or too many open files"
#define ERROR_RECOGNIZING_ARCHIVE        1001  // " The following archive cannot be recognized ! File may not be an XZIP 106 archive, or the archive header might be damaged"
#define ERROR_ARCHIVE_DAMAGED            1002  // " The following archive is damaged !"
#define ERROR_CRC_BAD_WRITE_FAILED       1003  // " Inconsistent data crc, data write failed !"
#define ERROR_CRC_BAD_READ_FAILED        1004  // " Inconsistent data crc, data read failed !"
#define ERROR_PACKING_FILE_HEADERS       1005  // " Error Packing Solid Header ! "
#define ERROR_MELTING_FILE_HEADER        1006  // " Error Melting Solid Header ! "
#define ERROR_WRITING_ARCHIVE_HEADER     1007  // " Error writting the archive header ! "
#define ERROR_READING_NEXT_HEADER        1008  // " Error reading the next header ! "
#define ERROR_UNEXPECTED_END_OF_ARCHIVE  1009  // " Unexpected END-OF-ARCHIVE encountered ! "
#define ERROR_SIZE_HAS_CHANGED           1010  // " The size of the following file changed since scanning ! Compression can no longer be performed."
#define ERROR_UNPROCESSABLE_ARCHIVE      1011  // " The following archive cannot be processed!"
#define ERROR_NO_FILES_TO_PROCESS        1012  // " The following archive seems to have no files !"
#define ERROR_MODIFYING_A_LOCKED_ARC     1013  // " Unable to modify LOCKED archive. Create a new archive by extracting files."
#define ERROR_RECOVERY_RECORDS_NOT_FOUND 1014  // " Recovery record not found ! The archive must have been protected first ('p'rotect command) !"
#define ERROR_RECOVERY_RECORDS_FOUND     1015  // " Recovery record found ! Archive is already protected ! "
#define ERROR_INCONSISTENT_HEADER        1016  // " Inconsistent header information ! If header CRC error was not reported, DLL received garbage data received."
#define ERROR_BAD_CRC                    1017  // " Solid archive seemes to have BAD CRC .... !"
#define ERROR_NOT_ENOUGH_SOLID_MEMORY    1018  // " Not enough memory!"
#define ERROR_USER_ABORT                 1019  // " User abort!"
#define ERROR_USERDEF_VOLSIZE_TOO_SMALL  1020  // " Volume size too small !"
#define ERROR_VOLUME_HEADER_DESYNC       1021  // " Volume information out of sync detected !"
#define ERROR_NOT_REMOVABLE_MEDIA        1022  // " The following disk is not a removable media : "
#define ERROR_GET_AUTOVOL_SIZE_FAILED    1023  // " Obtaining the available space on disk failed ! "
#define ERROR_LZ_INITIALIZATION_FAILED   1024  // " LZ Engine failed on initialization ! "
#define ERROR_BLOCK_COMPRESSION_FAILED   1025  // " Block compression failed ! "
#define ERROR_BLOCK_DECOMPRESSION_FAILED 1026  // " Block decompression failed ! "
#define ERROR_COMPRESSION_FAILED         1027  // " Compression failed ! "
#define ERROR_SENDING_COMPRESSED_BLOCK   1028  // " Sending current compressed block failed ! "
#define ERROR_COPY_ARCHIVE_ON_UPDATE_FAILED 1029 // " Cannot replace the existing archive with the temporary one! "
#define ERROR_CANNOT_MODIFY_VOLUME_ARC   1030  // " Cannot update volumed archives or non-volumed to volumed ones"

/****************************************************************************************
 *
 *  XZip Version 2.0
 *
 *  DLL initialization error codes and messages
 *
 ****************************************************************************************/


#define DLL_INIT_ERROR_0   2000  //"No archive name!"  
#define DLL_INIT_ERROR_1   2001  //"No temporary folder!"  
#define DLL_INIT_ERROR_2   2002  //"Function not supported!"  
#define DLL_INIT_ERROR_3   2003  //"Cannot allocate internal file list! Not enough memory or too many files."  
#define DLL_INIT_ERROR_4   2004  //"Voluming and ZipCloacking are not supported!"   
#define DLL_INIT_ERROR_5   2005  //"Internal voluming initialization/allocation failed!"  
#define DLL_INIT_ERROR_6   2006  //"Internal solid initialization/allocation failed!"  
#define DLL_INIT_ERROR_7   2007  //"Recursive value not recognized!"  
#define DLL_INIT_ERROR_8   2008  //"Non-solid archives are no longer supported by version 1.06 and up!"  
#define DLL_INIT_ERROR_9   2009  //"Solid archives are by default!"  
#define DLL_INIT_ERROR_10  2010  //"Table size not supported. Sizes range from 1(64Ktable) to 8(8Mbtable)!"  
#define DLL_INIT_ERROR_11  2011  //"Error in protection/recovery parameters!"  
#define DLL_INIT_ERROR_12  2012  //"No files given!"  
#define DLL_INIT_ERROR_13  2013  //"Global structure failed in initializing!"
#define DLL_INIT_ERROR_14  2014  //"NEW Name and OLD Name required to change a file name in the structure"


/****************************************************************************************
 *
 *  XZip Version 2.0 
 *
 *  Message INDEXES and Texts
 *
 ****************************************************************************************/


#define DLL_MESSAGE_0_    3000    
#define DLL_MESSAGE_1_    3001    
#define DLL_MESSAGE_2_    3002    
#define DLL_MESSAGE_3_    3003    
#define DLL_MESSAGE_4_    3004    
#define DLL_MESSAGE_5_    3005    
#define DLL_MESSAGE_6_    3006    
#define DLL_MESSAGE_7_    3007    
#define DLL_MESSAGE_8_    3008    
#define DLL_MESSAGE_9_    3009    
#define DLL_MESSAGE_10_   3010    
#define DLL_MESSAGE_11_   3011    
#define DLL_MESSAGE_12_   3012    
#define DLL_MESSAGE_13_   3013    
#define DLL_MESSAGE_14_   3014    
#define DLL_MESSAGE_15_   3015    
#define DLL_MESSAGE_16_   3016    
#define DLL_MESSAGE_17_   3017    
#define DLL_MESSAGE_18_   3018    
#define DLL_MESSAGE_19_   3019    
#define DLL_MESSAGE_20_   3020    
#define DLL_MESSAGE_21_   3021    
#define DLL_MESSAGE_22_   3022    
#define DLL_MESSAGE_23_   3023    
#define DLL_MESSAGE_24_   3024    
#define DLL_MESSAGE_25_   3025    
#define DLL_MESSAGE_26_   3026    
#define DLL_MESSAGE_27_   3027    
#define DLL_MESSAGE_28_   3028    
#define DLL_MESSAGE_29_   3029    
#define DLL_MESSAGE_30_   3030    
#define DLL_MESSAGE_31_   3031    
#define DLL_MESSAGE_32_   3032    
#define DLL_MESSAGE_33_   3033    
#define DLL_MESSAGE_34_   3034    
#define DLL_MESSAGE_35_   3035    
#define DLL_MESSAGE_36_   3036    
#define DLL_MESSAGE_37_   3037    
#define DLL_MESSAGE_38_   3038    
#define DLL_MESSAGE_39_   3039    
#define DLL_MESSAGE_40_   3040    
#define DLL_MESSAGE_41_   3041    
#define DLL_MESSAGE_42_   3042    
#define DLL_MESSAGE_43_   3043    
#define DLL_MESSAGE_44_   3044    
#define DLL_MESSAGE_45_   3045    
#define DLL_MESSAGE_46_   3046    
#define DLL_MESSAGE_47_   3047    
#define DLL_MESSAGE_48_   3048    
#define DLL_MESSAGE_49_   3049    
#define DLL_MESSAGE_50_   3050    
#define DLL_MESSAGE_51_   3051    
#define DLL_MESSAGE_52_   3052    
#define DLL_MESSAGE_53_   3053    
#define DLL_MESSAGE_54_   3054    
#define DLL_MESSAGE_55_   3055    
#define DLL_MESSAGE_56_   3056    
#define DLL_MESSAGE_57_   3057    
#define DLL_MESSAGE_58_   3058    
#define DLL_MESSAGE_59_   3059    
#define DLL_MESSAGE_60_   3060    
#define DLL_MESSAGE_61_   3061    
#define DLL_MESSAGE_62_   3062    
#define DLL_MESSAGE_63_   3063    
#define DLL_MESSAGE_64_   3064    
#define DLL_MESSAGE_65_   3065    
#define DLL_MESSAGE_66_   3066    
#define DLL_MESSAGE_67_   3067    
#define DLL_MESSAGE_68_   3068    
#define DLL_MESSAGE_69_   3069    
#define DLL_MESSAGE_70_   3070    
#define DLL_MESSAGE_71_   3071    
#define DLL_MESSAGE_72_   3072    
#define DLL_MESSAGE_73_   3073    
#define DLL_MESSAGE_74_   3074    
#define DLL_MESSAGE_75_   3075    
#define DLL_MESSAGE_76_   3076    
#define DLL_MESSAGE_77_   3077    
#define DLL_MESSAGE_78_   3078    
#define DLL_MESSAGE_79_   3079    
#define DLL_MESSAGE_80_   3080    
#define DLL_MESSAGE_81_   3081    
#define DLL_MESSAGE_82_   3082    
#define DLL_MESSAGE_83_   3083    
#define DLL_MESSAGE_84_   3084    
#define DLL_MESSAGE_85_   3085    
#define DLL_MESSAGE_86_   3086    
#define DLL_MESSAGE_87_   3087    
#define DLL_MESSAGE_88_   3088    
#define DLL_MESSAGE_89_   3089    
#define DLL_MESSAGE_90_   3090    
#define DLL_MESSAGE_91_   3091    
#define DLL_MESSAGE_92_   3092    
#define DLL_MESSAGE_93_   3093    
#define DLL_MESSAGE_94_   3094    
#define DLL_MESSAGE_95_   3095    
#define DLL_MESSAGE_96_   3096    
#define DLL_MESSAGE_97_   3097    
#define DLL_MESSAGE_98_   3098    
#define DLL_MESSAGE_99_   3099    
#define DLL_MESSAGE_100_  3100    
#define DLL_MESSAGE_101_  3101    
#define DLL_MESSAGE_102_  3102    
#define DLL_MESSAGE_103_  3103    
#define DLL_MESSAGE_104_  3104    
#define DLL_MESSAGE_105_  3105    


/****************************************************************************************
 *
 *  XZip Version 2.0
 *
 *  CALLBACK DEFINES, STRUCTURES and BUTTON IDS
 *
 ****************************************************************************************/


#define CALLBACK_REQUEST_PASSWORD_               201
#define CALLBACK_REQUEST_VOLUME_PATH_            202
#define CALLBACK_REQUEST_OVERWITE_PERMISSION     203
#define CALLBACK_REQUEST_ALL_VOLUMES_PERMISSION  204
#define CALLBACK_REQUEST_ANOTHER_REMOVABLE_MEDIA 205

#define CALLBACK_YES_       104
#define CALLBACK_NO_        105
#define CALLBACK_ALL_       106
#define CALLBACK_CANCEL_    107

#define CALL_BACK_MESSAGE_TYPE_0    0
#define CALL_BACK_MESSAGE_TYPE_1    1
#define CALL_BACK_MESSAGE_TYPE_2    2
#define CALL_BACK_MESSAGE_TYPE_3    3
#define CALL_BACK_MESSAGE_TYPE_4    4
#define CALL_BACK_MESSAGE_TYPE_5    5
#define CALL_BACK_MESSAGE_TYPE_6    6
#define CALL_BACK_MESSAGE_TYPE_7    7
#define CALL_BACK_MESSAGE_TYPE_8    8

typedef INT  (__stdcall  *XZIPCALLBACK1)(struct callback *callBack);
typedef VOID (__stdcall  *XZIPCALLBACK2)(struct callback *callBack);


#pragma pack(push, 1)
struct FileName
{
   char   *m_Name; 
   char    m_File;  // file or directory 
   char    relativePath;
};  
#pragma pack(pop)

#pragma pack(push, 1)
 typedef struct xzipArchiveItemStructure
 {
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    TCHAR* fileName;

 }XZIP_ARCHIVE_ITEM;
#pragma pack(pop)

  //////////////////////////////////////////////////////////////////////
  //
  //  Message types :
  //
  //   - Type 0 : " ..... "
  //   - Type 1 : " ... %s ... " 
  //   - Type 2 : " ... %d ... "
  //   - Type 3 : " ... %d:%d ... "
  //   - Type 4 : " ... %d:%s ... " 
  //   - Type 5 : " ... %s:%d ... " 
  //   - Type 6 : " ... %d %s %d %d... " 
  //   - Type 7 : " ... %s %d %s ... " 
  //   - Type 8 : " ... %d %d %d %d ... " 
  //
  ////////////////////////////////////////////////////////////////////// 
   

#pragma pack(push, 1)
struct callback
{
  //
  // m_structureSize : For history compatibility
  //
  int m_structureSize;
  //
  // m_structureSize : For history compatibility
  //

  int    m_percent;
  int    m_errorcode;
  int    m_button_no;  
  const char  *m_message;
  const char  *m_extramessage;

  int    m_messageType;  
  int    m_messageIndex;  
  const char  *m_sParameter1;
  int    m_iParameter1;
  const char  *m_sParameter2;
  int    m_iParameter2;
  const char  *m_sParameter3;
  int    m_iParameter3;
  const char  *m_sParameter4;
  int    m_iParameter4;

  //
  // We add at the end of the structure so that the older versions
  // wont get affected 
  //

  LPVOID m_userData;

};
#pragma pack(pop)

 

/****************************************************************************************
 *
 *  XZip Version 2.0
 *
 *  EXPORT STRUCTURE - THE Dll structure
 *
 ****************************************************************************************/


#pragma pack(push, 1)
typedef struct 
{
  //
  // m_structureSize : For history compatibility
  //
  int m_structureSize;
  //
  // m_structureSize : For history compatibility
  //

  char m_solid;
  char m_function;                          
  char m_tablesize;
  char m_recursive;
  char m_zipCloaking;
  char m_fullPathToExtract;
 
  char m_sfx;  
  char m_volume;  
  char m_compressEcc;                       
  char m_protectionMethod;                  
 
  int  m_userdefSize;  
  int  m_userDefExtraData;  
  int  m_selfExtractionEngineSize;

  struct FileName *m_SourceList;

  char *m_password;                           // password ...  
  char *m_archiveName;                        // archive name ...  
  char *m_extractionDirectory;                // extract to directory ...
  unsigned int m_MaxDirectories;
  

/*   int  (*EndDLLJob)(struct callback *);
  int  (*Utility_Function)(struct callback *);
  void (*ErrorMessageDisplayFunction)(struct callback *);
  void (*StatusMessageDisplayFunction)(struct callback *);  
 
  typedef INT  (__stdcall  *XZIPCALLBACK1)(struct callback *callBack);
  typedef VOID (__stdcall  *XZIPCALLBACK2)(struct callback *callBack);

  int  (*EndDLLJob)(struct callback *);
  int  (*Utility_Function)(struct callback *);
  void (*ErrorMessageDisplayFunction)(struct callback *);
  void (*StatusMessageDisplayFunction)(struct callback *);  
*/
  XZIPCALLBACK1 EndDLLJob;
  XZIPCALLBACK1 Utility_Function;
  XZIPCALLBACK2 ErrorMessageDisplayFunction;
  XZIPCALLBACK2 StatusMessageDisplayFunction;

  //
  // Whenever there is stuff to be added, we add in the end, 
  // so we know how IF to use the new added fields or not.
  //

  LPVOID m_userData;

} XZIP_CMDSTRUCTURE;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
 {
  //
  // m_structureSize : For history compatibility
  //
  int m_structureSize;
  //
  // m_structureSize : For history compatibility
  //

  char *m_password;                           // password ...  
  char  m_recursive;
  char  m_fullPathToExtract;
  char *m_extractionDirectory;                // extract to directory ...


/*  
  typedef INT  (__stdcall  *XZIPCALLBACK1)(struct callback *callBack);
  typedef VOID (__stdcall  *XZIPCALLBACK2)(struct callback *callBack);

  int  (*EndDLLJob)(struct callback *);
  int  (*Utility_Function)(struct callback *);
  void (*ErrorMessageDisplayFunction)(struct callback *);
  void (*StatusMessageDisplayFunction)(struct callback *);  
*/
  XZIPCALLBACK1 EndDLLJob;
  XZIPCALLBACK1 Utility_Function;
  XZIPCALLBACK2 ErrorMessageDisplayFunction;
  XZIPCALLBACK2 StatusMessageDisplayFunction;

  //
  // Whenever there is stuff to be added, we add in the end, 
  // so we know how IF to use the new added fields or not.
  //

  LPVOID m_userData;

} GETPARAM;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct  
{
  //
  // m_structureSize : For history compatibility
  //
  int m_structureSize;
  //
  // m_structureSize : For history compatibility
  //

  GETPARAM getParamStruct;
  char  m_function;                          
  char *m_archiveName;                        // archive name ...  
  int   m_selfExtractionEngineSize;

  //
  // Whenever there is stuff to be added, we add in the end, 
  // so we know how IF to use the new added fields or not.
  //

  LPVOID m_userData;

}SFXPARAMSTRUCTURE;

#pragma pack(pop)

extern XZIP_CMDSTRUCTURE  localXZipStructure;

#ifndef __XZIP_DLL
INT __stdcall XZip ( XZIP_CMDSTRUCTURE *xtrParamStructure);
#endif
 
#endif