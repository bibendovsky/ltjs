#include "s_oal.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "efx.h"
#include "efx-presets.h"
#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_file_stream.h"
#include "bibendovsky_spul_substream.h"
#include "bibendovsky_spul_memory_stream.h"
#include "bibendovsky_spul_scope_guard.h"
#include "ltjs_audio_decoder.h"
#include "ltjs_audio_utils.h"


struct OalSoundSys::Impl
{
	static constexpr auto min_aux_sends = 1;
	static constexpr auto default_aux_sends = 2;
	static constexpr auto default_doppler_factor = 1.0F;

	static constexpr auto pan_center = 64;


	using String = std::string;
	using ExtensionsStrings = std::vector<String>;

	using Clock = std::chrono::system_clock;
	using ClockTs = Clock::time_point;

	using MtThread = std::thread;
	using MtMutex = std::mutex;
	using MtMutexGuard = std::lock_guard<MtMutex>;
	using MtUniqueLock = std::unique_lock<MtMutex>;
	using MtCondVar = std::condition_variable;

	static constexpr auto max_user_data_count = 8;
	static constexpr auto max_user_data_index = max_user_data_count - 1;
	using UserDataArray = std::array<sint32, max_user_data_count>;


	struct Vector3d
	{
		float x_;
		float y_;
		float z_;


		bool operator==(
			const Vector3d& that) const
		{
			return x_ == that.x_ && y_ == that.y_ && z_ == that.z_;
		}

		static Vector3d to_oal(
			const Vector3d& vector_3d)
		{
			return {vector_3d.x_, vector_3d.y_, -vector_3d.z_};
		}
	}; // Vector3d

	struct Orientation3d
	{
		Vector3d at_;
		Vector3d up_;


		bool operator==(
			const Orientation3d& that) const
		{
			return at_ == that.at_ && up_ == that.up_;
		}

		static Orientation3d to_oal(
			const Orientation3d& direction_3d)
		{
			return {Vector3d::to_oal(direction_3d.at_), Vector3d::to_oal(direction_3d.up_)};
		}

		static const Orientation3d& get_listener_defaults()
		{
			static const auto direction_3d = Orientation3d{{0.0F, 0.0F, 1.0F}, {0.0F, 1.0F, 0.0F}};
			return direction_3d;
		}

		static const Orientation3d& get_object_defaults()
		{
			static const auto direction_3d = Orientation3d{};
			return direction_3d;
		}
	}; // Orientation3d

	struct Sample
	{
		enum class Status
		{
			none,
			stopped,
			playing,
			paused,
			failed,
		}; // Status

		static constexpr auto oal_max_buffer_count = 3;

		static constexpr auto left_index = 0;
		static constexpr auto right_index = 1;

		using Data = std::vector<std::uint8_t>;
		using OalPans = std::array<float, 2>;
		using OalSources = std::array<ALuint, 2>;
		using OalBuffers = std::array<ALuint, oal_max_buffer_count>;


		ul::WaveFormatEx format_;

		bool is_3d_;
		bool is_stream_;
		bool is_looping_;
		bool has_loop_block_;
		sint32 loop_begin_;
		sint32 loop_end_;
		sint32 volume_;
		sint32 pan_;
		Status status_;
		Data data_;
		UserDataArray user_data_array_;

		// 2D: two sources.
		// 3D: one source.
		int oal_source_count_;

		// 2D and 3D: 0 - non-loop block; 1 - pre-loop [0..loop_start); 2 - loop [loop_start..loop_end].
		// Stream: 0, 1, 2 - queue.
		OalBuffers oal_buffers_;

		// 2D and stream: 0 - left channel; 1 - right channel.
		// 3D: 0 - main; 1 - not available.
		OalSources oal_sources_;

		ALenum oal_buffer_format_;
		float oal_volume_;
		OalPans oal_pans_;
	}; // Sample

	using Samples = std::list<Sample>;


	struct Object3d
	{
		bool is_listener_;
		float min_distance_;
		float max_distance_;
		Vector3d position_;
		Vector3d velocity_;
		Orientation3d orientation_;
		float doppler_factor_;
		UserDataArray user_data_;
		Sample sample_;


		void reset()
		{
			min_distance_ = ltjs::AudioUtils::ds_default_min_distance;
			max_distance_ = ltjs::AudioUtils::ds_default_max_distance;
			position_ = {};
			velocity_ = {};
			orientation_ = (is_listener_ ? Orientation3d::get_listener_defaults() : Orientation3d::get_object_defaults());
			doppler_factor_ = ltjs::AudioUtils::ds_default_doppler_factor;
		}
	}; // Object3d

	using Objects3d = std::list<Object3d>;


	static constexpr auto max_streams = 16;

	struct Stream
	{
		static constexpr auto mix_size_ms = 20;

		using OalBuffers = std::vector<ALuint>;
		using MixBuffer = std::vector<std::uint8_t>;

		bool is_open_;
		bool is_playing_;
		int mix_size_;
		int data_size_;
		int data_offset_;
		Sample sample_;
		ul::FileStream file_stream_;
		ul::Substream file_substream_;
		ltjs::AudioDecoder decoder_;
		MixBuffer mix_buffer_;

		OalBuffers oal_queued_buffers_;
		OalBuffers oal_unqueued_buffers_;
	}; // Stream

	using Streams = std::vector<Stream>;
	using OpenStreams = std::list<Stream*>;



	// =========================================================================
	// Utils
	// =========================================================================
	//

	void uninitialize_stream(
		Stream& stream)
	{
		stream.is_open_ = false;

		stream.decoder_.close();
		stream.file_substream_.close();
		stream.file_stream_.close();

		destroy_sample(stream.sample_);
	}

	void destroy_streams()
	{
		if (streams_.empty())
		{
			return;
		}

		mt_is_stop_stream_worker_ = true;

		mt_notify_stream();

		if (mt_stream_thread_.joinable())
		{
			mt_stream_thread_.join();
		}

		mt_is_stop_stream_worker_ = false;

		for (auto& stream : streams_)
		{
			uninitialize_stream(stream);
		}

		streams_.clear();
		mt_open_streams_.clear();
	}

	bool initialize_stream(
		Stream& stream)
	{
		uninitialize_stream(stream);

		auto& sample = stream.sample_;

		sample.is_stream_ = true;
		sample.oal_source_count_ = 2;
		reset_sample(sample);
		create_sample(sample);

		return sample.status_ != Sample::Status::failed;
	}

	void create_streams()
	{
		destroy_streams();

		streams_.resize(max_streams);

		auto is_succeed = true;

		for (auto& stream : streams_)
		{
			is_succeed &= initialize_stream(stream);
		}

		if (!is_succeed)
		{
			destroy_streams();
		}

		if (is_succeed)
		{
			mt_stream_thread_ = MtThread{std::bind(&Impl::stream_worker, this)};
		}
	}

	void reset_stream(
		Stream& stream)
	{
		stream.is_playing_ = {};
		stream.mix_size_ = {};
		stream.data_size_ = {};
		stream.data_offset_ = {};

		stream.oal_queued_buffers_.clear();
		stream.oal_queued_buffers_.reserve(Sample::oal_max_buffer_count);

		stream.oal_unqueued_buffers_.clear();
		stream.oal_unqueued_buffers_.reserve(Sample::oal_max_buffer_count);
		stream.oal_unqueued_buffers_.assign(stream.sample_.oal_buffers_.cbegin(), stream.sample_.oal_buffers_.cend());
	}

	Stream* get_free_stream()
	{
		auto stream_end = streams_.end();

		auto stream_it = std::find_if(
			streams_.begin(),
			stream_end,
			[](const auto& stream)
			{
				return !stream.is_open_;
			}
		);

		if (stream_it == stream_end)
		{
			return nullptr;
		}

		return &(*stream_it);
	}

	int fill_stream_mix_buffer(
		Stream& stream)
	{
		const auto& sample = stream.sample_;

		const auto is_looping = sample.is_looping_;

		const auto data_begin_offset = static_cast<int>(
			is_looping && sample.has_loop_block_ ? sample.loop_begin_ : 0);

		auto data_end_offset = static_cast<int>(
			is_looping && sample.has_loop_block_ ? sample.loop_end_ : stream.data_size_);

		auto mix_offset = 0;

		while (mix_offset < stream.mix_size_)
		{
			const auto data_remain_size = data_end_offset - stream.data_offset_;

			if (data_remain_size == 0)
			{
				if (is_looping)
				{
					stream.data_offset_ = data_begin_offset;

					const auto sample_offset = data_begin_offset / stream.sample_.format_.block_align_;

					if (!stream.decoder_.set_position(sample_offset))
					{
						break;
					}

					continue;
				}
				else
				{
					break;
				}
			}

			const auto mix_remain_size = stream.mix_size_ - mix_offset;
			const auto to_decode_size = std::min(data_remain_size, mix_remain_size);

			if (to_decode_size == 0)
			{
				break;
			}

			const auto decoded_size = stream.decoder_.decode(&stream.mix_buffer_[mix_offset], to_decode_size);

			if (decoded_size > 0)
			{
				mix_offset += decoded_size;
				stream.data_offset_ += decoded_size;
			}
			else
			{
				if (decoded_size == 0)
				{
					// Adjust the actual data size if necessary.
					//
					const auto actual_data_size = stream.decoder_.get_decoded_size();

					if (stream.data_size_ != actual_data_size)
					{
						stream.data_size_ = actual_data_size;

						if (!is_looping)
						{
							data_end_offset = actual_data_size;
						}
					}
				}

				std::uninitialized_fill_n(
					stream.mix_buffer_.begin() + mix_offset,
					stream.mix_size_ - mix_offset,
					std::uint8_t{});

				break;
			}
		}

		return mix_offset;
	}

	void mt_notify_stream()
	{
		MtUniqueLock cv_lock{mt_stream_cv_mutex_};
		mt_stream_cv_flag_ = true;
		mt_stream_cv_.notify_one();
	}

	void mt_wait_for_stream_cv()
	{
		MtUniqueLock cv_lock{mt_stream_cv_mutex_};
		mt_stream_cv_.wait(cv_lock, [&](){ return mt_stream_cv_flag_; });
		mt_stream_cv_flag_ = false;
	}

	//
	// Fills a stream with data.
	//
	// Returns:
	//    - "true" - if stream is processed.
	//    - "false" - if stream is not processed (paused, failed, etc).
	//
	bool mix_stream(
		Stream& stream)
	{
		auto& sample = stream.sample_;

		if (sample.status_ == Sample::Status::none || sample.status_ == Sample::Status::failed)
		{
			return false;
		}

		const auto is_sample_playing = (sample.status_ == Sample::Status::playing);

		if (stream.is_playing_ && !is_sample_playing)
		{
			sample.status_ = Sample::Status::playing;
		}
		else if (!stream.is_playing_ && is_sample_playing)
		{
			sample.status_ = Sample::Status::paused;

			::alSourcePausev(2, sample.oal_sources_.data());

			return false;
		}
		else if (!stream.is_playing_ && !is_sample_playing)
		{
			return false;
		}

		auto oal_processed = ALint{};
		auto oal_queued = ALint{};

		::alGetSourcei(sample.oal_sources_[0], AL_BUFFERS_PROCESSED, &oal_processed);
		::alGetSourcei(sample.oal_sources_[0], AL_BUFFERS_QUEUED, &oal_queued);

		if (oal_processed > 0)
		{
			auto buffers = Sample::OalBuffers{};
			const auto old_size = stream.oal_unqueued_buffers_.size();

			stream.oal_unqueued_buffers_.resize(
				old_size + std::min(oal_processed, Sample::oal_max_buffer_count));

			::alSourceUnqueueBuffers(
				sample.oal_sources_[0],
				oal_processed,
				&stream.oal_unqueued_buffers_[old_size]);

			::alSourceUnqueueBuffers(
				sample.oal_sources_[1],
				oal_processed,
				buffers.data());
		}

		if (!sample.is_looping_ && stream.data_offset_ == stream.data_size_)
		{
			if (oal_queued == 0)
			{
				stream.is_playing_ = false;
				sample.status_ = Sample::Status::paused;
				return false;
			}

			if (oal_queued > 0)
			{
				return true;
			}
		}

		auto queued_count = 0;

		for (auto i = oal_queued; i < Sample::oal_max_buffer_count; ++i)
		{
			if (stream.oal_unqueued_buffers_.empty())
			{
				break;
			}

			auto decoded_mix_size = fill_stream_mix_buffer(stream);

			if (decoded_mix_size == 0)
			{
				break;
			}

			const auto oal_buffer = stream.oal_unqueued_buffers_.back();
			stream.oal_unqueued_buffers_.pop_back();
			stream.oal_queued_buffers_.push_back(oal_buffer);

			::alBufferData(
				oal_buffer,
				sample.oal_buffer_format_,
				stream.mix_buffer_.data(),
				stream.mix_size_,
				static_cast<ALsizei>(sample.format_.sample_rate_));

			for (auto j = 0; j < 2; ++j)
			{
				::alSourceQueueBuffers(sample.oal_sources_[j], 1, &oal_buffer);
			}

			queued_count += 1;

			if (decoded_mix_size < stream.mix_size_)
			{
				break;
			}
		}

		if (queued_count > 0)
		{
			auto oal_state = ALint{};

			::alGetSourcei(sample.oal_sources_[0], AL_SOURCE_STATE, &oal_state);

			if (oal_state != AL_PLAYING)
			{
				::alSourcePlayv(2, sample.oal_sources_.data());
			}
		}

		return oal_processed > 0 || queued_count > 0;
	}

	void stream_worker()
	{
		const auto sleep_delay = std::chrono::milliseconds{10};

		while (!mt_is_stop_stream_worker_)
		{
			auto is_mixed = false;
			auto is_idle = false;

			{
				MtUniqueLock lock{mt_stream_mutex_};

				if (mt_open_streams_.empty())
				{
					is_idle = true;
				}
				else
				{
					const auto total_count = static_cast<int>(mt_open_streams_.size());

					auto paused_count = 0;

					for (auto stream_ptr : mt_open_streams_)
					{
						auto& stream = *stream_ptr;

						is_mixed |= mix_stream(stream);

						if (stream.sample_.status_ != Sample::Status::playing)
						{
							paused_count += 1;
						}
					}

					if (paused_count == total_count)
					{
						is_idle = true;
					}
				}
			}

			if (is_idle)
			{
				mt_wait_for_stream_cv();
			}
			else if (!is_mixed)
			{
				std::this_thread::sleep_for(sleep_delay);
			}
		}
	}

	using EfxReverbPresets = std::array<EFXEAXREVERBPROPERTIES, ltjs::AudioUtils::eax_environment_count>;

	static const EfxReverbPresets efx_reverb_presets;

	static const EFXEAXREVERBPROPERTIES& get_efx_reverb_preset(
		const int preset_index)
	{
		auto new_preset_index = preset_index;

		if (new_preset_index < 0 || new_preset_index > ltjs::AudioUtils::eax_max_environment)
		{
			new_preset_index = ltjs::AudioUtils::eax_default_environment;
		}

		return efx_reverb_presets[new_preset_index];
	}

	static void lt_reverb_to_efx_reverb(
		const LTFILTERREVERB& lt_reverb,
		EFXEAXREVERBPROPERTIES& efx_reverb)
	{
		// Normalize EAX parameters.
		//
#ifdef USE_EAX20_HARDWARE_FILTERS
		const auto eax_environment = ul::Algorithm::clamp(
			lt_reverb.lEnvironment,
			ltjs::AudioUtils::eax_min_environment,
			ltjs::AudioUtils::eax_max_environment);
#endif // USE_EAX20_HARDWARE_FILTERS

		const auto eax_environment_diffusion = ul::Algorithm::clamp(
			lt_reverb.fDiffusion,
			ltjs::AudioUtils::eax_min_environment_diffusion,
			ltjs::AudioUtils::eax_max_environment_diffusion);

		const auto eax_room = ul::Algorithm::clamp(
			lt_reverb.lRoom,
			ltjs::AudioUtils::eax_min_room,
			ltjs::AudioUtils::eax_max_room);

		const auto eax_room_hf = ul::Algorithm::clamp(
			lt_reverb.lRoomHF,
			ltjs::AudioUtils::eax_min_room_hf,
			ltjs::AudioUtils::eax_max_room_hf);

		const auto eax_decay_time = ul::Algorithm::clamp(
			lt_reverb.fDecayTime,
			ltjs::AudioUtils::eax_min_decay_time,
			ltjs::AudioUtils::eax_max_decay_time);

		const auto eax_decay_hf_ratio = ul::Algorithm::clamp(
			lt_reverb.fDecayHFRatio,
			ltjs::AudioUtils::eax_min_decay_hf_ratio,
			ltjs::AudioUtils::eax_max_decay_hf_ratio);

		const auto eax_reflections = ul::Algorithm::clamp(
			lt_reverb.lReflections,
			ltjs::AudioUtils::eax_min_reflections,
			ltjs::AudioUtils::eax_max_reflections);

		const auto eax_reflections_delay = ul::Algorithm::clamp(
			lt_reverb.fReflectionsDelay,
			ltjs::AudioUtils::eax_min_reflections_delay,
			ltjs::AudioUtils::eax_max_reflections_delay);

		const auto eax_reverb = ul::Algorithm::clamp(
			lt_reverb.lReverb,
			ltjs::AudioUtils::eax_min_reverb,
			ltjs::AudioUtils::eax_max_reverb);

		const auto eax_reverb_delay = ul::Algorithm::clamp(
			lt_reverb.fReverbDelay,
			ltjs::AudioUtils::eax_min_reverb_delay,
			ltjs::AudioUtils::eax_max_reverb_delay);

		const auto eax_room_rolloff_factor = ul::Algorithm::clamp(
			lt_reverb.fRoomRolloffFactor,
			ltjs::AudioUtils::eax_min_room_rolloff_factor,
			ltjs::AudioUtils::eax_max_room_rolloff_factor);

#ifdef USE_EAX20_HARDWARE_FILTERS
		const auto eax_air_absorbtion_hf = ul::Algorithm::clamp(
			lt_reverb.fAirAbsorptionHF,
			ltjs::AudioUtils::eax_min_air_absorption_hf,
			ltjs::AudioUtils::eax_max_air_absorption_hf);
#endif // USE_EAX20_HARDWARE_FILTERS


		// Normalize EFX parameters.
		//
		const auto efx_diffusion = ul::Algorithm::clamp(
			eax_environment_diffusion,
			AL_EAXREVERB_MIN_DIFFUSION,
			AL_EAXREVERB_MAX_DIFFUSION);

		const auto efx_gain = ul::Algorithm::clamp(
			ltjs::AudioUtils::ds_volume_to_gain(eax_room),
			AL_EAXREVERB_MIN_GAIN,
			AL_EAXREVERB_MAX_GAIN);

		const auto efx_gain_hf = ul::Algorithm::clamp(
			ltjs::AudioUtils::ds_volume_to_gain(eax_room_hf),
			AL_EAXREVERB_MIN_GAINHF,
			AL_EAXREVERB_MAX_GAINHF);

		const auto efx_decay_time = ul::Algorithm::clamp(
			eax_decay_time,
			AL_EAXREVERB_MIN_DECAY_TIME,
			AL_EAXREVERB_MAX_DECAY_TIME);

		const auto efx_decay_hf_ratio = ul::Algorithm::clamp(
			eax_decay_hf_ratio,
			AL_EAXREVERB_MIN_DECAY_HFRATIO,
			AL_EAXREVERB_MAX_DECAY_HFRATIO);

		const auto efx_reflections_gain = ul::Algorithm::clamp(
			ltjs::AudioUtils::mb_volume_to_gain(eax_reflections),
			AL_EAXREVERB_MIN_REFLECTIONS_GAIN,
			AL_EAXREVERB_MAX_REFLECTIONS_GAIN);

		const auto efx_reflections_delay = ul::Algorithm::clamp(
			eax_reflections_delay,
			AL_EAXREVERB_MIN_REFLECTIONS_DELAY,
			AL_EAXREVERB_MAX_REFLECTIONS_DELAY);

		const auto efx_late_reverb_gain = ul::Algorithm::clamp(
			ltjs::AudioUtils::mb_volume_to_gain(eax_reverb),
			AL_EAXREVERB_MIN_LATE_REVERB_GAIN,
			AL_EAXREVERB_MAX_LATE_REVERB_GAIN);

		const auto efx_late_reverb_delay = ul::Algorithm::clamp(
			eax_reverb_delay,
			AL_EAXREVERB_MIN_LATE_REVERB_DELAY,
			AL_EAXREVERB_MAX_LATE_REVERB_DELAY);

		const auto efx_room_rolloff_factor = ul::Algorithm::clamp(
			eax_room_rolloff_factor,
			AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR,
			AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR);

#ifdef USE_EAX20_HARDWARE_FILTERS
		const auto efx_air_absorption_gain_hf = ul::Algorithm::clamp(
			ltjs::AudioUtils::mb_volume_to_gain(static_cast<int>(eax_air_absorbtion_hf)),
			AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF,
			AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF);
#endif // USE_EAX20_HARDWARE_FILTERS


		// Set all properties according to the environmetn value.
		//
#ifdef USE_EAX20_HARDWARE_FILTERS
		efx_reverb = get_efx_reverb_preset(eax_environment);
#else
		efx_reverb = get_efx_reverb_preset(ltjs::AudioUtils::eax_default_environment);
#endif // USE_EAX20_HARDWARE_FILTERS


		// Set specific properties.
		//
		efx_reverb.flDiffusion = efx_diffusion;
		efx_reverb.flGain = efx_gain;
		efx_reverb.flGainHF = efx_gain_hf;
		efx_reverb.flDecayTime = efx_decay_time;
		efx_reverb.flDecayHFRatio = efx_decay_hf_ratio;
		efx_reverb.flReflectionsGain = efx_reflections_gain;
		efx_reverb.flReflectionsDelay = efx_reflections_delay;
		efx_reverb.flLateReverbGain = efx_late_reverb_gain;
		efx_reverb.flLateReverbDelay = efx_late_reverb_delay;
		efx_reverb.flRoomRolloffFactor = efx_room_rolloff_factor;

#ifdef USE_EAX20_HARDWARE_FILTERS
		efx_reverb.flAirAbsorptionGainHF = efx_air_absorption_gain_hf;
#endif // USE_EAX20_HARDWARE_FILTERS
	}

	void set_efx_eax_reverb_properties(
		const ALuint oal_effect,
		const EFXEAXREVERBPROPERTIES& efx_reverb)
	{
		alEffectf_(oal_effect, AL_EAXREVERB_DENSITY, efx_reverb.flDensity);
		alEffectf_(oal_effect, AL_EAXREVERB_DIFFUSION, efx_reverb.flDiffusion);
		alEffectf_(oal_effect, AL_EAXREVERB_GAIN, efx_reverb.flGain);
		alEffectf_(oal_effect, AL_EAXREVERB_GAINHF, efx_reverb.flGainHF);
		alEffectf_(oal_effect, AL_EAXREVERB_GAINLF, efx_reverb.flGainLF);
		alEffectf_(oal_effect, AL_EAXREVERB_DECAY_TIME, efx_reverb.flDecayTime);
		alEffectf_(oal_effect, AL_EAXREVERB_DECAY_HFRATIO, efx_reverb.flDecayHFRatio);
		alEffectf_(oal_effect, AL_EAXREVERB_DECAY_LFRATIO, efx_reverb.flDecayLFRatio);
		alEffectf_(oal_effect, AL_EAXREVERB_REFLECTIONS_GAIN, efx_reverb.flReflectionsGain);
		alEffectf_(oal_effect, AL_EAXREVERB_REFLECTIONS_DELAY, efx_reverb.flReflectionsDelay);
		alEffectfv_(oal_effect, AL_EAXREVERB_REFLECTIONS_PAN, efx_reverb.flReflectionsPan);
		alEffectf_(oal_effect, AL_EAXREVERB_LATE_REVERB_GAIN, efx_reverb.flLateReverbGain);
		alEffectf_(oal_effect, AL_EAXREVERB_LATE_REVERB_DELAY, efx_reverb.flLateReverbDelay);
		alEffectfv_(oal_effect, AL_EAXREVERB_LATE_REVERB_PAN, efx_reverb.flLateReverbPan);
		alEffectf_(oal_effect, AL_EAXREVERB_ECHO_TIME, efx_reverb.flEchoTime);
		alEffectf_(oal_effect, AL_EAXREVERB_ECHO_DEPTH, efx_reverb.flEchoDepth);
		alEffectf_(oal_effect, AL_EAXREVERB_MODULATION_TIME, efx_reverb.flModulationTime);
		alEffectf_(oal_effect, AL_EAXREVERB_MODULATION_DEPTH, efx_reverb.flModulationDepth);
		alEffectf_(oal_effect, AL_EAXREVERB_HFREFERENCE, efx_reverb.flHFReference);
		alEffectf_(oal_effect, AL_EAXREVERB_LFREFERENCE, efx_reverb.flLFReference);
		alEffectf_(oal_effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, efx_reverb.flRoomRolloffFactor);
		alEffectf_(oal_effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, efx_reverb.flAirAbsorptionGainHF);
		alEffecti_(oal_effect, AL_EAXREVERB_DECAY_HFLIMIT, efx_reverb.iDecayHFLimit);
	}

	void set_efx_reverb_properties(
		const ALuint oal_effect,
		const EFXEAXREVERBPROPERTIES& efx_reverb)
	{
		alEffectf_(oal_effect, AL_REVERB_DENSITY, efx_reverb.flDensity);
		alEffectf_(oal_effect, AL_REVERB_DIFFUSION, efx_reverb.flDiffusion);
		alEffectf_(oal_effect, AL_REVERB_GAIN, efx_reverb.flGain);
		alEffectf_(oal_effect, AL_REVERB_GAINHF, efx_reverb.flGainHF);
		alEffectf_(oal_effect, AL_REVERB_DECAY_TIME, efx_reverb.flDecayTime);
		alEffectf_(oal_effect, AL_REVERB_DECAY_HFRATIO, efx_reverb.flDecayHFRatio);
		alEffectf_(oal_effect, AL_REVERB_REFLECTIONS_GAIN, efx_reverb.flReflectionsGain);
		alEffectf_(oal_effect, AL_REVERB_REFLECTIONS_DELAY, efx_reverb.flReflectionsDelay);
		alEffectf_(oal_effect, AL_REVERB_LATE_REVERB_GAIN, efx_reverb.flLateReverbGain);
		alEffectf_(oal_effect, AL_REVERB_LATE_REVERB_DELAY, efx_reverb.flLateReverbDelay);
		alEffectf_(oal_effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, efx_reverb.flRoomRolloffFactor);
		alEffectf_(oal_effect, AL_REVERB_AIR_ABSORPTION_GAINHF, efx_reverb.flAirAbsorptionGainHF);
		alEffecti_(oal_effect, AL_REVERB_DECAY_HFLIMIT, efx_reverb.iDecayHFLimit);
	}

	//
	// =========================================================================
	// Utils
	// =========================================================================


	// =========================================================================
	// OpenAL
	// =========================================================================
	//

	struct OalVersion
	{
		int major_;
		int minor_;


		constexpr bool is_empty() const
		{
			return major_ == 0 && minor_ == 0;
		}

		constexpr bool operator<(
			const OalVersion& that)
		{
			return major_ < that.major_ || (major_ == that.major_ && minor_ < that.minor_);
		}
	}; // OalVersion


	static constexpr auto oal_ref_version = OalVersion{1, 1};
	static constexpr auto oal_efx_ref_version = OalVersion{1, 0};


	const char* get_error_message() const
	{
		return error_message_.c_str();
	}

	static void oal_clear_error()
	{
		static_cast<void>(::alGetError());
	}

	static bool oal_is_succeed()
	{
		return ::alGetError() == AL_NO_ERROR;
	}

	OalVersion oal_get_version(
		const ALenum oal_major_id,
		const ALenum oal_minor_id)
	{
		oal_clear_error();

		auto major = ALCint{};
		auto minor = ALCint{};

		::alcGetIntegerv(oal_device_, oal_major_id, 1, &major);
		::alcGetIntegerv(oal_device_, oal_minor_id, 1, &minor);

		if (!oal_is_succeed())
		{
			return {};
		}

		return {major, minor};
	}

	ExtensionsStrings oal_parse_extensions_string(
		const char* const extensions_string)
	{
		auto iss = std::istringstream{extensions_string};

		return ExtensionsStrings{
			std::istream_iterator<String>{iss},
			std::istream_iterator<String>{}};
	}

	bool oal_get_global_strings()
	{
		oal_clear_error();

		auto version_string = ::alGetString(AL_VERSION);
		auto renderer_string = ::alGetString(AL_RENDERER);
		auto vendor_string = ::alGetString(AL_VENDOR);
		auto extensions_string = ::alGetString(AL_EXTENSIONS);

		if (!oal_is_succeed() || !version_string || !renderer_string || !vendor_string || !extensions_string)
		{
			error_message_ = "OAL: Failed to get global strings.";
			return false;
		}

		oal_version_string_ = version_string;
		oal_renderer_string_ = renderer_string;
		oal_vendor_string_ = vendor_string;
		oal_extentions_strings_ = oal_parse_extensions_string(extensions_string);

		return true;
	}

	bool oal_get_version()
	{
		oal_version_ = oal_get_version(ALC_MAJOR_VERSION, ALC_MINOR_VERSION);

		if (oal_version_.is_empty())
		{
			error_message_ = "OAL: Failed to get a version.";
			return false;
		}

		if (oal_version_ < oal_ref_version)
		{
			error_message_ = "OAL: Expected at least version 1.1.";
			return false;
		}

		return true;
	}

	bool oal_get_efx_version()
	{
		oal_efx_version_ = oal_get_version(ALC_EFX_MAJOR_VERSION, ALC_EFX_MINOR_VERSION);

		if (oal_efx_version_.is_empty() || oal_efx_version_ < oal_efx_ref_version)
		{
			return false;
		}

		return true;
	}

	bool oal_get_max_aux_sends()
	{
		oal_clear_error();

		auto max_aux_sends = ALCint{};
		::alcGetIntegerv(oal_device_, ALC_MAX_AUXILIARY_SENDS, 1, &max_aux_sends);

		if (!oal_is_succeed() || max_aux_sends == 0)
		{
			return false;
		}

		oal_max_aux_sends_ = max_aux_sends;

		return true;
	}

	template<typename T>
	static bool oal_get_efx_function(
		const char* name,
		T& function)
	{
		function = reinterpret_cast<T>(::alGetProcAddress(name));

		return function;
	}

	void oal_clear_efx_functions()
	{
		alGenEffects_ = nullptr;
		alDeleteEffects_ = nullptr;
		alIsEffect_ = nullptr;
		alEffecti_ = nullptr;
		alEffectiv_ = nullptr;
		alEffectf_ = nullptr;
		alEffectfv_ = nullptr;
		alGetEffecti_ = nullptr;
		alGetEffectiv_ = nullptr;
		alGetEffectf_ = nullptr;
		alGetEffectfv_ = nullptr;
		alGenFilters_ = nullptr;
		alDeleteFilters_ = nullptr;
		alIsFilter_ = nullptr;
		alFilteri_ = nullptr;
		alFilteriv_ = nullptr;
		alFilterf_ = nullptr;
		alFilterfv_ = nullptr;
		alGetFilteri_ = nullptr;
		alGetFilteriv_ = nullptr;
		alGetFilterf_ = nullptr;
		alGetFilterfv_ = nullptr;
		alGenAuxiliaryEffectSlots_ = nullptr;
		alDeleteAuxiliaryEffectSlots_ = nullptr;
		alIsAuxiliaryEffectSlot_ = nullptr;
		alAuxiliaryEffectSloti_ = nullptr;
		alAuxiliaryEffectSlotiv_ = nullptr;
		alAuxiliaryEffectSlotf_ = nullptr;
		alAuxiliaryEffectSlotfv_ = nullptr;
		alGetAuxiliaryEffectSloti_ = nullptr;
		alGetAuxiliaryEffectSlotiv_ = nullptr;
		alGetAuxiliaryEffectSlotf_ = nullptr;
		alGetAuxiliaryEffectSlotfv_ = nullptr;
	}

	bool oal_get_efx_symbols()
	{
		auto result = true;

		result &= oal_get_efx_function("alGenEffects", alGenEffects_);
		result &= oal_get_efx_function("alDeleteEffects", alDeleteEffects_);
		result &= oal_get_efx_function("alIsEffect", alIsEffect_);
		result &= oal_get_efx_function("alEffecti", alEffecti_);
		result &= oal_get_efx_function("alEffectiv", alEffectiv_);
		result &= oal_get_efx_function("alEffectf", alEffectf_);
		result &= oal_get_efx_function("alEffectfv", alEffectfv_);
		result &= oal_get_efx_function("alGetEffecti", alGetEffecti_);
		result &= oal_get_efx_function("alGetEffectiv", alGetEffectiv_);
		result &= oal_get_efx_function("alGetEffectf", alGetEffectf_);
		result &= oal_get_efx_function("alGetEffectfv", alGetEffectfv_);
		result &= oal_get_efx_function("alGenFilters", alGenFilters_);
		result &= oal_get_efx_function("alDeleteFilters", alDeleteFilters_);
		result &= oal_get_efx_function("alIsFilter", alIsFilter_);
		result &= oal_get_efx_function("alFilteri", alFilteri_);
		result &= oal_get_efx_function("alFilteriv", alFilteriv_);
		result &= oal_get_efx_function("alFilterf", alFilterf_);
		result &= oal_get_efx_function("alFilterfv", alFilterfv_);
		result &= oal_get_efx_function("alGetFilteri", alGetFilteri_);
		result &= oal_get_efx_function("alGetFilteriv", alGetFilteriv_);
		result &= oal_get_efx_function("alGetFilterf", alGetFilterf_);
		result &= oal_get_efx_function("alGetFilterfv", alGetFilterfv_);
		result &= oal_get_efx_function("alGenAuxiliaryEffectSlots", alGenAuxiliaryEffectSlots_);
		result &= oal_get_efx_function("alDeleteAuxiliaryEffectSlots", alDeleteAuxiliaryEffectSlots_);
		result &= oal_get_efx_function("alIsAuxiliaryEffectSlot", alIsAuxiliaryEffectSlot_);
		result &= oal_get_efx_function("alAuxiliaryEffectSloti", alAuxiliaryEffectSloti_);
		result &= oal_get_efx_function("alAuxiliaryEffectSlotiv", alAuxiliaryEffectSlotiv_);
		result &= oal_get_efx_function("alAuxiliaryEffectSlotf", alAuxiliaryEffectSlotf_);
		result &= oal_get_efx_function("alAuxiliaryEffectSlotfv", alAuxiliaryEffectSlotfv_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSloti", alGetAuxiliaryEffectSloti_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSlotiv", alGetAuxiliaryEffectSlotiv_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSlotf", alGetAuxiliaryEffectSlotf_);
		result &= oal_get_efx_function("alGetAuxiliaryEffectSlotfv", alGetAuxiliaryEffectSlotfv_);

		return result;
	}

	bool oal_has_efx()
	{
		if (!::alcIsExtensionPresent(oal_device_, ALC_EXT_EFX_NAME))
		{
			return false;
		}

		return true;
	}

	bool oal_detect_effect(
		const ALenum effect_type)
	{
		oal_clear_error();

		auto has_effect = false;
		auto effect_name = ALuint{};

		alGenEffects_(1, &effect_name);

		if (oal_is_succeed())
		{
			alEffecti_(effect_name, AL_EFFECT_TYPE, effect_type);

			if (oal_is_succeed())
			{
				has_effect = true;
			}
		}

		if (alIsEffect_(effect_name))
		{
			alDeleteEffects_(1, &effect_name);
		}

		return has_effect;
	}

	void oal_detect_effects()
	{
		oal_has_chorus_effect_ = oal_detect_effect(AL_EFFECT_CHORUS);
		oal_has_compressor_effect_ = oal_detect_effect(AL_EFFECT_COMPRESSOR);
		oal_has_distortion_effect_ = oal_detect_effect(AL_EFFECT_DISTORTION);
		oal_has_echo_effect_ = oal_detect_effect(AL_EFFECT_ECHO);
		oal_has_flange_effect_ = oal_detect_effect(AL_EFFECT_FLANGER);
		oal_has_equalizer_effect_ = oal_detect_effect(AL_EFFECT_EQUALIZER);
		oal_has_reverb_effect_ = oal_detect_effect(AL_EFFECT_REVERB);
		oal_has_eax_reverb_effect_ = oal_detect_effect(AL_EFFECT_EAXREVERB);
	}

	void oal_examine_efx()
	{
		auto is_succeed = true;

		auto guard_this = ul::ScopeGuard{
			[&]()
			{
				if (!is_succeed)
				{
					oal_clear_efx();
				}
			}
		};

		if (!oal_get_efx_version())
		{
			return;
		}

		if (!oal_get_max_aux_sends())
		{
			return;
		}

		if (!oal_get_efx_symbols())
		{
			return;
		}

		oal_detect_effects();

		is_succeed = true;
		oal_has_efx_ = true;
	}

	void oal_clear_efx()
	{
		oal_efx_version_ = {};

		oal_has_efx_ = false;

		oal_has_chorus_effect_ = false;
		oal_has_compressor_effect_ = false;
		oal_has_distortion_effect_ = false;
		oal_has_echo_effect_ = false;
		oal_has_flange_effect_ = false;
		oal_has_equalizer_effect_ = false;
		oal_has_reverb_effect_ = false;
		oal_has_eax_reverb_effect_ = false;
		oal_max_aux_sends_ = 0;

		oal_clear_efx_functions();
	}

	bool oal_open_device()
	{
		oal_device_ = ::alcOpenDevice(nullptr);

		if (!oal_device_)
		{
			error_message_ = "OAL: Failed to open a device.";
			return false;
		}

		return true;
	}

	void oal_close_device()
	{
		if (oal_device_)
		{
			static_cast<void>(::alcCloseDevice(oal_device_));
			oal_device_ = nullptr;
		}
	}

	bool oal_create_context()
	{
		static const ALint attribs[] =
		{
			ALC_MAX_AUXILIARY_SENDS, default_aux_sends,
			0, 0,
		}; // attribs

		oal_context_ = ::alcCreateContext(oal_device_, attribs);

		if (!oal_context_)
		{
			error_message_ = "OAL: Failed to create a context.";
			return false;
		}

		if (!::alcMakeContextCurrent(oal_context_))
		{
			error_message_ = "OAL: Failed to make a context current.";
			return false;
		}

		return true;
	}

	void oal_destroy_context()
	{
		if (oal_context_)
		{
			static_cast<void>(::alcMakeContextCurrent(nullptr));
			::alcDestroyContext(oal_context_);
			oal_context_ = nullptr;
		}
	}

	//
	// =========================================================================
	// OpenAL
	// =========================================================================


	// =========================================================================
	// API utils
	// =========================================================================
	//

	void update_sample_volume_and_pan(
		Sample& sample)
	{
		for (auto i = 0; i < 2; ++i)
		{
			auto& oal_source = sample.oal_sources_[i];

			const auto gain = sample.oal_volume_ * sample.oal_pans_[i];

			::alSourcef(oal_source, AL_GAIN, gain);
		}
	}

	void remove_samples()
	{
		for (auto& sample : samples_2d_)
		{
			destroy_sample(sample);
		}

		samples_2d_= {};


		for (auto& object_3d : objects_3d_)
		{
			destroy_sample(object_3d.sample_);
		}

		objects_3d_ = {};
	}

	//
	// =========================================================================
	// API utils
	// =========================================================================


	// =========================================================================
	// API
	// =========================================================================
	//

	bool startup()
	{
#ifdef USE_EAX20_HARDWARE_FILTERS
		is_eax20_filters_defined_ = true;
#else
		is_eax20_filters_defined_ = false;
#endif // USE_EAX20_HARDWARE_FILTERS

		clock_base_ = Clock::now();
		listener_3d_.is_listener_ = true;

		return true;
	}

	void shutdown()
	{
	}

	std::uint32_t get_milliseconds()
	{
		constexpr auto max_uint32_t = std::numeric_limits<std::uint32_t>::max();

		const auto time_diff = Clock::now() - clock_base_;
		const auto time_diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();

		return static_cast<std::uint32_t>(time_diff_ms % max_uint32_t);
	}

	void* allocate(
		const std::size_t size)
	{
		return new (std::nothrow) char[size];
	}

	void deallocate(
		void* ptr)
	{
		delete[] static_cast<char*>(ptr);
	}

	bool wave_out_open_internal()
	{
		auto is_succeed = false;

		auto guard_this = ul::ScopeGuard{
			[&]()
			{
				if (!is_succeed)
				{
					wave_out_close_internal();
				}
			}
		};

		if (!oal_open_device())
		{
			return false;
		}

		if (!oal_create_context())
		{
			return false;
		}

		if (!oal_get_version())
		{
			return false;
		}

		if (!oal_get_global_strings())
		{
			return false;
		}

		oal_examine_efx();

		initialize_eax20_filter();

		create_streams();

		is_succeed = true;
		master_volume_ = ltjs::AudioUtils::lt_max_volume;
		reset_3d_object(listener_3d_);

		return true;
	}

	sint32 wave_out_open(
		LHDIGDRIVER& driver,
		PHWAVEOUT& wave_out,
		const sint32 device_id,
		const ul::WaveFormatEx& wave_format)
	{
		static_cast<void>(device_id);
		static_cast<void>(wave_format);

		driver = nullptr;
		wave_out = nullptr;

		wave_out_close_internal();

		if (!wave_out_open_internal())
		{
			return LS_ERROR;
		}

		driver = oal_device_;

		return LS_OK;
	}

	void wave_out_close_internal()
	{
		uninitialize_eax20_filter();

		destroy_streams();

		remove_samples();

		oal_destroy_context();
		oal_close_device();
		oal_clear_efx();

		master_volume_ = {};
	}

	void wave_out_close(
		LHDIGDRIVER driver_ptr)
	{
		static_cast<void>(driver_ptr);

		wave_out_close_internal();
	}

	sint32 get_master_volume(
		LHDIGDRIVER driver_ptr) const
	{
		if (!driver_ptr || driver_ptr != oal_device_)
		{
			return {};
		}

		return master_volume_;
	}

	void set_master_volume(
		LHDIGDRIVER driver_ptr,
		const sint32 master_volume)
	{
		if (!driver_ptr || driver_ptr != oal_device_)
		{
			return;
		}

		const auto new_master_volume = ltjs::AudioUtils::clamp_lt_volume(master_volume);

		if (master_volume_ == new_master_volume)
		{
			return;
		}

		master_volume_ = new_master_volume;


		const auto oal_gain =
			static_cast<ALfloat>(new_master_volume - ltjs::AudioUtils::lt_min_volume) /
			static_cast<ALfloat>(ltjs::AudioUtils::lt_max_volume_delta);

		::alListenerf(AL_GAIN, oal_gain);
	}

	LHSAMPLE allocate_sample(
		LHDIGDRIVER driver_ptr)
	{
		if (driver_ptr != oal_device_)
		{
			return nullptr;
		}

		samples_2d_.emplace_back();

		auto& sample = samples_2d_.back();
		sample.is_3d_ = false;
		sample.is_stream_ = false;
		sample.oal_source_count_ = 2;

		create_sample(sample);

		return &sample;
	}

	void deallocate_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		destroy_sample(sample);

		samples_2d_.remove_if(
			[=](const auto& sample)
			{
				return &sample == sample_ptr;
			}
		);
	}

	void stop_sample(
		Sample& sample)
	{
		const auto status = get_sample_status(sample);

		switch (status)
		{
		case Sample::Status::failed:
		case Sample::Status::paused:
		case Sample::Status::stopped:
			return;

		default:
			break;
		}

		::alSourcePausev(sample.oal_source_count_, sample.oal_sources_.data());

		sample.status_ = Sample::Status::paused;
	}

	void stop_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		stop_sample(sample);
	}

	void start_sample(
		Sample& sample)
	{
		const auto status = get_sample_status(sample);

		if (status == Sample::Status::failed)
		{
			return;
		}

		::alSourcePlayv(sample.oal_source_count_, sample.oal_sources_.data());

		sample.status_ = Sample::Status::playing;
	}

	void start_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		start_sample(sample);
	}

	void resume_sample(
		Sample& sample)
	{
		const auto status = get_sample_status(sample);

		switch (status)
		{
		case Sample::Status::failed:
		case Sample::Status::playing:
			return;

		default:
			break;
		}

		::alSourcePlayv(sample.oal_source_count_, sample.oal_sources_.data());

		sample.status_ = Sample::Status::playing;
	}

	void resume_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		resume_sample(sample);
	}

	void end_sample(
		Sample& sample)
	{
		const auto status = get_sample_status(sample);

		switch (status)
		{
		case Sample::Status::failed:
		case Sample::Status::stopped:
			return;

		default:
			break;
		}

		::alSourceStopv(sample.oal_source_count_, sample.oal_sources_.data());

		sample.status_ = Sample::Status::stopped;
	}

	void end_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		end_sample(sample);
	}

	void set_sample_volume(
		Sample& sample,
		const sint32 volume)
	{
		const auto new_volume = ltjs::AudioUtils::clamp_lt_volume(volume);

		if (sample.volume_ == new_volume)
		{
			return;
		}

		sample.volume_ = new_volume;

		if (sample.status_ == Sample::Status::failed)
		{
			return;
		}

		const auto oal_volume = ltjs::AudioUtils::lt_volume_to_gain(new_volume);

		sample.oal_volume_ = oal_volume;

		update_sample_volume_and_pan(sample);
	}

	sint32 get_sample_volume(
		LHSAMPLE sample_ptr) const
	{
		if (!sample_ptr)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		return sample.volume_;
	}

	void set_sample_volume(
		LHSAMPLE sample_ptr,
		const sint32 volume)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		set_sample_volume(sample, volume);
	}

	void set_sample_pan(
		Sample& sample,
		const sint32 pan)
	{
		const auto new_pan = ltjs::AudioUtils::clamp_lt_pan(pan);

		if (sample.pan_ == new_pan)
		{
			return;
		}

		sample.pan_ = new_pan;

		if (sample.status_ == Sample::Status::failed)
		{
			return;
		}

		const auto oal_pan = ltjs::AudioUtils::lt_pan_to_gain(new_pan);

		if (new_pan == ltjs::AudioUtils::lt_pan_center)
		{
			// Left and right channels are at full volume.

			sample.oal_pans_.fill(ltjs::AudioUtils::gain_max);
		}
		else if (new_pan < ltjs::AudioUtils::lt_pan_center)
		{
			// Left channel is at full volume; right channels is attenuated.

			sample.oal_pans_[Sample::left_index] = ltjs::AudioUtils::gain_max;
			sample.oal_pans_[Sample::right_index] = std::abs(oal_pan);
		}
		else
		{
			// Right channel is at full volume; lrft channels is attenuated.

			sample.oal_pans_[Sample::left_index] = std::abs(oal_pan);
			sample.oal_pans_[Sample::right_index] = ltjs::AudioUtils::gain_max;
		}

		update_sample_volume_and_pan(sample);
	}

	sint32 get_sample_pan(
		LHSAMPLE sample_ptr) const
	{
		if (!sample_ptr)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		return sample.pan_;
	}

	void set_sample_pan(
		LHSAMPLE sample_ptr,
		const sint32 pan)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		set_sample_pan(sample, pan);
	}

	sint32 get_sample_user_data(
		LHSAMPLE sample_handle,
		const uint32 index) const
	{
		if (!sample_handle || index > max_user_data_index)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		return sample.user_data_array_[index];
	}

	void set_sample_user_data(
		LHSAMPLE sample_handle,
		const uint32 index,
		const sint32 value)
	{
		if (!sample_handle || index > max_user_data_index)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		sample.user_data_array_[index] = value;
	}

	void reset_sample(
		Sample& sample)
	{
		sample.format_ = {};

		sample.is_looping_ = {};
		sample.has_loop_block_ = {};
		sample.loop_begin_ = {};
		sample.loop_end_ = {};
		sample.volume_ = ltjs::AudioUtils::lt_max_volume;
		sample.pan_ = pan_center;

		sample.oal_buffer_format_ = AL_NONE;
		sample.oal_volume_ = ltjs::AudioUtils::gain_max;
		sample.oal_pans_.fill(ltjs::AudioUtils::gain_max);
	}

	static bool validate_wave_format_ex(
		const ul::WaveFormatEx& wave_format)
	{
		if (wave_format.tag_ != ul::WaveFormatTag::pcm)
		{
			return false;
		}

		if (wave_format.channel_count_ != 1 && wave_format.channel_count_ != 2)
		{
			return false;
		}

		if (wave_format.bit_depth_ != 8 && wave_format.bit_depth_ != 16)
		{
			return false;
		}

		if (wave_format.sample_rate_ <= 0)
		{
			return false;
		}

		return true;
	}

	static ALenum get_oal_buffer_format(
		const ul::WaveFormatEx& wave_format)
	{
		switch (wave_format.channel_count_)
		{
		case 1:
			switch (wave_format.bit_depth_)
			{
			case 8:
				return AL_FORMAT_MONO8;

			case 16:
				return AL_FORMAT_MONO16;

			default:
				return AL_NONE;
			}

			break;


		case 2:
			switch (wave_format.bit_depth_)
			{
			case 8:
				return AL_FORMAT_STEREO8;

			case 16:
				return AL_FORMAT_STEREO16;

			default:
				return AL_NONE;
			}

			break;

		default:
			return AL_NONE;
		}
	}

	void create_sample(
		Sample& sample)
	{
		oal_clear_error();

		::alGenBuffers(Sample::oal_max_buffer_count, sample.oal_buffers_.data());
		::alGenSources(sample.oal_source_count_, sample.oal_sources_.data());

		if (!oal_is_succeed())
		{
			destroy_sample(sample);

			sample.status_ = Sample::Status::failed;

			return;
		}

		sample.status_ = Sample::Status::none;
	}

	void destroy_sample(
		Sample& sample)
	{
		for (auto& oal_source : sample.oal_sources_)
		{
			if (oal_source != AL_NONE)
			{
				::alSourceStop(oal_source);
				::alSourcei(oal_source, AL_BUFFER, AL_NONE);
				::alDeleteSources(1, &oal_source);
				oal_source = AL_NONE;
			}
		}

		for (auto& oal_buffer : sample.oal_buffers_)
		{
			if (oal_buffer != AL_NONE)
			{
				::alDeleteBuffers(1, &oal_buffer);
				oal_buffer = AL_NONE;
			}
		}

		sample.status_ = Sample::Status::none;
		sample.oal_source_count_ = 0;
	}

	bool initialize_sample_from_address_generic(
		Sample& sample,
		const void* ptr,
		const uint32 length,
		const ul::WaveFormatEx& wave_format,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr)
	{
		static_cast<void>(filter_data_ptr);

		if (sample.status_ == Sample::Status::failed)
		{
			return false;
		}

		reset_sample(sample);

		if ((!sample.is_stream_ && length == 0) || !validate_wave_format_ex(wave_format) || playback_rate <= 0)
		{
			return false;
		}

		if (sample.is_3d_ && wave_format.channel_count_ != 1)
		{
			return false;
		}

		if (!sample.is_stream_ && !ptr && sample.data_.empty())
		{
			return false;
		}

		sample.format_ = wave_format;

		const auto has_pitch = (static_cast<int>(sample.format_.sample_rate_) != playback_rate);

		const auto pitch = (
			has_pitch ?
			static_cast<float>(playback_rate) / static_cast<float>(sample.format_.sample_rate_) :
			1.0F);

		sample.oal_buffer_format_ = get_oal_buffer_format(wave_format);

		if (ptr)
		{
			sample.data_.resize(length);

			std::uninitialized_copy_n(
				static_cast<const std::uint8_t*>(ptr),
				length,
				sample.data_.begin());
		}

		oal_clear_error();

		::alSourceStopv(sample.oal_source_count_, sample.oal_sources_.data());

		for (auto i = 0; i < sample.oal_source_count_; ++i)
		{
			::alSourcei(sample.oal_sources_[i], AL_BUFFER, AL_NONE);
		}

		if (sample.is_stream_)
		{
			auto processed_buffers = Sample::OalBuffers{};

			for (auto i = 0; i < sample.oal_source_count_; ++i)
			{
				const auto oal_source = sample.oal_sources_[i];

				auto processed_count = ALint{};

				::alGetSourcei(oal_source, AL_BUFFERS_PROCESSED, &processed_count);

				if (processed_count > 0)
				{
					::alSourceUnqueueBuffers(oal_source, processed_count, processed_buffers.data());
				}
			}
		}

		if (!sample.is_stream_)
		{
			::alBufferData(
				sample.oal_buffers_[0],
				sample.oal_buffer_format_,
				sample.data_.data(),
				static_cast<ALsizei>(sample.data_.size()),
				static_cast<ALsizei>(sample.format_.sample_rate_));
		}

		for (auto i = 0; i < sample.oal_source_count_; ++i)
		{
			const auto oal_source = sample.oal_sources_[i];

			if (!sample.is_stream_)
			{
				::alSourcei(oal_source, AL_BUFFER, sample.oal_buffers_[0]);
			}

			if (has_pitch)
			{
				::alSourcef(oal_source, AL_PITCH, pitch);
			}
			else
			{
				::alSourcef(oal_source, AL_PITCH, 1.0F);
			}

			if (sample.is_3d_)
			{
				::alSourcef(oal_source, AL_GAIN, ltjs::AudioUtils::gain_max);
			}

			::alSourcei(oal_source, AL_LOOPING, AL_FALSE);

			::alSourcef(oal_source, AL_REFERENCE_DISTANCE, ltjs::AudioUtils::ds_default_min_distance);
			::alSourcef(oal_source, AL_MAX_DISTANCE, ltjs::AudioUtils::ds_default_max_distance);
		}

		if (!sample.is_3d_)
		{
			update_sample_volume_and_pan(sample);

			for (auto i = 0; i < 2; ++i)
			{
				::alSourcei(sample.oal_sources_[i], AL_SOURCE_RELATIVE, AL_TRUE);
			}

			// Left channel.
			::alSource3f(sample.oal_sources_[Sample::left_index], AL_POSITION, -1.0F, 0.0F, 0.0F);

			// Right channel.
			::alSource3f(sample.oal_sources_[Sample::right_index], AL_POSITION, 1.0F, 0.0F, 0.0F);
		}

		if (oal_is_supports_eax20_filter_)
		{
			for (auto i_source = 0; i_source < sample.oal_source_count_; ++i_source)
			{
				const auto oal_source = sample.oal_sources_[i_source];

				::alSource3i(oal_source, AL_AUXILIARY_SEND_FILTER, oal_effect_slot_, 0, AL_FILTER_NULL);
			}
		}

		if (!oal_is_succeed())
		{
			sample.status_ = Sample::Status::failed;
			return false;
		}

		sample.status_ = Sample::Status::stopped;

		return true;
	}

	sint32 initialize_sample_generic(
		LHSAMPLE sample_handle,
		const void* ptr,
		const uint32 length,
		const ul::WaveFormatEx& wave_format,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr)
	{
		if (!sample_handle)
		{
			return false;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		if (sample.status_ == Sample::Status::failed)
		{
			return false;
		}

		if (!initialize_sample_from_address_generic(
			sample,
			ptr,
			length,
			wave_format,
			playback_rate,
			filter_data_ptr))
		{
			return false;
		}

		return true;
	}

	bool initialize_sample_from_file(
		Sample& sample,
		const void* file_image_ptr,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr)
	{
		static_cast<void>(filter_data_ptr);

		if (!file_image_ptr || playback_rate <= 0)
		{
			return false;
		}

		if (sample.status_ == Sample::Status::failed)
		{
			return false;
		}

		const auto wave_size = ltjs::AudioUtils::extract_wave_size(file_image_ptr);

		if (wave_size <= 0)
		{
			return false;
		}

		auto memory_stream = ul::MemoryStream{file_image_ptr, wave_size};

		if (!memory_stream.is_open())
		{
			return false;
		}

		if (!audio_decoder_.open(&memory_stream))
		{
			return false;
		}

		const auto data_size = audio_decoder_.get_data_size();

		if (data_size <= 0)
		{
			return false;
		}

		sample.data_.resize(data_size);

		const auto decoded_size = audio_decoder_.decode(sample.data_.data(), data_size);

		if (decoded_size <= 0)
		{
			return false;
		}

		sample.data_.resize(decoded_size);

		const auto wave_format = audio_decoder_.get_wave_format_ex();

		const auto result = initialize_sample_from_address_generic(
			sample,
			nullptr,
			decoded_size,
			wave_format,
			playback_rate,
			filter_data_ptr);

		return result;
	}

	bool initialize_sample_from_file(
		LHSAMPLE sample_handle,
		const void* file_image_ptr,
		const sint32 block,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr)
	{
		static_cast<void>(block);

		if (!sample_handle)
		{
			return false;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		return initialize_sample_from_file(sample, file_image_ptr, playback_rate, filter_data_ptr);
	}

	void set_sample_loop_block(
		Sample& sample,
		const sint32 loop_start_offset,
		const sint32 loop_end_offset,
		const bool is_enable)
	{
		const auto data_size = static_cast<int>(sample.data_.size());
		const auto sample_size = sample.format_.block_align_;

		auto new_start = loop_start_offset;

		if (new_start <= 0)
		{
			new_start = 0;
		}

		new_start /= sample_size;
		new_start *= sample_size;

		auto new_end = loop_end_offset;

		if (new_end < 0)
		{
			new_end = data_size;
		}

		new_end /= sample_size;
		new_end *= sample_size;

		sample.has_loop_block_ = is_enable;
		sample.loop_begin_ = new_start;
		sample.loop_end_ = new_end;

		if ((new_start == 0 && new_end == data_size) || new_start > new_end)
		{
			sample.has_loop_block_ = false;
		}
	}

	void set_sample_loop_block(
		LHSAMPLE sample_handle,
		const sint32 loop_start_offset,
		const sint32 loop_end_offset,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		set_sample_loop_block(sample, loop_start_offset, loop_end_offset, is_enable);
	}

	void set_sample_loop(
		Sample& sample,
		const bool is_enable)
	{
		if (sample.is_looping_ == is_enable)
		{
			return;
		}

		sample.is_looping_ = is_enable;

		if (sample.status_ == Sample::Status::failed)
		{
			return;
		}

		const auto status_before_stop = get_sample_status(sample);

		if (status_before_stop == Sample::Status::failed)
		{
			return;
		}

		oal_clear_error();

		::alSourceStopv(sample.oal_source_count_, sample.oal_sources_.data());

		auto sample_offset = ALint{-1};

		::alGetSourcei(sample.oal_sources_[0], AL_SAMPLE_OFFSET, &sample_offset);

		if (sample_offset < 0)
		{
			sample.status_ = Sample::Status::failed;
			return;
		}

		for (auto i = 0; i < sample.oal_source_count_; ++i)
		{
			::alSourcei(sample.oal_sources_[i], AL_BUFFER, AL_NONE);
		}

		if (is_enable)
		{
			if (sample.has_loop_block_)
			{
				::alBufferData(
					sample.oal_buffers_[1],
					sample.oal_buffer_format_,
					sample.data_.data(),
					static_cast<ALsizei>(sample.loop_begin_),
					static_cast<ALsizei>(sample.format_.sample_rate_));

				::alBufferData(
					sample.oal_buffers_[2],
					sample.oal_buffer_format_,
					&sample.data_[sample.loop_begin_],
					static_cast<ALsizei>(sample.loop_end_ - sample.loop_begin_),
					static_cast<ALsizei>(sample.format_.sample_rate_));

				for (auto i = 0; i < sample.oal_source_count_; ++i)
				{
					::alSourceQueueBuffers(sample.oal_sources_[i], 2, &sample.oal_buffers_[1]);
					::alSourcei(sample.oal_sources_[i], AL_SAMPLE_OFFSET, sample_offset);
				}
			}
			else
			{
				for (auto i = 0; i < sample.oal_source_count_; ++i)
				{
					::alSourcei(sample.oal_sources_[i], AL_BUFFER, sample.oal_buffers_[0]);
				}
			}

			for (auto i = 0; i < sample.oal_source_count_; ++i)
			{
				::alSourcei(sample.oal_sources_[i], AL_LOOPING, AL_TRUE);
			}
		}
		else
		{
			for (auto i = 0; i < sample.oal_source_count_; ++i)
			{
				::alSourcei(sample.oal_sources_[i], AL_BUFFER, sample.oal_buffers_[0]);
				::alSourcei(sample.oal_sources_[i], AL_LOOPING, AL_FALSE);
				::alSourcei(sample.oal_sources_[i], AL_SAMPLE_OFFSET, sample_offset);
			}
		}

		if (status_before_stop == Sample::Status::playing)
		{
			::alSourcePlayv(2, sample.oal_sources_.data());
		}

		if (!oal_is_succeed())
		{
			sample.status_ = Sample::Status::failed;
			return;
		}

		if (status_before_stop == Sample::Status::playing)
		{
			sample.status_ = Sample::Status::playing;
		}
		else
		{
			sample.status_ = Sample::Status::paused;
		}
	}

	void set_sample_loop(
		LHSAMPLE sample_handle,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		set_sample_loop(sample, is_enable);
	}

	void set_sample_ms_position(
		Sample& sample,
		const sint32 milliseconds)
	{
		if (sample.status_ == Sample::Status::failed)
		{
			return;
		}

		const auto sample_offset = static_cast<int>((sample.format_.sample_rate_ * milliseconds) / 1000LL);
		const auto data_offset = sample_offset * sample.format_.block_align_;
		const auto data_size = static_cast<int>(sample.data_.size());

		if (data_offset > data_size)
		{
			return;
		}

		for (auto i = 0; i < sample.oal_source_count_; ++i)
		{
			::alSourcei(sample.oal_sources_[i], AL_SAMPLE_OFFSET, sample_offset);
		}
	}

	void set_sample_ms_position(
		LHSAMPLE sample_handle,
		const sint32 milliseconds)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		set_sample_ms_position(sample, milliseconds);
	}

	Sample::Status get_sample_status(
		Sample& sample)
	{
		switch (sample.status_)
		{
		case Sample::Status::failed:
		case Sample::Status::paused:
		case Sample::Status::stopped:
			return sample.status_;

		default:
			break;
		}

		auto oal_state = ALint{};

		::alGetSourcei(sample.oal_sources_[Sample::left_index], AL_SOURCE_STATE, &oal_state);

		auto status = Sample::Status::none;

		switch (oal_state)
		{
		case AL_INITIAL:
		case AL_STOPPED:
			status = Sample::Status::stopped;
			break;

		case AL_PAUSED:
			status = Sample::Status::paused;
			break;

		case AL_PLAYING:
			status = Sample::Status::playing;
			break;

		default:
			status = Sample::Status::failed;
			break;
		}

		sample.status_ = status;

		return status;
	}

	uint32 get_sample_status(
		LHSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		const auto status = get_sample_status(sample);

		return status == Sample::Status::playing ? LS_PLAYING : LS_STOPPED;
	}

	LHSTREAM open_stream(
		const char* file_name,
		const uint32 file_offset,
		LHDIGDRIVER driver_ptr,
		const char* stream_ptr,
		const sint32 stream_memory_size)
	{
		static_cast<void>(stream_ptr);
		static_cast<void>(stream_memory_size);

		if (!file_name || driver_ptr != oal_device_ || streams_.empty())
		{
			return nullptr;
		}

		auto free_stream_ptr = get_free_stream();

		if (!free_stream_ptr)
		{
			return nullptr;
		}

		auto& stream = *free_stream_ptr;

		if (!stream.file_stream_.open(file_name, ul::Stream::OpenMode::read))
		{
			return nullptr;
		}

		if (!stream.file_substream_.open(&stream.file_stream_, file_offset))
		{
			return nullptr;
		}

		auto& decoder = stream.decoder_;

		if (!decoder.open(&stream.file_substream_))
		{
			return nullptr;
		}

		reset_stream(stream);

		stream.data_size_ = decoder.get_data_size();

		const auto sample_size = decoder.get_sample_size();
		const auto sample_rate = decoder.get_sample_rate();
		const auto mix_sample_count = (Stream::mix_size_ms * sample_rate) / 1000;
		stream.mix_size_ = mix_sample_count * sample_size;

		stream.mix_buffer_.resize(stream.mix_size_);

		auto format = decoder.get_wave_format_ex();

		auto& sample = stream.sample_;

		const auto initialize_sample_result = initialize_sample_from_address_generic(
			sample,
			nullptr,
			0,
			format,
			sample_rate,
			nullptr);

		if (!initialize_sample_result)
		{
			return nullptr;
		}

		stream.is_open_ = true;

		MtMutexGuard stream_lock{mt_stream_mutex_};

		mt_open_streams_.emplace_back(&stream);
		mt_notify_stream();

		return &stream;
	}

	void close_stream(
		LHSTREAM stream_ptr)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		{
			MtMutexGuard mt_stream_lock{mt_stream_mutex_};

			stream.is_open_ = false;

			mt_open_streams_.remove_if(
				[&](const auto& item)
				{
					return item == &stream;
				}
			);
		}

		::alSourceStopv(2, stream.sample_.oal_sources_.data());
	}

	void set_stream_loop(
		LHSTREAM stream_ptr,
		const bool is_loop)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.sample_.is_looping_ = is_loop;
	}

	void set_stream_position_ms(
		LHSTREAM stream_ptr,
		const sint32 milliseconds)
	{
		if (!stream_ptr || milliseconds < 0)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		const auto& format = stream.sample_.format_;

		auto position = static_cast<int>((milliseconds * format.sample_rate_) / 1000LL);
		position /= format.block_align_;
		position *= format.block_align_;

		if (position > stream.data_size_)
		{
			return;
		}

		stream.data_offset_ = position;
	}

	void set_stream_user_data(
		LHSTREAM stream_ptr,
		const uint32 index,
		const sint32 value)
	{
		if (!stream_ptr || index > max_user_data_index)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		stream.sample_.user_data_array_[index] = value;
	}

	sint32 get_stream_user_data(
		LHSTREAM stream_ptr,
		const uint32 index)
	{
		if (!stream_ptr || index > max_user_data_index)
		{
			return {};
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		return stream.sample_.user_data_array_[index];
	}

	void start_stream(
		LHSTREAM stream_ptr)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.is_playing_ = true;
		stream.data_offset_ = 0;

		mt_notify_stream();
	}

	void pause_stream(
		LHSTREAM stream_ptr,
		const sint32 is_pause)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.is_playing_ = (is_pause != 0);

		if (stream.is_playing_)
		{
			mt_notify_stream();
		}
	}

	void set_stream_volume(
		LHSTREAM stream_ptr,
		const sint32 volume)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		set_sample_volume(stream.sample_, volume);
	}

	void set_stream_pan(
		LHSTREAM stream_ptr,
		const sint32 pan)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		set_sample_pan(stream.sample_, pan);
	}

	sint32 set_stream_volume(
		LHSTREAM stream_ptr)
	{
		if (!stream_ptr)
		{
			return {};
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		return stream.sample_.volume_;
	}

	sint32 get_stream_pan(
		LHSTREAM stream_ptr)
	{
		if (!stream_ptr)
		{
			return {};
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		return stream.sample_.pan_;
	}

	uint32 get_stream_status(
		LHSTREAM stream_ptr)
	{
		if (!stream_ptr)
		{
			return LS_ERROR;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		if (stream.sample_.status_ != Sample::Status::playing)
		{
			return LS_STOPPED;
		}

		return LS_PLAYING;
	}

	sint32 decode_mp3(
		const void* srd_data_ptr,
		const uint32 src_size,
		const char* file_name_ext,
		void*& dst_wav,
		uint32& dst_wav_size,
		LTLENGTHYCB callback)
	{
		static_cast<void>(file_name_ext);
		static_cast<void>(callback);

		return ltjs::AudioUtils::decode_mp3(audio_decoder_, srd_data_ptr, src_size, dst_wav, dst_wav_size);
	}

	void reset_3d_object(
		Object3d& object_3d)
	{
		auto& sample = object_3d.sample_;
		const auto oal_source = sample.oal_sources_[0];

		object_3d.reset();
		oal_clear_error();


		if (!object_3d.is_listener_)
		{
			::alSourceStopv(1, sample.oal_sources_.data());
			::alSourcei(oal_source, AL_BUFFER, AL_NONE);
			::alSourcei(oal_source, AL_SOURCE_RELATIVE, AL_FALSE);
		}

		// Doppler factor.
		//
		if (object_3d.is_listener_)
		{
			::alDopplerFactor(object_3d.doppler_factor_);
		}

		// Minimum/Maximum distance.
		//
		if (!object_3d.is_listener_)
		{
			::alSourcef(oal_source, AL_REFERENCE_DISTANCE, object_3d.min_distance_);
			::alSourcef(oal_source, AL_MAX_DISTANCE, object_3d.max_distance_);
		}

		// Position.
		//
		const auto oal_position = Vector3d::to_oal(object_3d.position_);
		const auto oal_position_ptr = reinterpret_cast<const ALfloat*>(&oal_position);

		if (object_3d.is_listener_)
		{
			::alListenerfv(AL_POSITION, oal_position_ptr);
		}
		else
		{
			::alSourcefv(oal_source, AL_POSITION, oal_position_ptr);
		}


		// Velocity.
		//
		const auto oal_velocity = Vector3d::to_oal(object_3d.velocity_);
		const auto oal_velocity_ptr = reinterpret_cast<const ALfloat*>(&oal_velocity);

		if (object_3d.is_listener_)
		{
			::alListenerfv(AL_VELOCITY, oal_velocity_ptr);
		}
		else
		{
			::alSourcefv(oal_source, AL_VELOCITY, oal_velocity_ptr);
		}


		// Orientaion.
		//
		if (object_3d.is_listener_)
		{
			const auto oal_orientation = Orientation3d::to_oal(object_3d.orientation_);
			const auto oal_orientation_ptr = reinterpret_cast<const ALfloat*>(&oal_orientation);

			::alListenerfv(AL_ORIENTATION, oal_orientation_ptr);
		}

		if (!oal_is_succeed())
		{
			sample.status_ = Sample::Status::failed;
			return;
		}
	}

	void get_3d_provider_attribute(
		LHPROVIDER provider_id,
		const char* name,
		void* value)
	{
		if (!value)
		{
			return;
		}

		auto& int_value = *static_cast<sint32*>(value);

		int_value = -1;

		if (!name)
		{
			return;
		}

		const auto name_string = std::string{name};

		if (name_string != "Max samples")
		{
			return;
		}

		int_value = 255;
	}

	LH3DPOBJECT open_3d_listener(
		LHPROVIDER provider_id)
	{
		static_cast<void>(provider_id);

		return &listener_3d_;
	}

	void close_3d_listener(
		LH3DPOBJECT listener_ptr)
	{
		static_cast<void>(listener_ptr);
	}

	void set_3d_listener_doppler(
		LH3DPOBJECT listener_ptr,
		const float doppler)
	{
		if (listener_ptr != &listener_3d_)
		{
			return;
		}

		if (listener_3d_.doppler_factor_ == doppler)
		{
			return;
		}

		listener_3d_.doppler_factor_ = doppler;

		::alDopplerFactor(doppler);
	}

	void set_3d_position(
		LH3DPOBJECT object_ptr,
		const float x,
		const float y,
		const float z)
	{
		if (!object_ptr)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		const auto new_position = Vector3d{x, y, z};

		if (object_3d.position_ == new_position)
		{
			return;
		}

		object_3d.position_ = new_position;

		const auto oal_position = Vector3d::to_oal(new_position);
		const auto oal_position_ptr = reinterpret_cast<const ALfloat*>(&oal_position);

		if (object_3d.is_listener_)
		{
			::alListenerfv(AL_POSITION, oal_position_ptr);
		}
		else
		{
			::alSourcefv(object_3d.sample_.oal_sources_[0], AL_POSITION, oal_position_ptr);
		}
	}

	void set_3d_velocity(
		LH3DPOBJECT object_ptr,
		const float dx_per_s,
		const float dy_per_s,
		const float dz_per_s)
	{
		if (!object_ptr)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		const auto new_velocity = Vector3d{dx_per_s, dy_per_s, dz_per_s};

		if (object_3d.velocity_ == new_velocity)
		{
			return;
		}

		object_3d.velocity_ = new_velocity;

		const auto oal_velocity = Vector3d::to_oal(new_velocity);
		const auto oal_velocity_ptr = reinterpret_cast<const ALfloat*>(&oal_velocity);

		if (object_3d.is_listener_)
		{
			::alListenerfv(AL_VELOCITY, oal_velocity_ptr);
		}
		else
		{
			::alSourcefv(object_3d.sample_.oal_sources_[0], AL_VELOCITY, oal_velocity_ptr);
		}
	}

	void set_3d_orientation(
		LH3DPOBJECT object_ptr,
		const float x_face,
		const float y_face,
		const float z_face,
		const float x_up,
		const float y_up,
		const float z_up)
	{
		if (!object_ptr)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		const auto new_direction_ = Orientation3d{{x_face, y_face, z_face}, {x_up, y_up, z_up}};

		if (object_3d.orientation_ == new_direction_)
		{
			return;
		}

		object_3d.orientation_ = new_direction_;

		const auto oal_orientation = Orientation3d::to_oal(new_direction_);

		::alListenerfv(AL_ORIENTATION, reinterpret_cast<const ALfloat*>(&oal_orientation));
	}

	void get_3d_position(
		LH3DPOBJECT object_ptr,
		float& x,
		float& y,
		float& z)
	{
		x = 0.0F;
		y = 0.0F;
		z = 0.0F;

		if (!object_ptr)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		x = object_3d.position_.x_;
		y = object_3d.position_.y_;
		z = object_3d.position_.z_;
	}

	void get_3d_velocity(
		LH3DPOBJECT object_ptr,
		float& dx_per_ms,
		float& dy_per_ms,
		float& dz_per_ms)
	{
		dx_per_ms = 0.0F;
		dy_per_ms = 0.0F;
		dz_per_ms = 0.0F;

		if (!object_ptr)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		dx_per_ms = object_3d.velocity_.x_;
		dy_per_ms = object_3d.velocity_.y_;
		dz_per_ms = object_3d.velocity_.z_;
	}

	void get_3d_orientation(
		LH3DPOBJECT object_ptr,
		float& x_face,
		float& y_face,
		float& z_face,
		float& x_up,
		float& y_up,
		float& z_up)
	{
		x_face = 0.0F;
		y_face = 0.0F;
		z_face = 0.0F;

		x_up = 0.0F;
		y_up = 0.0F;
		z_up = 0.0F;

		if (!object_ptr)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		x_face = object_3d.orientation_.at_.x_;
		y_face = object_3d.orientation_.at_.y_;
		z_face = object_3d.orientation_.at_.z_;

		x_up = object_3d.orientation_.up_.x_;
		y_up = object_3d.orientation_.up_.y_;
		z_up = object_3d.orientation_.up_.z_;
	}

	LH3DSAMPLE allocate_3d_sample(
		LHPROVIDER provider_id)
	{
		static_cast<void>(provider_id);

		objects_3d_.emplace_back();

		auto& object_3d = objects_3d_.back();

		auto& sample = object_3d.sample_;
		sample.is_3d_ = true;
		sample.is_stream_ = false;
		sample.oal_source_count_ = 1;

		create_sample(sample);

		return &object_3d;
	}

	void release_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);

		destroy_sample(object_3d.sample_);

		objects_3d_.remove_if(
			[&](const auto& object_3d_item)
			{
				return &object_3d_item == &object_3d;
			}
		);
	}

	void set_3d_sample_volume(
		LH3DSAMPLE sample_handle,
		const sint32 volume)
	{
		auto& object_3d = *static_cast<Object3d*>(sample_handle);

		if (object_3d.is_listener_)
		{
			return;
		}

		const auto new_volume = ltjs::AudioUtils::clamp_lt_volume(volume);

		auto& sample = object_3d.sample_;

		if (sample.volume_ == new_volume)
		{
			return;
		}

		sample.volume_ = new_volume;

		if (sample.status_ == Sample::Status::failed)
		{
			return;
		}

		const auto oal_volume = ltjs::AudioUtils::lt_volume_to_gain(new_volume);

		sample.oal_volume_ = oal_volume;

		::alSourcef(sample.oal_sources_[0], AL_GAIN, oal_volume);
	}

	void set_3d_sample_distances(
		LH3DSAMPLE sample_handle,
		const float max_distance,
		const float min_distance)
	{
		if (!sample_handle || max_distance <= 0.0F || min_distance <= 0.0F || max_distance < min_distance)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);

		if (object_3d.min_distance_ == min_distance && object_3d.max_distance_ == max_distance)
		{
			return;
		}

		object_3d.min_distance_ = min_distance;
		object_3d.max_distance_ = max_distance;

		auto& sample = object_3d.sample_;

		::alSourcef(sample.oal_sources_[0], AL_REFERENCE_DISTANCE, min_distance);
		::alSourcef(sample.oal_sources_[0], AL_MAX_DISTANCE, max_distance);
	}

	void set_3d_user_data(
		LH3DPOBJECT object_ptr,
		const uint32 index,
		const sint32 value)
	{
		if (!object_ptr || index > max_user_data_index)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		object_3d.user_data_[index] = value;
	}

	sint32 get_3d_user_data(
		LH3DPOBJECT object_ptr,
		const uint32 index)
	{
		if (!object_ptr || index > max_user_data_index)
		{
			return {};
		}

		auto& object_3d = *static_cast<Object3d*>(object_ptr);

		return object_3d.user_data_[index];
	}

	void stop_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		stop_sample(sample);
	}

	void start_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		start_sample(sample);
	}

	void resume_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		resume_sample(sample);
	}

	void end_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		end_sample(sample);
	}

	sint32 initialize_3d_sample_from_address(
		LH3DSAMPLE sample_handle,
		const void* ptr,
		const uint32 length,
		const ul::WaveFormatEx& wave_format,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr)
	{
		if (!sample_handle)
		{
			return {};
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);

		reset_3d_object(object_3d);

		auto& sample = object_3d.sample_;

		return initialize_sample_from_address_generic(
			sample,
			ptr,
			length,
			wave_format,
			playback_rate,
			filter_data_ptr);
	}

	sint32 initialize_3d_sample_from_file(
		LH3DSAMPLE sample_handle,
		const void* file_image_ptr,
		const sint32 block,
		const sint32 playback_rate,
		const LTSOUNDFILTERDATA* filter_data_ptr)
	{
		static_cast<void>(block);

		if (!sample_handle)
		{
			return {};
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);

		reset_3d_object(object_3d);

		auto& sample = object_3d.sample_;

		return initialize_sample_from_file(sample, file_image_ptr, playback_rate, filter_data_ptr);
	}

	sint32 get_3d_sample_volume(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return {};
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		return sample.volume_;
	}

	uint32 get_3d_sample_status(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return LS_STOPPED;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		const auto status = get_sample_status(sample);

		return status == Sample::Status::playing ? LS_PLAYING : LS_STOPPED;
	}

	void set_3d_sample_ms_position(
		LHSAMPLE sample_handle,
		const sint32 milliseconds)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		set_sample_ms_position(sample, milliseconds);
	}

	void set_3d_sample_loop_block(
		LH3DSAMPLE sample_handle,
		const sint32 loop_start_offset,
		const sint32 loop_end_offset,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		set_sample_loop_block(sample, loop_start_offset, loop_end_offset, is_enable);
	}

	void set_3d_sample_loop(
		LH3DSAMPLE sample_handle,
		const bool is_loop)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		set_sample_loop(sample, is_loop);
	}

	void uninitialize_eax20_filter()
	{
		if (oal_effect_slot_ != AL_EFFECTSLOT_NULL)
		{
			alAuxiliaryEffectSloti_(oal_effect_slot_, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);

			alDeleteAuxiliaryEffectSlots_(1, &oal_effect_slot_);
			oal_effect_slot_ = AL_EFFECTSLOT_NULL;
		}

		if (oal_effect_ != AL_EFFECT_NULL)
		{
			alDeleteEffects_(1, &oal_effect_);
			oal_effect_ = AL_EFFECT_NULL;
		}

		if (oal_null_effect_ != AL_EFFECT_NULL)
		{
			alDeleteEffects_(1, &oal_null_effect_);
			oal_null_effect_ = AL_EFFECT_NULL;
		}

		oal_is_supports_eax20_filter_ = false;
	}

	void oal_update_reverb_effect()
	{
		if (oal_has_eax_reverb_effect_)
		{
			set_efx_eax_reverb_properties(oal_effect_, oal_efx_eax_reverb_properties_);
		}
		else
		{
			set_efx_reverb_properties(oal_effect_, oal_efx_eax_reverb_properties_);
		}
	}

	void initialize_eax20_filter()
	{
		if (!is_eax20_filters_defined_ ||
			!oal_has_efx_ ||
			oal_max_aux_sends_ <= 0 ||
			!(oal_has_reverb_effect_ || oal_has_eax_reverb_effect_))
		{
			return;
		}

		uninitialize_eax20_filter();

		oal_clear_error();

		alGenEffects_(1, &oal_effect_);
		alGenEffects_(1, &oal_null_effect_);
		alGenAuxiliaryEffectSlots_(1, &oal_effect_slot_);

		const auto oal_effect_type = (oal_has_eax_reverb_effect_ ? AL_EFFECT_EAXREVERB : AL_EFFECT_REVERB);

		alEffecti_(oal_effect_, AL_EFFECT_TYPE, oal_effect_type);

		if (!oal_is_succeed())
		{
			uninitialize_eax20_filter();
			return;
		}

		oal_is_supports_eax20_filter_ = true;
	}

	bool set_eax20_filter(
		const bool is_enable,
		const LTSOUNDFILTERDATA& filter_data)
	{
		if (filter_data.uiFilterType != FilterReverb || !oal_is_supports_eax20_filter_)
		{
			return false;
		}

		const auto& lt_reverb = *reinterpret_cast<const LTFILTERREVERB*>(filter_data.pSoundFilter);

		lt_reverb_to_efx_reverb(lt_reverb, oal_efx_eax_reverb_properties_);

		oal_clear_error();

		oal_update_reverb_effect();

		const auto oal_effect = (is_enable ? oal_effect_ : oal_null_effect_);
		alAuxiliaryEffectSloti_(oal_effect_slot_, AL_EFFECTSLOT_EFFECT, oal_effect);

		if (!oal_is_succeed())
		{
			return false;
		}

		return true;
	}

	bool supports_eax20_filter() const
	{
		return oal_is_supports_eax20_filter_;
	}

	bool set_eax20_buffer_settings(
		LHSAMPLE sample_handle,
		const LTSOUNDFILTERDATA& filter_data)
	{
		if (!sample_handle)
		{
			return false;
		}

		static_cast<void>(filter_data);

		return true;
	}

	//
	// =========================================================================
	// API
	// =========================================================================


	String error_message_;
	bool is_eax20_filters_defined_;

	ALCdevice* oal_device_;
	ALCcontext* oal_context_;

	OalVersion oal_version_;
	String oal_version_string_;
	String oal_vendor_string_;
	String oal_renderer_string_;
	ExtensionsStrings oal_extentions_strings_;

	OalVersion oal_efx_version_;
	bool oal_has_efx_;
	bool oal_has_chorus_effect_;
	bool oal_has_compressor_effect_;
	bool oal_has_distortion_effect_;
	bool oal_has_echo_effect_;
	bool oal_has_flange_effect_;
	bool oal_has_equalizer_effect_;
	bool oal_has_reverb_effect_;
	bool oal_has_eax_reverb_effect_;
	int oal_max_aux_sends_;

	bool oal_is_supports_eax20_filter_;
	ALuint oal_effect_;
	ALuint oal_null_effect_;
	ALuint oal_effect_slot_;
	EFXEAXREVERBPROPERTIES oal_efx_eax_reverb_properties_;

	LPALGENEFFECTS alGenEffects_;
	LPALDELETEEFFECTS alDeleteEffects_;
	LPALISEFFECT alIsEffect_;
	LPALEFFECTI alEffecti_;
	LPALEFFECTIV alEffectiv_;
	LPALEFFECTF alEffectf_;
	LPALEFFECTFV alEffectfv_;
	LPALGETEFFECTI alGetEffecti_;
	LPALGETEFFECTIV alGetEffectiv_;
	LPALGETEFFECTF alGetEffectf_;
	LPALGETEFFECTFV alGetEffectfv_;
	LPALGENFILTERS alGenFilters_;
	LPALDELETEFILTERS alDeleteFilters_;
	LPALISFILTER alIsFilter_;
	LPALFILTERI alFilteri_;
	LPALFILTERIV alFilteriv_;
	LPALFILTERF alFilterf_;
	LPALFILTERFV alFilterfv_;
	LPALGETFILTERI alGetFilteri_;
	LPALGETFILTERIV alGetFilteriv_;
	LPALGETFILTERF alGetFilterf_;
	LPALGETFILTERFV alGetFilterfv_;
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots_;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots_;
	LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot_;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti_;
	LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv_;
	LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf_;
	LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv_;
	LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti_;
	LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv_;
	LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf_;
	LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv_;


	ltjs::AudioDecoder audio_decoder_;

	ClockTs clock_base_;
	sint32 master_volume_;
	Samples samples_2d_;
	Object3d listener_3d_;
	Objects3d objects_3d_;
	Streams streams_;
	MtThread mt_stream_thread_;
	MtMutex mt_stream_mutex_;
	MtCondVar mt_stream_cv_;
	MtMutex mt_stream_cv_mutex_;
	OpenStreams mt_open_streams_;
	bool mt_is_stop_stream_worker_;
	bool mt_stream_cv_flag_;
}; // OalSoundSys::Impl


const OalSoundSys::Impl::EfxReverbPresets OalSoundSys::Impl::efx_reverb_presets =
{{
	EFX_REVERB_PRESET_GENERIC,
	EFX_REVERB_PRESET_PADDEDCELL,
	EFX_REVERB_PRESET_ROOM,
	EFX_REVERB_PRESET_BATHROOM,
	EFX_REVERB_PRESET_LIVINGROOM,
	EFX_REVERB_PRESET_STONEROOM,
	EFX_REVERB_PRESET_AUDITORIUM,
	EFX_REVERB_PRESET_CONCERTHALL,
	EFX_REVERB_PRESET_CAVE,
	EFX_REVERB_PRESET_ARENA,
	EFX_REVERB_PRESET_HANGAR,
	EFX_REVERB_PRESET_CARPETEDHALLWAY,
	EFX_REVERB_PRESET_HALLWAY,
	EFX_REVERB_PRESET_STONECORRIDOR,
	EFX_REVERB_PRESET_ALLEY,
	EFX_REVERB_PRESET_FOREST,
	EFX_REVERB_PRESET_CITY,
	EFX_REVERB_PRESET_MOUNTAINS,
	EFX_REVERB_PRESET_QUARRY,
	EFX_REVERB_PRESET_PLAIN,
	EFX_REVERB_PRESET_PARKINGLOT,
	EFX_REVERB_PRESET_SEWERPIPE,
	EFX_REVERB_PRESET_UNDERWATER,
	EFX_REVERB_PRESET_DRUGGED,
	EFX_REVERB_PRESET_DIZZY,
	EFX_REVERB_PRESET_PSYCHOTIC,
}}; // efx_reverb_presets


OalSoundSys::OalSoundSys()
	:
	pimpl_{new Impl{}}
{
}

OalSoundSys::OalSoundSys(
	OalSoundSys&& that)
	:
	pimpl_{std::move(that.pimpl_)}
{
}

OalSoundSys::~OalSoundSys()
{
}

bool OalSoundSys::Init()
{
	ltjs::AudioDecoder::initialize_current_thread();
	ltjs::AudioUtils::initialize();

	return true;
}

void OalSoundSys::Term()
{
}

void* OalSoundSys::GetDDInterface(
	const uint dd_interface_id)
{
	static_cast<void>(dd_interface_id);

	return {};
}

void OalSoundSys::Lock()
{
}

void OalSoundSys::Unlock()
{
}

sint32 OalSoundSys::Startup()
{
	if (!pimpl_->startup())
	{
		return LS_ERROR;
	}

	return LS_OK;
}

void OalSoundSys::Shutdown()
{
	pimpl_->shutdown();
}

uint32 OalSoundSys::MsCount()
{
	return pimpl_->get_milliseconds();
}

sint32 OalSoundSys::SetPreference(
	const uint32 number,
	const sint32 value)
{
	static_cast<void>(number);
	static_cast<void>(value);

	return LS_ERROR;
}

sint32 OalSoundSys::GetPreference(
	const uint32 number)
{
	static_cast<void>(number);

	return {};
}

void OalSoundSys::MemFreeLock(
	void* ptr)
{
	ltjs::AudioUtils::deallocate(ptr);
}

void* OalSoundSys::MemAllocLock(
	const uint32 size)
{
	return ltjs::AudioUtils::allocate(size);
}

const char* OalSoundSys::LastError()
{
	return pimpl_->get_error_message();
}

sint32 OalSoundSys::WaveOutOpen(
	LHDIGDRIVER& driver,
	PHWAVEOUT& wave_out,
	const sint32 device_id,
	const ul::WaveFormatEx& wave_format)
{
	return pimpl_->wave_out_open(driver, wave_out, device_id, wave_format);
}

void OalSoundSys::WaveOutClose(
	LHDIGDRIVER driver_ptr)
{
	pimpl_->wave_out_close(driver_ptr);
}

void OalSoundSys::SetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr,
	const sint32 master_volume)
{
	pimpl_->set_master_volume(driver_ptr, master_volume);
}

sint32 OalSoundSys::GetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr)
{
	return pimpl_->get_master_volume(driver_ptr);
}

sint32 OalSoundSys::DigitalHandleRelease(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

sint32 OalSoundSys::DigitalHandleReacquire(
	LHDIGDRIVER driver_ptr)
{
	static_cast<void>(driver_ptr);

	return {};
}

#ifdef USE_EAX20_HARDWARE_FILTERS
bool OalSoundSys::SetEAX20Filter(
	const bool is_enable,
	const LTSOUNDFILTERDATA& filter_data)
{
	return pimpl_->set_eax20_filter(is_enable, filter_data);
}

bool OalSoundSys::SupportsEAX20Filter()
{
	return pimpl_->supports_eax20_filter();
}

bool OalSoundSys::SetEAX20BufferSettings(
	LHSAMPLE sample_handle,
	const LTSOUNDFILTERDATA& filter_data)
{
	return pimpl_->set_eax20_buffer_settings(sample_handle, filter_data);
}
#endif // USE_EAX20_HARDWARE_FILTERS

void OalSoundSys::Set3DProviderMinBuffers(
	const uint32 min_buffers)
{
	static_cast<void>(min_buffers);
}

sint32 OalSoundSys::Open3DProvider(
	LHPROVIDER provider_id)
{
	switch (provider_id)
	{
	case SOUND3DPROVIDERID_DS3D_SOFTWARE:
	case SOUND3DPROVIDERID_DS3D_HARDWARE:
	case SOUND3DPROVIDERID_DS3D_DEFAULT:
		return true;

	default:
		return false;
	}
}

void OalSoundSys::Close3DProvider(
	LHPROVIDER provider_id)
{
	static_cast<void>(provider_id);
}

void OalSoundSys::Set3DProviderPreference(
	LHPROVIDER provider_id,
	const char* name,
	const void* value)
{
	static_cast<void>(provider_id);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalSoundSys::Get3DProviderAttribute(
	LHPROVIDER provider_id,
	const char* name,
	void* value)
{
	pimpl_->get_3d_provider_attribute(provider_id, name, value);
}

sint32 OalSoundSys::Enumerate3DProviders(
	LHPROENUM& next,
	LHPROVIDER& destination,
	const char*& name)
{
	const auto current = next++;

	destination = 0;
	name = nullptr;

	if (current < 0 || current > 0)
	{
		return false;
	}

	destination = SOUND3DPROVIDERID_DS3D_HARDWARE;
	name = "OpenAL";

	return true;
}

LH3DPOBJECT OalSoundSys::Open3DListener(
	LHPROVIDER provider_id)
{
	return pimpl_->open_3d_listener(provider_id);
}

void OalSoundSys::Close3DListener(
	LH3DPOBJECT listener_ptr)
{
	pimpl_->close_3d_listener(listener_ptr);
}

void OalSoundSys::SetListenerDoppler(
	LH3DPOBJECT listener_ptr,
	const float doppler)
{
	pimpl_->set_3d_listener_doppler(listener_ptr, doppler);
}

void OalSoundSys::CommitDeferred()
{
}

void OalSoundSys::Set3DPosition(
	LH3DPOBJECT object_ptr,
	const float x,
	const float y,
	const float z)
{
	pimpl_->set_3d_position(object_ptr, x, y, z);
}

void OalSoundSys::Set3DVelocityVector(
	LH3DPOBJECT object_ptr,
	const float dx_per_s,
	const float dy_per_s,
	const float dz_per_s)
{
	pimpl_->set_3d_velocity(object_ptr, dx_per_s, dy_per_s, dz_per_s);
}

void OalSoundSys::Set3DOrientation(
	LH3DPOBJECT object_ptr,
	const float x_face,
	const float y_face,
	const float z_face,
	const float x_up,
	const float y_up,
	const float z_up)
{
	pimpl_->set_3d_orientation(object_ptr, x_face, y_face, z_face, x_up, y_up, z_up);
}

void OalSoundSys::Set3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index,
	const sint32 value)
{
	pimpl_->set_3d_user_data(object_ptr, index, value);
}

void OalSoundSys::Get3DPosition(
	LH3DPOBJECT object_ptr,
	float& x,
	float& y,
	float& z)
{
	pimpl_->get_3d_position(object_ptr, x, y, z);
}

void OalSoundSys::Get3DVelocity(
	LH3DPOBJECT object_ptr,
	float& dx_per_ms,
	float& dy_per_ms,
	float& dz_per_ms)
{
	pimpl_->get_3d_velocity(object_ptr, dx_per_ms, dy_per_ms, dz_per_ms);
}

void OalSoundSys::Get3DOrientation(
	LH3DPOBJECT object_ptr,
	float& x_face,
	float& y_face,
	float& z_face,
	float& x_up,
	float& y_up,
	float& z_up)
{
	pimpl_->get_3d_orientation(object_ptr, x_face, y_face, z_face, x_up, y_up, z_up);
}

sint32 OalSoundSys::Get3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index)
{
	return pimpl_->get_3d_user_data(object_ptr, index);
}

LH3DSAMPLE OalSoundSys::Allocate3DSampleHandle(
	LHPROVIDER driver_id)
{
	return pimpl_->allocate_3d_sample(driver_id);
}

void OalSoundSys::Release3DSampleHandle(
	LH3DSAMPLE sample_handle)
{
	return pimpl_->release_3d_sample(sample_handle);
}

void OalSoundSys::Stop3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->stop_3d_sample(sample_handle);
}

void OalSoundSys::Start3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->start_3d_sample(sample_handle);
}

void OalSoundSys::Resume3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->resume_3d_sample(sample_handle);
}

void OalSoundSys::End3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->end_3d_sample(sample_handle);
}

sint32 OalSoundSys::Init3DSampleFromAddress(
	LH3DSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->initialize_3d_sample_from_address(sample_handle, ptr, length, wave_format, playback_rate, filter_data_ptr);
}

sint32 OalSoundSys::Init3DSampleFromFile(
	LH3DSAMPLE sample_handle,
	const void* file_image_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->initialize_3d_sample_from_file(sample_handle, file_image_ptr, block, playback_rate, filter_data_ptr);
}

sint32 OalSoundSys::Get3DSampleVolume(
	LH3DSAMPLE sample_handle)
{
	return pimpl_->get_3d_sample_volume(sample_handle);
}

void OalSoundSys::Set3DSampleVolume(
	LH3DSAMPLE sample_handle,
	const sint32 volume)
{
	pimpl_->set_3d_sample_volume(sample_handle, volume);
}

uint32 OalSoundSys::Get3DSampleStatus(
	LH3DSAMPLE sample_handle)
{
	return pimpl_->get_3d_sample_status(sample_handle);
}

void OalSoundSys::Set3DSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	pimpl_->set_3d_sample_ms_position(sample_handle, milliseconds);
}

sint32 OalSoundSys::Set3DSampleInfo(
	LH3DSAMPLE sample_handle,
	const LTSOUNDINFO& sound_info)
{
	static_cast<void>(sample_handle);
	static_cast<void>(sound_info);

	return {};
}

void OalSoundSys::Set3DSampleDistances(
	LH3DSAMPLE sample_handle,
	const float max_distance,
	const float min_distance)
{
	pimpl_->set_3d_sample_distances(sample_handle, max_distance, min_distance);
}

void OalSoundSys::Set3DSamplePreference(
	LH3DSAMPLE sample_handle,
	const char* name,
	const void* value)
{
	static_cast<void>(sample_handle);
	static_cast<void>(name);
	static_cast<void>(value);
}

void OalSoundSys::Set3DSampleLoopBlock(
	LH3DSAMPLE sample_handle,
	const sint32 loop_start_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	pimpl_->set_3d_sample_loop_block(sample_handle, loop_start_offset, loop_end_offset, is_enable);
}

void OalSoundSys::Set3DSampleLoop(
	LH3DSAMPLE sample_handle,
	const bool is_loop)
{
	pimpl_->set_3d_sample_loop(sample_handle, is_loop);
}

void OalSoundSys::Set3DSampleObstruction(
	LH3DSAMPLE sample_handle,
	const float obstruction)
{
	static_cast<void>(sample_handle);
	static_cast<void>(obstruction);
}

float OalSoundSys::Get3DSampleObstruction(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

void OalSoundSys::Set3DSampleOcclusion(
	LH3DSAMPLE sample_handle,
	const float occlusion)
{
	static_cast<void>(sample_handle);
	static_cast<void>(occlusion);
}

float OalSoundSys::Get3DSampleOcclusion(
	LH3DSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);

	return {};
}

LHSAMPLE OalSoundSys::AllocateSampleHandle(
	LHDIGDRIVER driver_ptr)
{
	return pimpl_->allocate_sample(driver_ptr);
}

void OalSoundSys::ReleaseSampleHandle(
	LHSAMPLE sample_handle)
{
	pimpl_->deallocate_sample(sample_handle);
}

void OalSoundSys::InitSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::StopSample(
	LHSAMPLE sample_handle)
{
	pimpl_->stop_sample(sample_handle);
}

void OalSoundSys::StartSample(
	LHSAMPLE sample_handle)
{
	pimpl_->start_sample(sample_handle);
}

void OalSoundSys::ResumeSample(
	LHSAMPLE sample_handle)
{
	pimpl_->resume_sample(sample_handle);
}

void OalSoundSys::EndSample(
	LHSAMPLE sample_handle)
{
	pimpl_->end_sample(sample_handle);
}

void OalSoundSys::SetSampleVolume(
	LHSAMPLE sample_handle,
	const sint32 volume)
{
	pimpl_->set_sample_volume(sample_handle, volume);
}

void OalSoundSys::SetSamplePan(
	LHSAMPLE sample_handle,
	const sint32 pan)
{
	pimpl_->set_sample_pan(sample_handle, pan);
}

sint32 OalSoundSys::GetSampleVolume(
	LHSAMPLE sample_handle)
{
	return pimpl_->get_sample_volume(sample_handle);
}

sint32 OalSoundSys::GetSamplePan(
	LHSAMPLE sample_handle)
{
	return pimpl_->get_sample_pan(sample_handle);
}

void OalSoundSys::SetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index,
	const sint32 value)
{
	pimpl_->set_sample_user_data(sample_handle, index, value);
}

void OalSoundSys::GetDirectSoundInfo(
	LHSAMPLE sample_handle,
	PTDIRECTSOUND& direct_sound,
	PTDIRECTSOUNDBUFFER& direct_sound_buffer)
{
	static_cast<void>(sample_handle);

	direct_sound = nullptr;
	direct_sound_buffer = nullptr;
}

void OalSoundSys::SetSampleReverb(
	LHSAMPLE sample_handle,
	const float reverb_level,
	const float reverb_reflect_time,
	const float reverb_decay_time)
{
	static_cast<void>(sample_handle);
	static_cast<void>(reverb_level);
	static_cast<void>(reverb_reflect_time);
	static_cast<void>(reverb_decay_time);
}

sint32 OalSoundSys::InitSampleFromAddress(
	LHSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->initialize_sample_generic(sample_handle, ptr, length, wave_format, playback_rate, filter_data_ptr);
}

sint32 OalSoundSys::InitSampleFromFile(
	LHSAMPLE sample_handle,
	const void* file_image_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->initialize_sample_from_file(sample_handle, file_image_ptr, block, playback_rate, filter_data_ptr);
}

void OalSoundSys::SetSampleLoopBlock(
	LHSAMPLE sample_handle,
	const sint32 loop_start_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	pimpl_->set_sample_loop_block(sample_handle, loop_start_offset, loop_end_offset, is_enable);
}

void OalSoundSys::SetSampleLoop(
	LHSAMPLE sample_handle,
	const bool is_loop)
{
	pimpl_->set_sample_loop(sample_handle, is_loop);
}

void OalSoundSys::SetSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	pimpl_->set_sample_ms_position(sample_handle, milliseconds);
}

sint32 OalSoundSys::GetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index)
{
	return pimpl_->get_sample_user_data(sample_handle, index);
}

uint32 OalSoundSys::GetSampleStatus(
	LHSAMPLE sample_handle)
{
	return pimpl_->get_sample_status(sample_handle);
}

LHSTREAM OalSoundSys::OpenStream(
	const char* file_name,
	const uint32 file_offset,
	LHDIGDRIVER driver_ptr,
	const char* stream_ptr,
	const sint32 stream_memory_size)
{
	return pimpl_->open_stream(file_name, file_offset, driver_ptr, stream_ptr, stream_memory_size);
}

void OalSoundSys::SetStreamLoop(
	LHSTREAM stream_ptr,
	const bool is_loop)
{
	pimpl_->set_stream_loop(stream_ptr, is_loop);
}

void OalSoundSys::SetStreamPlaybackRate(
	LHSTREAM stream_ptr,
	const sint32 rate)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(rate);
}

void OalSoundSys::SetStreamMsPosition(
	LHSTREAM stream_ptr,
	const sint32 milliseconds)
{
	pimpl_->set_stream_position_ms(stream_ptr, milliseconds);
}

void OalSoundSys::SetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index,
	const sint32 value)
{
	pimpl_->set_stream_user_data(stream_ptr, index,value);
}

sint32 OalSoundSys::GetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	return pimpl_->get_stream_user_data(stream_ptr, index);
}

void OalSoundSys::CloseStream(
	LHSTREAM stream_ptr)
{
	pimpl_->close_stream(stream_ptr);
}

void OalSoundSys::StartStream(
	LHSTREAM stream_ptr)
{
	pimpl_->start_stream(stream_ptr);
}

void OalSoundSys::PauseStream(
	LHSTREAM stream_ptr,
	const sint32 is_pause)
{
	pimpl_->pause_stream(stream_ptr, is_pause);
}

void OalSoundSys::ResetStream(
	LHSTREAM stream_ptr)
{
	static_cast<void>(stream_ptr);
}

void OalSoundSys::SetStreamVolume(
	LHSTREAM stream_ptr,
	const sint32 volume)
{
	pimpl_->set_stream_volume(stream_ptr, volume);
}

void OalSoundSys::SetStreamPan(
	LHSTREAM stream_ptr,
	const sint32 pan)
{
	pimpl_->set_stream_pan(stream_ptr, pan);
}

sint32 OalSoundSys::GetStreamVolume(
	LHSTREAM stream_ptr)
{
	return pimpl_->set_stream_volume(stream_ptr);
}

sint32 OalSoundSys::GetStreamPan(
	LHSTREAM stream_ptr)
{
	return pimpl_->get_stream_pan(stream_ptr);
}

uint32 OalSoundSys::GetStreamStatus(
	LHSTREAM stream_ptr)
{
	return pimpl_->get_stream_status(stream_ptr);
}

sint32 OalSoundSys::GetStreamBufferParam(
	LHSTREAM stream_ptr,
	const uint32 param)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(param);

	return {};
}

void OalSoundSys::ClearStreamBuffer(
	LHSTREAM stream_ptr,
	const bool is_clear_data_queue)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(is_clear_data_queue);
}

sint32 OalSoundSys::DecompressADPCM(
	const LTSOUNDINFO& sound_info,
	void*& dst_data,
	uint32& dst_size)
{
	static_cast<void>(sound_info);
	static_cast<void>(dst_data);
	static_cast<void>(dst_size);

	return {};
}

sint32 OalSoundSys::DecompressASI(
	const void* srd_data_ptr,
	const uint32 src_size,
	const char* file_name_ext,
	void*& dst_wav,
	uint32& dst_wav_size,
	LTLENGTHYCB callback)
{
	return pimpl_->decode_mp3(srd_data_ptr, src_size, file_name_ext, dst_wav, dst_wav_size, callback);
}

uint32 OalSoundSys::GetThreadedSoundTicks()
{
	return {};
}

bool OalSoundSys::HasOnBoardMemory()
{
	return {};
}

OalSoundSys& OalSoundSys::get_singleton()
{
	static auto singleton = OalSoundSys{};
	return singleton;
}


extern "C"
{
	__declspec(dllexport) char* SoundSysDesc();
	__declspec(dllexport) ILTSoundSys* SoundSysMake();
}

char* SoundSysDesc()
{
	static char* description = const_cast<char*>("OpenAL");
	return description;
}

ILTSoundSys* SoundSysMake()
{
	return &OalSoundSys::get_singleton();
}
