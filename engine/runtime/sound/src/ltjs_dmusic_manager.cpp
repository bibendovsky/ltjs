#include "bdefs.h"


#ifndef LTJS_USE_DIRECT_MUSIC8


#ifdef _DEBUG
#define LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS
#define LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2
#endif // _DEBUG


#include <cstdio>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_scope_guard.h"
#include "console.h"
#include "ltpvalue.h"
#include "soundmgr.h"
#include "ltdirectmusiccontrolfile.h"
#include "ltjs_audio_decoder.h"
#include "ltjs_dmusic_manager.h"
#include "ltjs_dmusic_segment.h"


#ifndef NOLITHTECH
extern int32 g_CV_LTDMConsoleOutput;
#else
extern signed int g_CV_LTDMConsoleOutput;
#endif // !NOLITHTECH


namespace ltjs
{


namespace ul = bibendovsky::spul;


define_interface(DMusicManager, ILTDirectMusicMgr)


class DMusicManager::Impl
{
public:
	Impl()
		:
		method_name_{},
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
		method_name_{std::move(that.method_name_)},
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
		log_info(3, "");

		if (is_initialized_)
		{
			log_error("Already initialized.");
			return LT_ERROR;
		}

		sound_sys_ = GetSoundSys();

		if (!sound_sys_)
		{
			log_error("No sound system.");
			return LT_ERROR;
		}

		ltjs::AudioDecoder::initialize_current_thread();

		is_initialized_ = true;

#ifdef LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS
		debug_test_all_musics();
#endif // LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS

		return LT_OK;
	}

	LTRESULT api_term()
	{
		log_info(3, "");

		if (!is_initialized_)
		{
			log_warning("Already uninitialized.");
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
		log_info(
			3,
			"working dir=%s control file=%s define1=%s define2=%s define3=%s",
			working_directory, control_file_name, define1, define2, define3);

		if (!is_initialized_)
		{
			log_error("Not initialized.");
			return LT_ERROR;
		}

		if (is_level_initialized_)
		{
			log_error("Already initialized.");
			return LT_ERROR;
		}

		if (!working_directory)
		{
			log_error("No working directory.");
			return LT_ERROR;
		}

		if (!control_file_name)
		{
			log_error("No control file.");
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
			log_error("Failed to open a control file.");
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
			log_error("Invalid intensity count: %d", intensity_count_);
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

		for (const auto& intensity : intensities_)
		{
			for (const auto& segment_name : intensity.segments_names_)
			{
				auto segment_path = ul::PathUtils::normalize(ul::PathUtils::append(working_directory_, segment_name));

				auto segment = DMusicSegment{};

				if (!segment.open(segment_path))
				{
					log_error("Failed to load a segment: \"%s\". %s",
						segment_name.c_str(),
						segment.get_error_message().c_str());

					return LT_ERROR;
				}
			}
		}

		is_level_initialized_ = true;

		return LT_OK;
	}

	LTRESULT api_term_level()
	{
		if (!is_initialized_)
		{
			log_error("Not initialized.");
			return LT_ERROR;
		}

		if (!is_level_initialized_)
		{
			log_error("Already terminated.");
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

		log_error(unsupported_method_message);

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

		log_error(unsupported_method_message);

		return LT_ERROR;
	}

	int api_get_cur_intensity()
	{
		return -1;
	}

	LTDMEnactTypes api_string_to_enact_type(
		const char* name)
	{
		log_error(unsupported_method_message);

		return LTDMEnactInvalid;
	}

	void api_enact_type_to_string(
		LTDMEnactTypes type,
		char* name)
	{
		log_error(unsupported_method_message);

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


	void set_method_name(
		const char* const method_name)
	{
		method_name_ = method_name;
	}


private:
	static constexpr auto max_intensity = 999;


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


	const char* method_name_;

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


	static const char* const unsupported_method_message;


	void log_message(
		const int level,
		const CONCOLOR color,
		const char* const message,
		va_list args_ptr)
	{
		if (!message || ::g_CV_LTDMConsoleOutput < level)
		{
			return;
		}

		constexpr auto max_message_size = 1024;

		char dst_message[max_message_size];
		*dst_message = '\0';

		static_cast<void>(::strcat(dst_message, "DMusicManager"));

		if (method_name_)
		{
			static_cast<void>(::strcat(dst_message, "::"));
			static_cast<void>(::strcat(dst_message, method_name_));
		}

		static_cast<void>(::strcat(dst_message, ": "));

		const auto prefix_length = ::strlen(dst_message);

		const auto written_size = ::vsnprintf(dst_message + prefix_length, max_message_size - prefix_length, message, args_ptr);

		if (written_size <= 0)
		{
			return;
		}

#ifndef NOLITHTECH
		const auto dst_message_length = ::strlen(dst_message);

		if (dst_message[dst_message_length] == '\n')
		{
			dst_message[dst_message_length] = '\0';
		}

		::con_PrintString(color, 0, dst_message);
#else
		::DebugConsoleOutput(dst_message, ::PValue_GetR(color), ::PValue_GetG(color), ::PValue_GetB(color));
#endif // !NOLITHTECH
	}

	void log_error(
		const char* const message,
		...)
	{
#ifndef NOLITHTECH
		const auto color = CONRGB(255, 0, 128);
#else
		const auto color = CONRGB(255, 0, 0);
#endif // !NOLITHTECH

		va_list marker;
		va_start(marker, message);
		log_message(0, color, message, marker);
		va_end(marker);
	}

	void log_warning(
		const char* const message,
		...)
	{
#ifndef NOLITHTECH
		const auto color = CONRGB(0, 255, 128);
#else
		const auto color = CONRGB(0, 255, 0);
#endif // !NOLITHTECH

		va_list marker;
		va_start(marker, message);
		log_message(2, color, message, marker);
		va_end(marker);
	}

	void log_info(
		const int level,
		const char* const message,
		...)
	{
#ifndef NOLITHTECH
		const auto color = CONRGB(128, 255, 128);
#else
		const auto color = CONRGB(0, 0, 0);
#endif // !NOLITHTECH

		va_list marker;
		va_start(marker, message);
		log_message(level, color, message, marker);
		va_end(marker);
	}


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
				log_error("Expected intensity number.");
				return false;
			}

			auto intensity_number = 0;

			if (!word_ptr->GetVal(intensity_number))
			{
				log_error("Failed to read an intensity number.");
				return false;
			}

			if (intensity_number <= 0 || intensity_number > intensity_count_)
			{
				log_error("Invalid intensity number: %d", intensity_number);
				return false;
			}


			// Loop count.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				log_error("Expected loop count.");
				return false;
			}

			auto loop_count = 0;

			if (!word_ptr->GetVal(loop_count))
			{
				log_error("Failed to read a loop count.");
				return false;
			}

			if (loop_count != 0)
			{
				log_error("Expected zero loop count.");
				return false;
			}


			// Next intensity number.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				log_error("Expected next intensity number.");
				return false;
			}

			auto next_intensity_number = 0;

			if (!word_ptr->GetVal(next_intensity_number))
			{
				log_error("Failed to read a next intensity number.");
				return false;
			}


			// Segment name list.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				log_error("Expected segment name list.");
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
				log_error("Expected transition intensity number \"from\".");
				return false;
			}

			auto from_number = 0;

			if (!word_ptr->GetVal(from_number))
			{
				log_error("Failed to read a transition intensity number.");
				return false;
			}

			if (from_number <= 0 || from_number > intensity_count_)
			{
				log_error("Invalid transition intensity number \"from\": %d", from_number);
				return false;
			}


			// To number.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				log_error("Expected transition intensity number \"to\".");
				return false;
			}

			auto to_number = 0;

			if (!word_ptr->GetVal(to_number))
			{
				log_error("Failed to read a transition intensity number.");
				return false;
			}

			if (to_number <= 0 || to_number > intensity_count_)
			{
				log_error("Invalid transition intensity number \"to\": %d", to_number);
				return false;
			}


			// When to enact.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				log_error("Expected transition enact type.");
				return false;
			}

			const auto enact_string = std::string{word_ptr->GetVal()};

			if (enact_string != "MEASURE")
			{
				log_error("Expected MEASURE transition enact type.");
				return false;
			}


			// Transition type.
			//
			word_ptr = word_ptr->Next();

			if (!word_ptr)
			{
				log_error("Expected transition type.");
				return false;
			}

			const auto transition_type_string = std::string{word_ptr->GetVal()};

			if (transition_type_string != "MANUAL")
			{
				log_error("Expected MANUAL transition type.");
				return false;
			}


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


	// ======================================================================
	// Debug stuff
	//

#ifdef LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS
	using DebugControlFilePaths = std::vector<std::string>;

	void debug_test_all_musics(
		const DebugControlFilePaths& control_file_names)
	{
		for (auto control_file_path : control_file_names)
		{
			const auto working_directory = ul::PathUtils::get_parent_path(control_file_path);
			const auto control_file_name = ul::PathUtils::get_file_name(control_file_path);

			const auto init_result = api_init_level(
				working_directory.c_str(),
				control_file_name.c_str(),
				nullptr,
				nullptr,
				nullptr);

			auto is_failed = false;

			if (init_result == LT_OK)
			{
				static_cast<void>(api_term_level());
			}
			else
			{
				is_failed = true;
			}
		}
	}
#endif // LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS

#ifdef LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2
	static const DebugControlFilePaths& debug_get_control_file_paths_nolf2()
	{
		static const DebugControlFilePaths nolf2_music_control_file_paths =
		{
			"Music\\Cinematics\\Cinematics.txt",
			"Music\\India\\India.txt",
			"Music\\India\\Low\\India.txt",
			"Music\\Island\\Island.txt",
			"Music\\Island\\IslandL.txt",
			"Music\\Island\\Low\\Island.txt",
			"Music\\Island\\Low\\IslandL.txt",
			"Music\\Japan\\Japan.txt",
			"Music\\Japan\\Low\\Japan.txt",
			"Music\\Japan\\Low\\Melvin.txt",
			"Music\\Japan\\Low\\Ohio.txt",
			"Music\\Japan\\Melvin.txt",
			"Music\\Japan\\Ohio.txt",
			"Music\\Siberia\\Low\\Siberia.txt",
			"Music\\Siberia\\Siberia.txt",
			"Music\\Underwater\\Antarctica.txt",
			"Music\\Underwater\\Low\\Antarctica.txt",
			"Music\\Underwater\\Low\\Underwater.txt",
			"Music\\Underwater\\Underwater.txt",
			"Music\\Unity\\Bicycle.txt",
			"Music\\Unity\\Low\\Bicycle.txt",
			"Music\\Unity\\Low\\Unity.txt",
			"Music\\Unity\\Unity.txt",
		}; // nolf2_music_control_file_paths

		return nolf2_music_control_file_paths;
	}
#endif // LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2

#ifdef LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS
	void debug_test_all_musics()
	{
		log_info(3, "DEBUG: BEGIN Testing all musics.");

#ifdef LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2
		debug_test_all_musics(debug_get_control_file_paths_nolf2());
#endif // LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2

		log_info(3, "DEBUG: END Testing all musics.");
	}
#endif // LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS

	//
	// Debug stuff
	// ======================================================================
}; // DMusicManager::Impl


const char* const DMusicManager::Impl::unsupported_method_message = "Unsupported method.";


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
	pimpl_->set_method_name(__func__);
	return pimpl_->api_init();
}

LTRESULT DMusicManager::Term()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_term();
}

LTRESULT DMusicManager::InitLevel(
	const char* working_directory,
	const char* control_file_name,
	const char* define1,
	const char* define2,
	const char* define3)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_init_level(working_directory, control_file_name, define1, define2, define3);
}

LTRESULT DMusicManager::TermLevel()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_term_level();
}

LTRESULT DMusicManager::Play()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_play();
}

LTRESULT DMusicManager::Stop(
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_stop(start_type);
}

LTRESULT DMusicManager::Pause(
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_pause(start_type);
}

LTRESULT DMusicManager::UnPause()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_unpause();
}

LTRESULT DMusicManager::SetVolume(
	const long volume)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_set_volume(volume);
}

LTRESULT DMusicManager::ChangeIntensity(
	const int new_intensity,
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_change_intensity(new_intensity, start_type);
}

LTRESULT DMusicManager::PlaySecondary(
	const char* segment_name,
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_play_secondary(segment_name, start_type);
}

LTRESULT DMusicManager::StopSecondary(
	const char* segment_name,
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_stop_secondary(segment_name, start_type);
}

LTRESULT DMusicManager::PlayMotif(
	const char* style_name,
	const char* motif_name,
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_play_motif(style_name, motif_name, start_type);
}

LTRESULT DMusicManager::StopMotif(
	const char* style_name,
	const char* motif_name,
	const LTDMEnactTypes start_type)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_stop_motif(style_name, motif_name, start_type);
}

int DMusicManager::GetCurIntensity()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_get_cur_intensity();
}

LTDMEnactTypes DMusicManager::StringToEnactType(
	const char* name)
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_string_to_enact_type(name);
}

void DMusicManager::EnactTypeToString(
	LTDMEnactTypes type,
	char* name)
{
	pimpl_->set_method_name(__func__);
	pimpl_->api_enact_type_to_string(type, name);
}

int DMusicManager::GetNumIntensities()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_get_num_intensities();
}

int DMusicManager::GetInitialIntensity()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_get_initial_intensity();
}

int DMusicManager::GetInitialVolume()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_get_initial_volume();
}

int DMusicManager::GetVolumeOffset()
{
	pimpl_->set_method_name(__func__);
	return pimpl_->api_get_volume_offset();
}


} // ltjs


#endif LTJS_USE_DIRECT_MUSIC8
