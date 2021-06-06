#ifndef LTJS_OAL_LT_SOUND_SYS_GENERIC_STREAM_INCLUDED
#define LTJS_OAL_LT_SOUND_SYS_GENERIC_STREAM_INCLUDED


#include <array>
#include <mutex>

#include "al.h"

#include "iltsound.h"


namespace ltjs
{


class OalLtSoundSysGenericStream final :
	public LtjsLtSoundSysGenericStream
{
public:
	using MtMutex = std::mutex;


	static constexpr auto channel_count = 2;
	static constexpr auto bit_depth = 16;
	static constexpr auto byte_depth = bit_depth / 8;
	static constexpr auto sample_size = channel_count * byte_depth;

	static constexpr auto queue_size = 3;


	OalLtSoundSysGenericStream(
		MtMutex& mutex);

	OalLtSoundSysGenericStream(
		const OalLtSoundSysGenericStream& that) = delete;

	OalLtSoundSysGenericStream& operator=(
		const OalLtSoundSysGenericStream& that) = delete;

	OalLtSoundSysGenericStream(
		OalLtSoundSysGenericStream&& that) noexcept;

	~OalLtSoundSysGenericStream();

	void close();

	bool open(
		const int sample_rate,
		const int buffer_size);


	// =========================================================================
	// LtjsLtSoundSysGenericStream

	int get_free_buffer_count() noexcept override;

	bool enqueue_buffer(
		const void* buffer) noexcept override;

	bool set_pause(
		bool is_pause) noexcept override;

	bool get_pause() noexcept override;

	bool set_volume(
		int level_mb) noexcept override;

	int get_volume() noexcept override;

	// LtjsLtSoundSysGenericStream
	// =========================================================================


private:
	static constexpr auto oal_buffer_format = ::ALenum{AL_FORMAT_STEREO16};


	using MtLockGuard = std::lock_guard<MtMutex>;
	using OalBuffers = std::array<::ALuint, queue_size>;


	MtMutex* mutex_;

	bool is_open_{};
	bool is_failed_{};
	bool is_playing_{};
	int queued_count_{};
	int buffer_size_{};
	int sample_rate_{};
	int level_mb_{};

	float oal_gain_{};

	bool oal_is_source_created_{};
	bool oal_are_buffers_created_{};

	::ALuint oal_source_{};
	OalBuffers oal_buffers_{};


	bool initialize_oal_objects();

	bool unqueue_processed_buffers();
}; // GenericStream


} // ltjs


#endif // !LTJS_OAL_LT_SOUND_SYS_GENERIC_STREAM_INCLUDED
