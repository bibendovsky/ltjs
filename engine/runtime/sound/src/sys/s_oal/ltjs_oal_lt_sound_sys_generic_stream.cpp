#include "ltjs_oal_lt_sound_sys_generic_stream.h"

#include <algorithm>

#include "ltjs_audio_utils.h"
#include "ltjs_oal_utils.h"


namespace ltjs
{


OalLtSoundSysGenericStream::OalLtSoundSysGenericStream()
	:
	is_open_{},
	is_failed_{},
	is_playing_{},
	queued_count_{},
	buffer_size_{},
	sample_rate_{},
	level_mb_{},
	oal_gain_{},
	oal_is_source_created_{},
	oal_are_buffers_created_{},
	oal_source_{},
	oal_buffers_{}
{
}

OalLtSoundSysGenericStream::OalLtSoundSysGenericStream(
	OalLtSoundSysGenericStream&& that)
	:
	is_open_{std::move(that.is_open_)},
	is_failed_{std::move(that.is_failed_)},
	is_playing_{std::move(that.is_playing_)},
	queued_count_{std::move(that.queued_count_)},
	buffer_size_{std::move(that.buffer_size_)},
	sample_rate_{std::move(that.sample_rate_)},
	level_mb_{std::move(that.level_mb_)},
	oal_gain_{std::move(that.oal_gain_)},
	oal_is_source_created_{std::move(that.oal_is_source_created_)},
	oal_are_buffers_created_{std::move(that.oal_are_buffers_created_)},
	oal_source_{std::move(that.oal_source_)},
	oal_buffers_{std::move(that.oal_buffers_)}
{
	that.is_open_ = false;
	that.oal_is_source_created_ = false;
	that.oal_are_buffers_created_ = false;
}

OalLtSoundSysGenericStream::~OalLtSoundSysGenericStream()
{
	close();
}

void OalLtSoundSysGenericStream::close()
{
	is_open_ = false;
	is_failed_ = false;
	is_playing_ = false;
	queued_count_ = 0;
	buffer_size_ = 0;
	sample_rate_ = 0;
	level_mb_ = 0;
	oal_gain_ = 0.0F;

	if (oal_is_source_created_)
	{
		oal_is_source_created_ = false;
		alDeleteSources(1, &oal_source_);
	}

	if (oal_are_buffers_created_)
	{
		oal_are_buffers_created_ = false;
		alDeleteBuffers(1, oal_buffers_.data());
	}
}

bool OalLtSoundSysGenericStream::open(
	const int sample_rate,
	const int buffer_size)
{
	close();

	if (sample_rate <= 0 ||
		buffer_size <= 0 ||
		(buffer_size % sample_size) != 0)
	{
		return false;
	}

	queued_count_ = 0;
	buffer_size_ = buffer_size;
	sample_rate_ = sample_rate;
	level_mb_ = AudioUtils::max_generic_stream_level_mb;
	oal_gain_ = AudioUtils::generic_stream_level_mb_to_gain(level_mb_);

	if (!initialize_oal_objects())
	{
		close();
		return false;
	}

	is_open_ = true;

	return true;
}

bool OalLtSoundSysGenericStream::set_pause(
	const bool is_pause)
{
	if (!is_open_ || is_failed_)
	{
		return false;
	}

	if (is_pause == !is_playing_)
	{
		return true;
	}

	is_playing_ = !is_pause;

	if (is_playing_)
	{
		auto oal_state = ALint{};

		alGetSourcei(oal_source_, AL_SOURCE_STATE, &oal_state);

		switch (oal_state)
		{
			case AL_INITIAL:
			case AL_PAUSED:
			case AL_STOPPED:
				LTJS_OAL_ENSURE_CALL_DEBUG(alSourcePlay(oal_source_));
				break;

			case AL_PLAYING:
				break;

			default:
				is_failed_ = true;
				return false;
		}

		return true;
	}
	else
	{
		LTJS_OAL_ENSURE_CALL_DEBUG(alSourcePause(oal_source_));

		return true;
	}
}

bool OalLtSoundSysGenericStream::get_pause() const
{
	return !is_playing_;
}

bool OalLtSoundSysGenericStream::set_volume(
	int level_mb)
{
	if (!is_open_ || is_failed_)
	{
		return false;
	}

	if (level_mb_ == level_mb)
	{
		return true;
	}

	level_mb_ = level_mb;

	oal_gain_ = AudioUtils::generic_stream_level_mb_to_gain(level_mb_);

	LTJS_OAL_ENSURE_CALL_DEBUG(alSourcef(oal_source_, AL_GAIN, oal_gain_));

	return true;
}

int OalLtSoundSysGenericStream::get_volume() const
{
	return level_mb_;
}

int OalLtSoundSysGenericStream::get_free_buffer_count()
{
	if (!is_open_ || is_failed_)
	{
		return false;
	}

	if (!unqueue_processed_buffers())
	{
		return false;
	}

	return queue_size - queued_count_;
}

bool OalLtSoundSysGenericStream::enqueue_data(
	const void* buffer)
{
	if (!is_open_ || is_failed_ || !buffer)
	{
		return false;
	}

	if (!unqueue_processed_buffers())
	{
		return false;
	}

	if (queued_count_ == queue_size)
	{
		return false;
	}

	const auto oal_free_buffer = oal_buffers_[queued_count_];

	LTJS_OAL_ENSURE_CALL_DEBUG(alBufferData(
		oal_free_buffer, oal_buffer_format, buffer, buffer_size_, sample_rate_));

	LTJS_OAL_ENSURE_CALL_DEBUG(alSourceQueueBuffers(oal_source_, 1, &oal_free_buffer));

	queued_count_ += 1;

	auto oal_state = ALint{};

	LTJS_OAL_ENSURE_CALL_DEBUG(alGetSourcei(oal_source_, AL_SOURCE_STATE, &oal_state));

	switch (oal_state)
	{
		case AL_INITIAL:
		case AL_PAUSED:
		case AL_STOPPED:
			if (is_playing_)
			{
				LTJS_OAL_ENSURE_CALL_DEBUG(alSourcePlay(oal_source_));
			}

			break;

		case AL_PLAYING:
		default:
			break;
	}

	if (!oal::is_succeed())
	{
		is_failed_ = true;
		return false;
	}

	return true;
}

bool OalLtSoundSysGenericStream::initialize_oal_objects()
{
	oal::clear_error();

	alGenSources(1, &oal_source_);

	if (oal::is_succeed())
	{
		oal_is_source_created_ = true;
	}
	else
	{
		return false;
	}

	alGenBuffers(queue_size, oal_buffers_.data());

	if (oal::is_succeed())
	{
		oal_are_buffers_created_ = true;
	}
	else
	{
		return false;
	}

	alSourcei(oal_source_, AL_SOURCE_RELATIVE, AL_TRUE);

	return oal::is_succeed();
}

bool OalLtSoundSysGenericStream::unqueue_processed_buffers()
{
	auto oal_processed_count = ALint{};

	alGetSourcei(oal_source_, AL_BUFFERS_PROCESSED, &oal_processed_count);

	if (!oal::is_succeed())
	{
		is_failed_ = true;
		return false;
	}

	if (oal_processed_count == 0)
	{
		return true;
	}

	std::rotate(oal_buffers_.begin(), oal_buffers_.begin() + oal_processed_count, oal_buffers_.end());

	queued_count_ -= oal_processed_count;

	alSourceUnqueueBuffers(oal_source_, oal_processed_count, &oal_buffers_[queue_size - oal_processed_count]);

	if (!oal::is_succeed())
	{
		is_failed_ = true;
		return false;
	}

	return true;
}


} // ltjs
