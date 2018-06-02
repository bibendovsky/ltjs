#include "bdefs.h"


#ifndef LTJS_USE_DIRECT_MUSIC8


#ifdef _DEBUG
#define LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS
#define LTJS_DEBUG_DMUSIC_TEST_ALL_MUSICS_NOLF2
#endif // _DEBUG


#include <cstdio>
#include <algorithm>
#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "bibendovsky_spul_ascii_utils.h"
#include "bibendovsky_spul_memory_stream.h"
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
		volume_offset_{},
		sample_rate_{},
		intensities_{},
		transition_map_{},
		segment_cache_{},
		mix_offset_{},
		mix_sample_count_{},
		mix_s16_size_{},
		mix_f_size_{},
		current_intensity_index_{},
		current_segment_index_{},
		current_segment_{},
		transition_segment_{},
		active_waves_{},
		inactive_waves_{},
		decoder_buffer_{},
		device_buffer_{},
		mix_buffer_{}
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
		current_intensity_index_{std::move(that.current_intensity_index_)},
		current_segment_index_{std::move(that.current_segment_index_)},
		current_segment_{std::move(that.current_segment_)},
		transition_segment_{std::move(that.transition_segment_)},
		active_waves_{std::move(that.active_waves_)},
		inactive_waves_{std::move(that.inactive_waves_)},
		decoder_buffer_{std::move(that.decoder_buffer_)},
		device_buffer_{std::move(that.device_buffer_)},
		mix_buffer_{std::move(that.mix_buffer_)}
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

		mix_offset_ = 0;
		mix_sample_count_ = (channel_count * sample_rate_ * mix_size_ms) / 1000;
		mix_s16_size_ = mix_sample_count_ * byte_depth;
		mix_f_size_ = mix_sample_count_ * byte_depth_f;

		current_intensity_index_ = -1;
		current_segment_index_ = {};
		current_segment_ = {};
		transition_segment_ = {};
		active_waves_.clear();
		inactive_waves_.clear();
		decoder_buffer_.resize(mix_sample_count_);
		device_buffer_.resize(mix_sample_count_);
		mix_buffer_.resize(mix_sample_count_);

		select_next_segment(0);

		while (true)
		{
			if (!mix())
			{
				break;
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
	static constexpr auto channel_count = 2;
	static constexpr auto bit_depth = 16;
	static constexpr auto byte_depth = bit_depth / 8;
	static constexpr auto byte_depth_f = static_cast<int>(sizeof(float));
	static constexpr auto sample_size = channel_count * byte_depth;

	// Size of decoder buffer, mix buffer and device buffer in milliseconds.
	static constexpr auto mix_size_ms = 50;

	static constexpr auto max_intensity = 999;


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
		bool is_started_;
		bool is_finished_;
		std::uint32_t channel_;
		int decoded_offset_; // (int bytes)
		int length_; // (in bytes)
		std::int64_t mix_offset_; // (in bytes)
		ul::MemoryStream stream_;
		ltjs::AudioDecoder decoder_;


		Wave()
			:
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
		DMusicSegment* d_segment_;


		Segment()
			:
			begin_mix_offset_{},
			end_mix_offset_{},
			d_segment_{}
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

	int current_intensity_index_; // (one-based)
	int current_segment_index_;
	Segment current_segment_;
	Segment transition_segment_;

	Waves active_waves_;
	Waves inactive_waves_;

	BufferS16 decoder_buffer_;
	BufferS16 device_buffer_;
	BufferF mix_buffer_;


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

	bool mix()
	{
		std::uninitialized_fill(mix_buffer_.begin(), mix_buffer_.end(), 0.0F);

		auto mix_offset = 0;
		auto end_mix_offset = mix_offset_ + mix_s16_size_;

		while (mix_offset < mix_s16_size_)
		{
			if (current_intensity_index_ == 0 && active_waves_.empty() && inactive_waves_.empty())
			{
				return false;
			}

			const auto remain_mix_size = mix_s16_size_ - mix_offset;

			for (auto i_wave = inactive_waves_.begin(); i_wave != inactive_waves_.end(); )
			{
				auto i = i_wave;
				auto i_next = ++i_wave;

				const auto adjusted_mix_offset = mix_offset_ + mix_offset;

				if (i->mix_offset_ >= adjusted_mix_offset && i->mix_offset_ < (adjusted_mix_offset + i->length_))
				{
					active_waves_.emplace_back(std::move(*i));
					inactive_waves_.erase(i);
				}

				i_wave = i_next;
			}

			for (auto& wave : active_waves_)
			{
				auto additional_mix_offset = 0;

				if (!wave.is_started_)
				{
					wave.is_started_ = true;

					if (wave.mix_offset_ < (mix_offset_ + mix_offset))
					{
						additional_mix_offset = static_cast<int>(mix_offset_ - wave.mix_offset_ - mix_offset);
						wave.decoded_offset_ = additional_mix_offset;
					}

					auto decoder_param = ltjs::AudioDecoder::OpenParameters{};
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

				if (remain_size > remain_mix_size)
				{
					remain_size = remain_mix_size;
				}

				remain_size /= byte_depth;
				remain_size *= byte_depth;

				const auto decoded_size = wave.decoder_.decode(decoder_buffer_.data(), remain_size);

				if (decoded_size > 0)
				{
					const auto sample_count = decoded_size / byte_depth;
					const auto adjusted_decoded_size = sample_count * byte_depth;

					for (auto i = 0; i < sample_count; ++i)
					{
						mix_buffer_[i + additional_mix_offset] += decoder_buffer_[i] / 32768.0F;
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

			std::remove_if(
				active_waves_.begin(),
				active_waves_.end(),
				[](const auto& item)
				{
					return item.is_finished_;
				}
			);

			if (current_intensity_index_ > 0 && current_segment_.end_mix_offset_ <= end_mix_offset)
			{
				const auto adjusted_mix_offset = mix_offset;

				mix_offset = static_cast<int>(end_mix_offset - current_segment_.end_mix_offset_);

				select_next_segment(adjusted_mix_offset);
			}
			else
			{
				mix_offset = mix_s16_size_;
			}
		}

		mix_offset_ += mix_s16_size_;

		return true;
	}

	void queue_waves(
		const int additional_mix_offset)
	{
		auto d_segment = current_segment_.d_segment_;

		const auto channel = d_segment->get_channel();
		const auto variation = d_segment->select_next_variation();
		const auto& segment_waves = d_segment->get_waves();

		current_segment_.begin_mix_offset_ = mix_offset_;
		current_segment_.end_mix_offset_ = current_segment_.begin_mix_offset_ + d_segment->get_length();

		for (const auto& segment_wave : segment_waves)
		{
			if ((segment_wave.variations_ & variation) == 0)
			{
				continue;
			}

			auto wave = Wave{};
			wave.length_ = segment_wave.length_;
			wave.channel_ = channel;
			wave.mix_offset_ = mix_offset_ + additional_mix_offset + segment_wave.mix_offset_;

			if (!wave.stream_.open(segment_wave.data_, segment_wave.data_size_))
			{
				continue;
			}

			inactive_waves_.emplace_back(std::move(wave));
		}
	}

	void select_next_segment(
		const int additional_mix_offset)
	{
		if (current_intensity_index_ == 0)
		{
			return;
		}

		if (current_intensity_index_ < 0)
		{
			// Fresh start.
			//

			active_waves_.clear();
			inactive_waves_.clear();

			current_intensity_index_ = initial_intensity_;
			current_segment_index_ = 0;

			if (current_intensity_index_ == 0)
			{
				return;
			}

			current_segment_.d_segment_ = intensities_[current_intensity_index_].segments_.front();

			current_segment_.begin_mix_offset_ = mix_offset_ + additional_mix_offset;
			current_segment_.end_mix_offset_ = current_segment_.begin_mix_offset_ + current_segment_.d_segment_->get_length();
		}
		else if (transition_segment_.d_segment_)
		{
			// From a transition segment to the next intensity.
			//

			current_intensity_index_ = intensities_[current_intensity_index_].next_number_;

			if (current_intensity_index_ == 0)
			{
				return;
			}

			current_segment_index_ = 0;
			current_segment_.d_segment_ = intensities_[current_intensity_index_].segments_.front();
			current_segment_.begin_mix_offset_ = mix_offset_ + additional_mix_offset;
			current_segment_.end_mix_offset_ = current_segment_.begin_mix_offset_ + current_segment_.d_segment_->get_length();

			transition_segment_.d_segment_ = nullptr;
		}
		else
		{
			// From a current intensity to the next segment or the next intensity.
			//

			const auto& intensity = intensities_[current_intensity_index_];

			const auto& segments = intensity.segments_;
			const auto segment_count = static_cast<int>(segments.size());

			if (current_segment_index_ == (segment_count - 1))
			{
				// The last segment.
				//

				// At-first, check for a transition segment.
				//
				const auto transition_key = TransitionMapKey::encode(intensity.number_, intensity.next_number_);

				auto map_it = transition_map_.find(transition_key);

				if (map_it != transition_map_.cend() && map_it->second.segment_)
				{
					// Has transition segment. Use it.
					//
					transition_segment_.d_segment_ = map_it->second.segment_;

					current_segment_.d_segment_ = transition_segment_.d_segment_;
					current_segment_.begin_mix_offset_ = mix_offset_ + additional_mix_offset;
					current_segment_.end_mix_offset_ = current_segment_.begin_mix_offset_ + current_segment_.d_segment_->get_length();
				}
				else
				{
					// Does not have transition segment. Move to the next intensity.
					//

					current_intensity_index_ = intensities_[current_intensity_index_].next_number_;

					if (current_intensity_index_ == 0)
					{
						return;
					}

					current_segment_index_ = 0;
					current_segment_.d_segment_ = intensities_[current_intensity_index_].segments_.front();
				}
			}
			else
			{
				// Not the last segment. Move to the next segment.
				//

				current_segment_index_ += 1;

				current_segment_.d_segment_ = intensities_[current_intensity_index_].segments_[current_segment_index_];

				current_segment_.begin_mix_offset_ = mix_offset_ + additional_mix_offset;
				current_segment_.end_mix_offset_ = current_segment_.begin_mix_offset_ + current_segment_.d_segment_->get_length();
			}
		}

		// Finally, queue the items of the current segment.
		//
		queue_waves(additional_mix_offset);
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
