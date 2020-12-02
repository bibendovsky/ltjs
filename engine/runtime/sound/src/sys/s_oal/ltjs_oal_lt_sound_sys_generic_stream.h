#ifndef LTJS_OAL_LT_SOUND_SYS_GENERIC_STREAM_INCLUDED
#define LTJS_OAL_LT_SOUND_SYS_GENERIC_STREAM_INCLUDED


#include <array>

#include "al.h"


namespace ltjs
{


class OalLtSoundSysGenericStream
{
public:
	static constexpr auto channel_count = 2;
	static constexpr auto bit_depth = 16;
	static constexpr auto byte_depth = bit_depth / 8;
	static constexpr auto sample_size = channel_count * byte_depth;

	static constexpr auto queue_size = 3;


	OalLtSoundSysGenericStream();

	OalLtSoundSysGenericStream(
		const OalLtSoundSysGenericStream& that) = delete;

	OalLtSoundSysGenericStream& operator=(
		const OalLtSoundSysGenericStream& that) = delete;

	OalLtSoundSysGenericStream(
		OalLtSoundSysGenericStream&& that);

	~OalLtSoundSysGenericStream();

	void close();

	bool open(
		const int sample_rate,
		const int buffer_size);

	bool set_pause(
		const bool is_pause);

	bool get_pause() const;

	bool set_volume(
		int level_mb);

	int get_volume() const;

	int get_free_buffer_count();

	bool enqueue_data(
		const void* buffer);


private:
	static constexpr auto oal_buffer_format = ALenum{AL_FORMAT_STEREO16};


	using OalBuffers = std::array<ALuint, queue_size>;


	bool is_open_;
	bool is_failed_;
	bool is_playing_;
	int queued_count_;
	int buffer_size_;
	int sample_rate_;
	int level_mb_;

	float oal_gain_;

	bool oal_is_source_created_;
	bool oal_are_buffers_created_;

	ALuint oal_source_;
	OalBuffers oal_buffers_;


	bool initialize_oal_objects();

	bool unqueue_processed_buffers();
}; // GenericStream


} // ltjs


#endif // !LTJS_OAL_LT_SOUND_SYS_GENERIC_STREAM_INCLUDED
