#ifndef LTJS_FMV_PLAYER_INCLUDED
#define LTJS_FMV_PLAYER_INCLUDED

#include <cstdint>
#include <memory>

namespace ltjs {

class FmvPlayer final
{
public:
	//
	// Callback to cancel presentation of all frames.
	//
	// Parameters:
	//    - user_data - user data from initialization parameter.
	//
	// Returns:
	//    - "true" if presentation shoul be cancelled.
	//    - "false" if presentation should be continued.
	//
	using IsCancelledFunc = bool (*)(void* user_data);

	//
	// Callback to present audio data.
	//
	// Parameters:
	//    - user_data - user data from initialization parameter.
	//    - data - audio data.
	//    - data_size - audio data size in bytes.
	//
	using AudioPresentFunc = void (*)(void* user_data, const void* data, int data_size);

	//
	// Callback to get free audio buffer count.
	//
	// Parameters:
	//    - user_data - user data from initialization parameter.
	//
	// Returns:
	//    - Free audio buffer count.
	using AudioGetFreeBufferCountFunc = int (*)(void* user_data);

	//
	// Callback to present video data.
	//
	// Parameters:
	//    - user_data - user data from initialization parameter.
	//    - data - video data in A8R8G8B8 format.
	//    - width - video frame width.
	//    - height - video frame height.
	//
	using VideoPresentFunc = void (*)(void* user_data, const void* data, int width, int height);

	// I/O seek origin.
	enum class SeekOrigin
	{
		none, // Invalid value.
		begin, // From the beginning.
		current, // From the current position.
		end, // From the end position.
		size, // Should return a size of the stream.
	};

	//
	// I/O read callback.
	//
	// Parameters:
	//    - user_data - user data from initialization parameter.
	//    - buffer - a buffer to write data into.
	//    - buffer_size - maximum size of the buffer.
	//
	// Returns:
	//    - Actual size of data written into the buffer.
	//    - Negative value on error.
	//
	using IoReadFunction = int (*)(void* user_data, std::uint8_t* buffer, int buffer_size);

	//
	// I/O seek callback.
	//
	// Parameters:
	//    - user_data - user data from initialization parameter.
	//    - offset - an offset for a new position.
	//    - origin - from where to change position.
	//
	// Returns:
	//    - A new position.
	//    - Negative value on error.
	//
	using IoSeekFunction = std::int64_t (*)(void* user_data, std::int64_t offset, SeekOrigin origin);

	// Initialization object parameter.
	struct InitializeParam
	{
		// Pass "true" to skip audio data.
		bool is_ignore_audio_;

		// Desired destination audio sample rate.
		// Pass zero value to use stream's sample rate.
		int dst_sample_rate_;

		// Desired audio buffer size in milliseconds for audio present functions.
		int audio_buffer_size_ms_;

		// Maximum allowed audio buffer count.
		int audio_max_buffer_count_;

		// User defined data for callbacks.
		void* user_data_;

		// I/O read callback.
		IoReadFunction io_read_func_;

		// I/O seek callback.
		IoSeekFunction io_seek_func_;

		// Cancellation callback.
		IsCancelledFunc is_cancelled_func_;

		// Video presentation callback.
		VideoPresentFunc video_present_func_;

		// Audio presentation callback.
		AudioPresentFunc audio_present_func_;

		// Audio callback to get free buffer count.
		AudioGetFreeBufferCountFunc audio_get_free_buffer_count_func_;

		//
		// Validates the instance.
		//
		// Returns:
		//    - "true" if input parameters are valid.
		//    - "false" otherwise.
		//
		bool validate() const;
	};

	FmvPlayer();
	FmvPlayer(const FmvPlayer& that) = delete;
	FmvPlayer& operator=(const FmvPlayer& that) = delete;
	FmvPlayer(FmvPlayer&& that) noexcept;
	~FmvPlayer();

	//
	// Initializes current thread.
	//
	// Notes:
	//    - Should be called before actual using of the player.
	//
	static void initialize_current_thread();

	//
	// Initializes the player.
	//
	// Parameters:
	//    - param - set of player's parameters.
	//
	// Returns:
	//    - "true" on success.
	//    - "false" otherwise.
	//
	bool initialize(const InitializeParam& param);

	//
	// Uninitializes the player.
	//
	void uninitialize();

	//
	// Gets status of initialization.
	//
	// Returns:
	//    - "true" if the player is initialized.
	//    - "false" otherwise.
	//
	bool is_initialized() const;


	//
	// Gets destination audio channel count.
	//
	// Returns:
	//    - Audio channel count.
	//
	static int get_channel_count();

	//
	// Gets destination audio bit depth.
	//
	// Returns:
	//    - Audio bit depth.
	//
	static int get_bit_depth();

	//
	// Gets destination audio sample size (all channels).
	//
	// Returns:
	//    - Audio sample size.
	//
	static int get_sample_size();

	//
	// Gets destination audio sample rate.
	//
	// Returns:
	//    - Audio sample rate.
	//
	int get_sample_rate() const;

	//
	// Gets recommended audio buffer size in bytes.
	//
	// Returns:
	//    - Audio buffer size.
	//
	int get_audio_buffer_size() const;

	//
	// Gets destination video width.
	//
	// Returns:
	//    - Video width.
	//
	int get_width() const;

	//
	// Gets destination video height.
	//
	// Returns:
	//    - Video height.
	//
	int get_height() const;

	//
	// Checks input stream for audio stream.
	//
	// Returns:
	//    - "true" if the stream has audio substream.
	//    - "false" if the stream has only video substream.
	//
	bool has_audio() const;

	//
	// Presents all frames.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	bool present();

	//
	// Presents one frame.
	//
	// Returns:
	//    - "true" if there is more frames available.
	//    - "false" if there is no frames available.
	//
	bool present_frame();

	//
	// Gets status of presentation.
	//
	// Returns:
	//    - "true" if all frames are presented.
	//    - "false" if there are more frames to present.
	//
	bool is_presentation_finished() const;


private:
	class Impl;
	using ImplUPtr = std::unique_ptr<Impl>;
	ImplUPtr impl_;
};

} // namespace ltjs

#endif // LTJS_FMV_PLAYER_INCLUDED
