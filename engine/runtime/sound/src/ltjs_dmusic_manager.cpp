#include "bdefs.h"


#ifndef LTJS_USE_DIRECT_MUSIC8


#include <string>
#include <utility>
#include "console.h"
#include "ltpvalue.h"
#include "soundmgr.h"
#include "ltjs_dmusic_manager.h"


// output error to console
extern void LTDMConOutError(char *pMsg, ...);

// output warning to console
extern void LTDMConOutWarning(char *pMsg, ...);

// output message to console
extern void LTDMConOutMsg(int nLevel, char *pMsg, ...);


namespace ltjs
{


namespace ul = bibendovsky::spul;


define_interface(DMusicManager, ILTDirectMusicMgr)


class DMusicManager::Impl
{
public:
	Impl()
		:
		sound_sys_{},
		is_initialized_{},
		is_level_initialized_{}
	{
	}

	Impl(
		const Impl& that) = delete;

	Impl& operator=(
		const Impl& that) = delete;

	Impl(
		Impl&& that)
		:
		sound_sys_{std::move(that.sound_sys_)},
		is_initialized_{std::move(that.is_initialized_)},
		is_level_initialized_{std::move(that.is_level_initialized_)}
	{
	}

	~Impl()
	{
	}


	// =========================================================================
	// API
	//

	LTRESULT api_init()
	{
		::LTDMConOutMsg(3, "DMusicManager: Init\n");

		if (is_initialized_)
		{
			::LTDMConOutError("DMusicManager: Already initialized.\n");
			return LT_ERROR;
		}

		sound_sys_ = GetSoundSys();

		if (!sound_sys_)
		{
			::LTDMConOutError("DMusicManager: No sound system.\n");
			return LT_ERROR;
		}

		is_initialized_ = true;

		return LT_OK;
	}

	LTRESULT api_term()
	{
		::LTDMConOutMsg(3, "DMusicManager: Term\n");

		if (!is_initialized_)
		{
			::LTDMConOutWarning("DMusicManager: Already uninitialized.");
			return LT_OK;
		}

		if (is_level_initialized_)
		{
			api_term_level();
		}

		sound_sys_ = nullptr;

		is_initialized_ = false;

		return LT_OK;
	}

	LTRESULT api_init_level(
		const char* working_directory,
		const char* control_file_name,
		const char* define1,
		const char* define2,
		const char* define3)
	{
		return LT_ERROR;
	}

	LTRESULT api_term_level()
	{
		return LT_OK;
	}

	LTRESULT api_play()
	{
		return LT_ERROR;
	}

	LTRESULT api_stop(
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	LTRESULT api_pause(
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	LTRESULT api_unpause()
	{
		return LT_ERROR;
	}

	LTRESULT api_set_volume(
		const long volume)
	{
		static_cast<void>(volume);

		return LT_ERROR;
	}

	LTRESULT api_change_intensity(
		const int new_intensity,
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(new_intensity);
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	LTRESULT api_play_secondary(
		const char* segment_name,
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(segment_name);
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	LTRESULT api_stop_secondary(
		const char* segment_name,
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(segment_name);
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	LTRESULT api_play_motif(
		const char* style_name,
		const char* motif_name,
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(style_name);
		static_cast<void>(motif_name);
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	LTRESULT api_stop_motif(
		const char* style_name,
		const char* motif_name,
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(style_name);
		static_cast<void>(motif_name);
		static_cast<void>(start_type);

		return LT_ERROR;
	}

	int api_get_cur_intensity()
	{
		return -1;
	}

	LTDMEnactTypes api_string_to_enact_type(
		const char* name)
	{
		return LTDMEnactInvalid;
	}

	void api_enact_type_to_string(
		LTDMEnactTypes type,
		char* name)
	{
		if (!name)
		{
			return;
		}

		*name = '\0';
	}

	int api_get_num_intensities()
	{
		return 0;
	}

	int api_get_initial_intensity()
	{
		return -1;
	}

	int api_get_initial_volume()
	{
		return 0;
	}

	int api_get_volume_offset()
	{
		return 0;
	}

	//
	// API
	// =========================================================================


private:
	ILTSoundSys* sound_sys_;

	bool is_initialized_;
	bool is_level_initialized_;


	static const std::string enact_invalid_name;
	static const std::string enact_default_name;
	static const std::string enact_immediatly_name;
	static const std::string enact_immediately_name;
	static const std::string enact_immediate_name;
	static const std::string enact_next_beat_name;
	static const std::string enact_next_measure_name;
	static const std::string enact_next_grid_name;
	static const std::string enact_next_segment_name;
	static const std::string enact_next_marker_name;
	static const std::string enact_beat_name;
	static const std::string enact_measure_name;
	static const std::string enact_grid_name;
	static const std::string enact_segment_name;
	static const std::string enact_marker_name;
}; // DMusicManager::Impl


const std::string DMusicManager::Impl::enact_invalid_name = "Invalid";
const std::string DMusicManager::Impl::enact_default_name = "Default";
const std::string DMusicManager::Impl::enact_immediatly_name = "Immediatly";
const std::string DMusicManager::Impl::enact_immediately_name = "Immediately";
const std::string DMusicManager::Impl::enact_immediate_name = "Immediate";
const std::string DMusicManager::Impl::enact_next_beat_name = "NextBeat";
const std::string DMusicManager::Impl::enact_next_measure_name = "NextMeasure";
const std::string DMusicManager::Impl::enact_next_grid_name = "NextGrid";
const std::string DMusicManager::Impl::enact_next_segment_name = "NextSegment";
const std::string DMusicManager::Impl::enact_next_marker_name = "NextMarker";
const std::string DMusicManager::Impl::enact_beat_name = "Beat";
const std::string DMusicManager::Impl::enact_measure_name = "Measure";
const std::string DMusicManager::Impl::enact_grid_name = "Grid";
const std::string DMusicManager::Impl::enact_segment_name = "Segment";
const std::string DMusicManager::Impl::enact_marker_name = "Marker";


DMusicManager::DMusicManager()
	:
	pimpl_{std::make_unique<Impl>()}
{
}

DMusicManager::DMusicManager(
	DMusicManager&& that)
	:
	pimpl_{std::move(that.pimpl_)}
{
}

DMusicManager::~DMusicManager()
{
}

LTRESULT DMusicManager::Init()
{
	return pimpl_->api_init();
}

LTRESULT DMusicManager::Term()
{
	return pimpl_->api_term();
}

LTRESULT DMusicManager::InitLevel(
	const char* working_directory,
	const char* control_file_name,
	const char* define1,
	const char* define2,
	const char* define3)
{
	return pimpl_->api_init_level(working_directory, control_file_name, define1, define2, define3);
}

LTRESULT DMusicManager::TermLevel()
{
	return pimpl_->api_term_level();
}

LTRESULT DMusicManager::Play()
{
	return pimpl_->api_play();
}

LTRESULT DMusicManager::Stop(
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_stop(start_type);
}

LTRESULT DMusicManager::Pause(
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_pause(start_type);
}

LTRESULT DMusicManager::UnPause()
{
	return pimpl_->api_unpause();
}

LTRESULT DMusicManager::SetVolume(
	const long volume)
{
	return pimpl_->api_set_volume(volume);
}

LTRESULT DMusicManager::ChangeIntensity(
	const int new_intensity,
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_change_intensity(new_intensity, start_type);
}

LTRESULT DMusicManager::PlaySecondary(
	const char* segment_name,
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_play_secondary(segment_name, start_type);
}

LTRESULT DMusicManager::StopSecondary(
	const char* segment_name,
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_stop_secondary(segment_name, start_type);
}

LTRESULT DMusicManager::PlayMotif(
	const char* style_name,
	const char* motif_name,
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_play_motif(style_name, motif_name, start_type);
}

LTRESULT DMusicManager::StopMotif(
	const char* style_name,
	const char* motif_name,
	const LTDMEnactTypes start_type)
{
	return pimpl_->api_stop_motif(style_name, motif_name, start_type);
}

int DMusicManager::GetCurIntensity()
{
	return pimpl_->api_get_cur_intensity();
}

LTDMEnactTypes DMusicManager::StringToEnactType(
	const char* name)
{
	return pimpl_->api_string_to_enact_type(name);
}

void DMusicManager::EnactTypeToString(
	LTDMEnactTypes type,
	char* name)
{
	pimpl_->api_enact_type_to_string(type, name);
}

int DMusicManager::GetNumIntensities()
{
	return pimpl_->api_get_num_intensities();
}

int DMusicManager::GetInitialIntensity()
{
	return pimpl_->api_get_initial_intensity();
}

int DMusicManager::GetInitialVolume()
{
	return pimpl_->api_get_initial_volume();
}

int DMusicManager::GetVolumeOffset()
{
	return pimpl_->api_get_volume_offset();
}


} // ltjs


#ifndef NOLITHTECH
extern int32 g_CV_LTDMConsoleOutput;
#else
extern signed int g_CV_LTDMConsoleOutput;
#endif // !NOLITHTECH


namespace
{


void ltdm_console_output(
	const int level,
	const CONCOLOR color,
	const char* const message,
	va_list args_ptr)
{
	if (::g_CV_LTDMConsoleOutput < level)
	{
		return;
	}

	char msg[500];
	*msg = '\0';

	::LTVSNPrintF(msg, sizeof(msg), message, args_ptr);

#ifndef NOLITHTECH
	if (msg[::strlen(msg) - 1] == '\n')
	{
		msg[::strlen(msg) - 1] = '\0';
	}

	::con_PrintString(color, 0, msg);
#else
	::DebugConsoleOutput(msg, PValue_GetR(color), PValue_GetG(color), PValue_GetB(color));
#endif // !NOLITHTECH
}


} // namespace


void LTDMConOutError(
	char* message,
	...)
{
#ifndef NOLITHTECH
	const auto color = CONRGB(255, 0, 128);
#else
	const auto color = CONRGB(255, 0, 0);
#endif

	va_list marker;
	va_start(marker, message);
	ltdm_console_output(0, color, message, marker);
	va_end(marker);
}

void LTDMConOutWarning(
	char* message,
	...)
{
#ifndef NOLITHTECH
	const auto color = CONRGB(0, 255, 128);
#else
	const auto color = CONRGB(0, 255, 0);
#endif

	va_list marker;
	va_start(marker, message);
	ltdm_console_output(2, color, message, marker);
	va_end(marker);
}

void LTDMConOutMsg(
	const int level,
	char *message,
	...)
{
#ifndef NOLITHTECH
	const auto color = CONRGB(128, 255, 128);
#else
	const auto color = CONRGB(0, 0, 0);
#endif

	va_list marker;
	va_start(marker, message);
	ltdm_console_output(level, color, message, marker);
	va_end(marker);
}


#endif LTJS_USE_DIRECT_MUSIC8
