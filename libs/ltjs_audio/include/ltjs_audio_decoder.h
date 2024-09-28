#ifndef LTJS_AUDIO_DECODER_INCLUDED
#define LTJS_AUDIO_DECODER_INCLUDED

#include <memory>

#include "bibendovsky_spul_stream.h"
#include "bibendovsky_spul_wave_format.h"

// ==========================================================================

namespace ltjs {

namespace ul = bibendovsky::spul;

class AudioDecoder
{
public:
	// Open object parameter.
	struct OpenParam
	{
		// An output channel count.
		// Set to zero to skip convertion.
		int dst_channel_count_;

		// An output bit depth.
		// Set to zero to skip convertion.
		int dst_bit_depth_;

		// An output sample rate.
		// Set to zero to skip convertion.
		int dst_sample_rate_;

		// An input data stream.
		ul::Stream* stream_ptr_;

		//
		// Validates all parameters.
		//
		// Returns:
		//    - "true" if all parameters are valid.
		//    - "false" otherwise.
		//
		bool validate() const noexcept;
	};


	//
	// Creates an empty instance.
	//
	AudioDecoder();

	//
	// Creates an initialized instance.
	//
	// Parameters:
	//    - parameters - a set of parameters.
	//
	explicit AudioDecoder(const OpenParam& param);

	AudioDecoder(const AudioDecoder&) = delete;
	AudioDecoder& operator=(const AudioDecoder&) = delete;
	AudioDecoder(AudioDecoder&& rhs) noexcept;
	AudioDecoder& operator=(AudioDecoder&& rhs) noexcept;
	~AudioDecoder();

	//
	// Initializes the instance.
	//
	// Parameters:
	//    - stream_ptr - an input data stream.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" on error.
	//
	bool open(const OpenParam& param) noexcept;

	//
	// Uninitializes the instance.
	//
	void close() noexcept;

	//
	// Decodes the samples.
	//
	// Parameters:
	//    - buffer - a buffer to decode samples to.
	//    - buffer_size - maximum buffer size in bytes.
	//
	// Returns:
	//    - A decoded size of data in bytes.
	//    - A negative value on error.
	//
	int decode(void* buffer, int buffer_size) noexcept;

	//
	// Sets an internal position to the first sample.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" on error.
	//
	bool rewind() noexcept;

	//
	// Sets an internal position to the specified sample.
	//
	// Parameters:
	//    - frame_offset - a frame index to set position to.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" on error.
	//
	bool set_position(int frame_offset) noexcept;

	//
	// Gets an initialization status of the decoder.
	//
	// Returns:
	//    - "true" - if the decode is initialized.
	//    - "false" - if the decode is not initialized.
	//
	bool is_open() const noexcept;

	//
	// Gets a fatal error status of the decoder.
	//
	// Returns:
	//    - "true" - if the decoder encountered a fatal error.
	//    - "false" - if the decoder not encountered a fatal error.
	//
	bool is_failed() const noexcept;

	//
	// Gets PCM encoded status of the stream.
	//
	// Returns:
	//    - "true" - if the stream is PCM encoded.
	//    - "false" - if the stream is not PCM encoded.
	//
	bool is_pcm() const noexcept;

	//
	// Gets Microsoft IMA ADPCM encoded status of the stream.
	//
	// Returns:
	//    - "true" - if the stream is Microsoft IMA ADPCM encoded.
	//    - "false" - if the stream is not Microsoft IMA ADPCM encoded.
	//
	bool is_ima_adpcm() const noexcept;

	//
	// Gets MP3 encoded status of the stream.
	//
	// Returns:
	//    - "true" - if the stream is MP3 encoded.
	//    - "false" - if the stream is not MP3 encoded.
	//
	bool is_mp3() const noexcept;

	//
	// Gets an input channel count.
	//
	// Returns:
	//    - Channel count.
	//
	int get_src_channel_count() const noexcept;

	//
	// Gets an input sample rate.
	//
	// Returns:
	//    - Sample rate.
	//
	int get_src_sample_rate() const noexcept;

	//
	// Gets an output channel count.
	//
	// Returns:
	//    - Channel count.
	//
	int get_dst_channel_count() const noexcept;

	//
	// Gets an output bit depth.
	//
	// Returns:
	//    - A bit depth.
	//
	int get_dst_bit_depth() const noexcept;

	//
	// Gets an output sample rate.
	//
	// Returns:
	//    - Sample rate.
	//
	int get_dst_sample_rate() const noexcept;

	//
	// Gets an output sample size.
	//
	// Returns:
	//    - Sample size.
	//
	int get_dst_sample_size() const noexcept;

	//
	// Gets audio parameters in WAVEFORMATEX-compatible structure.
	//
	// Returns:
	//    - A structure with parameters.
	//
	const ul::WaveFormatEx& get_wave_format_ex() const noexcept;

	//
	// Gets a maximum sample count of the decoded data.
	//
	// Returns:
	//    - A maximum sample count.
	//
	int get_sample_count() const noexcept;

	//
	// Gets a maximum data size of the decoded data.
	//
	// Returns:
	//    - A maximum data size.
	//
	int get_data_size() const noexcept;

	//
	// Gets a decoded data size so far.
	//
	// Returns:
	//    - A decoded size.
	//
	int get_decoded_size() const noexcept;

	//
	// Initializes a current thread.
	//
	static void initialize_current_thread() noexcept;


private:
	class Impl;

	struct ImplDeleter
	{
		void operator()(Impl* impl) const noexcept;
	};

	using ImplUPtr = std::unique_ptr<Impl, ImplDeleter>;

private:
	ImplUPtr impl_{};
};

} // namespace ltjs

#endif // LTJS_AUDIO_DECODER_INCLUDED
