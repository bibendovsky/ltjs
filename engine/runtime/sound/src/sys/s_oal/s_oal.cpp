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
#include "al.h"
#include "alc.h"
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

	class Sample
	{
	public:
		enum class Type
		{
			none,
			stereo,
			surround,
			stream,
		}; // Type

		enum class Status
		{
			none,
			stopped,
			playing,
			paused,
			failed,
		}; // Status

		static constexpr auto oal_max_buffer_count = 3;
		static constexpr auto oal_max_sources = 2;
		static constexpr auto oal_max_pans = 2;

		static constexpr auto left_index = 0;
		static constexpr auto right_index = 1;

		using Data = std::vector<std::uint8_t>;
		using OalPans = std::array<float, oal_max_pans>;
		using OalSources = std::array<ALuint, oal_max_sources>;
		using OalBuffers = std::array<ALuint, oal_max_buffer_count>;


		bool is_looping_;
		bool has_loop_block_;
		sint32 loop_begin_;
		sint32 loop_end_;
		sint32 volume_;
		sint32 pan_;
		Status status_;
		Data data_;
		UserDataArray user_data_array_;

		// 2D and 3D: 0 - non-loop block; 1 - pre-loop [0..loop_start); 2 - loop [loop_start..loop_end].
		// Stream: 0, 1, 2 - queue.
		OalBuffers oal_buffers_;

		// 2D and stream: 0 - left channel; 1 - right channel.
		// 3D: 0 - main; 1 - not available.
		OalSources oal_sources_;

		ALenum oal_buffer_format_;
		float oal_volume_;
		OalPans oal_pans_;


		Sample(
			const Sample& that) = delete;

		Sample& operator=(
			const Sample& that) = delete;

		Sample(
			Sample&& that)
			:
			is_looping_{std::move(that.is_looping_)},
			has_loop_block_{std::move(that.has_loop_block_)},
			loop_begin_{std::move(that.loop_begin_)},
			loop_end_{std::move(that.loop_end_)},
			volume_{std::move(that.volume_)},
			pan_{std::move(that.pan_)},
			status_{std::move(that.status_)},
			data_{std::move(that.data_)},
			user_data_array_{std::move(that.user_data_array_)},
			oal_buffers_{std::move(that.oal_buffers_)},
			oal_sources_{std::move(that.oal_sources_)},
			oal_buffer_format_{std::move(that.oal_buffer_format_)},
			oal_volume_{std::move(that.oal_volume_)},
			oal_pans_{std::move(that.oal_pans_)},
			type_{std::move(that.type_)},
			block_align_{std::move(that.block_align_)},
			sample_rate_{std::move(that.sample_rate_)},
			oal_source_count_{std::move(that.oal_source_count_)},
			oal_are_buffers_created_{std::move(that.oal_are_buffers_created_)},
			oal_are_sources_created_{std::move(that.oal_are_sources_created_)}
		{
			that.oal_are_sources_created_ = false;
			that.oal_are_sources_created_ = false;
		}

		~Sample()
		{
			destroy();
		}


		bool is_2d() const
		{
			return type_ == Type::stereo;
		}

		bool is_3d() const
		{
			return type_ == Type::surround;
		}

		bool is_stream() const
		{
			return type_ == Type::stream;
		}

		int get_block_align() const
		{
			return block_align_;
		}

		int get_sample_rate() const
		{
			return sample_rate_;
		}

		Status get_status()
		{
			switch (status_)
			{
			case Status::failed:
			case Status::paused:
			case Status::stopped:
				return status_;

			default:
				break;
			}

			auto oal_state = ALint{};

			::alGetSourcei(oal_sources_[left_index], AL_SOURCE_STATE, &oal_state);

			auto status = Status::none;

			switch (oal_state)
			{
			case AL_INITIAL:
			case AL_STOPPED:
				status = Status::stopped;
				break;

			case AL_PAUSED:
				status = Status::paused;
				break;

			case AL_PLAYING:
				status = Status::playing;
				break;

			default:
				status = Status::failed;
				break;
			}

			status_ = status;

			return status;
		}

		void update_volume_and_pan()
		{
			for (auto i = 0; i < 2; ++i)
			{
				auto& oal_source = oal_sources_[i];

				const auto gain = oal_volume_ * oal_pans_[i];

				::alSourcef(oal_source, AL_GAIN, gain);
			}
		}

		void pause()
		{
			const auto status = get_status();

			switch (status)
			{
			case Status::failed:
			case Status::paused:
			case Status::stopped:
				return;

			default:
				break;
			}

			::alSourcePausev(oal_source_count_, oal_sources_.data());

			status_ = Status::paused;
		}

		void play()
		{
			const auto status = get_status();

			if (status == Status::failed)
			{
				return;
			}

			::alSourcePlayv(oal_source_count_, oal_sources_.data());

			status_ = Status::playing;
		}

		void resume()
		{
			const auto status = get_status();

			switch (status)
			{
			case Status::failed:
			case Status::playing:
				return;

			default:
				break;
			}

			::alSourcePlayv(oal_source_count_, oal_sources_.data());

			status_ = Status::playing;
		}

		void stop()
		{
			const auto status = get_status();

			switch (status)
			{
			case Status::failed:
			case Status::stopped:
				return;

			default:
				break;
			}

			::alSourceStopv(oal_source_count_, oal_sources_.data());

			status_ = Status::stopped;
		}

		void set_volume(
			const sint32 volume)
		{
			const auto new_volume = ltjs::AudioUtils::clamp_lt_volume(volume);

			if (new_volume == volume_)
			{
				return;
			}

			volume_ = new_volume;

			if (status_ == Status::failed)
			{
				return;
			}

			const auto oal_volume = ltjs::AudioUtils::lt_volume_to_gain(new_volume);

			oal_volume_ = oal_volume;

			update_volume_and_pan();
		}

		void set_pan(
			const sint32 pan)
		{
			const auto new_pan = ltjs::AudioUtils::clamp_lt_pan(pan);

			if (pan_ == new_pan)
			{
				return;
			}

			pan_ = new_pan;

			if (status_ == Status::failed)
			{
				return;
			}

			const auto oal_pan = ltjs::AudioUtils::lt_pan_to_gain(new_pan);

			if (new_pan == ltjs::AudioUtils::lt_pan_center)
			{
				// Left and right channels are at full volume.

				oal_pans_.fill(ltjs::AudioUtils::gain_max);
			}
			else if (new_pan < ltjs::AudioUtils::lt_pan_center)
			{
				// Left channel is at full volume; right channels is attenuated.

				oal_pans_[left_index] = ltjs::AudioUtils::gain_max;
				oal_pans_[right_index] = std::abs(oal_pan);
			}
			else
			{
				// Right channel is at full volume; lrft channels is attenuated.

				oal_pans_[left_index] = std::abs(oal_pan);
				oal_pans_[right_index] = ltjs::AudioUtils::gain_max;
			}

			update_volume_and_pan();
		}

		void reset()
		{
			block_align_ = {};
			sample_rate_ = {};

			is_looping_ = {};
			has_loop_block_ = {};
			loop_begin_ = {};
			loop_end_ = {};
			volume_ = ltjs::AudioUtils::lt_max_volume;
			pan_ = pan_center;

			oal_buffer_format_ = AL_NONE;
			oal_volume_ = ltjs::AudioUtils::gain_max;
			oal_pans_.fill(ltjs::AudioUtils::gain_max);
		}

		bool initialize_from_address_generic(
			const void* storage_ptr,
			const uint32 storage_size,
			const ul::WaveFormatEx& wave_format,
			const sint32 playback_rate,
			const LTSOUNDFILTERDATA* filter_data_ptr,
			const bool oal_has_effect_slot,
			const ALuint oal_effect_slot)
		{
			static_cast<void>(filter_data_ptr);

			if (status_ == Status::failed)
			{
				return false;
			}

			reset();

			if ((!is_stream() && storage_size == 0) || !validate_wave_format_ex(wave_format) || playback_rate <= 0)
			{
				return false;
			}

			if (is_3d() && wave_format.channel_count_ != 1)
			{
				return false;
			}

			if (!is_stream() && !storage_ptr && data_.empty())
			{
				return false;
			}

			block_align_ = wave_format.block_align_;
			sample_rate_ = static_cast<int>(wave_format.sample_rate_);

			const auto has_pitch = (static_cast<int>(sample_rate_) != playback_rate);

			const auto pitch = (
				has_pitch ?
				static_cast<float>(playback_rate) / static_cast<float>(sample_rate_) :
				1.0F);

			oal_buffer_format_ = get_oal_buffer_format(wave_format);

			if (storage_ptr)
			{
				data_.resize(storage_size);

				std::uninitialized_copy_n(
					static_cast<const std::uint8_t*>(storage_ptr),
					storage_size,
					data_.begin());
			}

			oal_clear_error();

			::alSourceStopv(oal_source_count_, oal_sources_.data());

			for (auto i = 0; i < oal_source_count_; ++i)
			{
				::alSourcei(oal_sources_[i], AL_BUFFER, AL_NONE);
			}

			if (is_stream())
			{
				auto processed_buffers = OalBuffers{};

				for (auto i = 0; i < oal_source_count_; ++i)
				{
					const auto oal_source = oal_sources_[i];

					auto processed_count = ALint{};

					::alGetSourcei(oal_source, AL_BUFFERS_PROCESSED, &processed_count);

					if (processed_count > 0)
					{
						::alSourceUnqueueBuffers(oal_source, processed_count, processed_buffers.data());
					}
				}
			}

			if (!is_stream())
			{
				::alBufferData(
					oal_buffers_[0],
					oal_buffer_format_,
					data_.data(),
					static_cast<ALsizei>(data_.size()),
					static_cast<ALsizei>(sample_rate_));
			}

			for (auto i = 0; i < oal_source_count_; ++i)
			{
				const auto oal_source = oal_sources_[i];

				if (!is_stream())
				{
					::alSourcei(oal_source, AL_BUFFER, oal_buffers_[0]);
				}

				if (has_pitch)
				{
					::alSourcef(oal_source, AL_PITCH, pitch);
				}
				else
				{
					::alSourcef(oal_source, AL_PITCH, 1.0F);
				}

				if (is_3d())
				{
					::alSourcef(oal_source, AL_GAIN, ltjs::AudioUtils::gain_max);
				}

				::alSourcei(oal_source, AL_LOOPING, AL_FALSE);

				::alSourcef(oal_source, AL_REFERENCE_DISTANCE, ltjs::AudioUtils::ds_default_min_distance);
				::alSourcef(oal_source, AL_MAX_DISTANCE, ltjs::AudioUtils::ds_default_max_distance);
			}

			if (!is_3d())
			{
				update_volume_and_pan();

				for (auto i = 0; i < 2; ++i)
				{
					::alSourcei(oal_sources_[i], AL_SOURCE_RELATIVE, AL_TRUE);
				}

				// Left channel.
				::alSource3f(oal_sources_[left_index], AL_POSITION, -1.0F, 0.0F, 0.0F);

				// Right channel.
				::alSource3f(oal_sources_[right_index], AL_POSITION, 1.0F, 0.0F, 0.0F);
			}

			if (oal_has_effect_slot)
			{
				for (auto i_source = 0; i_source < oal_source_count_; ++i_source)
				{
					const auto oal_source = oal_sources_[i_source];

					::alSource3i(oal_source, AL_AUXILIARY_SEND_FILTER, oal_effect_slot, 0, AL_FILTER_NULL);
				}
			}

			if (!oal_is_succeed())
			{
				status_ = Status::failed;
				return false;
			}

			status_ = Status::stopped;

			return true;
		}

		bool initialize_from_file_generic(
			ltjs::AudioDecoder& audio_decoder,
			const void* storage_ptr,
			const sint32 playback_rate,
			const LTSOUNDFILTERDATA* filter_data_ptr,
			const bool oal_has_effect_slot,
			const ALuint oal_effect_slot)
		{
			static_cast<void>(filter_data_ptr);

			if (!storage_ptr || playback_rate <= 0)
			{
				return false;
			}

			if (status_ == Status::failed)
			{
				return false;
			}

			const auto wave_size = ltjs::AudioUtils::extract_wave_size(storage_ptr);

			if (wave_size <= 0)
			{
				return false;
			}

			auto memory_stream = ul::MemoryStream{storage_ptr, wave_size};

			if (!memory_stream.is_open())
			{
				return false;
			}

			if (!audio_decoder.open(&memory_stream))
			{
				return false;
			}

			const auto data_size = audio_decoder.get_data_size();

			if (data_size <= 0)
			{
				return false;
			}

			data_.resize(data_size);

			const auto decoded_size = audio_decoder.decode(data_.data(), data_size);

			if (decoded_size <= 0)
			{
				return false;
			}

			data_.resize(decoded_size);

			const auto wave_format = audio_decoder.get_wave_format_ex();

			const auto result = initialize_from_address_generic(
				nullptr,
				decoded_size,
				wave_format,
				playback_rate,
				filter_data_ptr,
				oal_has_effect_slot,
				oal_effect_slot);

			return result;
		}

		void set_loop_block(
			const sint32 loop_begin_offset,
			const sint32 loop_end_offset,
			const bool is_enable)
		{
			const auto data_size = static_cast<int>(data_.size());
			const auto sample_size = block_align_;

			auto new_start = loop_begin_offset;

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

			has_loop_block_ = is_enable;
			loop_begin_ = new_start;
			loop_end_ = new_end;

			if ((new_start == 0 && new_end == data_size) || new_start > new_end)
			{
				has_loop_block_ = false;
			}
		}

		void set_loop(
			const bool is_enable)
		{
			if (is_looping_ == is_enable)
			{
				return;
			}

			is_looping_ = is_enable;

			if (status_ == Status::failed)
			{
				return;
			}

			const auto status_before_stop = get_status();

			if (status_before_stop == Status::failed)
			{
				return;
			}

			oal_clear_error();

			::alSourceStopv(oal_source_count_, oal_sources_.data());

			auto sample_offset = ALint{-1};

			::alGetSourcei(oal_sources_[0], AL_SAMPLE_OFFSET, &sample_offset);

			if (sample_offset < 0)
			{
				status_ = Status::failed;
				return;
			}

			for (auto i = 0; i < oal_source_count_; ++i)
			{
				::alSourcei(oal_sources_[i], AL_BUFFER, AL_NONE);
			}

			if (is_enable)
			{
				if (has_loop_block_)
				{
					::alBufferData(
						oal_buffers_[1],
						oal_buffer_format_,
						data_.data(),
						static_cast<ALsizei>(loop_begin_),
						static_cast<ALsizei>(sample_rate_));

					::alBufferData(
						oal_buffers_[2],
						oal_buffer_format_,
						&data_[loop_begin_],
						static_cast<ALsizei>(loop_end_ - loop_begin_),
						static_cast<ALsizei>(sample_rate_));

					for (auto i = 0; i < oal_source_count_; ++i)
					{
						::alSourceQueueBuffers(oal_sources_[i], 2, &oal_buffers_[1]);
						::alSourcei(oal_sources_[i], AL_SAMPLE_OFFSET, sample_offset);
					}
				}
				else
				{
					for (auto i = 0; i < oal_source_count_; ++i)
					{
						::alSourcei(oal_sources_[i], AL_BUFFER, oal_buffers_[0]);
					}
				}

				for (auto i = 0; i < oal_source_count_; ++i)
				{
					::alSourcei(oal_sources_[i], AL_LOOPING, AL_TRUE);
				}
			}
			else
			{
				for (auto i = 0; i < oal_source_count_; ++i)
				{
					::alSourcei(oal_sources_[i], AL_BUFFER, oal_buffers_[0]);
					::alSourcei(oal_sources_[i], AL_LOOPING, AL_FALSE);
					::alSourcei(oal_sources_[i], AL_SAMPLE_OFFSET, sample_offset);
				}
			}

			if (status_before_stop == Status::playing)
			{
				::alSourcePlayv(2, oal_sources_.data());
			}

			if (!oal_is_succeed())
			{
				status_ = Status::failed;
				return;
			}

			if (status_before_stop == Status::playing)
			{
				status_ = Status::playing;
			}
			else
			{
				status_ = Status::paused;
			}
		}

		void set_ms_position(
			const sint32 milliseconds)
		{
			if (status_ == Status::failed)
			{
				return;
			}

			const auto sample_offset = static_cast<int>((sample_rate_ * milliseconds) / 1000LL);
			const auto data_offset = sample_offset * block_align_;
			const auto data_size = static_cast<int>(data_.size());

			if (data_offset > data_size)
			{
				return;
			}

			for (auto i = 0; i < oal_source_count_; ++i)
			{
				::alSourcei(oal_sources_[i], AL_SAMPLE_OFFSET, sample_offset);
			}
		}


	protected:
		Sample(
			const Type type)
			:
			is_looping_{},
			has_loop_block_{},
			loop_begin_{},
			loop_end_{},
			volume_{},
			pan_{},
			status_{},
			data_{},
			user_data_array_{},
			oal_buffers_{},
			oal_sources_{},
			oal_buffer_format_{},
			oal_volume_{},
			oal_pans_{},
			type_{type},
			block_align_{},
			sample_rate_{},
			oal_source_count_{},
			oal_are_buffers_created_{},
			oal_are_sources_created_{}
		{
			switch (type)
			{
			case Type::stereo:
			case Type::stream:
				oal_source_count_ = 2;
				break;

			case Type::surround:
				oal_source_count_ = 1;
				break;

			default:
				throw "Invalid type.";
			}

			create();
		}


	private:
		Type type_;

		int block_align_;
		int sample_rate_;

		// 2D: two sources.
		// 3D: one source.
		int oal_source_count_;

		bool oal_are_buffers_created_;
		bool oal_are_sources_created_;


		void create()
		{
			if (oal_source_count_ <= 0 || oal_source_count_ > oal_max_sources)
			{
				throw "Invalid source count.";
			}

			auto is_succeed = true;

			oal_clear_error();

			::alGenBuffers(oal_max_buffer_count, oal_buffers_.data());

			if (oal_is_succeed())
			{
				oal_are_buffers_created_ = true;
			}
			else
			{
				is_succeed = false;
				assert(!"alGenBuffers");
			}

			::alGenSources(oal_source_count_, oal_sources_.data());

			if (oal_is_succeed())
			{
				oal_are_sources_created_ = true;
			}
			else
			{
				is_succeed = false;
				assert(!"alGenSources");
			}

			if (!is_succeed)
			{
				destroy();

				status_ = Status::failed;

				return;
			}

			status_ = {};
		}

		void destroy()
		{
			if (oal_source_count_ <= 0 || oal_source_count_ > oal_max_sources)
			{
				throw "Invalid source count.";
			}

			if (oal_are_sources_created_)
			{
				oal_are_sources_created_ = false;

#ifdef _DEBUG
				oal_clear_error();
#endif // _DEBUG

				::alSourceStopv(oal_source_count_, oal_sources_.data());

#ifdef _DEBUG
				if (!oal_is_succeed())
				{
					assert(!"alSourceStopv");
				}
#endif // _DEBUG

				::alDeleteSources(oal_source_count_, oal_sources_.data());

#ifdef _DEBUG
				if (!oal_is_succeed())
				{
					assert(!"alDeleteSources");
				}
#endif // _DEBUG
			}

			if (oal_are_buffers_created_)
			{
				oal_are_buffers_created_ = false;

#ifdef _DEBUG
				oal_clear_error();
#endif // _DEBUG

				::alDeleteBuffers(oal_max_buffer_count, oal_buffers_.data());

#ifdef _DEBUG
				if (!oal_is_succeed())
				{
					assert(!"alDeleteBuffers");
				}
#endif // _DEBUG
			}

			status_ = {};
			oal_source_count_ = {};
		}
	}; // Sample


	class X2dSample :
		public Sample
	{
	public:
		X2dSample()
			:
			Sample{Type::stereo}
		{
		}

		X2dSample(
			const X2dSample& that) = delete;

		X2dSample& operator=(
			const X2dSample& that) = delete;

		X2dSample(
			X2dSample&& that)
			:
			Sample{std::move(that)}
		{
		}

		~X2dSample()
		{
		}
	}; // X2dSample

	class X3dSample :
		public Sample
	{
	public:
		X3dSample()
			:
			Sample{Type::surround}
		{
		}

		X3dSample(
			const X3dSample& that) = delete;

		X3dSample& operator=(
			const X3dSample& that) = delete;

		X3dSample(
			X3dSample&& that)
			:
			Sample{std::move(that)}
		{
		}

		~X3dSample()
		{
		}
	}; // X3dSample

	class StreamSample :
		public Sample
	{
	public:
		StreamSample()
			:
			Sample{Type::stream}
		{
		}

		StreamSample(
			const StreamSample& that) = delete;

		StreamSample& operator=(
			const StreamSample& that) = delete;

		StreamSample(
			StreamSample&& that)
			:
			Sample{std::move(that)}
		{
		}

		~StreamSample()
		{
		}
	}; // StreamSample

	using X2dSamples = std::list<X2dSample>;
	using X3dSamples = std::list<X3dSample>;
	using StreamSamples = std::list<StreamSample>;


	class Object3d
	{
	public:
		bool is_listener_;
		float min_distance_;
		float max_distance_;
		Vector3d position_;
		Vector3d velocity_;
		Orientation3d orientation_;
		float doppler_factor_;
		UserDataArray user_data_;
		X3dSample sample_;


		Object3d()
			:
			is_listener_{},
			min_distance_{},
			max_distance_{},
			position_{},
			velocity_{},
			orientation_{},
			doppler_factor_{},
			user_data_{},
			sample_{}
		{
		}

		Object3d(
			const Object3d& that) = delete;

		Object3d& operator=(
			const Object3d& that) = delete;

		Object3d(
			Object3d&& that)
			:
			is_listener_{std::move(that.is_listener_)},
			min_distance_{std::move(that.min_distance_)},
			max_distance_{std::move(that.max_distance_)},
			position_{std::move(that.position_)},
			velocity_{std::move(that.velocity_)},
			orientation_{std::move(that.orientation_)},
			doppler_factor_{std::move(that.doppler_factor_)},
			user_data_{std::move(that.user_data_)},
			sample_{std::move(that.sample_)}
		{
		}

		~Object3d()
		{
		}

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
	using Object3dUPtr = std::unique_ptr<Object3d>;


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
		StreamSample sample_;
		ul::FileStream file_stream_;
		ul::Substream file_substream_;
		ltjs::AudioDecoder decoder_;
		MixBuffer mix_buffer_;

		OalBuffers oal_queued_buffers_;
		OalBuffers oal_unqueued_buffers_;


		void uninitialize()
		{
			is_open_ = false;

			decoder_.close();
			file_substream_.close();
			file_stream_.close();

			sample_.reset();
		}

		bool initialize()
		{
			uninitialize();

			sample_.reset();

			return sample_.status_ != Sample::Status::failed;
		}

		void reset()
		{
			is_playing_ = {};
			mix_size_ = {};
			data_size_ = {};
			data_offset_ = {};

			oal_queued_buffers_.clear();
			oal_queued_buffers_.reserve(Sample::oal_max_buffer_count);

			oal_unqueued_buffers_.clear();
			oal_unqueued_buffers_.reserve(Sample::oal_max_buffer_count);
			oal_unqueued_buffers_.assign(sample_.oal_buffers_.cbegin(), sample_.oal_buffers_.cend());
		}

		int fill_mix_buffer()
		{
			const auto is_looping = sample_.is_looping_;

			const auto data_begin_offset = static_cast<int>(
				is_looping && sample_.has_loop_block_ ? sample_.loop_begin_ : 0);

			auto data_end_offset = static_cast<int>(
				is_looping && sample_.has_loop_block_ ? sample_.loop_end_ : data_size_);

			auto mix_offset = 0;

			while (mix_offset < mix_size_)
			{
				const auto data_remain_size = data_end_offset - data_offset_;

				if (data_remain_size == 0)
				{
					if (is_looping)
					{
						data_offset_ = data_begin_offset;

						const auto sample_offset = data_begin_offset / sample_.get_block_align();

						if (!decoder_.set_position(sample_offset))
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

				const auto mix_remain_size = mix_size_ - mix_offset;
				const auto to_decode_size = std::min(data_remain_size, mix_remain_size);

				if (to_decode_size == 0)
				{
					break;
				}

				const auto decoded_size = decoder_.decode(&mix_buffer_[mix_offset], to_decode_size);

				if (decoded_size > 0)
				{
					mix_offset += decoded_size;
					data_offset_ += decoded_size;
				}
				else
				{
					if (decoded_size == 0)
					{
						// Adjust the actual data size if necessary.
						//
						const auto actual_data_size = decoder_.get_decoded_size();

						if (data_size_ != actual_data_size)
						{
							data_size_ = actual_data_size;

							if (!is_looping)
							{
								data_end_offset = actual_data_size;
							}
						}
					}

					std::uninitialized_fill_n(mix_buffer_.begin() + mix_offset, mix_size_ - mix_offset, std::uint8_t{});

					break;
				}
			}

			return mix_offset;
		}

		//
		// Fills a stream with data.
		//
		// Returns:
		//    - "true" - if stream is processed.
		//    - "false" - if stream is not processed (paused, failed, etc).
		//
		bool mix()
		{
			if (sample_.status_ == Sample::Status::none || sample_.status_ == Sample::Status::failed)
			{
				return false;
			}

			const auto is_sample_playing = (sample_.status_ == Sample::Status::playing);

			if (is_playing_ && !is_sample_playing)
			{
				sample_.status_ = Sample::Status::playing;
			}
			else if (!is_playing_ && is_sample_playing)
			{
				sample_.status_ = Sample::Status::paused;

				::alSourcePausev(2, sample_.oal_sources_.data());

				return false;
			}
			else if (!is_playing_ && !is_sample_playing)
			{
				return false;
			}

			auto oal_processed = ALint{};
			auto oal_queued = ALint{};

			::alGetSourcei(sample_.oal_sources_[0], AL_BUFFERS_PROCESSED, &oal_processed);
			::alGetSourcei(sample_.oal_sources_[0], AL_BUFFERS_QUEUED, &oal_queued);

			if (oal_processed > 0)
			{
				auto buffers = Sample::OalBuffers{};
				const auto old_size = oal_unqueued_buffers_.size();

				oal_unqueued_buffers_.resize(
					old_size + std::min(oal_processed, Sample::oal_max_buffer_count));

				::alSourceUnqueueBuffers(
					sample_.oal_sources_[0],
					oal_processed,
					&oal_unqueued_buffers_[old_size]);

				::alSourceUnqueueBuffers(
					sample_.oal_sources_[1],
					oal_processed,
					buffers.data());
			}

			if (!sample_.is_looping_ && data_offset_ == data_size_)
			{
				if (oal_queued == 0)
				{
					is_playing_ = false;
					sample_.status_ = Sample::Status::paused;
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
				if (oal_unqueued_buffers_.empty())
				{
					break;
				}

				auto decoded_mix_size = fill_mix_buffer();

				if (decoded_mix_size == 0)
				{
					break;
				}

				const auto oal_buffer = oal_unqueued_buffers_.back();
				oal_unqueued_buffers_.pop_back();
				oal_queued_buffers_.push_back(oal_buffer);

				::alBufferData(
					oal_buffer,
					sample_.oal_buffer_format_,
					mix_buffer_.data(),
					mix_size_,
					static_cast<ALsizei>(sample_.get_sample_rate()));

				for (auto j = 0; j < 2; ++j)
				{
					::alSourceQueueBuffers(sample_.oal_sources_[j], 1, &oal_buffer);
				}

				queued_count += 1;

				if (decoded_mix_size < mix_size_)
				{
					break;
				}
			}

			if (queued_count > 0)
			{
				auto oal_state = ALint{};

				::alGetSourcei(sample_.oal_sources_[0], AL_SOURCE_STATE, &oal_state);

				if (oal_state != AL_PLAYING)
				{
					::alSourcePlayv(2, sample_.oal_sources_.data());
				}
			}

			return oal_processed > 0 || queued_count > 0;
		}
	}; // Stream

	using Streams = std::vector<Stream>;
	using OpenStreams = std::list<Stream*>;



	// =========================================================================
	// Utils
	// =========================================================================
	//

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
			stream.uninitialize();
		}

		streams_.clear();
		mt_open_streams_.clear();
	}

	void create_streams()
	{
		destroy_streams();

		streams_.resize(max_streams);

		auto is_succeed = true;

		for (auto& stream : streams_)
		{
			is_succeed &= stream.initialize();
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

						is_mixed |= stream.mix();

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


	const char* api_last_error() const
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

	void remove_samples()
	{
		samples_2d_.clear();
		objects_3d_.clear();
	}

	void update_listener_gain()
	{
		if (is_mute_)
		{
			::alListenerf(AL_GAIN, ltjs::AudioUtils::gain_min);
		}
		else
		{
			::alListenerf(AL_GAIN, oal_master_gain_);
		}
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
		oal_master_gain_ = ltjs::AudioUtils::gain_max;
		listener_3d_uptr_ = std::make_unique<Object3d>();
		reset_3d_object(*listener_3d_uptr_.get());

		return true;
	}

	void wave_out_close_internal()
	{
		uninitialize_eax20_filter();

		destroy_streams();

		remove_samples();

		listener_3d_uptr_ = {};

		oal_destroy_context();
		oal_close_device();
		oal_clear_efx();

		master_volume_ = {};
		oal_master_gain_ = {};
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

	sint32 initialize_sample_generic(
		LHSAMPLE sample_handle,
		const void* storage_ptr,
		const uint32 storage_size,
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

		if (!sample.initialize_from_address_generic(
			storage_ptr,
			storage_size,
			wave_format,
			playback_rate,
			filter_data_ptr,
			oal_is_supports_eax20_filter_,
			oal_effect_slot_))
		{
			return false;
		}

		return true;
	}

	void set_set_stream_loop(
		LHSTREAM stream_ptr,
		const bool is_enable)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.sample_.is_looping_ = is_enable;
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

	//
	// =========================================================================
	// API utils
	// =========================================================================


	// =========================================================================
	// API
	// =========================================================================
	//

	sint32 api_startup()
	{
#ifdef USE_EAX20_HARDWARE_FILTERS
		is_eax20_filters_defined_ = true;
#else
		is_eax20_filters_defined_ = false;
#endif // USE_EAX20_HARDWARE_FILTERS

		clock_base_ = Clock::now();

		return LS_OK;
	}

	void api_shutdown()
	{
	}

	std::uint32_t api_ms_count()
	{
		constexpr auto max_uint32_t = std::numeric_limits<std::uint32_t>::max();

		const auto time_diff = Clock::now() - clock_base_;
		const auto time_diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();

		return static_cast<std::uint32_t>(time_diff_ms % max_uint32_t);
	}

	sint32 api_wave_out_open(
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

	void api_wave_out_close(
		LHDIGDRIVER driver_ptr)
	{
		static_cast<void>(driver_ptr);

		wave_out_close_internal();
	}

	sint32 api_get_digital_master_volume(
		LHDIGDRIVER driver_ptr) const
	{
		if (!driver_ptr || driver_ptr != oal_device_)
		{
			return {};
		}

		return master_volume_;
	}

	void api_set_digital_master_volume(
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

		oal_master_gain_ = oal_gain;

		update_listener_gain();
	}

	LHSAMPLE api_allocate_sample_handle(
		LHDIGDRIVER driver_ptr)
	{
		if (driver_ptr != oal_device_)
		{
			return nullptr;
		}

		samples_2d_.emplace_back();

		return &samples_2d_.back();
	}

	void api_release_sample_handle(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		samples_2d_.remove_if(
			[&](const auto& sample)
			{
				return std::addressof(sample) == sample_ptr;
			}
		);
	}

	void api_stop_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		sample.pause();
	}

	void api_start_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		sample.play();
	}

	void api_resume_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		sample.resume();
	}

	void api_end_sample(
		LHSAMPLE sample_ptr)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		sample.stop();
	}

	sint32 api_get_sample_volume(
		LHSAMPLE sample_ptr) const
	{
		if (!sample_ptr)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		return sample.volume_;
	}

	void api_set_sample_volume(
		LHSAMPLE sample_ptr,
		const sint32 volume)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		sample.set_volume(volume);
	}

	sint32 api_get_sample_pan(
		LHSAMPLE sample_ptr) const
	{
		if (!sample_ptr)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		return sample.pan_;
	}

	void api_set_sample_pan(
		LHSAMPLE sample_ptr,
		const sint32 pan)
	{
		if (!sample_ptr)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_ptr);

		sample.set_pan(pan);
	}

	sint32 api_get_sample_user_data(
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

	void api_set_sample_user_data(
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

	bool api_initialize_sample_from_file(
		LHSAMPLE sample_handle,
		const void* storage_ptr,
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

		return sample.initialize_from_file_generic(
			audio_decoder_, storage_ptr, playback_rate, filter_data_ptr, oal_is_supports_eax20_filter_, oal_effect_slot_);
	}

	void api_set_sample_loop_block(
		LHSAMPLE sample_handle,
		const sint32 loop_begin_offset,
		const sint32 loop_end_offset,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		sample.set_loop_block(loop_begin_offset, loop_end_offset, is_enable);
	}

	void api_set_sample_loop(
		LHSAMPLE sample_handle,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		sample.set_loop(is_enable);
	}

	void api_set_sample_ms_position(
		LHSAMPLE sample_handle,
		const sint32 milliseconds)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		sample.set_ms_position(milliseconds);
	}

	uint32 api_get_sample_status(
		LHSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return {};
		}

		auto& sample = *static_cast<Sample*>(sample_handle);

		const auto status = sample.get_status();

		return status == Sample::Status::playing ? LS_PLAYING : LS_STOPPED;
	}

	LHSTREAM api_open_stream(
		const char* file_name,
		const uint32 file_offset,
		LHDIGDRIVER driver_ptr,
		const char* storage_ptr,
		const sint32 storage_size)
	{
		static_cast<void>(storage_ptr);
		static_cast<void>(storage_size);

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

		stream.reset();

		stream.data_size_ = decoder.get_data_size();

		const auto sample_size = decoder.get_sample_size();
		const auto sample_rate = decoder.get_sample_rate();
		const auto mix_sample_count = (Stream::mix_size_ms * sample_rate) / 1000;
		stream.mix_size_ = mix_sample_count * sample_size;

		stream.mix_buffer_.resize(stream.mix_size_);

		auto format = decoder.get_wave_format_ex();

		auto& sample = stream.sample_;

		const auto initialize_sample_result = sample.initialize_from_address_generic(
			nullptr,
			0,
			format,
			sample_rate,
			nullptr,
			oal_is_supports_eax20_filter_,
			oal_effect_slot_);

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

	void api_close_stream(
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

	void api_set_stream_ms_position(
		LHSTREAM stream_ptr,
		const sint32 milliseconds)
	{
		if (!stream_ptr || milliseconds < 0)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		const auto& sample = stream.sample_;
		const auto block_align = sample.get_block_align();
		const auto sample_rate = sample.get_sample_rate();

		auto position = static_cast<int>((milliseconds * sample_rate) / 1000LL);
		position /= block_align;
		position *= block_align;

		if (position > stream.data_size_)
		{
			return;
		}

		stream.data_offset_ = position;
	}

	void api_set_stream_user_data(
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

	sint32 api_get_stream_user_data(
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

	void api_start_stream(
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

	void api_pause_stream(
		LHSTREAM stream_ptr,
		const sint32 is_enable)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.is_playing_ = (is_enable != 0);

		if (stream.is_playing_)
		{
			mt_notify_stream();
		}
	}

	void api_set_stream_volume(
		LHSTREAM stream_ptr,
		const sint32 volume)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.sample_.set_volume(volume);
	}

	void api_set_stream_pan(
		LHSTREAM stream_ptr,
		const sint32 pan)
	{
		if (!stream_ptr)
		{
			return;
		}

		auto& stream = *static_cast<Stream*>(stream_ptr);

		MtMutexGuard mt_stream_lock{mt_stream_mutex_};

		stream.sample_.set_pan(pan);
	}

	sint32 api_set_stream_volume(
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

	sint32 api_get_stream_pan(
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

	uint32 api_get_stream_status(
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

	sint32 api_decode_mp3(
		const void* src_data_ptr,
		const uint32 src_data_size,
		const char* file_name_ext,
		void*& ds_wav_ptr,
		uint32& ds_wav_size,
		LTLENGTHYCB callback)
	{
		static_cast<void>(file_name_ext);
		static_cast<void>(callback);

		return ltjs::AudioUtils::decode_mp3(audio_decoder_, src_data_ptr, src_data_size, ds_wav_ptr, ds_wav_size);
	}

	void api_get_3d_provider_attribute(
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

	LH3DPOBJECT api_open_3d_listener(
		LHPROVIDER provider_id)
	{
		static_cast<void>(provider_id);

		return listener_3d_uptr_.get();
	}

	void api_close_3d_listener(
		LH3DPOBJECT listener_ptr)
	{
		static_cast<void>(listener_ptr);
	}

	void api_set_listener_doppler(
		LH3DPOBJECT listener_ptr,
		const float doppler_factor)
	{
		if (listener_ptr != listener_3d_uptr_.get())
		{
			return;
		}

		auto& listener = *listener_3d_uptr_.get();

		if (listener.doppler_factor_ == doppler_factor)
		{
			return;
		}

		listener.doppler_factor_ = doppler_factor;

		::alDopplerFactor(doppler_factor);
	}

	void api_set_3d_position(
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

	void api_set_3d_velocity_vector(
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

	void api_set_3d_orientation(
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

	void api_get_3d_position(
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

	void api_get_3d_velocity(
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

	void api_get_3d_orientation(
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

	LH3DSAMPLE api_allocate_3d_sample_handle(
		LHPROVIDER provider_id)
	{
		static_cast<void>(provider_id);

		objects_3d_.emplace_back();

		return &objects_3d_.back();
	}

	void api_release_3d_sample_handle(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		objects_3d_.remove_if(
			[&](const auto& object_3d_item)
			{
				return std::addressof(object_3d_item) == sample_handle;
			}
		);
	}

	void api_set_3d_sample_volume(
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

	void api_set_3d_sample_distances(
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

	void api_set_3d_user_data(
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

	sint32 api_get_3d_user_data(
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

	void api_stop_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.pause();
	}

	void api_start_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.play();
	}

	void api_resume_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.resume();
	}

	void api_end_3d_sample(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.stop();
	}

	sint32 api_initialize_3d_sample_from_address(
		LH3DSAMPLE sample_handle,
		const void* storage_ptr,
		const uint32 storage_size,
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

		return sample.initialize_from_address_generic(
			storage_ptr,
			storage_size,
			wave_format,
			playback_rate,
			filter_data_ptr,
			oal_is_supports_eax20_filter_,
			oal_effect_slot_);
	}

	sint32 api_initialize_3d_sample_from_file(
		LH3DSAMPLE sample_handle,
		const void* storage_ptr,
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

		return sample.initialize_from_file_generic(
			audio_decoder_, storage_ptr, playback_rate, filter_data_ptr, oal_is_supports_eax20_filter_, oal_effect_slot_);
	}

	sint32 api_get_3d_sample_volume(
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

	uint32 api_get_3d_sample_status(
		LH3DSAMPLE sample_handle)
	{
		if (!sample_handle)
		{
			return LS_STOPPED;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		const auto status = sample.get_status();

		return status == Sample::Status::playing ? LS_PLAYING : LS_STOPPED;
	}

	void api_set_3d_sample_ms_position(
		LHSAMPLE sample_handle,
		const sint32 milliseconds)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.set_ms_position(milliseconds);
	}

	void api_set_3d_sample_loop_block(
		LH3DSAMPLE sample_handle,
		const sint32 loop_begin_offset,
		const sint32 loop_end_offset,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.set_loop_block(loop_begin_offset, loop_end_offset, is_enable);
	}

	void api_set_3d_sample_loop(
		LH3DSAMPLE sample_handle,
		const bool is_enable)
	{
		if (!sample_handle)
		{
			return;
		}

		auto& object_3d = *static_cast<Object3d*>(sample_handle);
		auto& sample = object_3d.sample_;

		sample.set_loop(is_enable);
	}

	bool api_set_eax20_filter(
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

	bool api_supports_eax20_filter() const
	{
		return oal_is_supports_eax20_filter_;
	}

	bool api_set_eax20_buffer_settings(
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

	void api_handle_focus_lost(
		const bool is_focus_lost)
	{
		is_mute_ = is_focus_lost;

		update_listener_gain();
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
	float oal_master_gain_;
	bool is_mute_;
	X2dSamples samples_2d_;
	Object3dUPtr listener_3d_uptr_;
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
	return pimpl_->api_startup();
}

void OalSoundSys::Shutdown()
{
	pimpl_->api_shutdown();
}

uint32 OalSoundSys::MsCount()
{
	return pimpl_->api_ms_count();
}

sint32 OalSoundSys::SetPreference(
	const uint32 index,
	const sint32 value)
{
	static_cast<void>(index);
	static_cast<void>(value);

	return LS_ERROR;
}

sint32 OalSoundSys::GetPreference(
	const uint32 index)
{
	static_cast<void>(index);

	return {};
}

void OalSoundSys::MemFreeLock(
	void* storage_ptr)
{
	ltjs::AudioUtils::deallocate(storage_ptr);
}

void* OalSoundSys::MemAllocLock(
	const uint32 storage_size)
{
	return ltjs::AudioUtils::allocate(storage_size);
}

const char* OalSoundSys::LastError()
{
	return pimpl_->api_last_error();
}

sint32 OalSoundSys::WaveOutOpen(
	LHDIGDRIVER& driver,
	PHWAVEOUT& wave_out,
	const sint32 device_id,
	const ul::WaveFormatEx& wave_format)
{
	return pimpl_->api_wave_out_open(driver, wave_out, device_id, wave_format);
}

void OalSoundSys::WaveOutClose(
	LHDIGDRIVER driver_ptr)
{
	pimpl_->api_wave_out_close(driver_ptr);
}

void OalSoundSys::SetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr,
	const sint32 master_volume)
{
	pimpl_->api_set_digital_master_volume(driver_ptr, master_volume);
}

sint32 OalSoundSys::GetDigitalMasterVolume(
	LHDIGDRIVER driver_ptr)
{
	return pimpl_->api_get_digital_master_volume(driver_ptr);
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
	return pimpl_->api_set_eax20_filter(is_enable, filter_data);
}

bool OalSoundSys::SupportsEAX20Filter()
{
	return pimpl_->api_supports_eax20_filter();
}

bool OalSoundSys::SetEAX20BufferSettings(
	LHSAMPLE sample_handle,
	const LTSOUNDFILTERDATA& filter_data)
{
	return pimpl_->api_set_eax20_buffer_settings(sample_handle, filter_data);
}
#endif // USE_EAX20_HARDWARE_FILTERS

void OalSoundSys::Set3DProviderMinBuffers(
	const uint32 buffer_count)
{
	static_cast<void>(buffer_count);
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
	pimpl_->api_get_3d_provider_attribute(provider_id, name, value);
}

sint32 OalSoundSys::Enumerate3DProviders(
	LHPROENUM& index,
	LHPROVIDER& id,
	const char*& name)
{
	const auto current_index = index++;

	id = 0;
	name = nullptr;

	if (current_index < 0 || current_index > 0)
	{
		return false;
	}

	id = SOUND3DPROVIDERID_DS3D_HARDWARE;
	name = "OpenAL";

	return true;
}

LH3DPOBJECT OalSoundSys::Open3DListener(
	LHPROVIDER provider_id)
{
	return pimpl_->api_open_3d_listener(provider_id);
}

void OalSoundSys::Close3DListener(
	LH3DPOBJECT listener_ptr)
{
	pimpl_->api_close_3d_listener(listener_ptr);
}

void OalSoundSys::SetListenerDoppler(
	LH3DPOBJECT listener_ptr,
	const float doppler_factor)
{
	pimpl_->api_set_listener_doppler(listener_ptr, doppler_factor);
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
	pimpl_->api_set_3d_position(object_ptr, x, y, z);
}

void OalSoundSys::Set3DVelocityVector(
	LH3DPOBJECT object_ptr,
	const float dx_per_s,
	const float dy_per_s,
	const float dz_per_s)
{
	pimpl_->api_set_3d_velocity_vector(object_ptr, dx_per_s, dy_per_s, dz_per_s);
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
	pimpl_->api_set_3d_orientation(object_ptr, x_face, y_face, z_face, x_up, y_up, z_up);
}

void OalSoundSys::Set3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index,
	const sint32 value)
{
	pimpl_->api_set_3d_user_data(object_ptr, index, value);
}

void OalSoundSys::Get3DPosition(
	LH3DPOBJECT object_ptr,
	float& x,
	float& y,
	float& z)
{
	pimpl_->api_get_3d_position(object_ptr, x, y, z);
}

void OalSoundSys::Get3DVelocity(
	LH3DPOBJECT object_ptr,
	float& dx_per_ms,
	float& dy_per_ms,
	float& dz_per_ms)
{
	pimpl_->api_get_3d_velocity(object_ptr, dx_per_ms, dy_per_ms, dz_per_ms);
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
	pimpl_->api_get_3d_orientation(object_ptr, x_face, y_face, z_face, x_up, y_up, z_up);
}

sint32 OalSoundSys::Get3DUserData(
	LH3DPOBJECT object_ptr,
	const uint32 index)
{
	return pimpl_->api_get_3d_user_data(object_ptr, index);
}

LH3DSAMPLE OalSoundSys::Allocate3DSampleHandle(
	LHPROVIDER driver_id)
{
	return pimpl_->api_allocate_3d_sample_handle(driver_id);
}

void OalSoundSys::Release3DSampleHandle(
	LH3DSAMPLE sample_handle)
{
	return pimpl_->api_release_3d_sample_handle(sample_handle);
}

void OalSoundSys::Stop3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->api_stop_3d_sample(sample_handle);
}

void OalSoundSys::Start3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->api_start_3d_sample(sample_handle);
}

void OalSoundSys::Resume3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->api_resume_3d_sample(sample_handle);
}

void OalSoundSys::End3DSample(
	LH3DSAMPLE sample_handle)
{
	pimpl_->api_end_3d_sample(sample_handle);
}

sint32 OalSoundSys::Init3DSampleFromAddress(
	LH3DSAMPLE sample_handle,
	const void* ptr,
	const uint32 length,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->api_initialize_3d_sample_from_address(sample_handle, ptr, length, wave_format, playback_rate, filter_data_ptr);
}

sint32 OalSoundSys::Init3DSampleFromFile(
	LH3DSAMPLE sample_handle,
	const void* storage_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->api_initialize_3d_sample_from_file(sample_handle, storage_ptr, block, playback_rate, filter_data_ptr);
}

sint32 OalSoundSys::Get3DSampleVolume(
	LH3DSAMPLE sample_handle)
{
	return pimpl_->api_get_3d_sample_volume(sample_handle);
}

void OalSoundSys::Set3DSampleVolume(
	LH3DSAMPLE sample_handle,
	const sint32 volume)
{
	pimpl_->api_set_3d_sample_volume(sample_handle, volume);
}

uint32 OalSoundSys::Get3DSampleStatus(
	LH3DSAMPLE sample_handle)
{
	return pimpl_->api_get_3d_sample_status(sample_handle);
}

void OalSoundSys::Set3DSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	pimpl_->api_set_3d_sample_ms_position(sample_handle, milliseconds);
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
	pimpl_->api_set_3d_sample_distances(sample_handle, max_distance, min_distance);
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
	const sint32 loop_begin_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	pimpl_->api_set_3d_sample_loop_block(
		sample_handle, loop_begin_offset, loop_end_offset, is_enable);
}

void OalSoundSys::Set3DSampleLoop(
	LH3DSAMPLE sample_handle,
	const bool is_enable)
{
	pimpl_->api_set_3d_sample_loop(sample_handle, is_enable);
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
	return pimpl_->api_allocate_sample_handle(driver_ptr);
}

void OalSoundSys::ReleaseSampleHandle(
	LHSAMPLE sample_handle)
{
	pimpl_->api_release_sample_handle(sample_handle);
}

void OalSoundSys::InitSample(
	LHSAMPLE sample_handle)
{
	static_cast<void>(sample_handle);
}

void OalSoundSys::StopSample(
	LHSAMPLE sample_handle)
{
	pimpl_->api_stop_sample(sample_handle);
}

void OalSoundSys::StartSample(
	LHSAMPLE sample_handle)
{
	pimpl_->api_start_sample(sample_handle);
}

void OalSoundSys::ResumeSample(
	LHSAMPLE sample_handle)
{
	pimpl_->api_resume_sample(sample_handle);
}

void OalSoundSys::EndSample(
	LHSAMPLE sample_handle)
{
	pimpl_->api_end_sample(sample_handle);
}

void OalSoundSys::SetSampleVolume(
	LHSAMPLE sample_handle,
	const sint32 volume)
{
	pimpl_->api_set_sample_volume(sample_handle, volume);
}

void OalSoundSys::SetSamplePan(
	LHSAMPLE sample_handle,
	const sint32 pan)
{
	pimpl_->api_set_sample_pan(sample_handle, pan);
}

sint32 OalSoundSys::GetSampleVolume(
	LHSAMPLE sample_handle)
{
	return pimpl_->api_get_sample_volume(sample_handle);
}

sint32 OalSoundSys::GetSamplePan(
	LHSAMPLE sample_handle)
{
	return pimpl_->api_get_sample_pan(sample_handle);
}

void OalSoundSys::SetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index,
	const sint32 value)
{
	pimpl_->api_set_sample_user_data(sample_handle, index, value);
}

void OalSoundSys::GetDirectSoundInfo(
	LHSAMPLE sample_handle,
	PTDIRECTSOUND& ds_instance,
	PTDIRECTSOUNDBUFFER& ds_buffer)
{
	static_cast<void>(sample_handle);

	ds_instance = nullptr;
	ds_buffer = nullptr;
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
	const void* storage_ptr,
	const uint32 storage_size,
	const ul::WaveFormatEx& wave_format,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->initialize_sample_generic(
		sample_handle, storage_ptr, storage_size, wave_format, playback_rate, filter_data_ptr);
}

sint32 OalSoundSys::InitSampleFromFile(
	LHSAMPLE sample_handle,
	const void* storage_ptr,
	const sint32 block,
	const sint32 playback_rate,
	const LTSOUNDFILTERDATA* filter_data_ptr)
{
	return pimpl_->api_initialize_sample_from_file(
		sample_handle, storage_ptr, block, playback_rate, filter_data_ptr);
}

void OalSoundSys::SetSampleLoopBlock(
	LHSAMPLE sample_handle,
	const sint32 loop_begin_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	pimpl_->api_set_sample_loop_block(
		sample_handle, loop_begin_offset, loop_end_offset, is_enable);
}

void OalSoundSys::SetSampleLoop(
	LHSAMPLE sample_handle,
	const bool is_enable)
{
	pimpl_->api_set_sample_loop(sample_handle, is_enable);
}

void OalSoundSys::SetSampleMsPosition(
	LHSAMPLE sample_handle,
	const sint32 milliseconds)
{
	pimpl_->api_set_sample_ms_position(sample_handle, milliseconds);
}

sint32 OalSoundSys::GetSampleUserData(
	LHSAMPLE sample_handle,
	const uint32 index)
{
	return pimpl_->api_get_sample_user_data(sample_handle, index);
}

uint32 OalSoundSys::GetSampleStatus(
	LHSAMPLE sample_handle)
{
	return pimpl_->api_get_sample_status(sample_handle);
}

LHSTREAM OalSoundSys::OpenStream(
	const char* file_name,
	const uint32 file_offset,
	LHDIGDRIVER driver_ptr,
	const char* storage_ptr,
	const sint32 storage_size)
{
	return pimpl_->api_open_stream(
		file_name, file_offset, driver_ptr, storage_ptr, storage_size);
}

void OalSoundSys::SetStreamLoop(
	LHSTREAM stream_ptr,
	const bool is_enable)
{
	pimpl_->set_set_stream_loop(stream_ptr, is_enable);
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
	pimpl_->api_set_stream_ms_position(stream_ptr, milliseconds);
}

void OalSoundSys::SetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index,
	const sint32 value)
{
	pimpl_->api_set_stream_user_data(stream_ptr, index,value);
}

sint32 OalSoundSys::GetStreamUserData(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	return pimpl_->api_get_stream_user_data(stream_ptr, index);
}

void OalSoundSys::CloseStream(
	LHSTREAM stream_ptr)
{
	pimpl_->api_close_stream(stream_ptr);
}

void OalSoundSys::StartStream(
	LHSTREAM stream_ptr)
{
	pimpl_->api_start_stream(stream_ptr);
}

void OalSoundSys::PauseStream(
	LHSTREAM stream_ptr,
	const sint32 is_enable)
{
	pimpl_->api_pause_stream(stream_ptr, is_enable);
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
	pimpl_->api_set_stream_volume(stream_ptr, volume);
}

void OalSoundSys::SetStreamPan(
	LHSTREAM stream_ptr,
	const sint32 pan)
{
	pimpl_->api_set_stream_pan(stream_ptr, pan);
}

sint32 OalSoundSys::GetStreamVolume(
	LHSTREAM stream_ptr)
{
	return pimpl_->api_set_stream_volume(stream_ptr);
}

sint32 OalSoundSys::GetStreamPan(
	LHSTREAM stream_ptr)
{
	return pimpl_->api_get_stream_pan(stream_ptr);
}

uint32 OalSoundSys::GetStreamStatus(
	LHSTREAM stream_ptr)
{
	return pimpl_->api_get_stream_status(stream_ptr);
}

sint32 OalSoundSys::GetStreamBufferParam(
	LHSTREAM stream_ptr,
	const uint32 index)
{
	static_cast<void>(stream_ptr);
	static_cast<void>(index);

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
	const void* src_data_ptr,
	const uint32 src_data_size,
	const char* file_name_ext,
	void*& dst_wav_ptr,
	uint32& dst_wav_size,
	LTLENGTHYCB callback)
{
	return pimpl_->api_decode_mp3(src_data_ptr, src_data_size, file_name_ext, dst_wav_ptr, dst_wav_size, callback);
}

uint32 OalSoundSys::GetThreadedSoundTicks()
{
	return {};
}

bool OalSoundSys::HasOnBoardMemory()
{
	return {};
}

void OalSoundSys::handle_focus_lost(
	const bool is_focus_lost)
{
	pimpl_->api_handle_focus_lost(is_focus_lost);
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
