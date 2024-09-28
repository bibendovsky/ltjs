#ifndef LTJS_PCM_AUDIO_DECODER_INCLUDED
#define LTJS_PCM_AUDIO_DECODER_INCLUDED

#include <cstdint>

#include "bibendovsky_spul_stream.h"

#include "ltjs_basic_audio_decoder.h"

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

struct PcmAudioDecoderOpenParam
{
	int channel_count;
	int bit_depth;
	int sample_rate;
	ul::Stream* stream;
};

class PcmAudioDecoder final : public BasicAudioDecoder
{
public:
	PcmAudioDecoder() noexcept;
	explicit PcmAudioDecoder(const PcmAudioDecoderOpenParam& param) noexcept;
	PcmAudioDecoder(const PcmAudioDecoder&) = delete;
	PcmAudioDecoder& operator=(const PcmAudioDecoder&) = delete;
	~PcmAudioDecoder() override;

	bool open(const PcmAudioDecoderOpenParam& param) noexcept;
	void close() noexcept override;
	int decode(void* buffer, int buffer_size) noexcept override;
	bool set_clamped_position(int frame_offset) noexcept override;
	bool is_open() const noexcept override;
	bool is_failed() const noexcept override;
	int get_channel_count() const noexcept override;
	int get_bit_depth() const noexcept override;
	int get_sample_rate() const noexcept override;
	int get_frame_count() const noexcept override;
	int get_data_size() const noexcept override;

private:
	static constexpr auto cache_capacity = 1024;

private:
	using Cache = std::uint8_t[cache_capacity];

private:
	bool is_open_{};
	bool is_failed_{};
	int channel_count_{};
	int bit_depth_{};
	int sample_rate_{};
	int frame_count_{};
	int frame_size_{};
	int total_byte_count_{};
	int total_byte_offset_{};
	int cache_byte_count_{};
	int cache_byte_offset_{};
	ul::Stream* stream_{};
	Cache cache_{};

private:
	static bool validate(const PcmAudioDecoderOpenParam& param) noexcept;
	bool open_internal(const PcmAudioDecoderOpenParam& param) noexcept;
	bool fill_cache() noexcept;
};

} // namespace ltjs

#endif // LTJS_PCM_AUDIO_DECODER_INCLUDED
