#ifndef LTJS_BASIC_AUDIO_DECODER_INCLUDED
#define LTJS_BASIC_AUDIO_DECODER_INCLUDED

namespace ltjs {

class BasicAudioDecoder
{
public:
	BasicAudioDecoder() noexcept;
	virtual ~BasicAudioDecoder();

	virtual void close() noexcept = 0;
	virtual int decode(void* buffer, int buffer_size) noexcept = 0;
	virtual bool set_clamped_position(int frame_offset) noexcept = 0;
	virtual bool is_open() const noexcept = 0;
	virtual bool is_failed() const noexcept = 0;
	virtual int get_channel_count() const noexcept = 0;
	virtual int get_bit_depth() const noexcept = 0;
	virtual int get_sample_rate() const noexcept = 0;
	virtual int get_frame_count() const noexcept = 0;
	virtual int get_data_size() const noexcept = 0;
};

} // namespace ltjs

#endif // LTJS_BASIC_AUDIO_DECODER_INCLUDED
