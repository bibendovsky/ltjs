#include "ltjs_oal_lt_sound_sys_streaming_source.h"

#include "bibendovsky_spul_algorithm.h"
#include "bibendovsky_spul_memory_stream.h"

#include "ltjs_audio_utils.h"

#include "ltjs_oal_utils.h"


namespace ltjs
{


const float OalLtSoundSysStreamingSource::min_doppler_factor = 0.0F;
const float OalLtSoundSysStreamingSource::max_doppler_factor = 10.0F;
const float OalLtSoundSysStreamingSource::default_3d_doppler_factor = 1.0F;

const float OalLtSoundSysStreamingSource::min_min_distance = 0.0F;
const float OalLtSoundSysStreamingSource::max_min_distance = std::numeric_limits<float>::max();
const float OalLtSoundSysStreamingSource::default_3d_min_distance = 1.0F;

const float OalLtSoundSysStreamingSource::min_max_distance = 0.0F;
const float OalLtSoundSysStreamingSource::max_max_distance = std::numeric_limits<float>::max();
const float OalLtSoundSysStreamingSource::default_3d_max_distance = 1'000'000'000.0F;

const OalLtSoundSysVector3d OalLtSoundSysStreamingSource::default_3d_position = OalLtSoundSysVector3d{};
const OalLtSoundSysVector3d OalLtSoundSysStreamingSource::default_3d_velocity = OalLtSoundSysVector3d{};
const OalLtSoundSysVector3d OalLtSoundSysStreamingSource::default_3d_direction = OalLtSoundSysVector3d{};
const OalLtSoundSysOrientation3d OalLtSoundSysStreamingSource::default_3d_orientation = OalLtSoundSysOrientation3d{};


OalLtSoundSysStreamingSource::OalLtSoundSysStreamingSource(
	const OalLtSoundSysStreamingSourceType type,
	const OalLtSoundSysStreamingSourceSpatialType spatial_type)
	:
	type_{},
	spatial_type_{},
	storage_type_{},
	status_{},
	channel_count_{},
	bit_depth_{},
	block_align_{},
	sample_rate_{},
	is_looping_{},
	has_loop_block_{},
	loop_begin_{},
	loop_end_{},
	volume_{},
	pan_{},
	pitch_{},
	data_{},
	user_data_array_{},
	is_playing_{},
	mix_size_{},
	mix_sample_count_{},
	data_size_{},
	data_offset_{},
	file_stream_{},
	file_substream_{},
	decoder_{},
	mix_mono_buffer_{},
	mix_stereo_buffer_{},
	master_3d_listener_volume_{},
	is_3d_listener_muted_{},
	doppler_factor_{},
	min_distance_{},
	max_distance_{},
	position_{},
	velocity_{},
	direction_{},
	orientation_{},
	oal_are_buffers_created_{},
	oal_is_source_created_{},
	oal_master_3d_listener_gain_{},
	oal_gain_{},
	oal_pans_{},
	oal_source_{},
	oal_buffer_format_{},
	oal_buffers_{},
	oal_queued_count_{},
	lt_filter_direct_mb_{}
{
	switch (type)
	{
		case OalLtSoundSysStreamingSourceType::panning:
			break;

		case OalLtSoundSysStreamingSourceType::spatial:
			break;

		default:
			throw "Invalid type.";
	}

	switch (spatial_type)
	{
		case OalLtSoundSysStreamingSourceSpatialType::none:
			break;

		case OalLtSoundSysStreamingSourceSpatialType::source:
			break;

		case OalLtSoundSysStreamingSourceSpatialType::listener:
			break;

		default:
			throw "Invalid spatial type.";
	}

	type_ = type;
	spatial_type_ = spatial_type;

	create();
}

OalLtSoundSysStreamingSource::OalLtSoundSysStreamingSource(
	OalLtSoundSysStreamingSource&& that)
	:
	type_{std::move(that.type_)},
	spatial_type_{std::move(that.spatial_type_)},
	storage_type_{std::move(that.storage_type_)},
	status_{std::move(that.status_)},
	channel_count_{std::move(that.channel_count_)},
	bit_depth_{std::move(that.bit_depth_)},
	block_align_{std::move(that.block_align_)},
	sample_rate_{std::move(that.sample_rate_)},
	is_looping_{std::move(that.is_looping_)},
	has_loop_block_{std::move(that.has_loop_block_)},
	loop_begin_{std::move(that.loop_begin_)},
	loop_end_{std::move(that.loop_end_)},
	volume_{std::move(that.volume_)},
	pan_{std::move(that.pan_)},
	pitch_{std::move(that.pitch_)},
	data_{std::move(that.data_)},
	user_data_array_{std::move(that.user_data_array_)},
	is_playing_{std::move(that.is_playing_)},
	mix_size_{std::move(that.mix_size_)},
	mix_sample_count_{std::move(that.mix_sample_count_)},
	data_size_{std::move(that.data_size_)},
	data_offset_{std::move(that.data_offset_)},
	file_stream_{std::move(that.file_stream_)},
	file_substream_{std::move(that.file_substream_)},
	decoder_{std::move(that.decoder_)},
	mix_mono_buffer_{std::move(that.mix_mono_buffer_)},
	mix_stereo_buffer_{std::move(that.mix_stereo_buffer_)},
	master_3d_listener_volume_{std::move(that.master_3d_listener_volume_)},
	is_3d_listener_muted_{std::move(that.is_3d_listener_muted_)},
	doppler_factor_{std::move(that.doppler_factor_)},
	min_distance_{std::move(that.min_distance_)},
	max_distance_{std::move(that.max_distance_)},
	position_{std::move(that.position_)},
	velocity_{std::move(that.velocity_)},
	direction_{std::move(that.direction_)},
	orientation_{std::move(that.orientation_)},
	oal_are_buffers_created_{std::move(that.oal_are_buffers_created_)},
	oal_is_source_created_{std::move(that.oal_is_source_created_)},
	oal_master_3d_listener_gain_{std::move(that.oal_master_3d_listener_gain_)},
	oal_gain_{std::move(that.oal_gain_)},
	oal_pans_{std::move(that.oal_pans_)},
	oal_source_{std::move(that.oal_source_)},
	oal_buffer_format_{std::move(that.oal_buffer_format_)},
	oal_buffers_{std::move(that.oal_buffers_)},
	oal_queued_count_{std::move(that.oal_queued_count_)},
	lt_filter_direct_mb_{std::move(that.lt_filter_direct_mb_)}
{
	that.oal_are_buffers_created_ = false;
	that.oal_is_source_created_ = false;
}

OalLtSoundSysStreamingSource::~OalLtSoundSysStreamingSource()
{
	destroy();
}


ALuint OalLtSoundSysStreamingSource::get_al_source() const noexcept
{
	return oal_source_;
}

int& OalLtSoundSysStreamingSource::get_lt_filter_direct_mb() noexcept
{
	return lt_filter_direct_mb_;
}

bool OalLtSoundSysStreamingSource::is_panning() const
{
	return type_ == OalLtSoundSysStreamingSourceType::panning;
}

bool OalLtSoundSysStreamingSource::is_spatial() const
{
	return type_ == OalLtSoundSysStreamingSourceType::spatial;
}

bool OalLtSoundSysStreamingSource::is_listener() const
{
	return spatial_type_ == OalLtSoundSysStreamingSourceSpatialType::listener;
}

bool OalLtSoundSysStreamingSource::is_mono() const
{
	return channel_count_ == 1;
}

bool OalLtSoundSysStreamingSource::is_stereo() const
{
	return channel_count_ == 2;
}

int OalLtSoundSysStreamingSource::get_channel_count() const
{
	return channel_count_;
}

int OalLtSoundSysStreamingSource::get_bit_depth() const
{
	return bit_depth_;
}

int OalLtSoundSysStreamingSource::get_block_align() const
{
	return block_align_;
}

int OalLtSoundSysStreamingSource::get_dst_sample_rate() const
{
	return sample_rate_;
}

bool OalLtSoundSysStreamingSource::is_playing()
{
	return status_ == OalLtSoundSysStreamingSourceStatus::playing;
}

bool OalLtSoundSysStreamingSource::is_stopped()
{
	return get_status() == OalLtSoundSysStreamingSourceStatus::stopped;
}

bool OalLtSoundSysStreamingSource::is_failed()
{
	return status_ == OalLtSoundSysStreamingSourceStatus::failed;
}

void OalLtSoundSysStreamingSource::pause()
{
	if (is_failed())
	{
		return;
	}

	is_playing_ = false;
}

void OalLtSoundSysStreamingSource::resume()
{
	if (is_failed())
	{
		return;
	}

	is_playing_ = true;
}

void OalLtSoundSysStreamingSource::stop()
{
	if (is_failed())
	{
		return;
	}

	is_playing_ = false;

	data_offset_ = 0;

	if (is_looping_ && has_loop_block_)
	{
		data_offset_ = loop_begin_;
	}
}

int32 OalLtSoundSysStreamingSource::get_volume() const
{
	return volume_;
}

void OalLtSoundSysStreamingSource::set_volume(
	const sint32 volume)
{
	if (is_failed() || !is_panning())
	{
		return;
	}

	if (volume_ == volume)
	{
		return;
	}

	volume_ = volume;

	oal_gain_ = AudioUtils::lt_volume_to_gain(volume_);

	LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_GAIN, oal_gain_));
}

int32 OalLtSoundSysStreamingSource::get_pan() const
{
	return pan_;
}

void OalLtSoundSysStreamingSource::set_pan(
	const sint32 pan)
{
	if (is_failed() || !is_panning() || is_stereo())
	{
		return;
	}

	if (pan_ == pan)
	{
		return;
	}

	pan_ = pan;

	AudioUtils::lt_pan_to_gains(pan_, oal_pans_[0], oal_pans_[1]);
}

bool OalLtSoundSysStreamingSource::open(
	const OalLtSoundSysStreamingSourceOpenParam& param)
{
	if (!open_internal(param))
	{
		close_internal();
		return false;
	}

	return true;
}

void OalLtSoundSysStreamingSource::set_loop_block(
	const sint32 loop_begin_offset,
	const sint32 loop_end_offset,
	const bool is_enable)
{
	if (is_failed())
	{
		return;
	}

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

void OalLtSoundSysStreamingSource::set_loop(
	const bool is_enable)
{
	if (is_failed())
	{
		return;
	}

	is_looping_ = is_enable;
}

void OalLtSoundSysStreamingSource::set_ms_position(
	const sint32 milliseconds)
{
	if (is_failed())
	{
		return;
	}

	if (milliseconds <= 0)
	{
		data_offset_ = 0;
		return;
	}

	const auto sample_offset = static_cast<int>((sample_rate_ * milliseconds) / 1000LL);
	const auto data_offset = sample_offset * block_align_;

	if (data_offset <= data_size_)
	{
		data_offset_ = data_offset;
	}
	else
	{
		data_offset_ = 0;
	}
}

void OalLtSoundSysStreamingSource::mix()
{
	if (is_failed())
	{
		return;
	}

	const auto is_sample_playing = (status_ == OalLtSoundSysStreamingSourceStatus::playing);

	if (!is_playing_)
	{
		if (is_sample_playing)
		{
			LTJS_OAL_ENSURE_CALL_DEBUG(alSourcePause(oal_source_));

			status_ = OalLtSoundSysStreamingSourceStatus::stopped;
			return;
		}
		else
		{
			return;
		}
	}

	auto oal_processed = ALint{};

	LTJS_OAL_ENSURE_CALL_DEBUG(alGetSourcei(oal_source_, AL_BUFFERS_PROCESSED, &oal_processed));

	if (oal_processed > 0)
	{
		std::rotate(oal_buffers_.begin(), oal_buffers_.begin() + oal_processed, oal_buffers_.end());

		LTJS_OAL_ENSURE_CALL_DEBUG(alSourceUnqueueBuffers(
			oal_source_,
			oal_processed,
			&oal_buffers_[oal_max_buffer_count - oal_processed]));

		oal_queued_count_ -= oal_processed;
		assert(oal_queued_count_ >= 0);
	}

	if (!is_looping_ && data_offset_ == data_size_)
	{
		if (oal_queued_count_ == 0)
		{
			is_playing_ = false;
			LTJS_OAL_ENSURE_CALL_DEBUG(alSourcePause(oal_source_));

			status_ = OalLtSoundSysStreamingSourceStatus::stopped;
			return;
		}

		if (oal_queued_count_ > 0)
		{
			return;
		}
	}


	auto queued_count = 0;

	for (auto i = oal_queued_count_; i < oal_max_buffer_count; ++i)
	{
		auto decoded_mix_size = mix_fill_buffer();

		if (decoded_mix_size == 0)
		{
			break;
		}

		auto buffer_size = mix_size_;
		auto buffer_data = mix_mono_buffer_.data();

		if (!is_spatial() && is_mono())
		{
			buffer_size *= 2;
			buffer_data = mix_stereo_buffer_.data();
		}

		const auto oal_buffer = oal_buffers_[oal_queued_count_];

		LTJS_OAL_ENSURE_CALL_DEBUG(alBufferData(
			oal_buffer,
			oal_buffer_format_,
			buffer_data,
			buffer_size,
			get_dst_sample_rate()));

		LTJS_OAL_ENSURE_CALL_DEBUG(alSourceQueueBuffers(oal_source_, 1, &oal_buffer));

		queued_count += 1;
		oal_queued_count_ += 1;

		if (decoded_mix_size < mix_size_)
		{
			break;
		}
	}

	if (queued_count > 0)
	{
		const auto oal_status = get_status();

		if (is_failed())
		{
			return;
		}

		if (oal_status != OalLtSoundSysStreamingSourceStatus::playing)
		{
			LTJS_OAL_ENSURE_CALL_DEBUG(alSourcePlay(oal_source_));

			status_ = OalLtSoundSysStreamingSourceStatus::playing;
		}
	}
}

std::intptr_t OalLtSoundSysStreamingSource::get_user_value(
	const uint32 index) const
{
	if (index < 0 || index > oal_lt_sound_sys_max_user_data_index)
	{
		return 0;
	}

	return user_data_array_[index];
}

void OalLtSoundSysStreamingSource::set_user_value(
	const uint32 index,
	const std::intptr_t value)
{
	if (index < 0 || index > oal_lt_sound_sys_max_user_data_index)
	{
		return;
	}

	user_data_array_[index] = value;
}

sint32 OalLtSoundSysStreamingSource::get_master_3d_listener_volume() const
{
	return master_3d_listener_volume_;
}

void OalLtSoundSysStreamingSource::set_master_3d_listener_volume(
	const sint32 lt_volume)
{
	if (is_failed() || !is_spatial() && !is_listener())
	{
		return;
	}

	if (master_3d_listener_volume_ = lt_volume)
	{
		return;
	}

	master_3d_listener_volume_ = lt_volume;

	oal_master_3d_listener_gain_ = AudioUtils::lt_volume_to_gain(master_3d_listener_volume_);

	if (is_3d_listener_muted_)
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerf(AL_GAIN, AudioUtils::min_gain));
	}
	else
	{
		const auto oal_gain = oal_gain_ * oal_master_3d_listener_gain_;

		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerf(AL_GAIN, oal_gain));
	}
}

void OalLtSoundSysStreamingSource::mute_3d_listener(
	const bool is_muted)
{
	if (is_failed() || !is_spatial() && !is_listener())
	{
		return;
	}

	if (is_muted == is_3d_listener_muted_)
	{
		return;
	}

	is_3d_listener_muted_ = is_muted;


	if (is_3d_listener_muted_)
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerf(AL_GAIN, AudioUtils::min_gain));
	}
	else
	{
		const auto oal_gain = oal_gain_ * oal_master_3d_listener_gain_;

		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerf(AL_GAIN, oal_gain));
	}
}

void OalLtSoundSysStreamingSource::set_3d_volume(
	const sint32 lt_volume)
{
	if (is_failed() || !is_spatial())
	{
		return;
	}

	if (volume_ == lt_volume)
	{
		return;
	}

	volume_ = lt_volume;

	oal_gain_ = AudioUtils::lt_volume_to_gain(volume_);

	if (is_listener())
	{
		if (is_3d_listener_muted_)
		{
			oal_gain_ = 0.0F;
		}
		else
		{
			oal_gain_ *= oal_master_3d_listener_gain_;
		}

		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerf(AL_GAIN, oal_gain_));
	}
	else
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_GAIN, oal_gain_));
	}
}

sint32 OalLtSoundSysStreamingSource::get_3d_volume() const
{
	return volume_;
}

void OalLtSoundSysStreamingSource::set_3d_doppler_factor(
	const float doppler_factor)
{
	if (is_failed() || !is_spatial() || !is_listener() || std::isnan(doppler_factor))
	{
		return;
	}

	const auto new_doppler_factor = ul::Algorithm::clamp(
		doppler_factor, min_doppler_factor, max_doppler_factor);

	if (new_doppler_factor == doppler_factor_)
	{
		return;
	}

	doppler_factor_ = new_doppler_factor;

	LTJS_OAL_ENSURE_CALL_DEBUG(alDopplerFactor(doppler_factor_));
}

float OalLtSoundSysStreamingSource::get_3d_doppler_factor() const
{
	return doppler_factor_;
}

void OalLtSoundSysStreamingSource::set_3d_distances(
	const float min_distance,
	const float max_distance)
{
	if (is_failed() || !is_spatial() || is_listener() || std::isnan(min_distance) || std::isnan(max_distance))
	{
		return;
	}

	if (min_distance >= max_distance)
	{
		return;
	}

	const auto new_min_distance = ul::Algorithm::clamp(min_distance, min_min_distance, min_max_distance);
	const auto new_max_distance = ul::Algorithm::clamp(max_distance, max_min_distance, max_max_distance);

	if (new_min_distance == min_distance_ && new_max_distance == max_distance_)
	{
		return;
	}

	min_distance_ = new_min_distance;
	max_distance_ = new_max_distance;

	LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_REFERENCE_DISTANCE, min_distance_));
	LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_MAX_DISTANCE, max_distance_));
}

float OalLtSoundSysStreamingSource::get_3d_min_distance() const
{
	return min_distance_;
}

float OalLtSoundSysStreamingSource::get_3d_max_distance() const
{
	return max_distance_;
}

void OalLtSoundSysStreamingSource::set_3d_position(
	const OalLtSoundSysVector3d& position)
{
	if (is_failed() || !is_spatial() || position.has_nan())
	{
		return;
	}

	if (position == position_)
	{
		return;
	}

	position_ = position;

	const auto& oal_position = position_.to_rhs();
	const auto oal_position_c_array = oal_position.get_c_array();

	if (is_listener())
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerfv(AL_POSITION, oal_position_c_array));
	}
	else
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcefv(oal_source_, AL_POSITION, oal_position_c_array));
	}
}

const OalLtSoundSysVector3d& OalLtSoundSysStreamingSource::get_3d_position() const
{
	return position_;
}

void OalLtSoundSysStreamingSource::set_3d_velocity(
	const OalLtSoundSysVector3d& velocity)
{
	if (is_failed() || !is_spatial() || velocity.has_nan())
	{
		return;
	}

	if (velocity == velocity_)
	{
		return;
	}

	velocity_ = velocity;

	const auto& oal_velocity = velocity_.to_rhs();
	const auto oal_velocity_c_array = oal_velocity.get_c_array();

	if (is_listener())
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerfv(AL_VELOCITY, oal_velocity_c_array));
	}
	else
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcefv(oal_source_, AL_VELOCITY, oal_velocity_c_array));
	}
}

const OalLtSoundSysVector3d& OalLtSoundSysStreamingSource::get_3d_velocity() const
{
	return velocity_;
}

void OalLtSoundSysStreamingSource::set_3d_direction(
	const OalLtSoundSysVector3d& direction)
{
	if (is_failed() || !is_spatial() || is_listener() || direction.has_nan())
	{
		return;
	}

	if (direction == direction_)
	{
		return;
	}

	direction_ = direction;

	const auto& oal_direction = direction_.to_rhs();

	LTJS_OAL_ENSURE_CALL_DEBUG(alSourcefv(oal_source_, AL_DIRECTION, oal_direction.get_c_array()));
}

const OalLtSoundSysVector3d& OalLtSoundSysStreamingSource::get_3d_direction() const
{
	return direction_;
}

void OalLtSoundSysStreamingSource::set_3d_orientation(
	const OalLtSoundSysOrientation3d& orientation)
{
	if (is_failed() || !is_spatial() || !is_listener() || orientation.has_nan())
	{
		return;
	}

	if (orientation == orientation_)
	{
		return;
	}

	orientation_ = orientation;

	const auto& oal_orientation = orientation_.to_rhs();

	LTJS_OAL_ENSURE_CALL_DEBUG(alListenerfv(AL_ORIENTATION, oal_orientation.get_c_array()));
}

const OalLtSoundSysOrientation3d& OalLtSoundSysStreamingSource::get_3d_orientation() const
{
	return orientation_;
}

template<
	typename TDummy
>
struct OalLtSoundSysStreamingSource::MonoToStereoSample<8, TDummy>
{
	using Value = std::uint8_t;

	static Value get_silence()
	{
		return 0x80;
	}

	static float to_gain(
		const Value value)
	{
		return (value - 128) / 128.0F;
	}

	static Value to_volume(
		const float gain)
	{
		return static_cast<Value>(ul::Algorithm::clamp(static_cast<int>((gain * 128.0F) + 128.5F), 0, 255));
	}
}; // MonoToStereoSample<8>

template<
	typename TDummy
>
struct OalLtSoundSysStreamingSource::MonoToStereoSample<16, TDummy>
{
	using Value = std::int16_t;

	static Value get_silence()
	{
		return 0;
	}

	static float to_gain(
		const Value value)
	{
		return value / 32768.0F;
	}

	static Value to_volume(
		const float gain)
	{
		return static_cast<Value>(ul::Algorithm::clamp(static_cast<int>(gain * 32768.0F), -32768, 32767));
	}
}; // MonoToStereoSample<16>

ALenum OalLtSoundSysStreamingSource::get_oal_buffer_format(
	const int channel_count,
	const int bit_depth)
{
	switch (channel_count)
	{
	case 1:
		switch (bit_depth)
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
		switch (bit_depth)
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

bool OalLtSoundSysStreamingSource::validate_wave_format_ex(
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

void OalLtSoundSysStreamingSource::create()
{
	if (is_listener())
	{
		oal_reset_spatial();
		return;
	}

	auto is_succeed = true;

	oal::clear_error();

	alGenBuffers(oal_max_buffer_count, oal_buffers_.data());

	if (oal::is_succeed())
	{
		oal_are_buffers_created_ = true;
	}
	else
	{
		is_succeed = false;
		assert(!"alGenBuffers");
	}

	alGenSources(1, &oal_source_);

	if (oal::is_succeed())
	{
		oal_is_source_created_ = true;
	}
	else
	{
		is_succeed = false;
		assert(!"alGenSources");
	}

	if (!is_succeed)
	{
		destroy();

		status_ = OalLtSoundSysStreamingSourceStatus::failed;

		return;
	}

	status_ = OalLtSoundSysStreamingSourceStatus{};
}

void OalLtSoundSysStreamingSource::destroy()
{
	if (is_listener())
	{
		return;
	}

	if (oal_is_source_created_)
	{
		oal_is_source_created_ = false;

		LTJS_OAL_ENSURE_CALL_DEBUG(alSourceStop(oal_source_));
		LTJS_OAL_ENSURE_CALL_DEBUG(alDeleteSources(1, &oal_source_));
	}

	if (oal_are_buffers_created_)
	{
		oal_are_buffers_created_ = false;

		LTJS_OAL_ENSURE_CALL_DEBUG(alDeleteBuffers(oal_max_buffer_count, oal_buffers_.data()));
	}

	decoder_.close();
	file_substream_.close();
	file_stream_.close();

	status_ = OalLtSoundSysStreamingSourceStatus{};
}

OalLtSoundSysStreamingSourceStatus OalLtSoundSysStreamingSource::get_status()
{
	switch (status_)
	{
	case OalLtSoundSysStreamingSourceStatus::failed:
	case OalLtSoundSysStreamingSourceStatus::stopped:
		return status_;

	default:
		break;
	}

	auto oal_state = ALint{};

	LTJS_OAL_ENSURE_CALL_DEBUG(alGetSourcei(oal_source_, AL_SOURCE_STATE, &oal_state));

	auto status = OalLtSoundSysStreamingSourceStatus::none;

	switch (oal_state)
	{
	case AL_INITIAL:
	case AL_PAUSED:
	case AL_STOPPED:
		status = OalLtSoundSysStreamingSourceStatus::stopped;
		break;

	case AL_PLAYING:
		status = OalLtSoundSysStreamingSourceStatus::playing;
		break;

	default:
		status = OalLtSoundSysStreamingSourceStatus::failed;
		break;
	}

	status_ = status;

	return status;
}

void OalLtSoundSysStreamingSource::fail()
{
	is_playing_ = false;
	status_ = OalLtSoundSysStreamingSourceStatus::failed;

	alSourceStop(oal_source_);

	oal::clear_error();
}

void OalLtSoundSysStreamingSource::reset()
{
	decoder_.close();
	file_substream_.close();
	file_stream_.close();

	is_playing_ = false;
	mix_size_ = 0;
	mix_sample_count_ = 0;
	data_size_ = 0;
	data_offset_ = 0;

	oal_queued_count_ = 0;

	channel_count_ = 0;
	bit_depth_ = 0;
	block_align_ = 0;
	sample_rate_ = 0;

	is_looping_ = false;
	has_loop_block_ = false;
	loop_begin_ = 0;
	loop_end_ = 0;
	volume_ = AudioUtils::max_lt_volume;
	pan_ = pan_center;
}

void OalLtSoundSysStreamingSource::oal_reset_common()
{
	pitch_ = default_pitch;
	oal_buffer_format_ = AL_NONE;
	oal_gain_ = AudioUtils::default_gain;
	AudioUtils::lt_pan_to_gains(AudioUtils::lt_pan_center, oal_pans_[0], oal_pans_[1]);

	alSourceStop(oal_source_);
	alSourcei(oal_source_, AL_BUFFER, AL_NONE);
	alSourcef(oal_source_, AL_PITCH, default_pitch);
	alSourcef(oal_source_, AL_GAIN, AudioUtils::default_gain);
}

void OalLtSoundSysStreamingSource::oal_reset_spatial()
{
	if (!is_spatial())
	{
		return;
	}

	oal_gain_ = AudioUtils::default_gain;
	master_3d_listener_volume_ = AudioUtils::max_lt_volume;
	oal_master_3d_listener_gain_ = AudioUtils::default_gain;
	is_3d_listener_muted_ = false;

	doppler_factor_ = default_3d_doppler_factor;
	min_distance_ = default_3d_min_distance;
	max_distance_ = default_3d_max_distance;
	position_ = default_3d_position;
	velocity_ = default_3d_velocity;
	direction_ = default_3d_direction;
	orientation_ = default_3d_orientation;

	oal::clear_error();

	if (is_listener())
	{
		const auto oal_position = position_.to_rhs();
		const auto oal_velocity = velocity_.to_rhs();
		const auto oal_orientation = orientation_.to_rhs();

		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerf(AL_GAIN, oal_gain_));
		LTJS_OAL_ENSURE_CALL_DEBUG(alDopplerFactor(doppler_factor_));
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerfv(AL_POSITION, oal_position.get_c_array()));
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerfv(AL_VELOCITY, oal_velocity.get_c_array()));
		LTJS_OAL_ENSURE_CALL_DEBUG(alListenerfv(AL_ORIENTATION, oal_orientation.get_c_array()));
	}
	else
	{
		const auto oal_position = position_.to_rhs();
		const auto oal_velocity = velocity_.to_rhs();
		const auto oal_direction = direction_.to_rhs();

		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_GAIN, oal_gain_));
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcei(oal_source_, AL_SOURCE_RELATIVE, AL_FALSE));
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_REFERENCE_DISTANCE, min_distance_));
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_MAX_DISTANCE, max_distance_));
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcefv(oal_source_, AL_POSITION, oal_position.get_c_array()));
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcefv(oal_source_, AL_VELOCITY, oal_velocity.get_c_array()));
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcefv(oal_source_, AL_DIRECTION, oal_direction.get_c_array()));
	}

	if (!oal::is_succeed())
	{
		fail();
	}
}

void OalLtSoundSysStreamingSource::open_set_wave_format_internal(
	const ul::WaveFormatEx& wave_format)
{
	channel_count_ = wave_format.channel_count_;
	bit_depth_ = wave_format.bit_depth_;
	block_align_ = wave_format.block_align_;
	sample_rate_ = static_cast<int>(wave_format.sample_rate_);
}

bool OalLtSoundSysStreamingSource::open_file_internal(
	const OalLtSoundSysStreamingSourceOpenParam& param)
{
	if (!param.is_file_ || param.file_name_ == nullptr)
	{
		return false;
	}

	const auto open_file_result = file_stream_.open(param.file_name_, ul::Stream::OpenMode::read);

	if (!open_file_result)
	{
		return false;
	}

	const auto open_subfile_result = file_substream_.open(&file_stream_, param.file_offset_);

	if (!open_subfile_result)
	{
		return false;
	}

	auto decoder_param = AudioDecoder::OpenParam{};
	decoder_param.stream_ptr_ = &file_substream_;

	const auto open_decoder_result = decoder_.open(decoder_param);

	if (!open_decoder_result)
	{
		return false;
	}

	storage_type_ = OalLtSoundSysStreamingSourceStorageType::decoder;
	data_.clear();
	data_size_ = decoder_.get_data_size();

	open_set_wave_format_internal(decoder_.get_wave_format_ex());

	return true;
}

bool OalLtSoundSysStreamingSource::open_mapped_file_internal(
	const OalLtSoundSysStreamingSourceOpenParam& param)
{
	if (!param.is_mapped_file_ ||
		param.mapped_storage_ptr == nullptr ||
		param.mapped_decoder_ == nullptr)
	{
		return false;
	}

	const auto mapped_file_size = AudioUtils::extract_wave_size(param.mapped_storage_ptr);

	if (mapped_file_size <= 0)
	{
		return false;
	}

	auto memory_stream = ul::MemoryStream{param.mapped_storage_ptr, mapped_file_size, ul::Stream::OpenMode::read};

	if (!memory_stream.is_open())
	{
		return false;
	}

	auto decoder_param = AudioDecoder::OpenParam{};
	decoder_param.stream_ptr_ = &memory_stream;

	auto& decoder = *param.mapped_decoder_;

	if (!decoder.open(decoder_param))
	{
		return false;
	}

	const auto max_decoded_size = decoder.get_data_size();

	data_.resize(max_decoded_size);

	const auto decoded_size = decoder.decode(data_.data(), max_decoded_size);

	if (decoded_size <= 0)
	{
		return false;
	}

	storage_type_ = OalLtSoundSysStreamingSourceStorageType::internal_buffer;
	data_size_ = decoded_size;

	open_set_wave_format_internal(decoder.get_wave_format_ex());

	return true;
}

bool OalLtSoundSysStreamingSource::open_memory_internal(
	const OalLtSoundSysStreamingSourceOpenParam& param)
{
	if (!param.is_memory_ || param.memory_ptr_ == nullptr || param.memory_size_ <= 0)
	{
		return false;
	}

	if (!validate_wave_format_ex(param.memory_wave_format_))
	{
		return false;
	}

	data_.resize(param.memory_size_);

	std::uninitialized_copy_n(
		static_cast<const std::uint8_t*>(param.memory_ptr_),
		param.memory_size_,
		data_.begin());

	storage_type_ = OalLtSoundSysStreamingSourceStorageType::internal_buffer;

	data_size_ = static_cast<int>(param.memory_size_);

	open_set_wave_format_internal(param.memory_wave_format_);

	return true;
}

bool OalLtSoundSysStreamingSource::open_initialize_internal(
	const OalLtSoundSysStreamingSourceOpenParam& param)
{
	if (is_spatial() && !is_mono())
	{
		return false;
	}

	oal_reset_common();
	oal_reset_spatial();

	const auto has_pitch = (param.playback_rate_ > 0 && sample_rate_ != param.playback_rate_);

	if (has_pitch)
	{
		pitch_ = static_cast<float>(param.playback_rate_) / static_cast<float>(sample_rate_);
	}

	mix_sample_count_ = static_cast<int>((pitch_ * mix_size_ms * sample_rate_) / 1000.0F);
	mix_size_ = mix_sample_count_ * block_align_;

	mix_mono_buffer_.resize(mix_size_);

	if (is_spatial())
	{
		oal_buffer_format_ = get_oal_buffer_format(channel_count_, bit_depth_);
	}
	else
	{
		if (is_mono())
		{
			mix_stereo_buffer_.resize(mix_size_ * 2);
		}

		oal_buffer_format_ = get_oal_buffer_format(2, bit_depth_);
	}

	oal::clear_error();

	alSourcef(oal_source_, AL_PITCH, pitch_);

	if (!oal::is_succeed())
	{
		status_ = OalLtSoundSysStreamingSourceStatus::failed;
		return false;
	}

	is_playing_ = false;
	status_ = OalLtSoundSysStreamingSourceStatus::stopped;

	return true;
}

bool OalLtSoundSysStreamingSource::open_internal(
	const OalLtSoundSysStreamingSourceOpenParam& param)
{
	reset();

	if (param.is_file_)
	{
		if (!open_file_internal(param))
		{
			return false;
		}
	}
	else if (param.is_mapped_file_)
	{
		if (!open_mapped_file_internal(param))
		{
			return false;
		}
	}
	else if (param.is_memory_)
	{
		if (!open_memory_internal(param))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	if (!open_initialize_internal(param))
	{
		return false;
	}

	return true;
}

void OalLtSoundSysStreamingSource::close_internal()
{
	decoder_.close();
	file_substream_.close();
	file_stream_.close();
}

template<
	int TBitDepth
>
void OalLtSoundSysStreamingSource::mix_mono_to_stereo(
	const int byte_offset)
{
	using Sample = MonoToStereoSample<TBitDepth>;

	const auto sample_count = byte_offset / block_align_;
	const auto oal_left_pan = oal_pans_[0];
	const auto oal_right_pan = oal_pans_[1];

	const auto src_buffer = reinterpret_cast<const typename Sample::Value*>(mix_mono_buffer_.data());
	auto dst_buffer = reinterpret_cast<typename Sample::Value*>(mix_stereo_buffer_.data());

	auto dst_index = 0;

	for (auto i = 0; i < sample_count; ++i)
	{
		const auto oal_gain = Sample::to_gain(src_buffer[i]);
		const auto oal_left_gain = oal_gain * oal_left_pan;
		const auto oal_right_gain = oal_gain * oal_right_pan;

		const auto left_volume = Sample::to_volume(oal_left_gain);
		const auto right_volume = Sample::to_volume(oal_right_gain);

		dst_buffer[dst_index++] = left_volume;
		dst_buffer[dst_index++] = right_volume;
	}

	const auto remain_sample_count = mix_sample_count_ - sample_count;

	if (remain_sample_count > 0)
	{
		std::uninitialized_fill_n(
			reinterpret_cast<typename Sample::Value*>(&dst_buffer[dst_index]),
			remain_sample_count * 2,
			Sample::get_silence());
	}
}

int OalLtSoundSysStreamingSource::mix_fill_buffer()
{
	const auto is_looping = is_looping_;

	const auto data_begin_offset = static_cast<int>(
		is_looping && has_loop_block_ ? loop_begin_ : 0);

	auto data_end_offset = static_cast<int>(
		is_looping && has_loop_block_ ? loop_end_ : data_size_);

	auto mix_offset = 0;

	while (mix_offset < mix_size_)
	{
		const auto data_remain_size = data_end_offset - data_offset_;

		if (data_remain_size == 0)
		{
			if (is_looping)
			{
				data_offset_ = data_begin_offset;

				const auto sample_offset = data_begin_offset / get_block_align();

				if (storage_type_ == OalLtSoundSysStreamingSourceStorageType::decoder)
				{
					if (!decoder_.set_position(sample_offset))
					{
						fail();
						break;
					}
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

		auto decoded_size = 0;

		if (storage_type_ == OalLtSoundSysStreamingSourceStorageType::decoder)
		{
			decoded_size = decoder_.decode(&mix_mono_buffer_[mix_offset], to_decode_size);
		}
		else if (storage_type_ == OalLtSoundSysStreamingSourceStorageType::internal_buffer)
		{
			decoded_size = std::min(to_decode_size, data_end_offset - data_offset_);

			std::uninitialized_copy_n(
				data_.cbegin() + data_offset_,
				decoded_size,
				mix_mono_buffer_.begin() + mix_offset);
		}
		else
		{
			fail();
			break;
		}

		if (decoded_size > 0)
		{
			mix_offset += decoded_size;
			data_offset_ += decoded_size;
		}
		else
		{
			if (decoded_size == 0 && storage_type_ == OalLtSoundSysStreamingSourceStorageType::decoder)
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

			break;
		}
	}

	if (!is_spatial() && is_mono())
	{
		switch (get_bit_depth())
		{
		case 8:
			mix_mono_to_stereo<8>(mix_offset);
			break;

		case 16:
			mix_mono_to_stereo<16>(mix_offset);
			break;

		default:
			return 0;
		}
	}

	return mix_offset;
}


} // ltjs
