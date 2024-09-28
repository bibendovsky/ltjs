/*
References:
  - WAV
    https://en.wikipedia.org/wiki/WAV
  - PCMWAVEFORMAT structure
    https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-pcmwaveformat
  - WAVEFORMATEX structure
    https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex
  - Registered FOURCC Codes and WAVE Formats
    https://learn.microsoft.com/en-us/previous-versions/aa904731(v=vs.80)
*/

#include "ltjs_audio_decoder.h"

#include <cassert>
#include <cstdint>

#include "bibendovsky_spul_substream.h"

#include "ltjs_audio_converter.h"
#include "ltjs_audio_limits.h"
#include "ltjs_basic_audio_decoder.h"
#include "ltjs_ms_ima_adpcm_audio_decoder.h"
#include "ltjs_mp3_audio_decoder.h"
#include "ltjs_pcm_audio_decoder.h"

// ==========================================================================

namespace ltjs {

class AudioDecoder::Impl
{
public:
	Impl() = default;
	explicit Impl(const OpenParam& param) noexcept;
	~Impl() = default;

	bool open(const OpenParam& param) noexcept;
	void close() noexcept;
	int decode(void* buffer, int buffer_size) noexcept;
	bool rewind() noexcept;
	bool set_position(int frame_offset) noexcept;

	bool is_open() const noexcept;
	bool is_failed() const noexcept;
	bool is_pcm() const noexcept;
	bool is_ima_adpcm() const noexcept;
	bool is_mp3() const noexcept;
	int get_src_channel_count() const noexcept;
	int get_src_sample_rate() const noexcept;
	int get_dst_channel_count() const noexcept;
	int get_dst_bit_depth() const noexcept;
	int get_dst_sample_rate() const noexcept;
	const ul::WaveFormatEx& get_wave_format_ex() const noexcept;
	int get_dst_sample_size() const noexcept;
	int get_sample_count() const noexcept;
	int get_data_size() const noexcept;
	int get_decoded_size() const noexcept;

private:
	static constexpr auto wave_format_pcm = 0x0001;
	static constexpr auto wave_format_ima_adpcm = 0x0011;
	static constexpr auto wave_format_mpeglayer3 = 0x0055;

	static constexpr auto pcmwaveformat_size = 16;
	static constexpr auto waveformatex_size = 18;

	static constexpr auto max_wav_chunk_buffer_size = 64;

	static_assert(max_wav_chunk_buffer_size >= waveformatex_size, "Chunk buffer too small.");

	static constexpr auto cache_capacity = 1024;

private:
	using RiffChunkBuffer = std::uint8_t[max_wav_chunk_buffer_size];
	using Cache = std::uint8_t[cache_capacity];
	using DecodeFunc = int (Impl::*)(void* buffer, int buffer_size);
	using SetPositionFunc = bool (Impl::*)(int frame_offset);

private:
	bool is_open_{};
	bool is_failed_{};
	bool is_pcm_{};
	bool is_ms_ima_adpcm_{};
	bool is_mp3_{};
	int wav_block_align_{};
	int wav_frame_count_{};
	int src_channel_count_{};
	int src_bit_depth_{};
	int src_sample_rate_{};
	int dst_channel_count_{};
	int dst_bit_depth_{};
	int dst_sample_rate_{};
	int dst_frame_size_{};
	int dst_frame_count_{};
	int dst_data_size_{};
	int dst_decoded_size_{};
	int wav_chunk_size_{};
	int cache_byte_count_{};
	RiffChunkBuffer wav_chunk_buffer_{};
	ul::WaveFormatEx dst_wave_format_ex_{};
	ul::Stream* stream_{};
	ul::Substream substream_{};
	PcmAudioDecoder pcm_audio_decoder_{};
	MsImaAdpcmAudioDecoder ms_ima_adpcm_audio_decoder_{};
	Mp3AudioDecoder mp3_audio_decoder_{};
	BasicAudioDecoder* audio_decoder_{};
	AudioConverter audio_converter_{};
	DecodeFunc decode_func_{};
	SetPositionFunc set_position_func_{};
	Cache cache_{};

private:
	static bool validate_open_param(const OpenParam& param) noexcept;
	static int align_chunk_size(int size) noexcept;
	static uint16_t get_word(const std::uint8_t* bytes) noexcept;
	static uint32_t get_dword(const std::uint8_t* bytes) noexcept;

	bool read_wav_chunk(int chunk_size) noexcept;
	bool parse_wav_fmt() noexcept;
	bool parse_wav_fact() noexcept;
	bool parse_wav() noexcept;
	bool open_decoder() noexcept;
	bool open_converter() noexcept;
	void set_funcs() noexcept;
	bool open_internal(const OpenParam& param) noexcept;
	int decode_without_conversion(void* buffer, int buffer_size) noexcept;
	int decode_with_conversion(void* buffer, int buffer_size) noexcept;
	bool set_position_without_conversion(int frame_offset) noexcept;
	bool set_position_with_conversion(int frame_offset) noexcept;
};

// --------------------------------------------------------------------------

AudioDecoder::Impl::Impl(const OpenParam& param) noexcept
{
	open(param);
}

bool AudioDecoder::Impl::open(const OpenParam& param) noexcept
{
	close();

	if (!validate_open_param(param))
	{
		return false;
	}

	if (!open_internal(param))
	{
		close();
		return false;
	}

	is_open_ = true;
	return is_open_;
}

void AudioDecoder::Impl::close() noexcept
{
	is_open_ = false;
	is_failed_ = false;
	is_pcm_ = false;
	is_ms_ima_adpcm_ = false;
	is_mp3_ = false;
	src_channel_count_ = 0;
	src_bit_depth_ = 0;
	src_sample_rate_ = 0;
	dst_channel_count_ = 0;
	dst_bit_depth_ = 0;
	dst_sample_rate_ = 0;
	dst_frame_size_ = 0;
	dst_frame_count_ = 0;
	dst_data_size_ = 0;
	dst_decoded_size_ = 0;
	wav_chunk_size_ = 0;
	cache_byte_count_ = 0;
	dst_wave_format_ex_ = ul::WaveFormatEx{};
	stream_ = nullptr;
	substream_.close();
	pcm_audio_decoder_.close();
	ms_ima_adpcm_audio_decoder_.close();
	mp3_audio_decoder_.close();
	audio_decoder_ = nullptr;
	audio_converter_.close();
	decode_func_ = nullptr;
	set_position_func_ = nullptr;
}

int AudioDecoder::Impl::decode(void* buffer, int buffer_size) noexcept
{
	if (!is_open())
	{
		assert(false && "Closed.");
		return -1;
	}

	if (is_failed())
	{
		assert(false && "Failed.");
		return -1;
	}

	if (buffer == nullptr)
	{
		assert(false && "Null buffer.");
		return -1;
	}

	if (buffer_size < 0)
	{
		assert(false && "Negative buffer size.");
		return -1;
	}

	assert(decode_func_ != nullptr);
	return (this->*decode_func_)(buffer, buffer_size);
}

bool AudioDecoder::Impl::rewind() noexcept
{
	return set_position(0);
}

bool AudioDecoder::Impl::set_position(int frame_offset) noexcept
{
	if (!is_open())
	{
		assert(false && "Closed.");
		return false;
	}

	if (is_failed())
	{
		assert(false && "Failed.");
		return false;
	}

	assert(set_position_func_ != nullptr);
	return (this->*set_position_func_)(frame_offset);
}

bool AudioDecoder::Impl::is_open() const noexcept
{
	return is_open_;
}

bool AudioDecoder::Impl::is_failed() const noexcept
{
	return is_failed_;
}

bool AudioDecoder::Impl::is_pcm() const noexcept
{
	assert(is_open());
	return is_pcm_;
}

bool AudioDecoder::Impl::is_ima_adpcm() const noexcept
{
	assert(is_open());
	return is_ms_ima_adpcm_;
}

bool AudioDecoder::Impl::is_mp3() const noexcept
{
	assert(is_open());
	return is_mp3_;
}

int AudioDecoder::Impl::get_src_channel_count() const noexcept
{
	assert(is_open());
	return 0;
}

int AudioDecoder::Impl::get_src_sample_rate() const noexcept
{
	assert(is_open());
	return 0;
}

int AudioDecoder::Impl::get_dst_channel_count() const noexcept
{
	assert(is_open());
	return dst_channel_count_;
}

int AudioDecoder::Impl::get_dst_bit_depth() const noexcept
{
	assert(is_open());
	return dst_bit_depth_;
}

int AudioDecoder::Impl::get_dst_sample_rate() const noexcept
{
	assert(is_open());
	return dst_sample_rate_;
}

const ul::WaveFormatEx& AudioDecoder::Impl::get_wave_format_ex() const noexcept
{
	assert(is_open());
	return dst_wave_format_ex_;
}

int AudioDecoder::Impl::get_dst_sample_size() const noexcept
{
	assert(is_open());
	return dst_frame_size_;
}

int AudioDecoder::Impl::get_sample_count() const noexcept
{
	assert(is_open());
	return dst_frame_count_;
}

int AudioDecoder::Impl::get_data_size() const noexcept
{
	assert(is_open());
	return dst_data_size_;
}

int AudioDecoder::Impl::get_decoded_size() const noexcept
{
	assert(is_open());
	return dst_decoded_size_;
}

bool AudioDecoder::Impl::validate_open_param(const OpenParam& param) noexcept
{
	if (param.dst_channel_count_ < 0)
	{
		assert(false && "Negative channel count.");
		return false;
	}

	if (param.dst_bit_depth_ < 0)
	{
		assert(false && "Negative bit depth.");
		return false;
	}

	if (param.dst_sample_rate_ < 0)
	{
		assert(false && "Negative sample rate.");
		return false;
	}

	if (param.stream_ptr_ == nullptr)
	{
		assert(false && "Null stream.");
		return false;
	}

	if (!param.stream_ptr_->is_readable())
	{
		assert(false && "Non-readable stream.");
		return false;
	}

	if (!param.stream_ptr_->is_seekable())
	{
		assert(false && "Non-seekable stream.");
		return false;
	}

	return true;
}

int AudioDecoder::Impl::align_chunk_size(int size) noexcept
{
	assert(size >= 0);
	return ((size + 1) / 2) * 2;
}

uint16_t AudioDecoder::Impl::get_word(const std::uint8_t* bytes) noexcept
{
	const auto byte0 = static_cast<std::uint32_t>(bytes[0]);
	const auto byte1 = static_cast<std::uint32_t>(bytes[1]);
	return static_cast<std::uint16_t>(byte0 | (byte1 << 8));
}

uint32_t AudioDecoder::Impl::get_dword(const std::uint8_t* bytes) noexcept
{
	const auto byte0 = static_cast<std::uint32_t>(bytes[0]);
	const auto byte1 = static_cast<std::uint32_t>(bytes[1]);
	const auto byte2 = static_cast<std::uint32_t>(bytes[2]);
	const auto byte3 = static_cast<std::uint32_t>(bytes[3]);
	return byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
}

bool AudioDecoder::Impl::read_wav_chunk(int chunk_size) noexcept
{
	if (chunk_size < 0 || chunk_size > max_wav_chunk_buffer_size)
	{
		assert(false && "Unsupported RIFF chunk size.");
		return false;
	}

	if (stream_->read(wav_chunk_buffer_, chunk_size) != chunk_size)
	{
		assert(false && "Failed to read stream.");
		return false;
	}

	wav_chunk_size_ = chunk_size;
	return true;
}

bool AudioDecoder::Impl::parse_wav_fmt() noexcept
{
	if (false) {}
	else if (wav_chunk_size_ == pcmwaveformat_size) {}
	else if (wav_chunk_size_ >= waveformatex_size) {}
	else
	{
		assert(false && "Unsupported format chunk size.");
		return false;
	}

	// Get data.
	//
	const auto w_format_tag = get_word(&wav_chunk_buffer_[0]);
	const auto n_channels = get_word(&wav_chunk_buffer_[2]);
	const auto n_samples_per_sec = get_dword(&wav_chunk_buffer_[4]);
	//const auto n_avg_bytes_per_sec = get_dword(&wav_chunk_buffer_[8]);
	const auto n_block_align = get_word(&wav_chunk_buffer_[12]);
	const auto w_bits_per_sample = get_word(&wav_chunk_buffer_[14]);
	//const auto cb_size = get_word(&wav_chunk_buffer_[16]);

	// Validate.
	//
	if (n_channels < AudioLimits::min_channels || n_channels > AudioLimits::max_channels)
	{
		assert(false && "Unsupported channel count.");
		return false;
	}

	if (n_samples_per_sec < AudioLimits::min_sample_rate || n_samples_per_sec > AudioLimits::max_sample_rate)
	{
		assert(false && "Unsupported sample rate.");
		return false;
	}

	switch (w_format_tag)
	{
		case wave_format_pcm:
			is_pcm_ = true;
			break;

		case wave_format_ima_adpcm:
			is_ms_ima_adpcm_ = true;
			wav_block_align_ = n_block_align;
			break;

		case wave_format_mpeglayer3:
			is_mp3_ = true;
			break;

		default:
			assert(false && "Unsupported wave format.");
			return false;
	}

	src_channel_count_ = n_channels;
	src_bit_depth_ = w_bits_per_sample;
	src_sample_rate_ = static_cast<int>(n_samples_per_sec);
	return true;
}

bool AudioDecoder::Impl::parse_wav_fact() noexcept
{
	if (is_pcm_)
	{
		assert(false && "PCM with \"fact\" chunk.");
		return false;
	}

	if (wav_chunk_size_ != 4)
	{
		assert(false && "Invalid \"fact\" size.");
		return false;
	}

	const auto wav_frame_count = get_dword(wav_chunk_buffer_);

	if (wav_frame_count > AudioLimits::max_frame_count)
	{
		assert(false && "Too many frames.");
		return false;
	}

	wav_frame_count_ = static_cast<int>(wav_frame_count);
	return true;
}

bool AudioDecoder::Impl::parse_wav() noexcept
{
	if (!stream_->set_position(0))
	{
		assert(false && "Failed to rewind a stream.");
		return false;
	}

	// Read RIFF/WAVE header.
	//
	constexpr auto riff_chunk_class_size = 8;
	constexpr auto riff_wave_class_size = 12;

	if (stream_->read(&wav_chunk_buffer_, riff_wave_class_size) != riff_wave_class_size)
	{
		assert(false && "Failed to read RIFF WAVE chunk.");
		return false;
	}

	const auto riff_chunk_tag = get_dword(&wav_chunk_buffer_[0]);
	const auto riff_chunk_size = get_dword(&wav_chunk_buffer_[4]);
	const auto riff_chunk_type = get_dword(&wav_chunk_buffer_[8]);

	// Validate RIFF tag.
	//
	constexpr auto min_riff_size = 4; // WAVE
	constexpr auto max_riff_size = ((INT32_MAX - riff_chunk_class_size) / 2) * 2;
	constexpr auto riff_tag_value = 0x46464952U; // 'R' 'I' 'F' 'F'

	if (riff_chunk_tag != riff_tag_value)
	{
		assert(false && "Missing RIFF tag.");
		return false;
	}

	// Validate RIFF size.
	//
	if (riff_chunk_size < min_riff_size || riff_chunk_size > max_riff_size)
	{
		assert(false && "Unsupported RIFF size.");
		return false;
	}

	// Validate WAVE tag.
	//
	constexpr auto wave_tag_value = 0x45564157U; // 'W' 'A' 'V' 'E'

	if (riff_chunk_type != wave_tag_value)
	{
		assert(false && "Missing WAVE tag.");
		return false;
	}

	// Iterate over the chunks to the "data" chunk.
	//
	constexpr auto fmt_tag_value = 0x20746D66U; // 'f' 'm' 't' ' '
	constexpr auto fact_tag_value = 0x74636166U; // 'f' 'a' 'c' 't'
	constexpr auto data_tag_value = 0x61746164U; // 'd' 'a' 't' 'a'
	const auto riff_size = static_cast<int>(riff_chunk_size) + riff_chunk_class_size;
	auto riff_offset = riff_wave_class_size; // RIFF size[4] WAVE
	auto wave_data_size = 0;
	auto wave_data_stream_position = 0;
	auto has_fmt0x20 = false;
	auto has_fact = false;

	while (true)
	{
		// Read header.
		//
		if (stream_->read(wav_chunk_buffer_, riff_chunk_class_size) != riff_chunk_class_size)
		{
			assert(false && "Failed to read a chunk header.");
			return false;
		}

		const auto chunk_tag = get_dword(&wav_chunk_buffer_[0]);
		const auto chunk_size = get_dword(&wav_chunk_buffer_[4]);

		// Validate header.
		//
		if (chunk_size > max_riff_size)
		{
			assert(false && "Unsupported chunk size.");
			return false;
		}

		riff_offset += riff_chunk_class_size;
		const auto aligned_chunk_size = align_chunk_size(static_cast<int>(chunk_size));

		if (aligned_chunk_size > riff_size - riff_offset)
		{
			assert(false && "Chunk's tail outside of RIFF.");
			return false;
		}

		// Parse.
		//
		if (false) {}
		else if (chunk_tag == fmt_tag_value)
		{
			// "fmt "
			//
			if (has_fmt0x20)
			{
				assert(false && "Multiple format chunks.");
				return false;
			}

			if (!read_wav_chunk(static_cast<int>(chunk_size)))
			{
				return false;
			}

			if (!parse_wav_fmt())
			{
				return false;
			}

			has_fmt0x20 = true;
		}
		else if (chunk_tag == fact_tag_value)
		{
			// "fact"
			//
			if (has_fact)
			{
				assert(false && "Multiple \"fact\" chunks.");
				return false;
			}

			if (!has_fmt0x20)
			{
				assert(false && "Missing format chunk before the \"fact\" one.");
				return false;
			}

			if (!read_wav_chunk(static_cast<int>(chunk_size)))
			{
				return false;
			}

			if (!parse_wav_fact())
			{
				return false;
			}

			has_fact = true;
		}
		else if (chunk_tag == data_tag_value)
		{
			// "data"
			//
			if (!has_fmt0x20)
			{
				assert(false && "Missing format chunk before the \"data\" one.");
				return false;
			}

			if (!has_fact && !is_pcm_)
			{
				assert(false && "Missing \"fact\" chunk before the \"data\" one.");
				return false;
			}

			wave_data_size = static_cast<int>(chunk_size);
			wave_data_stream_position = riff_offset;
			break;
		}
		else
		{
			// Other.
			//
			if (stream_->set_position(aligned_chunk_size, ul::Stream::Origin::current) < 0)
			{
				assert(false && "Failed to skip a chunk.");
				return false;
			}
		}

		riff_offset += aligned_chunk_size;
	}

	if (!substream_.open(stream_, wave_data_stream_position, wave_data_size, ul::Substream::SyncPositionOnRead::enable))
	{
		assert(false && "Failed to open a substream.");
		return false;
	}

	return true;
}

bool AudioDecoder::Impl::open_decoder() noexcept
{
	if (false) {}
	else if (is_pcm_)
	{
		audio_decoder_ = &pcm_audio_decoder_;

		auto param = PcmAudioDecoderOpenParam{};
		param.channel_count = src_channel_count_;
		param.bit_depth = src_bit_depth_;
		param.sample_rate = src_sample_rate_;
		param.stream = &substream_;

		return pcm_audio_decoder_.open(param);
	}
	else if (is_ms_ima_adpcm_)
	{
		audio_decoder_ = &ms_ima_adpcm_audio_decoder_;

		auto param = MsImaAdpcmAudioDecoderOpenParam{};
		param.channel_count = src_channel_count_;
		param.bit_depth = src_bit_depth_;
		param.sample_rate = src_sample_rate_;
		param.frame_count = wav_frame_count_;
		param.block_size = wav_block_align_;
		param.stream = &substream_;

		return ms_ima_adpcm_audio_decoder_.open(param);
	}
	else if (is_mp3_)
	{
		audio_decoder_ = &mp3_audio_decoder_;

		auto param = Mp3AudioDecoderOpenParam{};
		param.channel_count = src_channel_count_;
		param.sample_rate = src_sample_rate_;
		param.frame_count = wav_frame_count_;
		param.dst_bit_depth = dst_bit_depth_ != 0 ? dst_bit_depth_ : 16;
		param.stream = &substream_;

		return mp3_audio_decoder_.open(param);
	}
	else
	{
		assert(false && "Unsupported codec.");
		return false;
	}
}

bool AudioDecoder::Impl::open_converter() noexcept
{
	auto param = AudioConverterOpenParam{};
	param.src_channel_count = src_channel_count_;
	param.src_bit_depth = src_bit_depth_;
	param.src_sample_rate = src_sample_rate_;
	param.dst_channel_count = dst_channel_count_;
	param.dst_bit_depth = dst_bit_depth_;
	param.dst_sample_rate = dst_sample_rate_;

	return audio_converter_.open(param);
}

void AudioDecoder::Impl::set_funcs() noexcept
{
	const auto is_conversion_required =
		src_channel_count_ != dst_channel_count_ ||
		src_bit_depth_ != dst_bit_depth_ ||
		src_sample_rate_ != dst_sample_rate_;

	if (is_conversion_required)
	{
		decode_func_ = &Impl::decode_with_conversion;
		set_position_func_ = &Impl::set_position_with_conversion;
	}
	else
	{
		decode_func_ = &Impl::decode_without_conversion;
		set_position_func_ = &Impl::set_position_without_conversion;
	}
}

bool AudioDecoder::Impl::open_internal(const OpenParam& param) noexcept
{
	stream_ = param.stream_ptr_;

	if (!parse_wav())
	{
		return false;
	}

	if (!open_decoder())
	{
		return false;
	}

	src_channel_count_ = audio_decoder_->get_channel_count();
	src_bit_depth_ = audio_decoder_->get_bit_depth();
	src_sample_rate_ = audio_decoder_->get_sample_rate();

	dst_channel_count_ = param.dst_channel_count_ != 0 ? param.dst_channel_count_ : src_channel_count_;
	dst_bit_depth_ = param.dst_bit_depth_ != 0 ? param.dst_bit_depth_ : src_bit_depth_;
	dst_sample_rate_ = param.dst_sample_rate_ != 0 ? param.dst_sample_rate_ : src_sample_rate_;

	if (!open_converter())
	{
		return false;
	}

	dst_frame_size_ = dst_channel_count_ * (dst_bit_depth_ / 8);

	auto dst_frame_count = static_cast<std::int_least64_t>(audio_decoder_->get_frame_count());

	if (src_sample_rate_ != dst_sample_rate_)
	{
		dst_frame_count *= dst_sample_rate_;
		//dst_frame_count += src_sample_rate_;
		//dst_frame_count -= 1;
		dst_frame_count /= src_sample_rate_;
	}

	dst_frame_count_ = static_cast<int>(dst_frame_count);
	dst_data_size_ = dst_frame_count_ * dst_frame_size_;

	const auto block_align = dst_channel_count_ * (dst_bit_depth_ / 8);
	dst_wave_format_ex_.tag_ = ul::WaveFormatTag::pcm;
	dst_wave_format_ex_.channel_count_ = static_cast<std::uint16_t>(dst_channel_count_);
	dst_wave_format_ex_.bit_depth_ = static_cast<std::uint16_t>(dst_bit_depth_);
	dst_wave_format_ex_.sample_rate_ = static_cast<std::uint32_t>(dst_sample_rate_);
	dst_wave_format_ex_.block_align_ = static_cast<std::uint16_t>(block_align);
	dst_wave_format_ex_.avg_bytes_per_sec_ = static_cast<std::uint32_t>(dst_sample_rate_ * block_align);
	dst_wave_format_ex_.extra_size_ = 0;

	set_funcs();
	return true;
}

int AudioDecoder::Impl::decode_without_conversion(void* buffer, int buffer_size) noexcept
{
	const auto dst_bytes = static_cast<std::uint8_t*>(buffer);
	auto dst_offset = 0;

	while (dst_offset < buffer_size)
	{
		const auto decoded_byte_count = audio_decoder_->decode(&dst_bytes[dst_offset], buffer_size);

		if (decoded_byte_count < 0)
		{
			is_failed_ = true;
			return -1;
		}

		if (decoded_byte_count == 0)
		{
			break;
		}

		dst_offset += decoded_byte_count;
	}

	return dst_offset;
}

int AudioDecoder::Impl::decode_with_conversion(void* buffer, int buffer_size) noexcept
{
	buffer_size = (buffer_size / dst_frame_size_) * dst_frame_size_;
	const auto dst_bytes = static_cast<std::uint8_t*>(buffer);
	auto dst_offset = 0;

	while (dst_offset < buffer_size)
	{
		const auto converted_byte_count = audio_converter_.convert(&dst_bytes[dst_offset], buffer_size - dst_offset);

		if (converted_byte_count < 0)
		{
			is_failed_ = true;
			return -1;
		}

		if (converted_byte_count > 0)
		{
			dst_offset += converted_byte_count;
			dst_decoded_size_ += converted_byte_count;
		}
		else
		{
			if (audio_converter_.is_filled())
			{
				break;
			}

			const auto decoded_byte_count = audio_decoder_->decode(cache_, cache_capacity);

			if (decoded_byte_count < 0)
			{
				is_failed_ = true;
				return -1;
			}

			cache_byte_count_ = decoded_byte_count;

			if (cache_byte_count_ == 0)
			{
				break;
			}

			if (audio_converter_.fill(cache_, cache_byte_count_) != cache_byte_count_)
			{
				is_failed_ = true;
				return -1;
			}
		}
	}

	return dst_offset;
}

bool AudioDecoder::Impl::set_position_without_conversion(int frame_offset) noexcept
{
	if (!audio_decoder_->set_clamped_position(static_cast<int>(frame_offset)))
	{
		is_failed_ = true;
		return false;
	}

	dst_decoded_size_ = frame_offset * dst_frame_size_;
	return true;
}

bool AudioDecoder::Impl::set_position_with_conversion(int frame_offset) noexcept
{
	auto dst_frame_offset = static_cast<std::int_least64_t>(frame_offset);
	dst_frame_offset *= dst_sample_rate_;
	dst_frame_offset /= src_sample_rate_;

	if (!audio_decoder_->set_clamped_position(static_cast<int>(dst_frame_offset)))
	{
		is_failed_ = true;
		return false;
	}

	dst_decoded_size_ = frame_offset * dst_frame_size_;
	audio_converter_.reset();
	return true;
}

// ==========================================================================

void AudioDecoder::ImplDeleter::operator()(Impl* impl) const noexcept
{
	delete impl;
}

// ==========================================================================

bool AudioDecoder::OpenParam::validate() const noexcept
{
	return
		(dst_channel_count_ == 0 || dst_channel_count_ == 1 || dst_channel_count_ == 2) &&
		(dst_bit_depth_ == 0 || dst_bit_depth_ == 8 || dst_bit_depth_ == 16) &&
		dst_sample_rate_ >= 0 &&
		stream_ptr_ != nullptr &&
		stream_ptr_->is_open() &&
		stream_ptr_->is_readable() &&
		stream_ptr_->is_seekable();
}

// --------------------------------------------------------------------------

AudioDecoder::AudioDecoder()
	:
	impl_{new Impl{}}
{}

AudioDecoder::AudioDecoder(const OpenParam& param)
	:
	impl_{new Impl{param}}
{}

AudioDecoder::AudioDecoder(AudioDecoder&&) noexcept = default;

AudioDecoder& AudioDecoder::operator=(AudioDecoder&&) noexcept = default;

AudioDecoder::~AudioDecoder() = default;

bool AudioDecoder::open(const OpenParam& param) noexcept
{
	return impl_->open(param);
}

void AudioDecoder::close() noexcept
{
	impl_->close();
}

int AudioDecoder::decode(void* buffer, int buffer_size) noexcept
{
	return impl_->decode(buffer, buffer_size);
}

bool AudioDecoder::rewind() noexcept
{
	return impl_->rewind();
}

bool AudioDecoder::set_position(int frame_offset) noexcept
{
	return impl_->set_position(frame_offset);
}

bool AudioDecoder::is_open() const noexcept
{
	return impl_->is_open();
}

bool AudioDecoder::is_failed() const noexcept
{
	return impl_->is_failed();
}

bool AudioDecoder::is_pcm() const noexcept
{
	return impl_->is_pcm();
}

bool AudioDecoder::is_ima_adpcm() const noexcept
{
	return impl_->is_ima_adpcm();
}

bool AudioDecoder::is_mp3() const noexcept
{
	return impl_->is_mp3();
}

int AudioDecoder::get_src_channel_count() const noexcept
{
	return impl_->get_src_channel_count();
}

int AudioDecoder::get_src_sample_rate() const noexcept
{
	return impl_->get_src_sample_rate();
}

int AudioDecoder::get_dst_channel_count() const noexcept
{
	return impl_->get_dst_channel_count();
}

int AudioDecoder::get_dst_bit_depth() const noexcept
{
	return impl_->get_dst_bit_depth();
}

int AudioDecoder::get_dst_sample_rate() const noexcept
{
	return impl_->get_dst_sample_rate();
}

int AudioDecoder::get_dst_sample_size() const noexcept
{
	return impl_->get_dst_sample_size();
}

const ul::WaveFormatEx& AudioDecoder::get_wave_format_ex() const noexcept
{
	return impl_->get_wave_format_ex();
}

int AudioDecoder::get_sample_count() const noexcept
{
	return impl_->get_sample_count();
}

int AudioDecoder::get_data_size() const noexcept
{
	return impl_->get_data_size();
}

int AudioDecoder::get_decoded_size() const noexcept
{
	return impl_->get_decoded_size();
}

void AudioDecoder::initialize_current_thread() noexcept {}

} // namespace ltjs
