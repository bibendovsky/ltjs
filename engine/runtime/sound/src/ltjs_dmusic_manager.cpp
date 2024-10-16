#include "bdefs.h"


#ifndef LTJS_USE_DIRECT_MUSIC8


#ifdef _DEBUG
//#define LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS
//#define LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2

//#define LTJS_DEBUG_DMUSIC_DUMP_CUSTOM
//#define LTJS_DEBUG_DMUSIC_DUMP_CUSTOM_NOLF2
#endif // _DEBUG


#include <cstdio>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_ascii_utils.h"
#include "bibendovsky_spul_endian.h"
#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_path_utils.h"
#include "bibendovsky_spul_riff_four_ccs.h"
#include "bibendovsky_spul_scope_guard.h"
#include "bibendovsky_spul_wave_format_utils.h"
#include "bibendovsky_spul_wave_four_ccs.h"
#include "console.h"
#include "ltpvalue.h"
#include "soundmgr.h"
#include "ltdirectmusiccontrolfile.h"
#include "ltjs_audio_decoder.h"
#include "ltjs_dmusic_manager.h"
#include "ltjs_dmusic_segment.h"
#include "ltjs_audio_utils.h"


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
		volume_offset_{},
		sample_rate_{},
		intensities_{},
		transition_map_{},
		segment_cache_{},
		mix_offset_{},
		mix_sample_count_{},
		mix_s16_size_{},
		mix_f_size_{},
		enforced_intensity_index_{},
		current_intensity_index_{},
		current_segment_index_{},
		current_segment_{},
		transition_dm_segment_{},
		waves_{},
		decoder_buffer_{},
		device_buffer_{},
		mix_buffer_{},
		music_stream_{},
		mt_mixer_thread_{},
		mt_mixer_cv_{},
		mt_mixer_cv_mutex_{},
		mt_mixer_mutex_{},
		mt_mixer_cv_flag_{},
		mt_is_quit_mixer_{}
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
		volume_offset_{std::move(that.volume_offset_)},
		sample_rate_{std::move(that.sample_rate_)},
		intensities_{std::move(that.intensities_)},
		transition_map_{std::move(that.transition_map_)},
		segment_cache_{std::move(that.segment_cache_)},
		mix_offset_{std::move(that.mix_offset_)},
		mix_sample_count_{std::move(that.mix_sample_count_)},
		mix_s16_size_{std::move(that.mix_s16_size_)},
		mix_f_size_{std::move(that.mix_f_size_)},
		enforced_intensity_index_{std::move(that.enforced_intensity_index_)},
		current_intensity_index_{std::move(that.current_intensity_index_)},
		current_segment_index_{std::move(that.current_segment_index_)},
		current_segment_{std::move(that.current_segment_)},
		transition_dm_segment_{std::move(that.transition_dm_segment_)},
		waves_{std::move(that.waves_)},
		decoder_buffer_{std::move(that.decoder_buffer_)},
		device_buffer_{std::move(that.device_buffer_)},
		mix_buffer_{std::move(that.mix_buffer_)},
		music_stream_{std::move(that.music_stream_)},
		mt_mixer_thread_{std::move(that.mt_mixer_thread_)},
		mt_mixer_cv_{},
		mt_mixer_cv_mutex_{},
		mt_mixer_mutex_{},
		mt_mixer_cv_flag_{std::move(that.mt_mixer_cv_flag_)},
		mt_is_quit_mixer_{std::move(that.mt_is_quit_mixer_)}
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
		sample_rate_ = 0;

		static_cast<void>(control_file.GetKeyVal(nullptr, "NUMINTENSITIES", intensity_count_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "INITIALINTENSITY", initial_intensity_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "INITIALVOLUME", initial_volume_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "VOLUMEOFFSET", volume_offset_));
		static_cast<void>(control_file.GetKeyVal(nullptr, "SYNTHSAMPLERATE", sample_rate_));

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

		// Load all segments for intensities.
		//
		for (auto& intensity : intensities_)
		{
			const auto n_segments = static_cast<int>(intensity.segments_names_.size());

			intensity.segments_.resize(n_segments);

			for (auto i_segment = 0; i_segment < n_segments; ++i_segment)
			{
				const auto& segment_name = intensity.segments_names_[i_segment];

				auto segment = cache_segment(segment_name);

				if (!segment)
				{
					return LT_ERROR;
				}

				intensity.segments_[i_segment] = segment;
			}
		}

		// Load all intensities for transitions.
		//
		for (auto& transition_item : transition_map_)
		{
			auto& transition_value = transition_item.second;

			if (transition_value.segment_name_.empty())
			{
				continue;
			}

			const auto segment_name = transition_value.segment_name_;

			auto segment = cache_segment(segment_name);

			if (!segment)
			{
				return LT_ERROR;
			}

			transition_item.second.segment_ = segment;
		}

		// Initialize a music stream.
		//
		const auto mix_sample_size = byte_depth * channel_count;

		mix_s16_size_ = (mix_sample_size * sample_rate_ * mix_size_ms) / 1000;
		mix_s16_size_ += mix_sample_size - 1;
		mix_s16_size_ /= mix_sample_size;
		mix_s16_size_ *= mix_sample_size;

		mix_sample_count_ = mix_s16_size_ / byte_depth;
		mix_f_size_ = mix_sample_count_ * byte_depth_f;

		music_stream_ = sound_sys_->ltjs_open_generic_stream(sample_rate_, mix_s16_size_);

		if (!music_stream_)
		{
			log_error("Failed to initialize a music stream.");
			return LT_ERROR;
		}

		// Initialize misc fields.
		//
		mix_offset_ = 0;
		current_intensity_index_ = -1;
		current_segment_index_ = 0;
		current_segment_ = {};
		transition_dm_segment_ = nullptr;
		waves_.clear();
		decoder_buffer_.resize(mix_sample_count_);
		device_buffer_.resize(mix_sample_count_);
		mix_buffer_.resize(mix_sample_count_);

#ifdef LTJS_DEBUG_DMUSIC_DUMP_CUSTOM
		dbg_002_is_active_ = false;
#endif // LTJS_DEBUG_DMUSIC_DUMP_CUSTOM

		// Multi-threading stuff.
		//
		mt_mixer_cv_flag_ = false;
		mt_is_quit_mixer_ = false;
		mt_mixer_thread_ = MtThread{std::bind(&Impl::mt_mixer_worker, this)};

		is_level_initialized_ = true;
		music_stream_->set_volume(AudioUtils::max_generic_stream_level_mb);

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

		if (mt_mixer_thread_.joinable())
		{
			mt_is_quit_mixer_ = true;
			mt_notify_mixer();

			mt_mixer_thread_.join();
		}

		if (music_stream_)
		{
			sound_sys_->ltjs_close_generic_stream(music_stream_);
			music_stream_ = nullptr;
		}

		working_directory_.clear();
		control_file_name_.clear();

		intensity_count_ = 0;
		initial_intensity_ = 0;
		initial_volume_ = 0;
		volume_offset_ = 0;
		sample_rate_ = 0;

		intensities_.clear();
		transition_map_.clear();
		segment_cache_.clear();

		mix_sample_count_ = 0;
		mix_s16_size_ = 0;
		mix_f_size_ = 0;
		mix_offset_ = 0;

		enforced_intensity_index_ = -1;
		current_intensity_index_ = -1;
		current_segment_index_ = -1;

		current_segment_ = {};
		transition_dm_segment_ = nullptr;

		waves_.clear();

		decoder_buffer_.clear();
		device_buffer_.clear();
		mix_buffer_.clear();

		is_level_initialized_ = false;

		return LT_OK;
	}

	LTRESULT api_play()
	{
		if (!is_level_initialized_)
		{
			return LT_ERROR;
		}

		{
			MtLockGuard mixer_lock{mt_mixer_mutex_};

			enforced_intensity_index_ = initial_intensity_;
		}

		if (!music_stream_->set_pause(false))
		{
			return LT_ERROR;
		}

		mt_notify_mixer();

		return LT_OK;
	}

	LTRESULT api_stop(
		const LTDMEnactTypes start_type)
	{
		if (!is_level_initialized_)
		{
			return LT_ERROR;
		}

		if (!music_stream_->set_pause(true))
		{
			return LT_ERROR;
		}

		{
			MtLockGuard mixer_lock{mt_mixer_mutex_};

			enforced_intensity_index_ = initial_intensity_;
		}

		mt_notify_mixer();

		return LT_OK;
	}

	LTRESULT api_pause(
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(start_type);

		if (!is_level_initialized_)
		{
			return LT_ERROR;
		}

		if (!music_stream_->set_pause(true))
		{
			return LT_ERROR;
		}

		mt_notify_mixer();

		return LT_OK;
	}

	LTRESULT api_unpause()
	{
		if (!is_level_initialized_)
		{
			return LT_ERROR;
		}

		if (!music_stream_->set_pause(false))
		{
			return LT_ERROR;
		}

		mt_notify_mixer();

		return LT_OK;
	}

	LTRESULT api_set_volume(
		const long volume)
	{
		if (!is_level_initialized_)
		{
			return LT_ERROR;
		}

		if (!music_stream_->set_volume(volume))
		{
			return LT_ERROR;
		}

		return LT_OK;
	}

	LTRESULT api_change_intensity(
		const int new_intensity,
		const LTDMEnactTypes start_type)
	{
		static_cast<void>(start_type);

		if (!is_level_initialized_)
		{
			return LT_ERROR;
		}

		{
			MtLockGuard mixer_lock{mt_mixer_mutex_};

			if (new_intensity == current_intensity_index_)
			{
				return LT_OK;
			}

			enforced_intensity_index_ = new_intensity;
		}

		mt_notify_mixer();

		return LT_OK;
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
		if (!is_level_initialized_)
		{
			return -1;
		}

		MtLockGuard mixer_lock{mt_mixer_mutex_};

		return current_intensity_index_;
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
		if (!is_level_initialized_)
		{
			return 0;
		}

		return static_cast<int>(intensities_.size());
	}

	int api_get_initial_intensity()
	{
		if (!is_level_initialized_)
		{
			return -1;
		}

		return initial_intensity_;
	}

	int api_get_initial_volume()
	{
		if (!is_level_initialized_)
		{
			return 0;
		}

		return initial_volume_;
	}

	int api_get_volume_offset()
	{
		if (!is_level_initialized_)
		{
			return 0;
		}

		return volume_offset_;
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
	static constexpr auto channel_count = 2;
	static constexpr auto bit_depth = 16;
	static constexpr auto byte_depth = bit_depth / 8;
	static constexpr auto byte_depth_f = static_cast<int>(sizeof(float));
	static constexpr auto sample_size = channel_count * byte_depth;

	// Size of decoder buffer, mix buffer and device buffer in milliseconds.
	static constexpr auto mix_size_ms = 50;

	static constexpr auto max_intensity = 999;


	using MtThread = std::thread;
	using MtCondVar = std::condition_variable;
	using MtMutex = std::mutex;
	using MtLockGuard = std::lock_guard<MtMutex>;
	using MtUniqueLock = std::unique_lock<MtMutex>;

	using Strings = std::vector<std::string>;
	using SegmentPtrList = std::vector<DMusicSegment*>;


	struct Intensity
	{
		int number_;
		int loop_count_;
		int next_number_;
		Strings segments_names_;
		SegmentPtrList segments_;
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

	struct TransitionMapValue
	{
		std::string segment_name_;
		DMusicSegment* segment_;


		TransitionMapValue()
			:
			segment_name_{},
			segment_{}
		{
		}
	}; // TransitionMapValue

	using TransitionMap = std::unordered_map<int, TransitionMapValue>;

	using SegmentCache = std::unordered_map<std::string, DMusicSegment>;


	struct Wave
	{
		bool is_active_;
		bool is_started_;
		bool is_finished_;
		std::uint32_t channel_;
		int decoded_offset_; // (in bytes)
		int length_; // (in bytes)
		std::int64_t mix_offset_; // (in bytes)
		ul::MemoryStream stream_;
		ltjs::AudioDecoder decoder_;


		Wave()
			:
			is_active_{},
			is_started_{},
			is_finished_{},
			channel_{},
			decoded_offset_{},
			length_{},
			mix_offset_{},
			stream_{},
			decoder_{}
		{
		}
	}; // Wave

	struct Segment
	{
		std::int64_t begin_mix_offset_;
		std::int64_t end_mix_offset_;


		Segment()
			:
			begin_mix_offset_{},
			end_mix_offset_{}
		{
		}
	}; // Segment

	using Waves = std::list<Wave>;

	using BufferS16 = std::vector<std::int16_t>;
	using BufferF = std::vector<float>;


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
	int sample_rate_;

	Intensities intensities_;
	TransitionMap transition_map_;
	SegmentCache segment_cache_;

	int mix_sample_count_;
	int mix_s16_size_;
	int mix_f_size_;
	std::int64_t mix_offset_;

	// -1 - don't enforce.
	// 0 - don't queue any items anymore.
	// 1..n - set a new intensity index.
	int enforced_intensity_index_;

	// -1 - fresh start.
	// 0 - don't queue any items anymore.
	// 1..n - the intensity index.
	int current_intensity_index_;

	// -1 - select first.
	// 0..n - select next.
	int current_segment_index_;

	Segment current_segment_;
	DMusicSegment* transition_dm_segment_;

	Waves waves_;

	BufferS16 decoder_buffer_;
	BufferS16 device_buffer_;
	BufferF mix_buffer_;
	LtjsLtSoundSysGenericStream* music_stream_;
	MtThread mt_mixer_thread_;
	MtCondVar mt_mixer_cv_;
	MtMutex mt_mixer_cv_mutex_;
	MtMutex mt_mixer_mutex_;
	bool mt_mixer_cv_flag_;
	bool mt_is_quit_mixer_;


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

			auto transition_value = TransitionMapValue{};
			transition_value.segment_name_ = segment_name;

			transition_map_.emplace(transition_key, std::move(transition_value));


			// Move to next key.
			//
			pKey = pKey->NextWithSameName();
		}

		return true;
	}

	DMusicSegment* cache_segment(
		const std::string& segment_name)
	{
		const auto segment_name_lc = ul::AsciiUtils::to_lower(segment_name);

		auto found_segment_it = segment_cache_.find(segment_name_lc);

		if (found_segment_it != segment_cache_.cend())
		{
			return &found_segment_it->second;
		}

		auto segment = DMusicSegment{};
		auto segment_path = ul::PathUtils::normalize(ul::PathUtils::append(working_directory_, segment_name));

		if (!segment.open(segment_path, sample_rate_))
		{
			log_error("Failed to load a segment: \"%s\". %s",
				segment_name.c_str(),
				segment.get_error_message().c_str());

			return nullptr;
		}

		auto cache_item_it = segment_cache_.emplace(segment_name_lc, std::move(segment)).first;

		return &cache_item_it->second;
	}

	bool mix_prepare_segments()
	{
		MtLockGuard lock{mt_mixer_mutex_};

		auto mix_delta_offset = 0;

		while (true)
		{
			if (current_intensity_index_ < 0)
			{
				// Fresh start.
				//

				select_next_segment(mix_delta_offset);
				continue;
			}

			if (enforced_intensity_index_ >= 0)
			{
				current_intensity_index_ = enforced_intensity_index_;
				enforced_intensity_index_ = -1;

				waves_.remove_if(
					[&](const auto& item)
					{
						return item.mix_offset_ > (mix_offset_ + mix_s16_size_);
					}
				);
			}

			if (current_intensity_index_ == 0)
			{
				// No more segments.
				// Just playback queued items.
				//
				break;
			}

			if (current_segment_.end_mix_offset_ >= mix_offset_ &&
				current_segment_.end_mix_offset_ < (mix_offset_ + mix_s16_size_))
			{
				// The current segment is ended in this mixing window.
				// Select the next segment, queue items and etc.
				//
				mix_delta_offset = static_cast<int>(current_segment_.end_mix_offset_ - mix_offset_);
				select_next_segment(mix_delta_offset);
			}
			else
			{
				break;
			}
		}

		return current_intensity_index_ > 0;
	}

	//
	// Mixes the segments.
	//
	// Returns:
	//    - "true" - if there is more data available.
	//    - "false" - this is a last data block.
	//
	bool mix()
	{
		std::uninitialized_fill(mix_buffer_.begin(), mix_buffer_.end(), 0.0F);

		if (!mix_prepare_segments() && waves_.empty())
		{
			// No more segments and no queued items left.
			//
			return false;
		}

		for (auto& wave : waves_)
		{
			if (!wave.is_active_)
			{
				// Try to active the item.
				//
				if (wave.mix_offset_ >= mix_offset_ && wave.mix_offset_ < (mix_offset_ + mix_s16_size_))
				{
					// Item's mix offset inside the current mixing window. Activate it.
					//
					wave.is_active_ = true;
				}
			}

			if (!wave.is_active_)
			{
				continue;
			}

			auto mix_byte_offset = 0;

			if (!wave.is_started_)
			{
				// Initialize wave decoding.
				//
				wave.is_started_ = true;

				// Adjust output index for mix buffer.
				//
				if (wave.mix_offset_ > mix_offset_)
				{
					mix_byte_offset = static_cast<int>(wave.mix_offset_ - mix_offset_);
				}

				auto decoder_param = ltjs::AudioDecoder::OpenParam{};
				decoder_param.dst_bit_depth_ = bit_depth;
				decoder_param.dst_channel_count_ = channel_count;
				decoder_param.dst_sample_rate_ = sample_rate_;
				decoder_param.stream_ptr_ = &wave.stream_;

				if (!wave.decoder_.open(decoder_param))
				{
					wave.is_finished_ = true;
					continue;
				}
			}

			auto remain_size = wave.length_ - wave.decoded_offset_;

			if (remain_size > (mix_s16_size_ - mix_byte_offset))
			{
				remain_size = mix_s16_size_ - mix_byte_offset;
			}

			// Always align the size by the byte depth.
			//
			remain_size /= byte_depth;
			remain_size *= byte_depth;

			const auto decoded_size = wave.decoder_.decode(decoder_buffer_.data(), remain_size);

			if (decoded_size > 0)
			{
				const auto sample_count = decoded_size / byte_depth;
				const auto adjusted_decoded_size = sample_count * byte_depth;
				const auto mix_sample_offset = mix_byte_offset / byte_depth;

				for (auto i = 0; i < sample_count; ++i)
				{
					mix_buffer_[i + mix_sample_offset] += decoder_buffer_[i] / 32768.0F;
				}

				wave.decoded_offset_ += adjusted_decoded_size;

				if (wave.decoded_offset_ == wave.length_)
				{
					wave.is_finished_ = true;
				}
			}
			else
			{
				wave.is_finished_ = true;
			}
		}

		// Remove finished waves.
		//
		waves_.remove_if(
			[](const auto& item)
			{
				return item.is_finished_;
			}
		);

		mix_offset_ += mix_s16_size_;

		return true;
	}

	//
	// Queues wave items.
	//
	void queue_waves(
		const int mix_offset_delta)
	{
		if (current_intensity_index_ <= 0)
		{
			return;
		}

		auto dm_segment = transition_dm_segment_;

		if (!dm_segment)
		{
			dm_segment = intensities_[current_intensity_index_].segments_[current_segment_index_];
		}

		const auto channel = dm_segment->get_channel();
		const auto variation = dm_segment->select_next_variation();
		const auto& segment_waves = dm_segment->get_waves();

		current_segment_.begin_mix_offset_ = mix_offset_ + mix_offset_delta;
		current_segment_.end_mix_offset_ = current_segment_.begin_mix_offset_ + dm_segment->get_length();

		for (const auto& segment_wave : segment_waves)
		{
			if ((segment_wave.variations_ & variation) == 0)
			{
				// Wave does not belong to the currently selected variation.
				//
				continue;
			}

			auto wave = Wave{};
			wave.length_ = segment_wave.length_;
			wave.channel_ = channel;
			wave.mix_offset_ = current_segment_.begin_mix_offset_ + segment_wave.mix_offset_;

			if (!wave.stream_.open(segment_wave.data_, segment_wave.data_size_))
			{
				continue;
			}

			waves_.emplace_back(std::move(wave));
		}
	}

	//
	// Selects the next segment.
	//
	void select_next_segment(
		const int mix_delta_offset)
	{
#ifdef LTJS_DEBUG_DMUSIC_DUMP_CUSTOM
		const auto& initial_intensity_index = (dbg_002_is_active_ ? dbg_002_initial_intensity_ : initial_intensity_);
		auto& intensities = (dbg_002_is_active_ ? dbg_002_intensities_ : intensities_);
#else
		auto& initial_intensity_index = initial_intensity_;
		auto& intensities = intensities_;
#endif // LTJS_DEBUG_DMUSIC_DUMP_CUSTOM

		if (current_intensity_index_ == 0)
		{
			// No more segments.
			//
			return;
		}

		if (current_intensity_index_ < 0)
		{
			// Fresh start.
			//

			mix_offset_ = 0;

			waves_.clear();

			current_intensity_index_ = initial_intensity_index;
			current_segment_index_ = 0;
			transition_dm_segment_ = nullptr;
		}
		else if (transition_dm_segment_)
		{
			// From a transition segment to the next intensity.
			//

			current_intensity_index_ = intensities[current_intensity_index_].next_number_;
			current_segment_index_ = 0;
			transition_dm_segment_ = nullptr;
		}
		else
		{
			// From a current intensity to the next segment or the next intensity.
			//

			const auto& intensity = intensities[current_intensity_index_];

			const auto& segments = intensity.segments_;
			const auto segment_count = static_cast<int>(segments.size());

			if (current_segment_index_ < 0)
			{
				current_segment_index_ = 0;
			}
			else if (current_segment_index_ >= (segment_count - 1))
			{
				// The last segment.
				//

				// Check for a transition segment.
				//
				const auto transition_key = TransitionMapKey::encode(intensity.number_, intensity.next_number_);

				auto map_it = transition_map_.find(transition_key);

				if (map_it != transition_map_.cend() && map_it->second.segment_)
				{
					// Has transition segment. Use it.
					//
					transition_dm_segment_ = map_it->second.segment_;
				}
				else
				{
					// Does not have transition segment. Move to the next intensity.
					//

					current_intensity_index_ = intensities[current_intensity_index_].next_number_;
					current_segment_index_ = 0;
				}
			}
			else
			{
				// Not the last segment. Move to the next one.
				//

				current_segment_index_ += 1;
			}
		}

		// Finally, queue the items of the current segment.
		//
		queue_waves(mix_delta_offset);
	}

	void mt_notify_mixer()
	{
		MtUniqueLock cv_lock{mt_mixer_cv_mutex_};
		mt_mixer_cv_flag_ = true;
		mt_mixer_cv_.notify_one();
	}

	void mt_wait_for_mixer_cv()
	{
		MtUniqueLock cv_lock{mt_mixer_cv_mutex_};
		mt_mixer_cv_.wait(cv_lock, [&](){ return mt_mixer_cv_flag_; });
		mt_mixer_cv_flag_ = false;
	}

	void update_device_buffer()
	{
		auto min_amplitude = -1.0F;
		auto max_amplitude = 1.0F;

		for (auto i = 0; i < mix_sample_count_; ++i)
		{
			if (mix_buffer_[i] < min_amplitude)
			{
				min_amplitude = mix_buffer_[i];
			}

			if (mix_buffer_[i] > max_amplitude)
			{
				max_amplitude = mix_buffer_[i];
			}
		}

		auto scale = 32768.0F;

		if (min_amplitude != -1.0F || max_amplitude != 1.0F)
		{
			scale /= max_amplitude - min_amplitude;
		}

		for (auto i = 0; i < mix_sample_count_; ++i)
		{
			const auto sample = ul::Algorithm::clamp(
				static_cast<int>(mix_buffer_[i] * scale), -32768, 32767);

			device_buffer_[i] = static_cast<std::int16_t>(sample);
		}
	}

	void mt_mixer_worker()
	{
		const auto delay = std::chrono::milliseconds{25};

		while (!mt_is_quit_mixer_)
		{
			auto is_delay = true;

			if (!music_stream_->get_pause())
			{
				const auto free_buffer_count = music_stream_->get_free_buffer_count();

				if (free_buffer_count > 0)
				{
					for (auto i = 0; i < free_buffer_count; ++i)
					{
						const auto is_last = !mix();

						update_device_buffer();

						if (!music_stream_->enqueue_buffer(device_buffer_.data()))
						{
							return;
						}

						if (is_last)
						{
							is_delay = false;
							break;
						}
					}
				}
			}
			else
			{
				is_delay = false;
			}

			if (is_delay)
			{
				std::this_thread::sleep_for(delay);
			}
			else
			{
				mt_wait_for_mixer_cv();
			}
		}
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
#ifdef LTJS_DEBUG_DMUSIC_DUMP_CUSTOM
				debug_dump_custom(working_directory, control_file_name);
#endif // LTJS_DEBUG_DMUSIC_DUMP_CUSTOM

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


#ifdef LTJS_DEBUG_DMUSIC_DUMP_CUSTOM
	bool dbg_002_is_active_;
	int dbg_002_initial_intensity_;
	Intensities dbg_002_intensities_;

	void debug_dump_custom(
		const std::string& working_directory,
		const std::string& control_file_name)
	{
#ifdef LTJS_DEBUG_DMUSIC_DUMP_CUSTOM_NOLF2
		debug_dump_custom_nolf2(working_directory, control_file_name);
#endif // LTJS_DEBUG_DMUSIC_DUMP_CUSTOM_NOLF2
	}
#endif // LTJS_DEBUG_DMUSIC_DUMP_CUSTOM

#ifdef LTJS_DEBUG_DMUSIC_DUMP_CUSTOM_NOLF2
	void debug_dump_custom_nolf2(
		const std::string& working_directory,
		const std::string& control_file_name)
	{
		// TODO variations?

		const auto control_file_name_lc = ul::AsciiUtils::to_lower(ul::PathUtils::append(
			working_directory, control_file_name));

		if (false)
		{
		}
		else if (control_file_name_lc == "music\\island\\island.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[2].next_number_ = 0; // explore (main menu)
		}
		else if (control_file_name_lc == "music\\island\\islandl.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[16].next_number_ = 21; // explore
			dbg_002_intensities_[30].next_number_ = 31; // warning
			dbg_002_intensities_[44].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\india\\india.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[4].next_number_ = 5; // explore
			dbg_002_intensities_[16].next_number_ = 17; // warning
			dbg_002_intensities_[30].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\japan\\japan.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[12].next_number_ = 13; // explore
			dbg_002_intensities_[22].next_number_ = 23; // warning
			dbg_002_intensities_[40].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\japan\\melvin.txt")
		{
			dbg_002_initial_intensity_ = 41;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[55].next_number_ = 13; // explore
			dbg_002_intensities_[22].next_number_ = 23; // warning
			dbg_002_intensities_[40].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\japan\\ohio.txt")
		{
			dbg_002_initial_intensity_ = 41;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[55].next_number_ = 13; // explore
			dbg_002_intensities_[22].next_number_ = 23; // warning
			dbg_002_intensities_[40].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\siberia\\siberia.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[27].next_number_ = 31; // explore
			dbg_002_intensities_[49].next_number_ = 51; // warning
			dbg_002_intensities_[72].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\underwater\\antarctica.txt")
		{
			dbg_002_initial_intensity_ = 51;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[63].next_number_ = 10; // explore
			dbg_002_intensities_[30].next_number_ = 31; // warning
			dbg_002_intensities_[48].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\underwater\\underwater.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[9].next_number_ = 10; // explore
			dbg_002_intensities_[30].next_number_ = 31; // warning
			dbg_002_intensities_[48].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\unity\\bicycle.txt")
		{
			dbg_002_initial_intensity_ = 21;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[36].next_number_ = 41; // warning
			dbg_002_intensities_[62].next_number_ = 0; // combat
		}
		else if (control_file_name_lc == "music\\unity\\unity.txt")
		{
			dbg_002_initial_intensity_ = 2;
			dbg_002_intensities_ = intensities_;

			dbg_002_intensities_[17].next_number_ = 21; // explore
			dbg_002_intensities_[36].next_number_ = 41; // warning
			dbg_002_intensities_[62].next_number_ = 0; // combat
		}
		else
		{
			return;
		}

		auto file_path = control_file_name_lc;
		std::replace(file_path.begin(), file_path.end(), '\\', '_');
		std::replace(file_path.begin(), file_path.end(), '.', '_');
		file_path = "ltjs_dbg_dmus002_" + file_path + ".wav";

		auto file = ul::FileStream{file_path, ul::Stream::OpenMode::write | ul::Stream::OpenMode::truncate};

		if (!file.is_open())
		{
			return;
		}

		dbg_002_is_active_ = true;
		current_intensity_index_ = -1;


		const auto wave_prefix_size =
			4 + 4 + 4 + // "RIFF"<size>"WAVE"
			4 + 4 + ul::PcmWaveFormat::class_size + // "fmt "<size><data>
			4 + 4 + // "data"<size>
			0;

		if (!file.set_position(wave_prefix_size))
		{
			return;
		}


		auto mix_buffer = BufferS16{};
		mix_buffer.resize(mix_sample_count_);

		auto is_finished = false;
		auto total_decoded_size = 0;

		while (!is_finished)
		{
			is_finished = !mix();

			for (auto i = 0; i < mix_sample_count_; ++i)
			{
				auto sample = static_cast<int>(mix_buffer_[i] * 32768.0F);

				if (sample < -32768)
				{
					sample = -32768;
				}
				else if (sample > 32767)
				{
					sample = 32767;
				}

				mix_buffer[i] = static_cast<std::int16_t>(sample);
			}

			if (file.write(mix_buffer.data(), mix_s16_size_) != mix_s16_size_)
			{
				is_finished = true;
			}

			total_decoded_size += mix_s16_size_;
		}

		static_cast<void>(file.set_position(0));

		const auto riff_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::RiffFourCcs::riff));
		const auto riff_size_le = ul::Endian::little(static_cast<std::uint32_t>(wave_prefix_size + total_decoded_size - 8));
		const auto wave_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::WaveFourCcs::wave));
		const auto fmt_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::WaveFourCcs::fmt));
		const auto fmt_size_le = ul::Endian::little(static_cast<std::uint32_t>(ul::PcmWaveFormat::class_size));
		const auto data_id_le = ul::Endian::little(static_cast<std::uint32_t>(ul::WaveFourCcs::data));
		const auto data_size_le = ul::Endian::little(static_cast<std::uint32_t>(total_decoded_size));

		auto waveformat = ul::PcmWaveFormat{};
		waveformat.tag_ = ul::WaveFormatTag::pcm;
		waveformat.bit_depth_ = bit_depth;
		waveformat.channel_count_ = channel_count;
		waveformat.sample_rate_ = sample_rate_;
		waveformat.block_align_ = waveformat.channel_count_ * (waveformat.bit_depth_ / 8);
		waveformat.avg_bytes_per_sec_ = waveformat.block_align_ * waveformat.sample_rate_;

		static_cast<void>(file.write(&riff_id_le, 4));
		static_cast<void>(file.write(&riff_size_le, 4));
		static_cast<void>(file.write(&wave_id_le, 4));
		static_cast<void>(file.write(&fmt_id_le, 4));
		static_cast<void>(file.write(&fmt_size_le, 4));
		static_cast<void>(ul::WaveformatUtils::write(waveformat, &file));
		static_cast<void>(file.write(&data_id_le, 4));
		static_cast<void>(file.write(&data_size_le, 4));

		file.close();

		dbg_002_is_active_ = false;
		current_intensity_index_ = -1;
	}
#endif // LTJS_DEBUG_DMUSIC_DUMP_CUSTOM_NOLF2

	//
	// Debug stuff
	// ======================================================================
}; // DMusicManager::Impl


const char* const DMusicManager::Impl::unsupported_method_message = "Unsupported method.";


DMusicManager::DMusicManager()
	:
	impl_{std::make_unique<Impl>()}
{
}

DMusicManager::DMusicManager(
	DMusicManager&& that)
	:
	impl_{std::move(that.impl_)}
{
}

DMusicManager::~DMusicManager()
{
}

LTRESULT DMusicManager::Init()
{
	impl_->set_method_name(__func__);
	return impl_->api_init();
}

LTRESULT DMusicManager::Term()
{
	impl_->set_method_name(__func__);
	return impl_->api_term();
}

LTRESULT DMusicManager::InitLevel(
	const char* working_directory,
	const char* control_file_name,
	const char* define1,
	const char* define2,
	const char* define3)
{
	impl_->set_method_name(__func__);
	return impl_->api_init_level(working_directory, control_file_name, define1, define2, define3);
}

LTRESULT DMusicManager::TermLevel()
{
	impl_->set_method_name(__func__);
	return impl_->api_term_level();
}

LTRESULT DMusicManager::Play()
{
	impl_->set_method_name(__func__);
	return impl_->api_play();
}

LTRESULT DMusicManager::Stop(
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_stop(start_type);
}

LTRESULT DMusicManager::Pause(
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_pause(start_type);
}

LTRESULT DMusicManager::UnPause()
{
	impl_->set_method_name(__func__);
	return impl_->api_unpause();
}

LTRESULT DMusicManager::SetVolume(
	const long volume)
{
	impl_->set_method_name(__func__);
	return impl_->api_set_volume(volume);
}

LTRESULT DMusicManager::ChangeIntensity(
	const int new_intensity,
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_change_intensity(new_intensity, start_type);
}

LTRESULT DMusicManager::PlaySecondary(
	const char* segment_name,
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_play_secondary(segment_name, start_type);
}

LTRESULT DMusicManager::StopSecondary(
	const char* segment_name,
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_stop_secondary(segment_name, start_type);
}

LTRESULT DMusicManager::PlayMotif(
	const char* style_name,
	const char* motif_name,
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_play_motif(style_name, motif_name, start_type);
}

LTRESULT DMusicManager::StopMotif(
	const char* style_name,
	const char* motif_name,
	const LTDMEnactTypes start_type)
{
	impl_->set_method_name(__func__);
	return impl_->api_stop_motif(style_name, motif_name, start_type);
}

int DMusicManager::GetCurIntensity()
{
	impl_->set_method_name(__func__);
	return impl_->api_get_cur_intensity();
}

LTDMEnactTypes DMusicManager::StringToEnactType(
	const char* name)
{
	impl_->set_method_name(__func__);
	return impl_->api_string_to_enact_type(name);
}

void DMusicManager::EnactTypeToString(
	LTDMEnactTypes type,
	char* name)
{
	impl_->set_method_name(__func__);
	impl_->api_enact_type_to_string(type, name);
}

int DMusicManager::GetNumIntensities()
{
	impl_->set_method_name(__func__);
	return impl_->api_get_num_intensities();
}

int DMusicManager::GetInitialIntensity()
{
	impl_->set_method_name(__func__);
	return impl_->api_get_initial_intensity();
}

int DMusicManager::GetInitialVolume()
{
	impl_->set_method_name(__func__);
	return impl_->api_get_initial_volume();
}

int DMusicManager::GetVolumeOffset()
{
	impl_->set_method_name(__func__);
	return impl_->api_get_volume_offset();
}


} // ltjs


#endif // LTJS_USE_DIRECT_MUSIC8
