#include "s_oal.h"

#include "ltjs_oal_lt_sound_sys.h"


char* SoundSysDesc()
{
	static char* description = const_cast<char*>("OpenAL");
	return description;
}

ILTSoundSys* SoundSysMake()
{
	static ltjs::OalLtSoundSys oal_lt_sound_sys{};
	return &oal_lt_sound_sys;
}
