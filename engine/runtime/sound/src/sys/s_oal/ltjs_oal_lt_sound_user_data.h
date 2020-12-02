#ifndef LTJS_OAL_LT_SOUND_USER_DATA_INCLUDED
#define LTJS_OAL_LT_SOUND_USER_DATA_INCLUDED


#include <array>


namespace ltjs
{


static constexpr auto oal_lt_sound_sys_max_user_data_count = 8;
static constexpr auto oal_lt_sound_sys_max_user_data_index = oal_lt_sound_sys_max_user_data_count - 1;

using OalLtSoundSysUserDataArray = std::array<std::intptr_t, oal_lt_sound_sys_max_user_data_count>;


} // ltjs


#endif // !LTJS_OAL_LT_SOUND_USER_DATA_INCLUDED
