#ifndef LTJS_AUDIO_DECODER_INCLUDED
#define LTJS_AUDIO_DECODER_INCLUDED


#include <cstdint>
#include <memory>
#include "bibendovsky_spul_stream.h"
#include "bibendovsky_spul_wave_format.h"


namespace ltjs
{


namespace ul = bibendovsky::spul;


class AudioDecoder final
{
public:
	//
	// Creates an uninitialized instance.
	//
	AudioDecoder();

	//
	// Creates an initialized instance.
	//
	// Parameters:
	//    - stream_ptr - an input data stream.
	//
	explicit AudioDecoder(
		ul::Stream* stream_ptr);

	AudioDecoder(
		const AudioDecoder& that) = delete;

	AudioDecoder& operator=(
		const AudioDecoder& that) = delete;

	AudioDecoder(
		AudioDecoder&& that);

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
	bool open(
		ul::Stream* stream_ptr);

	//
	// Uninitializes the instance.
	//
	void close();

	//
	// Decodes the samples.
	//
	// Parameters:
	//    - buffer - a buffer to decode samples to.
	//    - buffer_size - a maximum buffer size in bytes.
	//
	// Returns:
	//    - A decoded size of samples in bytes.
	//    - A negative value on error.
	//
	int decode(
		void* buffer,
		const int buffer_size);

	//
	// Sets an internal position to the first sample.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" on error.
	//
	bool rewind();

	//
	// Sets an internal position to the specified sample.
	//
	// Parameters:
	//    - sample_offset - a sample index to set position to.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" on error.
	//
	bool set_position(
		const int sample_offset);

	//
	// Gets an initialization status of the decoder.
	//
	// Returns:
	//    - "true" - if the decode is initialized.
	//    - "false" - if the decode is not initialized.
	//
	bool is_open() const;

	//
	// Gets a fatal error status of the decoder.
	//
	// Returns:
	//    - "true" - if the decoder encountered a fatal error.
	//    - "true" - if the decoder not encountered a fatal error.
	//
	bool is_failed() const;

	//
	// Gets PCM encoded status of the stream.
	//
	// Returns:
	//    - "true" - if the stream is PCM encoded.
	//    - "false" - if the stream is not PCM encoded or on error.
	//
	bool is_pcm() const;

	//
	// Gets IMA ADPCM encoded status of the stream.
	//
	// Returns:
	//    - "true" - if the stream is IMA ADPCM encoded.
	//    - "false" - if the stream is not IMA ADPCM encoded or on error.
	//
	bool is_ima_adpcm() const;

	//
	// Gets MP3 encoded status of the stream.
	//
	// Returns:
	//    - "true" - if the stream is MP3 encoded.
	//    - "false" - if the stream is not MP3 encoded or on error.
	//
	bool is_mp3() const;

	//
	// Gets a channel count.
	//
	// Returns:
	//    - Channel count.
	//    - Zero on error.
	//
	int get_channel_count() const;

	//
	// Gets a bit depth.
	//
	// Returns:
	//    - A bit depth.
	//    - Zero on error.
	//
	int get_bit_depth() const;

	//
	// Gets a sample rate.
	//
	// Returns:
	//    - Sample rate.
	//    - Zero on error.
	//
	int get_sample_rate() const;

	//
	// Gets a sample size.
	//
	// Returns:
	//    - Sample size.
	//    - Zero on error.
	//
	int get_sample_size() const;

	//
	// Gets audio parameters in WAVEFORMATEX-compatible structure.
	//
	// Returns:
	//    - A structure with parameters.
	//    - An empty structure on error.
	//
	ul::WaveFormatEx get_wave_format_ex() const;

	//
	// Gets a maximum sample count of the decoded data.
	//
	// Returns:
	//    - A maximum sample count.
	//
	int get_sample_count() const;

	//
	// Gets a maximum data size of the decoded data.
	//
	// Returns:
	//    - A maximum data size.
	//
	int get_data_size() const;

	//
	// Gets a decoded data size so far.
	//
	// Returns:
	//    - A decoded size.
	//
	int get_decoded_size() const;

	//
	// Initializes a current thread.
	//
	static void initialize_current_thread();


private:
	struct Impl;

	using ImplUPtr = std::unique_ptr<Impl>;

	ImplUPtr pimpl_;
}; // AudioDecoder


} // ltjs


#endif // LTJS_AUDIO_DECODER_INCLUDED
