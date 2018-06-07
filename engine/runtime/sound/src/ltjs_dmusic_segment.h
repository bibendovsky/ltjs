#ifndef LTJS_DMUSIC_SEGMENT_INCLUDED
#define LTJS_DMUSIC_SEGMENT_INCLUDED


#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace ltjs
{


class DMusicSegment final
{
public:
	struct Wave
	{
		// A length of the wave in the segment. In bytes.
		int length_;

		// When to start mixing the wave. In bytes.
		int mix_offset_;

		// Available variations for the wave.
		std::uint32_t variations_;

		// Associated wave file image date.
		const void* data_;

		// A size of the file image data.
		int data_size_;
	}; // Wave

	using Waves = std::vector<Wave>;


	DMusicSegment();

	DMusicSegment(
		const DMusicSegment& that) = delete;

	DMusicSegment& operator=(
		const DMusicSegment& that) = delete;

	DMusicSegment(
		DMusicSegment&& that);

	DMusicSegment& operator=(
		DMusicSegment&& that);

	~DMusicSegment();


	//
	// Opens the segment.
	//
	// Parameters:
	//    - file_name - a file name of the DirectMusic segment file (.sgt).
	//    - sample_rate - a destination sample rate.
	//
	// Returns:
	//    - "true" on sucess.
	//    - "false" otherwise.
	//
	bool open(
		const std::string& file_name,
		const int sample_rate);

	//
	// Closes the segment.
	//
	void close();


	//
	// Gets a segment length.
	//
	// Returns:
	//    - A segment length in bytes.
	//
	int get_length() const;

	//
	// Gets a performance channel assigned to the segment.
	//
	// Returns:
	//    - A performance channel.
	//
	int get_channel() const;

	//
	// Gets a current variation mask.
	//
	// Returns:
	//    - Returns a current variation mask.
	//
	std::uint32_t get_current_variation() const;

	//
	// Get a list of waves.
	//
	// Returns:
	//    - A list of waves.
	//
	const Waves& get_waves() const;

	//
	// Selects the next variation mask.
	//
	// Returns:
	//    - A selected variation mask.
	//
	std::uint32_t select_next_variation();


	//
	// Gets a last error message.
	//
	// Returns:
	//    - A last error message.
	//
	const std::string& get_error_message() const;


private:
	class Impl;


	using ImplUPtr = std::unique_ptr<Impl>;


	ImplUPtr impl_;
}; // DMusicSegment



}; // ltjs


#endif // LTJS_DMUSIC_SEGMENT_INCLUDED
