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
		int length_; // (in bytes)
		int mix_offset_; // (int bytes)
		std::uint32_t variations_;
		const void* data_;
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


	bool open(
		const std::string& file_name,
		const int sample_rate);

	void close();


	int get_length() const;

	std::uint32_t get_current_variations() const;

	const Waves& get_waves() const;

	std::uint32_t select_next_variation();


	const std::string& get_error_message() const;


private:
	class Impl;


	using ImplUPtr = std::unique_ptr<Impl>;


	ImplUPtr pimpl_;
}; // DMusicSegment



}; // ltjs


#endif // LTJS_DMUSIC_SEGMENT_INCLUDED
