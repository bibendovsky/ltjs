#ifndef LTJS_S_OAL_INCLUDED
#define LTJS_S_OAL_INCLUDED


#include "iltsound.h"


extern "C"
{

	
__declspec(dllexport) char* SoundSysDesc();
__declspec(dllexport) ILTSoundSys* SoundSysMake();


}


#endif // LTJS_S_OAL_INCLUDED
