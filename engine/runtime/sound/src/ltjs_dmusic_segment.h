#ifndef LTJS_DMUSIC_SEGMENT_INCLUDED
#define LTJS_DMUSIC_SEGMENT_INCLUDED


#include <cstdint>
#include <memory>
#include <string>


namespace ltjs
{


class DMusicSegment final
{
public:
	DMusicSegment();

	DMusicSegment(
		const DMusicSegment& that) = delete;

	DMusicSegment& operator=(
		const DMusicSegment& that) = delete;

	DMusicSegment(
		DMusicSegment&& that);

	~DMusicSegment();


	bool open(
		const std::string& file_name,
		const int sample_rate);

	void close();

	bool rewind();

	int mix(
		const int src_decode_size,
		std::int16_t* dst_decode_buffer,
		float* dst_mix_buffer);

	int get_length() const;

	bool is_finished() const;

	bool is_silence() const;


	const std::string& get_error_message() const;


private:
	class Impl;


	using ImplUPtr = std::unique_ptr<Impl>;


	ImplUPtr pimpl_;
}; // DMusicSegment



}; // ltjs


#endif // LTJS_DMUSIC_SEGMENT_INCLUDED
