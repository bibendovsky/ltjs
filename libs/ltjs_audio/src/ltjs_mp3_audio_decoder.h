#ifndef LTJS_MP3_AUDIO_DECODER_INCLUDED
#define LTJS_MP3_AUDIO_DECODER_INCLUDED

#define MINIMP3_FLOAT_OUTPUT
#define MINIMP3_NO_STDIO
#define MINIMP3_ONLY_MP3

#include <cstddef>
#include <cstdint>

#include "bibendovsky_spul_stream.h"

#include "minimp3_ex.h"

#include "ltjs_basic_audio_decoder.h"

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

struct Mp3AudioDecoderOpenParam
{
	int channel_count;
	int sample_rate;
	int frame_count;
	int dst_bit_depth;
	ul::Stream* stream;
};

class Mp3AudioDecoder final : public BasicAudioDecoder
{
public:
	Mp3AudioDecoder() noexcept;
	explicit Mp3AudioDecoder(const Mp3AudioDecoderOpenParam& param) noexcept;
	Mp3AudioDecoder(const Mp3AudioDecoder&) = delete;
	Mp3AudioDecoder& operator=(const Mp3AudioDecoder&) = delete;
	~Mp3AudioDecoder() override;

	bool open(const Mp3AudioDecoderOpenParam& param) noexcept;
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
	static constexpr auto max_channels = 2;

	static constexpr auto max_dst_bit_depth = 16;
	static constexpr auto max_dst_byte_depth = max_dst_bit_depth / 8;

	static constexpr auto sample_cache_capacity = 256;

	static constexpr auto byte_cache_capacity = sample_cache_capacity * max_channels * max_dst_byte_depth;

private:
	using Mp3SampleCache = float[sample_cache_capacity];
	using ConvertedSampleCache = std::uint8_t[byte_cache_capacity];
	using ConvertFunc = void (Mp3AudioDecoder::*)(int sample_count);

private:
	bool is_open_{};
	bool is_failed_{};
	bool is_mp3_context_dirty_{};
	int src_channel_count_{};
	int src_sample_rate_{};
	int dst_bit_depth_{};
	int dst_frame_size_{};
	int frame_count_{};
	int dst_total_byte_count_{};
	int dst_total_byte_offset_{};
	int cache_byte_count_{};
	int cache_byte_offset_{};
	mp3dec_io_t mp3_io_{};
	mp3dec_ex_t mp3_context_{};
	Mp3SampleCache mp3_sample_cache_{};
	ConvertedSampleCache converted_sample_cache_{};
	ConvertFunc convert_func_{};

private:
	static std::size_t read_callback_minimp3(void* buf, std::size_t size, void* user_data) noexcept;
	static int seek_callback_minimp3(std::uint64_t position, void* user_data) noexcept;
	
	void convert_f32_to_8_bit(int sample_count) noexcept;
	void convert_f32_to_16_bit(int sample_count) noexcept;

	static bool validate(const Mp3AudioDecoderOpenParam& param) noexcept;
	void close_mp3_context() noexcept;
	bool open_internal(const Mp3AudioDecoderOpenParam& param) noexcept;
	void fill_cache() noexcept;
};

} // namespace ltjs

#endif // LTJS_MP3_AUDIO_DECODER_INCLUDED
