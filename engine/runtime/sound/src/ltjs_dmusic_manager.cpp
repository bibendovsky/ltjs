#include "bdefs.h"


#ifndef LTJS_USE_DIRECT_MUSIC8


#include <string>
#include <unordered_map>
#include <utility>
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_scope_guard.h"
#include "console.h"
#include "ltpvalue.h"
#include "soundmgr.h"
#include "ltdirectmusiccontrolfile.h"
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
		is_level_initialized_{},
		working_directory_{},
		control_file_name_{},
		intensity_count_{},
		initial_intensity_{},
		initial_volume_{},
		volume_offset_{}
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
		is_level_initialized_{std::move(that.is_level_initialized_)},
		working_directory_{std::move(that.working_directory_)},
		control_file_name_{std::move(that.control_file_name_)},
		intensity_count_{std::move(that.intensity_count_)},
		initial_intensity_{std::move(that.initial_intensity_)},
		initial_volume_{std::move(that.initial_volume_)},
		volume_offset_{std::move(that.volume_offset_)}
	{
		that.is_initialized_ = false;
		that.is_level_initialized_ = false;
	}

	~Impl()
	{
		assert(!is_initialized_ && !is_level_initialized_);
	}


	// =========================================================================
	// API
	//

	LTRESULT api_init()
	{
		const auto method_name = "DMusicManager::Init";

		::LTDMConOutMsg(3, "%s.\n", method_name);

		if (is_initialized_)
		{
			::LTDMConOutError("%s: Already initialized.\n", method_name);
			return LT_ERROR;
		}

		sound_sys_ = GetSoundSys();

		if (!sound_sys_)
		{
			::LTDMConOutError("%s: No sound system.\n", method_name);
			return LT_ERROR;
		}

		is_initialized_ = true;

		return LT_OK;
	}

	LTRESULT api_term()
	{
		const auto method_name = "DMusicManager::Term";

		::LTDMConOutMsg(3, "%s.\n", method_name);

		if (!is_initialized_)
		{
			::LTDMConOutWarning("%s: Already uninitialized.\n", method_name);
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
		const auto method_name = "DMusicManager::InitLevel";

		::LTDMConOutMsg(
			3,
			"%s: working dir=%s control file=%s define1=%s define2=%s define3=%s\n",
			method_name, working_directory, control_file_name, define1, define2, define3);

		if (!is_initialized_)
		{
			::LTDMConOutError("%s: Not initialized.", method_name);
			return LT_ERROR;
		}

		if (is_level_initialized_)
		{
			::LTDMConOutError("%s::InitLevel: Already initialized.", method_name);
			return LT_ERROR;
		}

		if (!working_directory)
		{
			::LTDMConOutError("%s::InitLevel: No working directory.", method_name);
			return LT_ERROR;
		}

		if (!control_file_name)
		{
			::LTDMConOutError("%s::InitLevel: No control file.", method_name);
			return LT_ERROR;
		}

		working_directory_ = working_directory;
		control_file_name_ = control_file_name;

		auto control_file_path = control_file_name_;

		if (!ul::PathUtils::has_any_separator(control_file_name))
		{
			control_file_path = ul::PathUtils::append(working_directory_, control_file_name_);
		}

#ifdef NOLITHTECH
		auto control_file = CControlFileMgrRezFile{m_sRezFileName};
#else
		auto control_file = CControlFileMgrDStream{};
#endif // NOLITHTECH

		control_file.AddDefine(define1);
		control_file.AddDefine(define2);
		control_file.AddDefine(define3);

		if (!control_file.Init(control_file_path.c_str()))
		{
			::LTDMConOutError("%s: Failed to open a control file.", method_name);
			return LT_ERROR;
		}

		auto guard_control_file = ul::ScopeGuard
		{
			[&]()
			{
				control_file.Term();
			}
		};

		intensity_count_ = 0;
		initial_intensity_ = 0;
		initial_volume_ = 0;
		volume_offset_ = 0;

		static_cast<void>(control_file.GetKeyVal(nullptr, "NUMINTENSITIES", intensity_count_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "INITIALINTENSITY", initial_intensity_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "INITIALVOLUME", initial_volume_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "VOLUMEOFFSET", volume_offset_));

		if (intensity_count_ <= 0 || intensity_count_ > max_intensity)
		{
			::LTDMConOutError("%s: Invalid intensity count: %d", method_name, intensity_count_);
			return LT_ERROR;
		}

		if (!read_intensities(control_file))
		{
			return LT_ERROR;
		}

		if (!read_transitions(control_file))
		{
			return LT_ERROR;
		}

		is_level_initialized_ = true;

		return LT_OK;
	}

	LTRESULT api_term_level()
	{
		const auto method_name = "DMusicManager::TermLevel";

		if (!is_initialized_)
		{
			::LTDMConOutError("%s: Not initialized.", method_name);
			return LT_ERROR;
		}

		if (!is_level_initialized_)
		{
			::LTDMConOutError("%s: Already terminated.", method_name);
			return LT_OK;
		}

		intensities_.clear();
		transition_map_.clear();

		working_directory_.clear();
		control_file_name_.clear();

		is_level_initialized_ = false;

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
	static constexpr auto max_intensity = 255;


	using Strings = std::vector<std::string>;


	struct Intensity
	{
		int number_;
		int loop_count_;
		int next_number_;
		Strings segments_names_;
	}; // Intensity

	using Intensities = std::vector<Intensity>;


	struct TransitionMapKey
	{
		static constexpr int encode(
			const int from_number,
			const int to_number)
		{
			return (1'000 * from_number) + to_number;
		}
	}; // TransitionMapKey

	using TransitionMap = std::unordered_map<int, std::string>;


	ILTSoundSys* sound_sys_;

	bool is_initialized_;
	bool is_level_initialized_;

	std::string working_directory_;
	std::string control_file_name_;

	int intensity_count_;
	int initial_intensity_;
	int initial_volume_;
	int volume_offset_;

	Intensities intensities_;
	TransitionMap transition_map_;


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


	bool read_intensities(
		CControlFileMgr& control_file)
	{
		intensities_.clear();
		intensities_.resize(intensity_count_ + 1);

		auto segments_names = Strings{};

		auto key_ptr = control_file.GetKey(nullptr, "INTENSITY");

		while (key_ptr)
		{
			// Intensity number.
			//
			auto word_ptr = key_ptr->GetFirstWord();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected intensity number.", "DMusicManager");
				return false;
			}

			auto intensity_number = 0;

			word_ptr->GetVal(intensity_number);

			if (intensity_number <= 0 || intensity_number > intensity_count_)
			{
				::LTDMConOutError("%s: Invalid intensity number: %d", "DMusicManager", intensity_number);
				return false;
			}


			// Loop count.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected loop count.", "DMusicManager");
				return false;
			}

			auto loop_count = -1;
			word_ptr->GetVal(loop_count);


			// Next intensity number.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected next intensity number.", "DMusicManager");
				return false;
			}

			auto next_intensity_number = 0;
			word_ptr->GetVal(next_intensity_number);


			// Segment name list.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected segment name list.", "DMusicManager");
				return false;
			}

			segments_names.clear();

			while (word_ptr)
			{
				const auto segment_name = word_ptr->GetVal();
				segments_names.emplace_back(segment_name);
				word_ptr = word_ptr->Next();
			}


			// Add intensity.
			//
			auto& intensity = intensities_[intensity_number];
			intensity.number_ = intensity_number;
			intensity.loop_count_ = loop_count;
			intensity.next_number_ = next_intensity_number;
			intensity.segments_names_ = segments_names;


			// Move to next key.
			//
			key_ptr = key_ptr->NextWithSameName();
		}

		return true;
	}

	bool read_transitions(
		CControlFileMgr& control_file)
	{
		transition_map_.clear();

		auto segment_name = std::string{};

		auto pKey = control_file.GetKey(nullptr, "TRANSITION");

		while (pKey)
		{
			// From number.
			//
			auto word_ptr = pKey->GetFirstWord();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected transition intensity number \"from\".", "DMusicManager");
				return false;
			}

			auto from_number = 0;
			word_ptr->GetVal(from_number);

			if (from_number <= 0 || from_number > intensity_count_)
			{
				::LTDMConOutError(
					"%s: Invalid transition intensity number \"from\": %d",
					"DMusicManager", from_number);

				return false;
			}


			// To number.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected transition intensity number \"to\".", "DMusicManager");
				return false;
			}

			auto to_number = 0;
			word_ptr->GetVal(to_number);

			if (to_number <= 0 || to_number > intensity_count_)
			{
				::LTDMConOutError(
					"%s: Invalid transition intensity number \"to\": %d",
					"DMusicManager", to_number);

				return false;
			}


			// When to enact.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected transition enact type.", "DMusicManager");
				return false;
			}

			const auto enact_string = word_ptr->GetVal();
			static_cast<void>(enact_string);


			// Transition type.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				::LTDMConOutError("%s: Expected transition type.", "DMusicManager");
				return false;
			}

			const auto transition_type_string = word_ptr->GetVal();
			static_cast<void>(transition_type_string);


			// Segment name.
			//
			segment_name.clear();

			word_ptr = word_ptr->Next();

			if (word_ptr)
			{
				segment_name = word_ptr->GetVal();
			}


			// Map transition.
			//
			const auto transition_key = TransitionMapKey::encode(from_number, to_number);

			transition_map_.emplace(transition_key, segment_name);


			// Move to next key.
			//
			pKey = pKey->NextWithSameName();
		}

		return true;
	}
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
