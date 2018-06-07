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
	// Open object parameter.
	//
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
		bool validate() const;
	}; // OpenParam


	//
	// Creates an uninitialized instance.
	//
	AudioDecoder();

	//
	// Creates an initialized instance.
	//
	// Parameters:
	//    - parameters - a set of parameters.
	//
	explicit AudioDecoder(
		const OpenParam& param);

	AudioDecoder(
		const AudioDecoder& that) = delete;

	AudioDecoder& operator=(
		const AudioDecoder& that) = delete;

	AudioDecoder(
		AudioDecoder&& that) noexcept;

	AudioDecoder& operator=(
		AudioDecoder&& that) noexcept;

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
		const OpenParam& param);

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
	// Gets an input channel count.
	//
	// Returns:
	//    - Channel count.
	//    - Zero on error.
	//
	int get_src_channel_count() const;

	//
	// Gets an input sample rate.
	//
	// Returns:
	//    - Sample rate.
	//    - Zero on error.
	//
	int get_src_sample_rate() const;

	//
	// Gets an output channel count.
	//
	// Returns:
	//    - Channel count.
	//    - Zero on error.
	//
	int get_dst_channel_count() const;

	//
	// Gets an output bit depth.
	//
	// Returns:
	//    - A bit depth.
	//    - Zero on error.
	//
	int get_dst_bit_depth() const;

	//
	// Gets an output sample rate.
	//
	// Returns:
	//    - Sample rate.
	//    - Zero on error.
	//
	int get_dst_sample_rate() const;

	//
	// Gets an output sample size.
	//
	// Returns:
	//    - Sample size.
	//    - Zero on error.
	//
	int get_dst_sample_size() const;

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

	ImplUPtr impl_;
}; // AudioDecoder


} // ltjs


#endif // LTJS_AUDIO_DECODER_INCLUDED
