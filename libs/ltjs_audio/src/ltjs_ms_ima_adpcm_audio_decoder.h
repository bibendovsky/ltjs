#ifndef LTJS_MS_IMA_ADPCM_AUDIO_DECODER_INCLUDED
#define LTJS_MS_IMA_ADPCM_AUDIO_DECODER_INCLUDED

#include <cstdint>

#include "bibendovsky_spul_stream.h"

#include "ltjs_basic_audio_decoder.h"

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

struct MsImaAdpcmAudioDecoderOpenParam
{
	int channel_count;
	int bit_depth;
	int sample_rate;
	int frame_count;
	int block_size;
	ul::Stream* stream;
};

class MsImaAdpcmAudioDecoder final : public BasicAudioDecoder
{
public:
	MsImaAdpcmAudioDecoder() noexcept;
	explicit MsImaAdpcmAudioDecoder(const MsImaAdpcmAudioDecoderOpenParam& param) noexcept;
	MsImaAdpcmAudioDecoder(const MsImaAdpcmAudioDecoder&) = delete;
	MsImaAdpcmAudioDecoder& operator=(const MsImaAdpcmAudioDecoder&) = delete;
	~MsImaAdpcmAudioDecoder() override;

	bool open(const MsImaAdpcmAudioDecoderOpenParam& param) noexcept;
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

	static constexpr auto dst_bit_depth = 16;
	static constexpr auto dst_byte_depth = dst_bit_depth / 8;

	static constexpr auto dst_sample_size = dst_byte_depth;

	static constexpr auto max_dst_frame_size = max_channels * dst_sample_size;

	static constexpr auto index_table_size = 16;
	static constexpr auto stepsize_table_size = 89;

	static constexpr auto max_block_size = 2048;
	static constexpr auto nibbles_per_byte = 2;

private:
	using IndexTable = std::int8_t[index_table_size];
	using StepsizeTable = std::int16_t[stepsize_table_size];

	using PredictedSamples = int[max_channels];
	using StepsizeIndices = int[max_channels];
	using BlockCache = std::uint8_t[max_block_size];
	using SampleCache = std::uint8_t[2 * 2 * max_block_size];

private:
	static const IndexTable index_table;
	static const StepsizeTable stepsize_table;

	bool is_open_{};
	bool is_failed_{};
	int dst_channel_count_{};
	int dst_bit_depth_{};
	int dst_sample_rate_{};
	int total_frame_count_{};
	int total_byte_count_{};
	int total_byte_offset_{};
	int block_size_{};
	int block_header_size_{};
	int cache_byte_count_{};
	int cache_byte_offset_{};
	ul::Stream* stream_{};
	BlockCache block_cache_{};
	SampleCache sample_cache_{};
	PredictedSamples predicted_samples_{};
	StepsizeIndices stepsize_indices_{};

private:
	template<typename T>
	static constexpr T clamp(T x, T x_min, T x_max) noexcept
	{
		return x < x_min ? x_min : (x > x_max ? x_max : x);
	}

	static bool validate(const MsImaAdpcmAudioDecoderOpenParam& param);
	static int clamp_index(int index);
	static int clamp_sample(int sample);
	void decode_block_header();
	static int get_lo_nibble(int byte) noexcept;
	static int get_hi_nibble(int byte) noexcept;
	static std::int16_t decode_nibble(int nibble, int& predicted_sample, int& step_index);
	bool fill_cache();
	void decode_mono(int block_size);
	void decode_stereo(int block_size);
	bool open_internal(const MsImaAdpcmAudioDecoderOpenParam& param) noexcept;
};

} // namespace ltjs

#endif // LTJS_MS_IMA_ADPCM_AUDIO_DECODER_INCLUDED
